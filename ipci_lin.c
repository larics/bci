/*************************************************************************
**    IXXAT Automation GmbH
**************************************************************************
**
**  $Workfile: $
**    Summary: XC LIN Support
**   $Archive: $
**  $Revision: 1.00 $
**      $Date: 2008/26/02 12:33:06 $
**     Author: Alexander Kuzmich
**
**************************************************************************
**************************************************************************
**
**  Functions:
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
**    include-files
*************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "integral.h"
#include "bci.h"
#include "bci_int.h"
#include "dpram.h"
#include "can.h"
#include "can_ioctl.h"
#include "ipci.h"
#include "ipci_lin.h"
#include "filterlist.h"

/*************************************************************************
**    static constants, types, macros, variables
*************************************************************************/
#ifdef PRINTI
#undef PRINTI
#define PRINTI(format, args...) printf(format, ## args)
#endif

#ifdef PRINTD
#undef PRINTD
#if (BCI_DEBUG == 1)
#define PRINTD(format, args...) printf(format, ## args)
#else
#define PRINTD(format, args...)
#endif
#endif

static UINT8 Command[BAP_CMND_BUFFER_SIZE], *CommandPtr;
static UINT8 Answer[BAP_CMND_BUFFER_SIZE];
static UINT8 CommandSize = 0;
static INT32 AnswerSize;
static UINT8 CommandCode;

/*************************************************************************
**    static function-prototypes
*************************************************************************/
static UINT8 CreateChecksum (BCI_ts_LinMsgData * p_msg);
static UINT8 CreateID (UINT8 b_id);
static INT32 UpdateLinTxBufDpramEntry(PInterface pInterface,
                                      BCI_ts_LinMsgData *ps_dataMsg);
static INT32 GetSema(PInterface pInterface, UINT8 bSemaNum,
                     UINT32 dwNumTries);                                      
static INT32 FreeSema(PInterface pInterface, UINT8 bSemaNum);                                      

/*************************************************************************
**    global functions
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
int iPCI_LIN_Open (PInterface pInterface)
{

  /* init LIN queues */
  ((Queues *) pInterface->QueuesPtr)->LIN_TxQueue0 = 0;
  ((Queues *) pInterface->QueuesPtr)->LIN_RxQueue0 = 0;

  PRINTD ("RXC %d TXC %d RXQP 0x%p TXQP 0x%p\n",
          ((Queues *) pInterface->QueuesPtr)->LIN_RxQueue0,
          ((Queues *) pInterface->QueuesPtr)->LIN_TxQueue0,
          ((DPRAM *) (pInterface->MemoryPtr))->LIN_RxQueue0,
          ((DPRAM *) (pInterface->MemoryPtr))->LIN_TxQueue0);

  switch (pInterface->Type)
  {
    case DEVICE_iPCIXC161_PCI:
    case DEVICE_iPCIXC161_PCIE:
      {
        int iface;

        /* Only interface with LIN support */
        PRINTD ("LIN Interface type [%d] opened\n", pInterface->Type);
        for (iface = 0; iface < BCI_MAX_LIN_NUM; iface++)
        {
          pInterface->LIN.OpMode[iface] = BCI_LIN_OPMODE_UNKNOWN;
        }
      }
      break;

    case DEVICE_iPCI165_ISA:
    case DEVICE_iPCI165_PCI:
    case DEVICE_iPCI320_ISA:
    case DEVICE_iPCI320_PCI:
      /* LIN not supported */
      return (BCI_LIN_NOT_SUPP);

    default:
      PRINTD ("Unknow interface type [%d]\n", pInterface->Type);
      return (BCI_SERV_ERR);
  }

  return (BCI_OK);
}

