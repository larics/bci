/*************************************************************************
**    IXXAT Automation GmbH
**************************************************************************
**
**  $Workfile: $
**    Summary: BCI library API functions LIN part
**   $Archive: $
**  $Revision: 1.38 $
**      $Date: 2008/02/29 11:47:50 $
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
**    compiler directives
*************************************************************************/

/*************************************************************************
**    include-files
*************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "integral.h"
#include "bci.h"
#include "bci_int.h"
#include "ipci.h"
#include "pc_i.h"
#include "can.h"
#include "can_ioctl.h"
#include "filterlist.h"

/*************************************************************************
**    global variables
*************************************************************************/

/*************************************************************************
**    static constants, types, macros, variables
*************************************************************************/

#ifdef PRINTI
#undef PRINTI
#define PRINTI(format, args...) printf(format, ## args)
#endif


#ifdef PRINTD
#undef PRINTD
#if (BCI_IPCI_DEBUG == 1)
#define PRINTD(format, args...) printf(format, ## args)
#else
#define PRINTD(format, args...)
#endif
#endif

/*************************************************************************
**    static function-prototypes
*************************************************************************/

/*************************************************************************
**    global functions
*************************************************************************/

/*************************************************************************
**
** Function    : BCI_LinControlStart
**
** Description : start LIN controller
**
** Parameters:
**  board_hdl    (I/O)  board handle
**  LINNum       (IN)   controller number
**
** Returnvalue: BCI_OK           - OK
**              BCI_BUSY         - controller is busy
**              BCI_PARA_ERR     - invalid parameter value
**              BCI_TIMEOUT      - no answer from controller,
**                                 timeout reached
**              BCI_LIN_NOT_SUPP - LIN is not supported by hardware
**
*************************************************************************/
int BCI_LinControlStart (BCI_BRD_HDL board_hdl, UINT8 LINNum)
{
  int ret;
  t_Interface *pInterface = (t_Interface *) board_hdl;

  PRINTD ("Start for [%d] %s LINNum %d\n",
          pInterface->DeviceFileDesc[0], pInterface->Name, LINNum);

  // Is the Start function defined ?
  if (pInterface->LIN.Start == NULL)
  {
    return BCI_LIN_NOT_SUPP;
  }

  ret = pInterface->LIN.CheckState (pInterface, LINNum, BCI_INITIALIZED);

  if (ret != BCI_OK)
  {
    return ret;
  }

  // Call start function for this interface
  ret = pInterface->LIN.Start (pInterface, LINNum);

  if (ret == BCI_OK)
  {
    pInterface->LIN.SetState (pInterface, LINNum, BCI_RUNNING);
  }

  return ret;
}

/*************************************************************************
**
** Function    : BCI_LinControlStop
**
** Description : stop LIN controller
**
** Parameters:
**  board_hdl   (I/O) board handle
**  LINNum      (IN)  controller number
**
** Returnvalue: BCI_OK           - OK
**              BCI_BUSY         - controller is busy
**              BCI_PARA_ERR     - invalid parameter value
**              BCI_TIMEOUT      - no answer from controller,
**                                 timeout reached
**              BCI_LIN_NOT_SUPP - LIN is not supported by hardware
**
*************************************************************************/
int BCI_LinControlStop (BCI_BRD_HDL board_hdl, UINT8 LINNum)
{
  int ret;
  t_Interface *pInterface = (t_Interface *) board_hdl;

  PRINTD ("Stop for [%d] %s LINNum %d\n",
          pInterface->DeviceFileDesc[0], pInterface->Name, LINNum);

  // Is the Stop function defined ?
  if (pInterface->LIN.Stop == NULL)
  {
    return BCI_LIN_NOT_SUPP;
  }

  ret = pInterface->LIN.CheckState (pInterface, LINNum, BCI_RUNNING);

  if (ret != BCI_OK)
  {
    return ret;
  }

  // Call stop function for this interface
  ret = (pInterface->LIN.Stop ((t_Interface *) board_hdl, LINNum));

  if (ret == BCI_OK)
  {
    pInterface->LIN.SetState (pInterface, LINNum, BCI_INITIALIZED);
  }
  return ret;
}

