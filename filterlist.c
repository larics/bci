
/*************************************************************************
**    IXXAT Automation GmbH
**************************************************************************
**
**  $Workfile: $
**    Summary: Filter List realization
**   $Archive: $
**  $Revision: 1.9 $
**      $Date: 2004/01/28 11:47:50 $
**     Author: Alexander Kuzmich
**
**************************************************************************
**    all rights reserved
*************************************************************************/

/*************************************************************************
**    compiler directives
*************************************************************************/

/*************************************************************************
**    include-files
*************************************************************************/
#include "integral.h"
#include "filterlist.h"

/*************************************************************************
**    global variables
*************************************************************************/

/*************************************************************************
**    static constants, types, macros, variables
*************************************************************************/

// #define PRINTD printf
// #define PRINTE printf
// #define PRINTI printf

#ifndef PRINTD
#define PRINTD(a...)
#endif

#ifndef PRINTE
#define PRINTE(a...)
#endif

#ifndef PRINTI
#define PRINTI(a...)
#endif

/*************************************************************************
**    static function-prototypes
*************************************************************************/

/*************************************************************************
**    global functions
*************************************************************************/

/*************************************************************************
**
** Function    : FilterListSearch  
**
** Description : Searches the filter list for specified ID 
**                
** Parameters  : FilterList - pointer to the filter list
**               ID         - CAN message ID            
** Returnvalue : positive number - ID position in the list
**               negative number - ID not found 
**
*************************************************************************/
int FilterListSearch (FilterList_t * FilterList, UINT32 ID)
{

  int ret = -1;
  UINT32 LeftLimit = 0, RightLimit = FilterList->ItemNumber - 1, Index;

  if (FilterList->ItemNumber == 0)
  {
    PRINTD ("FL (empty): check for ID 0x%08lx\n", ID);
    return -1;
  }

  while (LeftLimit < RightLimit)
  {
    Index = (LeftLimit + RightLimit) / 2;

    if (FilterList->List[Index] < ID)
    {
      LeftLimit = Index + 1;
    }
    else
    {
      if (FilterList->List[Index] == ID)
      {
        ret = Index;
        break;
      }
      else
      {
        RightLimit = Index;
      }
    }
//      PRINTD ("Left %d Right %d Index %d [%d/%d]\n", LeftLimit,
//            RightLimit, Index, ID, FilterList[LeftLimit]);

  }

  if (FilterList->List[RightLimit] == ID)
  {
    ret = RightLimit;
    PRINTD ("FL (%ld) found ID 0x%08lx\n", FilterList->ItemNumber, ID);
  }
  else
  {
    PRINTD ("FL (%ld) not found ID 0x%08lx\n", FilterList->ItemNumber, ID);
  }

  return ret;

}

/*************************************************************************
**
** Function    : FilterListSort  
**
** Description : Sorts the filter list and calculates acceptance
**               filter values  
**                
** Parameters  : FilterList - pointer to the filter list
** Returnvalue : 0  - Success
**               -1 - List is empty 
*************************************************************************/
int FilterListSort (FilterList_t * FilterList)
{
  UINT32 LeftLimit, RightLimit, Index, SortItem, i, j, CurID, LastID;

  PRINTD ("Sort the filter list. Number of items %ld\n", FilterList->ItemNumber);

  FilterListShow (FilterList);

  if (FilterList->ItemNumber == 0)
  {
    return -1;
  }

  for (i = 1; i < FilterList->ItemNumber; i++)
  {
    SortItem = FilterList->List[i];
    LeftLimit = 0;
    RightLimit = i;
    // Find the insert position
    while (LeftLimit < RightLimit)
    {
      Index = (LeftLimit + RightLimit) / 2;

      if (FilterList->List[Index] <= SortItem)
      {
        LeftLimit = Index + 1;
      }
      else
      {
        RightLimit = Index;
      }
    }
    // Position found, now shifting
    for (j = i; j >= RightLimit + 1; j--)
    {
      FilterList->List[j] = FilterList->List[j - 1];
    }
    // Insert sorted element
    FilterList->List[RightLimit] = SortItem;
  }

  FilterListShow (FilterList);

  // Calculate the acceptance filter setting
  LastID = FilterList->List[0];
  FilterList->AccMask = 0x1fffffff;

  for (i = 0; i < FilterList->ItemNumber; i++)
  {
    CurID = FilterList->List[i];
    FilterList->AccMask &= ~(CurID ^ LastID);
    LastID = CurID;
  }
  FilterList->AccCode = (LastID & FilterList->AccMask);

  return 0;
}

/*************************************************************************
**
** Function    : FilterListAddID 
**
** Description : Adds the specified ID to the filter list  
**                
** Parameters  : FilterList - pointer to the filter list
**               ID         - CAN message ID            
** Returnvalue : 0  - Success
**               -1 - List is full
**
*************************************************************************/
int FilterListAddID (FilterList_t * FilterList, UINT32 ID)
{
  if (FilterList->ItemNumber < FILTER_LIST_SIZE)
  {
    FilterList->List[FilterList->ItemNumber] = ID;
    FilterList->ItemNumber++;
    PRINTD ("Value 0x%lx added\n", ID);
    return 0;
  }
  else
  {
    return -1;
  }
}

/*************************************************************************
**
** Function    : FilterListRemoveID  
**
** Description : Removes the specified ID to the filter list 
**                
** Parameters  : FilterList - pointer to the filter list
**               ID         - CAN message ID            
** Returnvalue : 0  - Success
**               -1 - ID is not in list 
**
*************************************************************************/
int FilterListRemoveID (FilterList_t * FilterList, UINT32 ID)
{
  int ret;
  unsigned int i;

  if (FilterList->ItemNumber == 0)
  {
    return -1;
  }
  ret = FilterListSearch (FilterList, ID);
  if (ret >= 0)
  {
    PRINTD ("Value 0x%lx removed at position %d (0x%lx)\n", ID, ret,
            FilterList->List[ret]);
    // Shift 
    for (i = ret; i < FilterList->ItemNumber; i++)
    {
      FilterList->List[i] = FilterList->List[i + 1];
    }
    FilterList->ItemNumber--;
  }
  else
  {
    PRINTD ("Value 0x%lx not found !\n", ID);
    return -1;
  }
  FilterListShow (FilterList);
  return 0;
}

/*************************************************************************
**
** Function    : FilterListShow  
**
** Description : Shows the content of the filter list 
**                
** Parameters  : FilterList - pointer to the filter list
** Returnvalue : 0  - Success
**
*************************************************************************/
int FilterListShow (FilterList_t * FilterList)
{
  UINT32 i;

  PRINTD ("There are %ld ID in the filter list\n", FilterList->ItemNumber);

  if (FilterList->ItemNumber == 0)
  {
    return 0;
  }

  for (i = 0; i < FilterList->ItemNumber; i++)
  {
    PRINTD ("[%ld]=0x%lx ", i, FilterList->List[i]);
  }
  PRINTD ("\n");
  return 0;
}

/*************************************************************************
**
** Function    : FilterListInit  
**
** Description : Initializes the filter list 
**                
** Parameters  : FilterList - pointer to the filter list
** Returnvalue : 0  - Success
**
*************************************************************************/
int FilterListInit (FilterList_t * FilterList)
{
  UINT32 i;

  FilterList->ItemNumber = 0;

  for (i = 0; i < FILTER_LIST_SIZE; i++)
  {
    FilterList->List[i] = 0;
  }
  FilterList->AccCode = 0;
  FilterList->AccMask = 0;

  return 0;
}
