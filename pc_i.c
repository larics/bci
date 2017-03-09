
/*************************************************************************
**    IXXAT Automation GmbH
**************************************************************************
**
**  $Workfile: $
**    Summary: Passive cards BCI support 
**   $Archive: $
**  $Revision: 1.16 $
**      $Date: 2004/02/02 09:58:40 $
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

/* {{{ Includes */

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
#include "can.h"
#include "can_ioctl.h"
#include "pc_i.h"
#include "ipci.h"

/* }}} */

/* {{{ Definitions */

/* }}} */

/*************************************************************************
**
** Function    : PC_IDownloadFirmware
**
** Description : load firmware  loopback
** Parameters  : pInterface  (I/O)  CAN interface pointer
**               HEXFilename (IN)  file which contain firmware code code
** Returnvalue : BCI_OK                - OK
**               BCI_TIMEOUT           - timeout reached
**               BCI_FW_ERROR - firmware load error
**
*************************************************************************/
int PC_IDownloadFirmware (PInterface pInterface, UINT8 FirmwareMode,
                          UINT8 * HEXFileName)
{
  /* Nothing to do here for passive cards */
  return (BCI_OK);
}

/*************************************************************************
**
** Function    : PC_IStartFirmware
**
** Description : start firmware loopback
**
** Parameters  : pInterface   (I/O)  CAN interface pointer
** Returnvalue : BCI_OK          - OK
**
*************************************************************************/
int PC_IStartFirmware (PInterface pInterface)
{
  /* Nothing to do here for passive cards */
  return (BCI_OK);
}

/*************************************************************************
**
** Function    : PC_IGenerateInterrupt
**
** Description : GenerateInterrupt loopback
**
** Parameters  : pInterface   (I/O)  CAN interface pointer
** Returnvalue : BCI_OK          - OK
**
*************************************************************************/
int PC_IGenerateInterrupt (PInterface pInterface)
{
  /* Nothing to do here for passive cards */
  return (BCI_OK);
}

/*************************************************************************
**
** Function    : PCIOpen
**
** Description : Open device file, map DPRAM, allocate dynamical memory for queues,
**               initialize queues counter and service offsets
** Parameters  : pInterface (I/O)  CAN interface pointer
** Returnvalue : BCI_OK
**               BCI_BOARD_ERR  - device file not found
**
*************************************************************************/
int PC_IOpen (PInterface pInterface)
{
  strcpy (pInterface->DeviceFile[1], pInterface->DeviceFile[0]);
  // Use next device file for second port (f.e. can1 for can0)
  (char) pInterface->DeviceFile[1][strlen (pInterface->DeviceFile[1]) - 1]++;

  printf ("Names %s and %s\n", pInterface->DeviceFile[0], pInterface->DeviceFile[1]);
  if ((pInterface->DeviceFileDesc[0] =
       open (pInterface->DeviceFile[0], O_RDWR)) == -1)
  {
#ifdef DEBUG
    printf ("Device [%s] not found. Error !\n", pInterface->DeviceFile[0]);
#endif
    return (BCI_SERV_ERR);
  }
  else
  {
#ifdef DEBUG
    printf ("Device [%s] found !\n", pInterface->DeviceFile[0]);
#endif
  }

  if ((pInterface->DeviceFileDesc[1] =
       open (pInterface->DeviceFile[1], O_RDWR)) == -1)
  {
#ifdef DEBUG
    printf ("Device [%s] not found. Port 1 will be disabled! \n",
            pInterface->DeviceFile[1]);
#endif
    pInterface->DeviceFileDesc[1] = 0;
  }
  else
  {
#ifdef DEBUG
    printf ("Device [%s] found !\n", pInterface->DeviceFile[1]);
#endif
  }

  BCI_MDelay (500);
  return (BCI_OK);
}

/* }}} */

/* {{{ PC_IClose */

/*************************************************************************
**
**  Function: PC_IClose
**
**  Description: Close device file, unmap DPRAM, free queues dynamical memory
**  Parameters:
**   * pInterface (I/O)  CAN interface handler
**  Returnvalue: BCI_OK
**
*************************************************************************/

int PC_IClose (PInterface pInterface)
{
  int Port;

  for (Port = 0; Port < MAX_PORTS_ON_BOARD; Port++)
  {
#ifdef DEBUG
    printf ("Port %d Device [%s] fd %d closing\n", Port,
            pInterface->DeviceFile[Port], pInterface->DeviceFileDesc[Port]);
#endif
    if (pInterface->DeviceFileDesc[Port] > 0)
    {
      close (pInterface->DeviceFileDesc[Port]);
    }
    else
    {
#ifdef DEBUG
      printf ("Port [%d] is not opened\n", Port);
#endif
      return (BCI_SERV_ERR);
    }
  }
  return BCI_OK;
}