/*************************************************************************
**
** Function    : BCI_LinControlReset
**
** Description : reset LIN controller
**
** Parameters:
**  board_hdl  (I/O) board handle
**  LINNum     (IN)  controller number
**
** Returnvalue: BCI_OK           - OK
**              BCI_BUSY         - controller is busy
**              BCI_PARA_ERR     - invalid parameter value
**              BCI_TIMEOUT      - no answer from controller,
**                                 timeout reached
**              BCI_LIN_NOT_SUPP - LIN is not supported by hardware
**
*************************************************************************/
int BCI_LinControlReset (BCI_BRD_HDL board_hdl, UINT8 LINNum)
{
  int ret;
  t_Interface *pInterface = (t_Interface *) board_hdl;

  PRINTD ("Reset for [%d] %s LINNum %d\n",
          pInterface->DeviceFileDesc[0], pInterface->Name, LINNum);

  // Is the Reset function defined ?
  if (pInterface->LIN.Reset == NULL)
  {
    return BCI_LIN_NOT_SUPP;
  }

  ret = pInterface->LIN.CheckState (pInterface, LINNum,
                                    (BCI_INITIALIZED | BCI_RUNNING |
                                     BCI_BOARD_INITIALIZED));

  if (ret != BCI_OK)
  {
    return ret;
  }

  // Call Reset function for this interface
  ret = (pInterface->LIN.Reset ((t_Interface *) board_hdl, LINNum));

  if (ret == BCI_OK)
  {
    pInterface->LIN.SetState (pInterface, LINNum, BCI_BOARD_INITIALIZED);
  }

  return ret;
}

/*************************************************************************
**
** Function    : BCI_LinGetStatus
**
** Description : This function reads LIN status register from the DPRAM,
**               which is cyclically updated.
**
** Parameters  : board_hdl (IN)  - board handle
**               LINNum    (IN)  - number of the  controller (0,1)
**               LINSts    (OUT) - pointer to store the LIN status:
**
** Returnvalue: BCI_OK           - OK
**              BCI_BUSY         - controller is busy
**              BCI_PARA_ERR     - invalid parameter value
**              BCI_TIMEOUT      - no answer from controller,
**                                 timeout reached
**              BCI_LIN_NOT_SUPP - LIN is not supported by hardware
**
*************************************************************************/
int BCI_LinGetStatus (BCI_BRD_HDL board_hdl, UINT8 LINNum, BCI_ts_linBrdSts * LINSts)
{
  t_Interface *pInterface = (t_Interface *) board_hdl;
  int ret;

  if (LINSts == NULL)
  {
    return BCI_PARA_ERR;
  }

  // Is the GetBrdStatus function defined ?
  if (pInterface->LIN.GetBrdStatus == NULL)
  {
    return BCI_LIN_NOT_SUPP;
  }

  ret = pInterface->LIN.GetBrdStatus (pInterface, LINSts);

  if (ret != BCI_OK)
  {
    return ret;
  }
  return BCI_OK;
}