/*************************************************************************
**
** Function    : iPCI_LIN_Close
**
** Description : close the LIN interface
** Parameters  : pInterface   (I/O)  CAN interface pointer
** Returnvalue : BCI_OK          - OK
**
*************************************************************************/
int iPCI_LIN_Close (PInterface pInterface)
{
  return BCI_OK;
}

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
int iPCI_LIN_Start (PInterface pInterface, UINT8 LINNum)
{

  int ret;

  /* Clear interrupt status */
  ret = (((PInterface) pInterface)->WaitForData (pInterface, 0, BCI_NO_WAIT));

  CommandCode = LIN_CMD_START;

  CommandPtr = Command;
  CommandSize =
    PushCmdData ((char **) &CommandPtr, (char *) &CommandCode, sizeof (UINT8));
  CommandSize +=
    PushCmdData ((char **) &CommandPtr, (char *) &LINNum, sizeof (UINT8));

  if ((ret = iPCISendCommand (pInterface, Command, CommandSize)) != BCI_OK)
  {
    /* Error */
    return (ret);
  }

  ret = (iPCIGetResponse (pInterface, Answer, &AnswerSize));
  return ret;
}

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
int iPCI_LIN_Stop (PInterface pInterface, UINT8 LINNum)
{

  int ret;

  CommandCode = LIN_CMD_STOP;

  CommandPtr = Command;
  CommandSize =
    PushCmdData ((char **) &CommandPtr, (char *) &CommandCode, sizeof (UINT8));
  CommandSize +=
    PushCmdData ((char **) &CommandPtr, (char *) &LINNum, sizeof (UINT8));

  if ((ret = iPCISendCommand (pInterface, Command, CommandSize)) != BCI_OK)
  {
    /* Error */
    return (ret);
  }

  return (iPCIGetResponse (pInterface, Answer, &AnswerSize));
}

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
int iPCI_LIN_Reset (PInterface pInterface, UINT8 LINNum)
{
  int ret;

  DPRAM *ps_dpram;


  CommandCode = LIN_CMD_RESET;

  CommandPtr = Command;
  CommandSize =
    PushCmdData ((char **) &CommandPtr, (char *) &CommandCode, sizeof (UINT8));
  CommandSize +=
    PushCmdData ((char **) &CommandPtr, (char *) &LINNum, sizeof (UINT8));

  if ((ret = iPCISendCommand (pInterface, Command, CommandSize)) != BCI_OK)
  {
    /* Error */
    return (ret);
  }

  ret = (iPCIGetResponse (pInterface, Answer, &AnswerSize));

  /* Clear Tx Buf Images */
  ps_dpram = (DPRAM *) (pInterface->MemoryPtr);

  MEMSET(&(ps_dpram->LIN_TxBufImage0), 0x0, sizeof(ps_dpram->LIN_TxBufImage0));  
  MEMSET(&(ps_dpram->LIN_TxBufImage1), 0x0, sizeof(ps_dpram->LIN_TxBufImage1));
  
  FreeSema(pInterface, LIN_STATUS_IMAGE_SEMA);
  FreeSema(pInterface, LIN_TX_BUF_IMAGE0_SEMA);
  FreeSema(pInterface, LIN_TX_BUF_IMAGE1_SEMA);

  PRINTD ("LIN%d: Reset, images and semaphores resetted\n", LINNum);

  return ret;
}

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
int iPCI_LIN_GetStatusImage (PInterface pInterface, BCI_ts_linBrdSts * linBrdStatus)
{
  int ret;

  ret = GetSema(pInterface, LIN_STATUS_IMAGE_SEMA, 10);
  if (ret == BCI_OK)
  {
    MEMCPY (linBrdStatus, ((DPRAM *) pInterface->MemoryPtr)->LIN_ImageBuffer,
            sizeof(BCI_ts_linBrdSts));
    ret = BCI_OK;
  }
  else
  {
    MEMSET (linBrdStatus, 0x0, sizeof(BCI_ts_linBrdSts));
    ret = BCI_BUSY;
  }
  FreeSema(pInterface, LIN_STATUS_IMAGE_SEMA);
  return (ret);
}

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
                               BCI_ts_linBrdSts * linBrdStatus)
{
  int ret;
  UINT8 dummy;

  CommandCode = LIN_CMD_GET_STAT;

  CommandPtr = Command;
  dummy = 0;
  CommandSize =
    PushCmdData ((char **) &CommandPtr, (char *) &CommandCode, sizeof (UINT8));
  CommandSize +=
    PushCmdData ((char **) &CommandPtr, (char *) &dummy, sizeof (UINT8));

  if ((ret = iPCISendCommand (pInterface, Command, CommandSize)) != BCI_OK)
  {
    return (ret);                           /* Error  */
  }

  ret = iPCIGetResponse (pInterface, Answer, &AnswerSize);
  if (ret == BCI_OK)
  {
    MEMCPY (linBrdStatus, Answer + 2,       /* Skip command and dummy bytes */
            sizeof (BCI_ts_linBrdSts));
  }
  return (ret);
}

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
                   UINT8 intmode, UINT16 bitrate)
{
  int ret;
  UINT8 dummy = 0, fw_mode;
  UINT8 CtrlNum = BCI_MAX_CAN_NUM + LINNum;

  if ((ret = iPCI_LIN_Reset (pInterface, LINNum)) != BCI_OK)
  {
    /* Error */
    return (ret);
  }

  PRINTD ("LIN%d: Reset OK, Init Mode 0x%x Int 0x%x bitrate %d \n",
          LINNum,  LINmode, intmode, bitrate);

  pInterface->LIN.OpMode[LINNum] = LINmode;

  if (LINmode & BCI_LIN_OPMODE_SLAVE)
  {
    fw_mode = BCI_LIN_FW_OPMODE_MODE_SLAVE;
  }
  else if (LINmode & BCI_LIN_OPMODE_MASTER)
  {
    fw_mode = BCI_LIN_FW_OPMODE_MODE_MASTER;
  }
  else
  {
    pInterface->LIN.OpMode[LINNum] = BCI_LIN_OPMODE_UNKNOWN;
    return BCI_PARA_ERR;
  }

  CommandCode = LIN_CMD_INIT;

  CommandPtr = Command;
  CommandSize =
    PushCmdData ((char **) &CommandPtr, (char *) &CommandCode, sizeof (UINT8));
  CommandSize +=
    PushCmdData ((char **) &CommandPtr, (char *) &LINNum, sizeof (UINT8));

  CommandSize +=
    PushCmdData ((char **) &CommandPtr, (char *) &fw_mode, sizeof (UINT8));
  CommandSize +=
    PushCmdData ((char **) &CommandPtr, (char *) &dummy, sizeof (UINT8));
  CommandSize +=
    PushCmdData ((char **) &CommandPtr, (char *) &bitrate, sizeof (UINT16));

  if ((ret = iPCISendCommand (pInterface, Command, CommandSize)) != BCI_OK)
  {
    /* Error */
    return (ret);
  }

  if ((ret = iPCIGetResponse (pInterface, Answer, &AnswerSize)) != BCI_OK)
  {
    /* Error */
    return (ret);
  }

  CommandCode = CAN_CMD_CONFIG_RX_QUEUE;
  CommandPtr = Command;
  CommandSize =
    PushCmdData ((char **) &CommandPtr, (char *) &CommandCode, sizeof (UINT8));
  CommandSize +=
    PushCmdData ((char **) &CommandPtr, (char *) &CtrlNum, sizeof (UINT8));
  CommandSize +=
    PushCmdData ((char **) &CommandPtr, (char *) &intmode, sizeof (UINT8));

  if ((ret = iPCISendCommand (pInterface, Command, CommandSize)) != BCI_OK)
  {
    /* Error */
    return (ret);
  }

  ret = (iPCIGetResponse (pInterface, Answer, &AnswerSize));
  return ret;

}