/* }}} */

/* {{{ PC_IGetBoardInfo */

int PC_IGetBoardInfo (PInterface pInterface, BCI_ts_BrdInfo * BrdInfo)
{
  int ret = BCI_OK;

  BrdInfo->fw_version = 0;                  /* firmware version (BCD coded, e.g. 100H = V1.00) */
  /* number of CAN controllers (1,2) */
  BrdInfo->num_can =
    ((pInterface->DeviceFileDesc[0] == 0) ? 0 : 1) +
    ((pInterface->DeviceFileDesc[1] == 0) ? 0 : 1);
  /* CAN type, e. g. "SJA1000", "82527" */
  strcpy (BrdInfo->can_type[0],
          (pInterface->DeviceFileDesc[0] == 0) ? "None" : "SJA1000");
  strcpy (BrdInfo->can_type[1],
          (pInterface->DeviceFileDesc[1] == 0) ? "None" : "SJA1000");

  return (ret);
}

/* }}} */

/* {{{ PC_IResetCAN */

/*************************************************************************
**
** Function: PC_IResetCAN
**
** Description: reset CAN controller
** Parameters:
**  * pInterface  (I/O)  CAN interface handler
**  CANNum       (IN)  controller number
** Returnvalue: BCI_OK       - OK
**              BCI_BUSY     - command is busy
**              BCI_PARA_ERR - incorrect command
**              BCI_TIMEOUT  - timeout reached
**
*************************************************************************/
int PC_IResetCAN (PInterface pInterface, UINT8 CANNum)
{
  int ret;

  ret = ioctl (pInterface->DeviceFileDesc[CANNum], IOCTL_RESET, 0);
  if (ret != 0)
  {
    return (BCI_PARA_ERR);
  }
  return (BCI_OK);
}

/* }}} */

/* {{{ PC_IResetOverrun */

/*************************************************************************
**
** Function: PC_IResetOverrun
**
** Description: reset CAN controller overruns
** Parameters:
**  * pInterface  (I/O)  CAN interface handler
**  CANNum       (IN)  controller number
** Returnvalue: BCI_OK       - OK
**              BCI_BUSY     - command is busy
**              BCI_PARA_ERR - incorrect command
**              BCI_TIMEOUT  - timeout reached
**
*************************************************************************/
int PC_IResetOverrun (PInterface pInterface, UINT8 CANNum)
{
  int ret;

#ifdef DEBUG
  printf ("Reset CAN overrun\n");
#endif

  ret = ioctl (pInterface->DeviceFileDesc[CANNum], IOCTL_RESET_OVERRUN, 0);

  if (ret != 0)
  {
    return (BCI_PARA_ERR);
  }
  return (BCI_OK);
}

/* }}} */

/* {{{ PC_ITestCAN */

/*************************************************************************
**
** Function: PC_ITestCAN
**
** Description: test CAN interface.
**              this function sends the command TEST to the board
**              and waits for a response. In the received message,
**              the data bytes have to be inverted.
** Parameters:
**  * pInterface  (I/O)  CAN interface handler
** Returnvalue: BCI_OK       - OK
**              BCI_BUSY     - command is busy
**              BCI_PARA_ERR - incorrect command
**              BCI_TIMEOUT  - timeout reached
**              BCI_TEST_ERR - testing error
**
*************************************************************************/
int PC_ITestCAN (PInterface pInterface)
{
#ifdef DEBUG
  printf (" OK\n");
#endif
  return (BCI_OK);
}

/* }}} */

/* {{{ PC_IInitCAN */

/*************************************************************************
**
** Function:
**  PCIInitCAN
**
** Description:
**  init CAN controller
** Parameters:
**  * pInterface  (I/O)  CAN interface handler
**   CANNum      (IN)  controller number (0, 1)
**   bt0         (IN) - value for the Bus-Timing-0
**   bt1         (IN) - value for the Bus-Timing-1
**   mode        (IN) - selection of the physical bus transceiver
**                      0 = high speed (ISO/IS 11898-2) transceiver 
**                      1 = low speed (ISO/IS 11898-3) transceiver 
**
** Returnvalue: BCI_OK       - OK
**              BCI_BUSY     - command is busy
**              BCI_PARA_ERR - incorrect command
**              BCI_TIMEOUT  - timeout reached
**
***************************************************************************/
int PC_IInitCAN (PInterface pInterface, UINT8 CANNum, UINT8 bt0, UINT8 bt1,
                 UINT8 mode)
{
  int ret;

  ret = ioctl (pInterface->DeviceFileDesc[CANNum],
               IOCTL_SET_TIMING, bt0 | ((unsigned short) bt1 << 8));
  if (ret != 0)
  {
    return (BCI_PARA_ERR);
  }
  return (BCI_OK);
}