/*************************************************************************
**
** Function    : BCI_LinControlInitialize
**
** Description : init LIN controller
** Parameters:
**   board_hdl  (I/O) board handle
**   LINNum     (IN)  controller number (0, 1)
**   LINmode    (IN)  LIN controller communication mode selection:
**                   - BCI_LIN_OPMODE_SLAVE
**                   - BCI_LIN_OPMODE_MASTER
**   intmode    (IN) - event handling mode:
**                   - BCI_LATENCY_MODE:
**                     After every received message an
**                     interrupt/event is generated.
**                     This mode guarantees the best
**                     reactivity time on received messages.
**
**                   - BCI_THROUGHPUT_MODE:
**                     An interrupt/event
**                     is generated only after a predefined number of
**                     received messages.
**                     This mode guarantees the best
**                     data throughput because of the reduced
**                     interrupt load for the PC.
**
**                   - BCI_POLL_MODE:
**                     Use this mode if your OS does not support
**                     interrupt handling or your application
**                     does not use the event handling for
**                     LIN message reception.
**
**   bitrate    (IN) bitrate in bits/s
**
** Returnvalue: BCI_OK           - OK
**              BCI_BUSY         - controller is busy
**              BCI_PARA_ERR     - invalid parameter value
**              BCI_TIMEOUT      - no answer from controller,
**                                 timeout reached
**              BCI_LIN_NOT_SUPP - LIN is not supported by hardware
**
*************************************************************************/
int BCI_LinControlInitialize (BCI_BRD_HDL board_hdl, UINT8 LINNum, UINT8 LINmode,
                              UINT8 intmode, UINT16 bitrate)
{
  int ret;
  t_Interface *pInterface = (t_Interface *) board_hdl;

  PRINTD ("Init for [%d] %s LINNum %d mode %d bitrate %d\n",
          pInterface->DeviceFileDesc[0], pInterface->Name, LINNum, intmode, bitrate);

  // Is the Init function defined ?
  if (pInterface->LIN.Init == NULL)
  {
    return BCI_LIN_NOT_SUPP;
  }

  ret = pInterface->LIN.CheckState (pInterface, LINNum, BCI_BOARD_INITIALIZED);

  if (ret != BCI_OK)
  {
    return ret;
  }

  // Call Init function for this interface
  ret = pInterface->LIN.Init ((t_Interface *) board_hdl,
                              LINNum, LINmode, intmode, bitrate);

  if (ret == BCI_OK)
  {
    pInterface->LIN.SetState (pInterface, LINNum, BCI_INITIALIZED);
  }
  return ret;
}

/*************************************************************************
**
** Function    : BCI_LinControlWriteMsg
**
** Description : write the LIN control message
**
** Parameters:
**  board_hdl           (I/O) board handle
**  LINum               (IN)  controller number
**  fSend               (IN)  send mode flag
**                            BCI_LIN_WRITE_DIRECT - direct send
**                            BCI_LIN_WRITE_TABLE - update send table
**  *plinMsg            (IN)  message to write
**
** Returnvalue: BCI_OK           - OK
**              BCI_BUSY         - transmit queue is full
**              BCI_PARA_ERR     - invalid parameter value
**              BCI_TIMEOUT      - no answer from controller,
**                                 timeout reached
**              BCI_LIN_NOT_SUPP - LIN is not supported by hardware
**
*************************************************************************/
int
BCI_LinControlWriteMsg (BCI_BRD_HDL board_hdl, UINT8 LINNum,
                        UINT8 fSend, BCI_ts_LinMsg * plinMsg)
{
  t_Interface *pInterface = (t_Interface *) board_hdl;

  // Is the WriteMsg function defined ?
  if (pInterface->LIN.WriteMsg == NULL)
  {
    return BCI_LIN_NOT_SUPP;
  }

  return (pInterface->LIN.WriteMsg
          ((t_Interface *) board_hdl, LINNum, fSend, plinMsg));
}