/*************************************************************************
**
** Function:
**  iPCI_LIN_WriteMsg
**
** Description:
**  init CAN controller
** Parameters:
**  pInterface          (I/O)  LIN interface handler
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
                       UINT8 fSend, BCI_ts_LinMsg * plinMsg)
{
  int ret;

  plinMsg->sHdr.bType = BCI_LIN_MSGTYPE_DATA;
  plinMsg->sHdr.dwTimeStamp = 0;

  plinMsg->uMsg.Data.bId = CreateID (plinMsg->uMsg.Data.bId);
  plinMsg->uMsg.Data.bCheckSum = CreateChecksum (&(plinMsg->uMsg.Data));

  switch (fSend)
  {
    case BCI_LIN_WRITE_DIRECT:
      {
        /* Write message direct to the queue */
        UINT8 *QueuePtr;
        UINT8 *TxQueue;

        /* Only master is allowed to do that */
        if (!(pInterface->LIN.OpMode[LINNum] & BCI_LIN_OPMODE_MASTER))
        {
          PRINTD ("LIN%d: Tx wrong mode 0x%x, not Master\n",
                  LINNum, pInterface->LIN.OpMode[LINNum]);
          return BCI_LIN_WRONG_MODE;
        }

        switch (LINNum)
        {
          case 0:
            TxQueue = &((Queues *) (pInterface->QueuesPtr))->LIN_TxQueue0;
            QueuePtr = ((DPRAM *) (pInterface->MemoryPtr))->LIN_TxQueue0[*TxQueue];
            break;

          default:
            return (BCI_PARA_ERR);
        }

        if (*QueuePtr == 0)
        {
          /* Free entry */
          UINT8 bMsgSize = sizeof (BCI_ts_LinMsg);
          /* copy LIN message */
          MEMCPY ((QueuePtr + 2), (char *) plinMsg, bMsgSize);
          /* set message length */
          MEMSET ((QueuePtr + 1), bMsgSize, 1);
          /* set message status as valid */
          MEMSET (QueuePtr, 1, 1);

          *TxQueue = (UINT8) (((*TxQueue) + 1) % LIN_NUM_QUEUES_ENTRIES);
          pInterface->GenerateInterrupt (pInterface);

          PRINTD ("Tx LIN%d : item number %d transmitted size %d struct size %d\n",
                   LINNum, *TxQueue, bMsgSize, sizeof(BCI_ts_LinMsg));
          return (BCI_OK);
        }
        else
        {
              PRINTD ("Tx LIN%d : queue is full\n", LINNum);
              return (BCI_BUSY);
        }
      }
      break;

    case BCI_LIN_WRITE_TABLE:
      {
#if (LIN_UPDATE_TABLE_COMMAND == 1)
        /* Old command interface */ 
        /* Prepare update table command */
        CommandCode = LIN_CMD_UPDATE_BUF;

        CommandPtr = Command;
        CommandSize = PushCmdData ((char **) &CommandPtr,
                                   (char *) &CommandCode, sizeof (UINT8));
        CommandSize += PushCmdData ((char **) &CommandPtr, (char *) &LINNum,
                                    sizeof (UINT8));

        CommandSize +=
          PushCmdData ((char **) &CommandPtr, (char *) &(plinMsg->uMsg.Data),
                       sizeof (BCI_ts_LinMsgData));

        if ((ret = iPCISendCommand (pInterface, Command, CommandSize)) != BCI_OK)
        {
          /* Error */
          return (ret);
        }
        ret = iPCIGetResponse (pInterface, Answer, &AnswerSize);
#else
        /* New Tx image buffer interface */
        /* Update messge in Image Buffer 0 and 1 */
        ret = UpdateLinTxBufDpramEntry(pInterface, &(plinMsg->uMsg.Data));
        
#endif        
        PRINTD ("Tx LIN%d : buffer %d updated\n",
                 LINNum,
                 plinMsg->uMsg.Data.bId & 0x3f);
        return ret;
      }
      break;

    default:
      return BCI_PARA_ERR;
  }
}