/* }}} */

/* {{{ PC_IConfigRxQueue */

/*************************************************************************
**
** Function    : PCIConfigRxQueue
**
** Description : Configure CAN controller Rx queue
** Parameters  : pInterface  (I/O)  CAN interface pointer
**               CANNum      (IN)  controller number (0, 1)
**               int_mode    (IN) - interrupt mode MC to PC
**
**                                - BCI_POLL_MODE:
**                                use this mode, when no interrupt
**                                service routine is installed, the
**                                function ReceiveCanObj() must
**                                be called cyclic to get the
**                                received CAN messages
**
**                              - BCI_LATENCY_MODE:
**                                use this mode, when a interrupt
**                                service routine is installed, the
**                                interrupt service routine must
**                                call the function
**                                PCIReceiveCanObj() to get the
**                                received CAN messages. Note, that
**                                you must call the function
**                                PCIReceiveCanObj() as long as
**                                CAN messages are received. After
**                                every received CAN message an
**                                interrupt to the PC is generated.
**                                This mode guaranted the best
**                                reactivity time.
**
**                              - BCI_THROUGHPUT_MODE:
**                                use this mode, when a interrupt
**                                service routine is installed, the
**                                interrupt service routine must
**                                call the function
**                                PCIReceiveCanObj() to get the
**                                received CAN messages. Note, that
**                                you must call the function
**                                PCIReceiveCanObj() as long as
**                                CAN messages are received. Only
**                                after a defined number of received
**                                CAN message an interrupt to the PC
**                                is generated.
**                                This mode guaranted the best
**                                data throughput.
** Returnvalue : BCI_OK       - OK
**               BCI_BUSY     - command is busy
**               BCI_PARA_ERR - incorrect command
**               BCI_TIMEOUT  - timeout reached
**
*************************************************************************/
int PC_IConfigRxQueue (PInterface pInterface, UINT8 CANNum, UINT8 IntMode)
{
  /* Nothing to do here for passive cards */
  return (BCI_OK);
}

/* }}} */

/* {{{ PC_IStartCAN */

/*************************************************************************
**
** Function    : PCIStartCAN
**
** Description : start CAN controller
** Parameters  : pInterface  (I/O)  CAN interface pointer
**               CANNum       (IN)  controller number
** Returnvalue : BCI_OK       - OK
**               BCI_BUSY     - command is busy
**               BCI_PARA_ERR - incorrect command
**               BCI_TIMEOUT  - timeout reached
**
*************************************************************************/
int PC_IStartCAN (PInterface pInterface, UINT8 CANNum)
{
  int ret;

  ret = ioctl (pInterface->DeviceFileDesc[CANNum], IOCTL_PORT_START, 0);
  if (ret != 0)
  {
    return (BCI_PARA_ERR);
  }
  return (BCI_OK);
}

/* }}} */

/* {{{ PC_IStopCAN */

/*************************************************************************
**
** Function    : PCIStopCAN
**
** Description : stop CAN controller
** Parameters  : pInterface  (I/O)  CAN interface pointer
**               CANNum       (IN)  controller number
** Returnvalue : BCI_OK       - OK
**               BCI_BUSY     - command is busy
**               BCI_PARA_ERR - incorrect command
**               BCI_TIMEOUT  - timeout reached
**
*************************************************************************/
int PC_IStopCAN (PInterface pInterface, UINT8 CANNum)
{
  int ret;

  ret = ioctl (pInterface->DeviceFileDesc[CANNum], IOCTL_PORT_STOP, 0);
  if (ret != 0)
  {
    return (BCI_PARA_ERR);
  }
  return (BCI_OK);
}

/* }}} */

/* {{{ PC_ISetAccMask */

