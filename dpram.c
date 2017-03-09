
/*************************************************************************
**    IXXAT Automation GmbH
**************************************************************************
**
**  $Workfile: $
**    Summary: DPRAM management
**   $Archive: $
**  $Revision: 1.10 $
**      $Date: 2003/03/19 13:11:19 $
**     Author: Alexander Kuzmich
**
**************************************************************************
**************************************************************************
**
**  Functions: GetDPRAMPtr, FreeDPRAMPtr
**
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

/*************************************************************************
**    compiler directives
*************************************************************************/

/*************************************************************************
**    include-files
*************************************************************************/

#include "stdio.h"
#include "integral.h"
#include "bci.h"
#include "bci_int.h"
#include "can.h"

/*************************************************************************
**    static constants, types, macros, variables
*************************************************************************/

/*************************************************************************
**
** Function    : GetDPRAMPtr
**
** Description : generates a pointer to mmaped area
** Parameters  : device_fd (IN) Character device file descriptor
** Returnvalue : MMapAreaPtr - ptr at address
**
*************************************************************************/
void *GetDPRAMPtr (t_Interface * Interface)
{
  void *MMapAreaPtr = NULL;

  MMapAreaPtr =
    mmap (0, Interface->BoardMemSize, PROT_READ | PROT_WRITE,
          MAP_SHARED, Interface->DeviceFileDesc[0], 0);
#ifdef DEBUG
  printf ("DPRAM get ptr: size %d at 0x%p\n", Interface->BoardMemSize, MMapAreaPtr);
#endif

  return MMapAreaPtr;
}

/*************************************************************************
**
** Function    : FreeDPRAMPtr
**
** Description : frees a mmaped memory
** Parameters  : *DPRAMPtr (IN) - pointer to memory allocated with
**                            GetDPRAMPointer
** Returnvalue : 0
**
*************************************************************************/
int FreeDPRAMPtr (t_Interface * Interface)
{

#ifdef DEBUG
  printf ("DPRAM free ptr: size %d at 0x%p\n",
          Interface->BoardMemSize, Interface->MemoryPtr);
#endif
  msync (Interface->MemoryPtr, Interface->BoardMemSize, MS_SYNC);
  munmap (Interface->MemoryPtr, Interface->BoardMemSize);

  return 0;
}