/*************************************************************************
**
** Function    : UpdateLinTxBufDpramEntry
**
** Description : updates LIN message in both LIN image buffers
** Parameters  : pInterface          (I/O  LIN interface handler   
**               plinMsg             (OUT) pointer to new LIN message
** Returnvalue : BCI_OK - OK
**               BCI_NO - receive queue is empty
**
*************************************************************************/
static INT32 UpdateLinTxBufDpramEntry(PInterface pInterface,
                                      BCI_ts_LinMsgData *ps_dataMsg)
{
  INT32 ret = BCI_NO;
  UINT8 bId;
  DPRAM *ps_dpram;
  BOOL oBuf0Updated, oBuf1Updated;
  UINT32 dwTry;
  
  ps_dpram = (DPRAM *) (pInterface->MemoryPtr);
  do
  {
    if (ps_dataMsg == NULL)
    {
      ret = BCI_LIN_PARAMETER_ERROR;
      break;
    }
    
    bId = (ps_dataMsg->bId & 0x3f);
    
    if (bId >= LIN_TX_BUF_IMAGE_ENTRIES)
    {
      ret = BCI_LIN_PARAMETER_ERROR;
      break;
    }
    
    dwTry = 0;
    oBuf0Updated = FALSE;
    oBuf1Updated = FALSE;    
    
    do 
    {     
      if (oBuf0Updated == FALSE)
      {
        /* Lock TxBuf Image 0 */
        ret = GetSema(pInterface, LIN_TX_BUF_IMAGE0_SEMA, 10);
        if (ret == BCI_OK)
        {
          /* Update TxBuf Image 0 */
          MEMCPY(&(ps_dpram->LIN_TxBufImage0[bId]), ps_dataMsg, sizeof(BCI_ts_LinMsgData));
          oBuf0Updated = TRUE;
        }  
        /* Unlock TxBuf Image 0 */
        FreeSema(pInterface, LIN_TX_BUF_IMAGE0_SEMA);
      }  
      
      if (oBuf1Updated == FALSE)
      {
        /* Lock TxBuf Image 1 */
        ret = GetSema(pInterface, LIN_TX_BUF_IMAGE1_SEMA, 10);
        if (ret == BCI_OK)
        {    
          /* Update TxBuf Image 1 */
          MEMCPY(&(ps_dpram->LIN_TxBufImage1[bId]), ps_dataMsg, sizeof(BCI_ts_LinMsgData));
          oBuf1Updated = TRUE;
        }  
        /* Unlock TxBuf Image 1 */
        FreeSema(pInterface, LIN_TX_BUF_IMAGE1_SEMA);
      }
      
      dwTry += 1;
      /* The Tx Image Buffer could be locked from firmware side up to 20 usec */
      if (dwTry > 10000)
      {
        return BCI_BUSY;
      }
      
    } while ((oBuf0Updated == FALSE) || (oBuf1Updated == FALSE));
    
    ret = BCI_OK;
  } while (FALSE); 
  return ret;
}