/*************************************************************************
**
** Function    : PC_ISetAccMask
**
** Description : Sets one of the acceptance filter masks for message
**               filtering by the given values acc_mask and acc_code.
**               The given bit pattern for acc_mask specifies, which
**               bit positions within the identifier are relevant for
**               the filtering:
**                 bit = 0: means "don't care"
**                 bit = 1: means relevant
**               The given bit pattern for acc_code specifies the
**               value of the relevant bit positions within the identifier.
**               Example (11-bit identifier):
**                 acc_mask (bin) = 11111111000
**                 acc_code (bin) = 00100000000
**                         result = 00100000XXX
**                 -> All identifiers from 100H to 107H will be received.
**               This function can be called only once per filter mask:
**               CAN0/11-bit, CAN0/29-bit, CAN1/11-bit, CAN1/29-bit.
**               The CAN controller must be initialized, that means
**               the BCI must be in the state CAN-INITIALIZED.
**
** Parameters  : brd_hdl  (IN) - board handle
**               can_num  (IN) - number of the CAN controller (0,1)
**               MaskNum  (IN) - code/mask pair number
**               type     (IN) - type of filter mask (BCI_11B_MASK,
**                               BCI_29B_MASK)
**               acc_code (IN) - acceptance code value (left adjusted)
**               acc_mask (IN) - acceptance mask value (left adjusted)
**
** Returnvalue : BCI_OK        - OK
**               BCI_HDL_ERR   - wrong or unknown board handle
**               BCI_CMND_ERR  - command buffer is busy
**               BCI_RESP_ERR  - no board response
**               BCI_SERV_ERR  - error in service
**               BCI_PARA_ERR  - Invalid can number
**               BCI_STATE_ERR - BCI is in wrong state
**
*************************************************************************/

int PC_ISetAccMask (PInterface pInterface,
                    UINT8 CANNum, UINT8 MaskNum,
                    UINT8 type, UINT32 acc_code, UINT32 acc_mask)
{
  UINT32 AccMask, AccCode;
  int ret;

  AccCode = acc_code & acc_mask;
  AccMask = acc_mask;
  switch (type)
  {
    case (BCI_11B_MASK):
      ret =
        ioctl (pInterface->DeviceFileDesc[CANNum], IOCTL_SET_ACC_CODE_STD, AccCode);
      if (ret == 0)
      {
        ret =
          ioctl (pInterface->DeviceFileDesc[CANNum], IOCTL_SET_ACC_MASK_STD,
                 AccMask);
      }
      break;
    case (BCI_29B_MASK):
      ret =
        ioctl (pInterface->DeviceFileDesc[CANNum], IOCTL_SET_ACC_CODE_EXT, AccCode);
      if (ret == 0)
      {
        ret =
          ioctl (pInterface->DeviceFileDesc[CANNum], IOCTL_SET_ACC_MASK_EXT,
                 AccMask);
      }
      break;
    default:
      return (BCI_PARA_ERR);
  }

  return (ret == 0 ? BCI_OK : BCI_SERV_ERR);

}

/* }}} */

/* {{{ PC_IRegRxID */

/*************************************************************************
**
** Function    : PC_IRegRxID
**
** Description : Registers an identifier for message reception.
**               This function must be called for all messages to
**               be received. Up to 4096 identifiers per CAN controller
**               can be registered (11- and 29-bit identifiers in all).
**               For reception of all possible messages, the
**               mode "accept all messages" (BCI_ACC_ALL) can be used.
**               The CAN controller must be initialized, that means
**               the BCI must be in the state CAN-INITIALIZED.
**
** Parameters  : brd_hdl (IN) - board handle
**               can_num (IN) - number of the CAN controller (0,1)
**               mff     (IN) - message frame format:
**                              BCI_MFF_11_DAT: data frame with 11 bit identifier
**                              BCI_MFF_11_RMT: remote frame with 11 bit identifier
**                              BCI_MFF_29_DAT: data frame with 29 bit identifier
**                              BCI_MFF_29_RMT: remote frame with 29 bit identifier
**               id      (IN) - identifier to be registered
**
** Returnvalue : BCI_OK        - OK
**               BCI_HDL_ERR   - wrong or unknown board handle
**               BCI_RESP_ERR  - no board response
**               BCI_SERV_ERR  - error in service
**               BCI_PARA_ERR  - Invalid can number
**               BCI_STATE_ERR - BCI is in wrong state
**
*************************************************************************/
int PC_IRegRxID (PInterface pInterface, UINT8 CANNum, UINT8 mff, UINT32 id)
{

  int ret;

#ifdef DEBUG
  printf ("CAN%d : mff %d id 0x%lx\n", CANNum, mff, id);
#endif

  switch (mff)
  {
    case (BCI_MFF_11_DAT):
      ret = ioctl (pInterface->DeviceFileDesc[CANNum], IOCTL_REGISTER_STD_ID, id);
      break;
    case (BCI_MFF_11_RMT):
      ret =
        ioctl (pInterface->DeviceFileDesc[CANNum], IOCTL_REGISTER_STD_ID,
               id | BCI_FILTER_RTR_MASK);
      break;
    case (BCI_MFF_29_DAT):
      ret = ioctl (pInterface->DeviceFileDesc[CANNum], IOCTL_REGISTER_EXT_ID, id);
      break;
    case (BCI_MFF_29_RMT):
      ret =
        ioctl (pInterface->DeviceFileDesc[CANNum], IOCTL_REGISTER_EXT_ID,
               id | BCI_FILTER_RTR_MASK);
      break;

    default:
      return (BCI_PARA_ERR);
  }

  return (BCI_OK);
}