/*************************************************************************
**
** Function    : BCI_LinRecvMsg
**
** Description : receive a LIN message.
**               Try to find unprocessed object in the LIN receive queue;
**               if there is no such objects, return BCI_NO
**
** Parameters:
**  board_hdl            (I/O) board handle
**  LINNum               (IN)  controller number
**  dwMsTimeout          (IN)  wait timeout in ms
**  plinMsg              (IN)  pointer to store received LIN message
**
** Returnvalue: BCI_OK           - OK
**              BCI_NO           - receive queue is empty
**              BCI_PARA_ERR     - invalid parameter value
**              BCI_TIMEOUT      - no answer from controller,
**                                 timeout reached
**              BCI_LIN_NOT_SUPP - LIN is not supported by hardware
**
*************************************************************************/
int BCI_LinRecvMsg (BCI_BRD_HDL board_hdl, UINT8 LINNum,
                    UINT32 dwMsTimeout, BCI_ts_LinMsg * plinMsg)
{
  int ret = BCI_NO;
  t_Interface *pInterface = (t_Interface *) board_hdl;

  do
  {
    if (plinMsg == NULL)
    {
      ret = BCI_PARA_ERR;
      break;
    }

    PRINTD ("Receive LIN object for [%d] %s LINNum %d \n",
            pInterface->DeviceFileDesc[0], pInterface->Name, LINNum);

    // Check queue first
    ret = (pInterface->LIN.ReadMsg
           ((t_Interface *) board_hdl, LINNum, dwMsTimeout, plinMsg));

    if (ret == BCI_OK)
    {
      /* Message is received, return */
      break;
    }

    /* We have message in queue */
    if (dwMsTimeout != BCI_NO_WAIT)
    {
      /* Queue is empty, lets wait for Timeout */
      PRINTD ("WaitForData for [%d] %s\n",
              pInterface->DeviceFileDesc[0], pInterface->Name);

      ret = (pInterface->WaitForData ((t_Interface *) board_hdl, LINNum, dwMsTimeout));
      if (ret == BCI_OK)
      {
        /* There was a message received during timeout */
        ret = (pInterface->LIN.ReadMsg
               ((t_Interface *) board_hdl, LINNum, dwMsTimeout, plinMsg));
      }
    }
  }
  while (0);
  return ret;
}


/*************************************************************************
**
** Function    : BCI_LinSlaveUpdateTxBuffer
**
** Description : updates the slave transmit buffer
**
** Parameters:
**  board_hdl            (I/O) board handle
**  LINNum               (IN)  controller number
**  bId                  (IN)  LIN message Id
**  bCRCType             (IN)  LIN checksum type, possible values:
**                             BCI_LIN_CRC_CLASSIC
**                             BCI_LIN_CRC_ENHANCED
**                             BCI_LIN_CRC_ERROR
**  pbData               (IN)  pointer to data which is used to
**                             update slave transmit buffer.
**                             If pbData == NULL, transmit buffer
**                             will be disabled
**  bLen                 (IN)  data length
**
** Returnvalue: BCI_OK           - OK
**              BCI_BUSY         - controller is busy
**              BCI_PARA_ERR     - invalid parameter value
**              BCI_TIMEOUT      - no answer from controller,
**                                 timeout reached
**              BCI_LIN_NOT_SUPP - LIN is not supported by hardware
**
*************************************************************************/
int BCI_LinSlaveUpdateTxBuffer (BCI_BRD_HDL board_hdl, UINT8 LIN_Controller,
                                UINT8 bId, UINT8 bCRCType, UINT8 * pbData, UINT8 bLen)
{
  int ret = BCI_NO;
  BCI_ts_LinMsg slinUpdateBufMsg;
  MEMSET(&slinUpdateBufMsg, 0x0, sizeof(slinUpdateBufMsg));
  do
  {

    if (bLen > LIN_MAX_DATA_LEN)
    {
      ret = BCI_PARA_ERR;
      break;
    }

    
    if ((bCRCType != BCI_LIN_CRC_CLASSIC) &&
        (bCRCType != BCI_LIN_CRC_ERROR) &&
        (bCRCType != BCI_LIN_CRC_ENHANCED))
    {
      ret = BCI_PARA_ERR;
      break;
    }

    /* Header */
    slinUpdateBufMsg.sHdr.dwTimeStamp = 0;
    slinUpdateBufMsg.sHdr.bType = BCI_LIN_MSGTYPE_DATA;
    slinUpdateBufMsg.sHdr.bInfo = BCI_LIN_INFOTYPE_NONE;

    /* Message */
    slinUpdateBufMsg.uMsg.Data.bId = bId;     /* message identifier */
    slinUpdateBufMsg.uMsg.Data.bModel = bCRCType;
    slinUpdateBufMsg.uMsg.Data.bCheckSum = 0;

    if (pbData == NULL)
    {
      /* Disable Tx buffer */
      slinUpdateBufMsg.uMsg.Data.bLength = 0;
      memset (slinUpdateBufMsg.uMsg.Data.abDataByte, 0x0,
              slinUpdateBufMsg.uMsg.Data.bLength);
      slinUpdateBufMsg.uMsg.Data.bSendData = BCI_LIN_SEND_NO_DATA;
      PRINTD ("LIN buffer id 0x%x disable\n", slinUpdateBufMsg.uMsg.Data.bId);
    }
    else
    {
      /* Update Tx buffer */
      slinUpdateBufMsg.uMsg.Data.bLength = bLen;
      memcpy (slinUpdateBufMsg.uMsg.Data.abDataByte, pbData,
              slinUpdateBufMsg.uMsg.Data.bLength);
      slinUpdateBufMsg.uMsg.Data.bSendData = BCI_LIN_SEND_DATA;
      PRINTD ("LIN buffer id 0x%x update\n", slinUpdateBufMsg.uMsg.Data.bId);
    }

    slinUpdateBufMsg.uMsg.Data.bRes = 0;

    ret = BCI_LinControlWriteMsg (board_hdl, LIN_Controller,
                                  BCI_LIN_WRITE_TABLE, &slinUpdateBufMsg);

    if (ret == BCI_OK)
    {
      PRINTD ("LIN buffer id 0x%x command success\n", slinUpdateBufMsg.uMsg.Data.bId);
    }
    else
    {
      PRINTD ("LIN buffer id 0x%x command failed\n", slinUpdateBufMsg.uMsg.Data.bId);
    }
  }
  while (0);

  return ret;
}


