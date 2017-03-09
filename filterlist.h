
/*************************************************************************
**    IXXAT Automation GmbH
**************************************************************************
**
**  $Workfile: $
**    Summary: Filter List defitions 
**   $Archive: $
**  $Revision: 1.5 $
**      $Date: 2003/06/25 15:00:26 $
**     Author: Alexander Kuzmich
**
**************************************************************************
**************************************************************************
**
**  Functions:  
**
**
**   Compiler: gcc 2.95.3
**    Remarks:
**
**   $History: $
**
**************************************************************************
**    all rights reserved
*************************************************************************/
#ifndef FILTERLIST_H
#define FILTERLIST_H

/*************************************************************************
**    constants and macros
*************************************************************************/
#ifndef FILTER_LIST_SIZE
#define FILTER_LIST_SIZE 4096
#endif

#ifdef BCI_PACK_STRUCT_ATTR
 #undef BCI_PACK_STRUCT_ATTR
#endif

#ifdef __linux__
/* Pack Linux structures */
#define BCI_PACK_STRUCT_ATTR __attribute__((__packed__))
#else
 #warning "BCI structures are not packed ! Please check !"
 #define BCI_PACK_STRUCT_ATTR
#endif /* ifdef LINUX */

/*************************************************************************
**    data types
*************************************************************************/
typedef struct
{
  UINT32 List[FILTER_LIST_SIZE];
  UINT32 ItemNumber;
  UINT32 AccCode;
  UINT32 AccMask;
}
BCI_PACK_STRUCT_ATTR FilterList_t;

/*************************************************************************
**    function prototypes
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
int FilterListSearch (FilterList_t * FilterList, UINT32 ID);

/*************************************************************************
**
** Function    : FilterListSort  
**
** Description : Sorts the filter list and calculates acceptance
**               filter values  
**                
** Parameters  : FilterList - pointer to the filter list
** Returnvalue : 0 - Success
**
*************************************************************************/
int FilterListSort (FilterList_t * FilterList);

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
int FilterListAddID (FilterList_t * FilterList, UINT32 ID);

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
int FilterListRemoveID (FilterList_t * FilterList, UINT32 ID);

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
int FilterListShow (FilterList_t * FilterList);

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
int FilterListInit (FilterList_t * FilterList);

#endif