/* }}} */

/* {{{ PC_IUnregRxID */

/*************************************************************************
**
** Function    : PC_IUnregRxId
**
** Description : Unregisters an identifier for message reception.
**               The CAN controller must be initialized, that means
**               the BCI must be in the state CAN-INITIALIZED.
**
** Parameters  : brd_hdl (IN) - board handle
**               can_num (IN) - number of the CAN controller (0,1)
**               mff     (IN) - message frame format:
**                              BCI_MFF_11_DAT: data frame with 11 bit identifier
**                              BCI_MFF_11_RMT: remote frame with 11 bit identifier
**                              BCI_MFF_29_DAT: data frame with 29 bit identifier
**                              BCI_MFF_29_RMT: remote frame with 29 bit identifier
**               id      (IN) - identifier to be unregistered
**
** Returnvalue : BCI_OK        - OK
**               BCI_HDL_ERR   - wrong or unknown board handle
**               BCI_CMND_ERR  - command buffer is busy
**               BCI_RESP_ERR  - no board response
**               BCI_SERV_ERR  - error in service
**               BCI_PARA_ERR  - invalid can number
**               BCI_STATE_ERR - BCI is in wrong state
**
*************************************************************************/

int PC_IUnregRxID (PInterface pInterface, UINT8 CANNum, UINT8 mff, UINT32 id)
{

  int ret;

#ifdef DEBUG
  printf ("CAN%d : mff %d id 0x%lx\n", CANNum, mff, id);
#endif

  switch (mff)
  {
    case (BCI_MFF_11_DAT):
      ret = ioctl (pInterface->DeviceFileDesc[CANNum], IOCTL_UNREGISTER_STD_ID, id);
      break;
    case (BCI_MFF_11_RMT):
      ret =
        ioctl (pInterface->DeviceFileDesc[CANNum], IOCTL_UNREGISTER_STD_ID,
               id | BCI_FILTER_RTR_MASK);
      break;
    case (BCI_MFF_29_DAT):
      ret = ioctl (pInterface->DeviceFileDesc[CANNum], IOCTL_UNREGISTER_EXT_ID, id);
      break;
    case (BCI_MFF_29_RMT):
      ret =
        ioctl (pInterface->DeviceFileDesc[CANNum], IOCTL_UNREGISTER_EXT_ID,
               id | BCI_FILTER_RTR_MASK);
      break;

    default:
      return (BCI_PARA_ERR);
  }

  return (BCI_OK);
}

/* }}} */

/* {{{ PC_IReceiveCANObj */

/*************************************************************************
**
** Function    : PCIReceiveCANObj
**
** Description : receive CAN object.
**               Try to find unprocessed object in the receive queue;
**               if there is no such objects, return BCI_NO
** Parameters  : *pnterface          (I/O)  CAN interface pointer
**               CANNum               (IN)  controller number
**               *ReceivedCANMsg  (OUT)   received CAN object
** Returnvalue : BCI_OK          - OK
**               BCI_NO - receive queue is empty
**
*************************************************************************/
int
PC_IReceiveCANObj (PInterface pInterface, UINT8 CANNum,
                   BCI_ts_CanMsg * ReceivedCANMsg)
{
  int ret;

  switch (CANNum)
  {
    case 0:
    case 1:
      ret = read (pInterface->DeviceFileDesc[CANNum], ReceivedCANMsg, 18);
      break;

    default:
      return (BCI_PARA_ERR);
  }

  if (ret == BCI_QUEUE_ITEM_SIZE)
  {
//    ReceivedCANMsg->time_stamp /= 125;
#ifdef DEBUG
    printf ("Read from %d port with res %d\n", CANNum, ret);
#endif
    return (BCI_OK);
  }

  if (ret == 0)
  {
#ifdef DEBUG
    printf ("Rx CAN%d : queue is empty\n", CANNum);
#endif
    return (BCI_NO);
  }

  if (ret < 0)
  {
#ifdef DEBUG
    printf ("Rx CAN%d : error when receiving message !\n", CANNum);
#endif
    return (BCI_PARA_ERR);
  }

  return (BCI_OK);

}