/*************************************************************************
**
** Function    : BCI_LinMasterSendMsg
**
** Description : send the master LIN message
**
** Parameters:
**  board_hdl            (I/O) board handle
**  LINNum               (IN)  controller number
**  bId                  (IN)  LIN message Id
**  bCRCType             (IN)  LIN checksum type, possible values:
**                             BCI_LIN_CRC_CLASSIC
**                             BCI_LIN_CRC_ENHANCED
**                             BCI_LIN_CRC_ERROR
**  pbData               (IN)  data pointer to update buffer
**  bLen                 (IN)  data length
**
** Returnvalue: BCI_OK           - OK
**              BCI_BUSY         - controller is busy
**              BCI_PARA_ERR     - invalid parameter value
**              BCI_TIMEOUT      - no answer from controller,
**                                 timeout reached
**              BCI_LIN_NOT_SUPP - LIN is not supported by hardware
**
*************************************************************************/
int BCI_LinMasterSendMsg (BCI_BRD_HDL board_hdl, UINT8 LIN_Controller,
                          UINT8 bId, UINT8 bCRCType, UINT8 * pbData, UINT8 bLen)
{
  int ret = BCI_NO;
  BCI_ts_LinMsg slinTxMsg;
  do
  {
    if (pbData == NULL)
    {
      ret = BCI_PARA_ERR;
      break;
    }

    if (bLen > LIN_MAX_DATA_LEN)
    {
      ret = BCI_PARA_ERR;
      break;
    }

    if ((bCRCType != BCI_LIN_CRC_CLASSIC) &&
        (bCRCType != BCI_LIN_CRC_ERROR) &&
        (bCRCType != BCI_LIN_CRC_ENHANCED))
    {
      ret = BCI_PARA_ERR;
      break;
    }

    /* Header */
    slinTxMsg.sHdr.dwTimeStamp = 0;
    slinTxMsg.sHdr.bType = BCI_LIN_MSGTYPE_DATA;
    slinTxMsg.sHdr.bInfo = BCI_LIN_INFOTYPE_NONE;

    /* Message */
    slinTxMsg.uMsg.Data.bId = bId;            /* message identifier */
    slinTxMsg.uMsg.Data.bModel = bCRCType;
    slinTxMsg.uMsg.Data.bCheckSum = 0;
    slinTxMsg.uMsg.Data.bSendData = BCI_LIN_SEND_DATA;
    slinTxMsg.uMsg.Data.bLength = bLen;
    slinTxMsg.uMsg.Data.bRes = 0;
    memcpy (slinTxMsg.uMsg.Data.abDataByte, pbData, slinTxMsg.uMsg.Data.bLength);

    ret = BCI_LinControlWriteMsg (board_hdl, LIN_Controller,
                                  BCI_LIN_WRITE_DIRECT, &slinTxMsg);

    if (ret == BCI_OK)
    {
      PRINTD ("LIN message id 0x%x len %d sent\n",
              slinTxMsg.uMsg.Data.bId, slinTxMsg.uMsg.Data.bLength);
    }
    else
    {
      PRINTD ("LIN message send failed !!!\n");
    }

  }
  while (0);

  return ret;
}