/*************************************************************************
**
** Function    : GetSema
**
** Description : gets the DPRAM semaphore
** Parameters  : pInterface          (I/O LIN interface handler   
**               bSemaNum            (IN) semaphore number 
**               dwNumTries          (IN) number of tries to get 
**                                        a semaphore          
** Returnvalue : BCI_OK   - OK
**               BCI_BUSY - cannot get the semaphore
**
*************************************************************************/
static INT32 GetSema(PInterface pInterface, UINT8 bSemaNum, UINT32 dwNumTries)
{
  UINT32 dwTry;
  INT32  ret = BCI_NO;
  PRINTD("Get  sema %d size %d DPRAM %p ", bSemaNum, pInterface->SemaSize, pInterface->MemoryPtr);
  
  if (bSemaNum > pInterface->SemaNum)
  {
    return BCI_PARA_ERR;
  }
  
  dwTry = 0;
  while (TRUE)
  {  
    if (pInterface->SemaSize == 1)
    {
      volatile UINT8 *pb_sema;
      /* Get the first semaphore pointer */
      pb_sema = DPRAM_OFFSET(pInterface, SEMA_0);
      PRINTD("1 old %d ", pb_sema[bSemaNum]);
      /* Get the semaphore */ 
      pb_sema[bSemaNum] = 0;
      PRINTD("1 new %d ", pb_sema[bSemaNum]);
      /* Check semaphore */
      if (pb_sema[bSemaNum] == 0)
      {
        ret = BCI_OK;
        break;
      }      
    }
    else if (pInterface->SemaSize == 2)
    {
      volatile UINT16 *pw_sema;
      /* Get the first semaphore pointer */
      pw_sema = DPRAM_OFFSET(pInterface, SEMA_0);
      /* Get the semaphore */ 
      PRINTD("SEMA %p 2 old %04x ", &pw_sema[bSemaNum], pw_sema[bSemaNum]);
      pw_sema[bSemaNum] = 0;
      PRINTD("2 new %04x ", pw_sema[bSemaNum]);
      /* Check semaphore */
      if (pw_sema[bSemaNum] == 0)
      {
        ret = BCI_OK;
        break;
      }  
    }  
    else
    {
      return BCI_PARA_ERR;
    }
    
    dwTry += 1;
    
    if (dwTry > dwNumTries)
    {
      ret = BCI_BUSY;
      break;
    }        
  }  
  PRINTD("%s\n", (ret == BCI_OK) ? "OK": "FAILED");
  return ret;
}

