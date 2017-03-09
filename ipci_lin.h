
/*************************************************************************
**    IXXAT Automation GmbH
**************************************************************************
**
**       File: ipci_lin.h
**    Summary: Internal header file for iPCI boards LIN support functions
**    Version: @(VERSION)
**       Date: @(DATE)
**     Author: Alexander Kuzmich
**
**************************************************************************
**************************************************************************
**
**  Functions:
**
**
**
**   Compiler: @(COMPILER)
**    Remarks:
** 
**
**   History:
**
**************************************************************************
**    all rights reserved
*************************************************************************/

#ifndef IPCI_LIN_H
#define IPCI_LIN_H

/*************************************************************************
**    constants and macros
*************************************************************************/

/*
** Defines for the LIN Buffer commands
*/
#define LIN_CMD_GET_STAT                41  /* Get identification string */
#define LIN_CMD_START                   42  /* Start the LIN controller */
#define LIN_CMD_STOP                    43  /* Stop the LIN controller */
#define LIN_CMD_RESET                   44  /* Reset the LIN controller */
#define LIN_CMD_INIT                    45  /* Init the LIN controller */
#define LIN_CMD_UPDATE_BUF              46  /* Update the buffer */

/*************************************************************************
**    data types
*************************************************************************/
//*** common structs to get access to the bits of a byte
typedef struct
{
  UINT8 o0:1, o1:1, o2:1, o3:1, o4:1, o5:1, o6:1, o7:1;
}
BCI_PACK_STRUCT_ATTR t_BITSINBYTE_S;

typedef union
{
  UINT8 b;
  t_BITSINBYTE_S u;
}
BCI_PACK_STRUCT_ATTR t_BITSINBYTE_U;

/*************************************************************************
**    global variables
*************************************************************************/

/*************************************************************************
**    function prototypes
*************************************************************************/

/*************************************************************************
**
** Function    : iPCI_LIN_Open
**
** Description : initialize LIN part of interface
** Parameters  : pInterface (I/O)  nterface pointer
** Returnvalue : BCI_OK
**               BCI_LIN_NOT_SUPP - LIN not supported 
**               BCI_SERV_ERR  - device file not found
**
*************************************************************************/
int iPCI_LIN_Open (PInterface pInterface);

/*************************************************************************
**
** Function    : iPCI_LIN_Close
**
** Description : close the LIN interface
** Parameters  : pInterface   (I/O)  CAN interface pointer
** Returnvalue : BCI_OK          - OK
**
*************************************************************************/
int iPCI_LIN_Close (PInterface pInterface);

/*************************************************************************
**
** Function    : iPCI_LIN_Start
**
** Description : start LIN controller
** Parameters  : pInterface  (I/O)  interface pointer
**               LINNum       (IN)  controller number
** Returnvalue : BCI_OK       - OK
**               BCI_BUSY     - command is busy
**               BCI_PARA_ERR - incorrect command
**               BCI_TIMEOUT  - timeout reached
**
*************************************************************************/
int iPCI_LIN_Start (PInterface pInterface, UINT8 LINNum);

/*************************************************************************
**
** Function    : iPCI_LIN_Stop
**
** Description : stop LIN controller
** Parameters  : pInterface   (I/O) interface pointer
**               LINNum       (IN)  controller number
** Returnvalue : BCI_OK       - OK
**               BCI_BUSY     - command is busy
**               BCI_PARA_ERR - incorrect command
**               BCI_TIMEOUT  - timeout reached
**
*************************************************************************/
int iPCI_LIN_Stop (PInterface pInterface, UINT8 LINNum);

/*************************************************************************
**
** Function    : iPCI_LIN_Reset
**
** Description : reset LIN controller
** Parameters  : pInterface  (I/O)  interface pointer
**               LINNum       (IN)  controller number
** Returnvalue : BCI_OK       - OK
**               BCI_BUSY     - command is busy
**               BCI_PARA_ERR - incorrect command
**               BCI_TIMEOUT  - timeout reached
**
*************************************************************************/
int iPCI_LIN_Reset (PInterface pInterface, UINT8 LINNum);