/* }}} */

/* {{{ PC_ITransmitCANObj */

/*************************************************************************
**
** Function    : PCITransmitCANObj
**
** Description : transmit CAN object.
**               Try to find free slot in the transmit queue;
**               if there is no free slots, return BCI_BUSY
** Parameters  : pInterface          (I/O)  CAN interface pointer
**               CANNum              (IN)  controller number
**               *ToSendCANMsg       (IN)  CAN object to transmit
** Returnvalue : BCI_OK          - OK
**               BCI_BUSY  - transmit queue is full
**
*************************************************************************/
int
PC_ITransmitCANObj (PInterface pInterface, UINT8 CANNum,
                    BCI_ts_CanMsg * ToSendCANMsg)
{
  int ret;

  switch (CANNum)
  {
    case 0:
    case 1:
      ret = write (pInterface->DeviceFileDesc[CANNum], ToSendCANMsg, 18);
      break;

    default:
      return (BCI_PARA_ERR);
  }

  if (ret == BCI_QUEUE_ITEM_SIZE)
  {
#ifdef DEBUG
    printf ("Write to %d port with res %d\n", CANNum, ret);
#endif
    return (BCI_OK);
  }

  if (ret == 0)
  {
#ifdef DEBUG
    printf ("Tx CAN%d : queue is full\n", CANNum);
#endif
    return (BCI_BUSY);
  }

  if (ret < 0)
  {
#ifdef DEBUG
    printf ("Tx CAN%d : error when sending message !\n", CANNum);
#endif
    return (BCI_PARA_ERR);
  }

  return (BCI_OK);

}

/* }}} */

/* {{{ PC_IGetBrdStatus */

/*************************************************************************
**
** Function    : PCIGetBrdStatus
**
** Description : get CAN board status
** Parameters  : pInterface          (I/O)  CAN interface pointer
**               *BrdStatus          (IN)  current CAN board status
** Returnvalue : BCI_OK          - OK
**               BCI_BUSY        - failed
**
*************************************************************************/
int PC_IGetBrdStatus (PInterface pInterface, BCI_ts_BrdSts * BrdStatus)
{
  int ret = BCI_OK;

  ret =
    ioctl (pInterface->DeviceFileDesc[0], IOCTL_GET_STATUS, &BrdStatus->can0_status);
#ifdef DEBUG
  printf ("Received status0 [0x%x] from driver\n", BrdStatus->can0_status);
#endif
  ret =
    ioctl (pInterface->DeviceFileDesc[1], IOCTL_GET_STATUS, &BrdStatus->can1_status);
#ifdef DEBUG
  printf ("Received status1 [0x%x] from driver\n", BrdStatus->can1_status);
#endif

  BrdStatus->can0_busload = 0;              /* CAN 0 busload */
  BrdStatus->can1_busload = 0;              /* CAN 1 busload */
  BrdStatus->cpu_load = 0;                  /* CPU load */
  BrdStatus->counter = 0;                   /* firmeware live sign counter */

  return (ret == 0 ? BCI_OK : BCI_BUSY);
}

/* }}} */

/*************************************************************************
**
** Function: PCIWaitForData
**
** Description: wait for data. Only for
**              BCI_LATENCY_MODE (Interrupt after every received message) and
**              BCI_THROUGHPUT_MODE (Interrupt when queue is full) modes.
** Parameters:
**  * pInterface  (I/O)  CAN interface handler
** Returnvalue:
**   TRUE : - data available
**   FALSE: - no data
**
*************************************************************************/
int PC_IWaitForData (PInterface pInterface, UINT8 CANNum, UINT32 Timeout_msec)
{
  struct timeval timeout;
  fd_set read_fds;

  FD_ZERO (&read_fds);
  FD_SET (pInterface->DeviceFileDesc[CANNum], &read_fds);

  msec2timeval (Timeout_msec, &timeout);

  return (select
          (pInterface->DeviceFileDesc[CANNum] + 1, &read_fds, NULL, NULL, &timeout));
}

/* }}} */