/*************************************************************************
**
** Function    : FreeSema
**
** Description : frees the DPRAM semaphore
** Parameters  : pInterface          (I/O LIN interface handler   
**               bSemaNum            (IN) semaphore number 
**
** Returnvalue : BCI_OK   - OK
**               BCI_BUSY - cannot free the semaphore
**
*************************************************************************/
static INT32 FreeSema(PInterface pInterface, UINT8 bSemaNum)
{
  INT32  ret = BCI_NO;
  
  PRINTD("Free sema %d size %d DPRAM %p ", bSemaNum, pInterface->SemaSize, pInterface->MemoryPtr);
  if (bSemaNum > pInterface->SemaNum)
  {
    return BCI_PARA_ERR;
  }  
  
  if (pInterface->SemaSize == 1)
  {
    volatile UINT8 *pb_sema;
    /* Get the first semaphore pointer */
    pb_sema = DPRAM_OFFSET(pInterface, SEMA_0);
    /* Free the semaphore */ 
    pb_sema[bSemaNum] = 0xff;
    /* Check semaphore */
    if (pb_sema[bSemaNum] != 0)
    {
      ret = BCI_OK;
    }
    else
    {
      ret = BCI_BUSY;
    }  
  }
  else if (pInterface->SemaSize == 2)
  {
    volatile UINT16 *pw_sema;
    /* Get the first semaphore pointer */
    pw_sema = DPRAM_OFFSET(pInterface, SEMA_0);
    /* Get the semaphore */ 
    PRINTD("SEMA %p 2 old %04x ", &pw_sema[bSemaNum], pw_sema[bSemaNum]);
    pw_sema[bSemaNum] = 0xffff;
    PRINTD("2 new %04x ", pw_sema[bSemaNum]);
    /* Check semaphore */
    if (pw_sema[bSemaNum] != 0)
    {
      ret = BCI_OK;
    }
    else
    {
      ret = BCI_BUSY;
    }  
  }  
  else
  {
    return BCI_PARA_ERR;
  }
  
  PRINTD("%s\n", (ret == BCI_OK) ? "OK": "FAILED");
   
  return ret;
}



