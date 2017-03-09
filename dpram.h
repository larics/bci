
/*************************************************************************
**    IXXAT Automation GmbH
**************************************************************************
**
**  $Workfile: $
**    Summary: DPRAM management
**   $Archive: $
**  $Revision: 1.5 $
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

#ifndef DPRAM_H
#define DPRAM_H

/*************************************************************************
**    constants and macros
*************************************************************************/

#define ID_STRING_SIZE 40
#define COMMAND_SIZE 10

/*************************************************************************
**    function prototypes
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
void *GetDPRAMPtr (t_Interface * pInterface);

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
int FreeDPRAMPtr (t_Interface * pInterface);

#endif