/*************************************************************************
**
** Function    : BCI_LinMasterRequestId
**
** Description : request a buffer content with Id bId from slave.
**               To get a data, BCI_LinRecvMsg() should be called later.
**               Warning: bId and bLen should be exactly the same as slave
**               buffer configuration !
**
** Parameters:
**  board_hdl            (I/O) board handle
**  LINNum               (IN)  controller number
**  bId                  (IN)  slave buffer Id
**  bCRCType             (IN)  LIN checksum type, possible values:
**                             BCI_LIN_CRC_CLASSIC
**                             BCI_LIN_CRC_ENHANCED
**                             BCI_LIN_CRC_ERROR
**  bLen                 (IN)  slave buffer data length
**
** Returnvalue: BCI_OK           - OK
**              BCI_BUSY         - controller is busy
**              BCI_PARA_ERR     - invalid parameter value
**              BCI_TIMEOUT      - no answer from controller,
**                                 timeout reached
**              BCI_LIN_NOT_SUPP - LIN is not supported by hardware
**
*************************************************************************/
int BCI_LinMasterRequestId (BCI_BRD_HDL board_hdl, UINT8 LIN_Controller,
                            UINT8 bId, UINT8 bCRCType, UINT8 bLen)
{
  int ret;
  BCI_ts_LinMsg slinTxMsg;
  do
  {
    if (bLen > LIN_MAX_DATA_LEN)
    {
      ret = BCI_PARA_ERR;
      break;
    }

    if ((bCRCType != BCI_LIN_CRC_CLASSIC) &&
        (bCRCType != BCI_LIN_CRC_ERROR) &&    
        (bCRCType != BCI_LIN_CRC_ENHANCED))
    {
      ret = BCI_PARA_ERR;
      break;
    }

    /* Header */
    slinTxMsg.sHdr.dwTimeStamp = 0;
    slinTxMsg.sHdr.bType = BCI_LIN_MSGTYPE_DATA;
    slinTxMsg.sHdr.bInfo = BCI_LIN_INFOTYPE_ID_ONLY;

    /* Message */
    slinTxMsg.uMsg.Data.bId = bId;            /* message identifier */
    slinTxMsg.uMsg.Data.bModel = bCRCType;
    slinTxMsg.uMsg.Data.bCheckSum = 0;
    slinTxMsg.uMsg.Data.bSendData = BCI_LIN_SEND_NO_DATA;
    slinTxMsg.uMsg.Data.bLength = bLen;
    slinTxMsg.uMsg.Data.bRes = 0;
    memset (slinTxMsg.uMsg.Data.abDataByte, 0x0,
            sizeof (slinTxMsg.uMsg.Data.abDataByte));

    ret = BCI_LinControlWriteMsg (board_hdl, LIN_Controller,
                                  BCI_LIN_WRITE_DIRECT, &slinTxMsg);

    if (ret == BCI_OK)
    {
      PRINTD ("LIN message id 0x%x len %d sent\n",
              slinTxMsg.uMsg.Data.bId, slinTxMsg.uMsg.Data.bLength);
    }
    else
    {
      PRINTD ("LIN message send failed !!!\n");
    }

  }
  while (0);
  return ret;
}