/*************************************************************************
**
** Function    : CreateChecksum
**
** Description : Creates the Checksum in Classic and enhanced form and
**               returns the Checksum
**
** Parameters  : p_msg (IN)      - LIN-Message
**
** Returnvalue : Checksum
**
*************************************************************************/
static UINT8 CreateChecksum (BCI_ts_LinMsgData * p_msg)
{
  UINT8 i;
  UINT16 w_LocalHelp = 0;

  // Enhanced checksum
  if (p_msg->bModel == BCI_LIN_CRC_ENHANCED)
  {
    w_LocalHelp = p_msg->bId;
    // if addition is over 0xFF then algorithm,
    if (w_LocalHelp > 0xFF)
    {
      w_LocalHelp -= 0xFF;       // see above
    }
  }

  for (i = 0; i < p_msg->bLength; i++)
  {
    w_LocalHelp = w_LocalHelp + (UINT8) p_msg->abDataByte[i];
    // if addition is over 0xFF then algorithm,
    if (w_LocalHelp > 0xFF)
    {
      w_LocalHelp -= 0xFF;       // see above
    }
  }
  
  if (p_msg->bModel == BCI_LIN_CRC_ERROR)
  {
    UINT16 w_crcClassic, w_crcEnhanced, w_crcErr; 
    w_crcClassic = w_LocalHelp;
    w_crcEnhanced = w_LocalHelp + p_msg->bId;
    if (w_crcEnhanced > 0xFF)
    {
      w_crcEnhanced -= 0xFF;       // see above
    }
    
    // find the incorrect checksum 
    for (w_crcErr = 0; w_crcErr < 3; w_crcErr ++)
    {
      if ((w_crcErr != w_crcClassic) && (w_crcErr != w_crcEnhanced))
      {
        w_LocalHelp = w_crcErr;
        break;
      }
    }
  }
  
  return (UINT8) (~w_LocalHelp); // return the inverted modulo-256 checksum
}

/*************************************************************************
**
** Function    : CreateID
**
** Description : Creates the Protected Identifier
**
** Parameters  : b_id (IN)      - ID
**
** Returnvalue : Protected Identifier (ID-FIELD)
**
*************************************************************************/
static UINT8 CreateID (UINT8 b_id)
{
  t_BITSINBYTE_U u_id;

  u_id.b = b_id;                                                /* get the id */
  u_id.u.o6 = u_id.u.o0 ^ u_id.u.o1 ^ u_id.u.o2 ^ u_id.u.o4;    /* create even parity p0 */
  u_id.u.o7 = ~(u_id.u.o1 ^ u_id.u.o3 ^ u_id.u.o4 ^ u_id.u.o5); /* create odd parity p1 */

  return (UINT8) u_id.b;
}

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
                  UINT32 dwMsTimeout, BCI_ts_LinMsg * plinMsg)
{
  UINT8 *QueuePtr;
  UINT8 *RxQueue;

  switch (LINNum)
  {
    case 0:
      RxQueue = &((Queues *) (pInterface->QueuesPtr))->LIN_RxQueue0;
      QueuePtr = ((DPRAM *) (pInterface->MemoryPtr))->LIN_RxQueue0[*RxQueue];
      break;

    default:
      return (BCI_PARA_ERR);
  }

  PRINTD ("LIN%d : RxQueue %d  Ptr 0x%p\n", LINNum, *RxQueue, QueuePtr);

  if (*QueuePtr != 0)
  {
    UINT32 dwMsgLen;
    dwMsgLen = *(QueuePtr + 1);
    PRINTD ("LIN%d : QueuePtr != 0\n", LINNum);

    if (sizeof(BCI_ts_LinMsg) < dwMsgLen)
    {
      PRINTD ("LIN%d : Msg size too big: expected %d got %d bytes\n", LINNum, sizeof(BCI_ts_LinMsg), dwMsgLen);
      return (BCI_PARA_ERR);
    }

    MEMCPY ((char *) plinMsg, (QueuePtr + 2), dwMsgLen);
    MEMSET (QueuePtr, 0, 1);

    *RxQueue = (*RxQueue + 1) % LIN_NUM_QUEUES_ENTRIES;
    return BCI_OK;
  }

  PRINTD ("Rx LIN%d : queue is empty\n", LINNum);
  return (BCI_NO);
}