/*************************************************************************
**
** Function    : iPCI_LIN_GetStatusImage
**
** Description : get LIN status from DPRAM image
** Parameters  : pInterface          (I/O) interface pointer
**               *BrdStatus          (IN)  current LIN board status
** Returnvalue : BCI_OK          - OK
**               BCI_BUSY        - semafor set failed
**
*************************************************************************/
int iPCI_LIN_GetStatusImage (PInterface pInterface, BCI_ts_linBrdSts * linBrdStatus);

/*************************************************************************
**
** Function    : iPCI_LIN_GetStatusCommand
**
** Description : get LIN status using iPCI command
** Parameters  : pInterface          (I/O) interface pointer
**               *BrdStatus          (IN)  current LIN board status
** Returnvalue : BCI_OK          - OK
**               BCI_BUSY        - semafor set failed
**
*************************************************************************/
int iPCI_LIN_GetStatusCommand (PInterface pInterface,
                               BCI_ts_linBrdSts * linBrdStatus);

/*************************************************************************
**
** Function:
**  iPCI_LIN_Init
**
** Description:
**  init CAN controller
** Parameters:
**   pInterface  (I/O)  LIN interface handler
**   LINNum      (IN)   controller number (0, 1)
**   LINmode     (IN)   LIN controller communication mode selection:
**               - BCI_LIN_OPMODE_SLAVE
**               - BCI_LIN_OPMODE_MASTER
**               - BCI_LIN_OPMODE_ERRORS
**   intmode    (IN) - event handling mode:
**               - BCI_LATENCY_MODE:
**                 After every received CAN message an
**                 interrupt/event is generated.
**                 This mode guarantees the best
**                 reactivity time on received messages.
**
**               - BCI_THROUGHPUT_MODE:
**                 An interrupt/event
**                 is generated only after a predefined number of received
**                 CAN messages.
**                 This mode guarantees the best
**                 data throughput because of the reduced
**                 interrupt load for the PC.
**
**               - BCI_POLL_MODE:
**                 Use this mode if your OS does not support
**                 interrupt handling or your application
**                 does not use the event handling for
**                 CAN message reception.
**   bitrate     (IN) bitrate in bits/s
**
** Returnvalue: BCI_OK       - OK
**              BCI_BUSY     - command is busy
**              BCI_PARA_ERR - incorrect command
**              BCI_TIMEOUT  - timeout reached
**
***************************************************************************/
int iPCI_LIN_Init (PInterface pInterface, UINT8 LINNum, UINT8 LINmode,
                   UINT8 intmode, UINT16 bitrate);

/*************************************************************************
**
** Function:
**  iPCI_LIN_WriteMsg
**
** Description:
**  init CAN controller
** Parameters:
**   pInterface  (I/O)  LIN interface handler
**  LINum               (IN)  controller number
**  fSend               (IN)  send mode flag
**                            BCI_LIN_WRITE_DIRECT - direct send 
**                            BCI_LIN_WRITE_TABLE - update send table
**         
**  *plinMsg            (IN)  message to write 
**
** Returnvalue: BCI_OK       - OK
**              BCI_BUSY     - command is busy
**              BCI_PARA_ERR - incorrect command
**              BCI_TIMEOUT  - timeout reached
**
***************************************************************************/
int iPCI_LIN_WriteMsg (PInterface pInterface, UINT8 LINNum,
                       UINT8 fSend, BCI_ts_LinMsg * plinMsg);

/*************************************************************************
**
** Function    : iPCI_LIN_ReadMsg
**
** Description : receive a LIN message.
**               Try to find unprocessed object in the LIN receive queue;
**               if there is no such objects, return BCI_NO
** Parameters  : *pInterface          (I/O) CAN interface pointer
**               LINNum               (IN) controller number
**               dwMsTimeout          (IN) wait timeout in ms
**               *plinMsg             (OUT) pointer to received LIN message
** Returnvalue : BCI_OK          - OK
**               BCI_NO - receive queue is empty
**
*************************************************************************/
int
iPCI_LIN_ReadMsg (PInterface pInterface, UINT8 LINNum,
                  UINT32 dwMsTimeout, BCI_ts_LinMsg * plinMsg);

#endif
