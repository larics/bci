
/*************************************************************************
**    IXXAT Automation GmbH
**************************************************************************
**
**       File: ipci.h
**    Summary: Internal header file for PC-I boards support functions
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
** ext. Units:
**
@@    History:
@@
**************************************************************************
**    all rights reserved
*************************************************************************/

#ifndef PC_I_H
#define PC_I_H

/*************************************************************************
**    constants and macros
*************************************************************************/
#define RESPONSE_TIMEOUT  3000              /* Wait for response to command (msec) */

#define MEMSET memset
#define MEMCPY memcpy
#define MEMCMP memcmp

/*************************************************************************
**    data types
*************************************************************************/

/*************************************************************************
**    global variables
*************************************************************************/

/*************************************************************************
**    function prototypes
*************************************************************************/

/*************************************************************************
**
** Function    : PC_IDownloadFirmware
**
** Description : load firmware code into microcontroller
** Parameters  : pInterface  (I/O)  CAN interface pointer
**               HEXFilename (IN)  file which contain firmware code code
** Returnvalue : BCI_OK                - OK
**               BCI_TIMEOUT           - timeout reached
**               BCI_FW_ERROR - firmware load error
**
*************************************************************************/
int
PC_IDownloadFirmware (PInterface pInterface, UINT8 FirmwareMode,
                      UINT8 * HEXFileName);

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
int PC_IStartFirmware (PInterface pInterface);

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
int PC_IGenerateInterrupt (PInterface pInterface);

/*************************************************************************
**
** Function    : PC_IOpen
**
** Description : Open device file, map DPRAM, allocate dynamical memory for queues,
**               initialize queues counter and service offsets
** Parameters  : pInterface (I/O)  CAN interface pointer
** Returnvalue : BCI_OK
**               BCI_BOARD_ERR  - device file not found
**
*************************************************************************/
int PC_IOpen (PInterface pInterface);

/*************************************************************************
**
** Function    : PC_IClose
**
** Description : Close device file, unmap DPRAM, free queues
**               dynamical memory
** Parameters  : pInterface   (I/O)  CAN interface pointer
** Returnvalue : BCI_OK          - OK
**
*************************************************************************/
int PC_IClose (PInterface pInterface);

/*************************************************************************
**
** Function    : PC_IStartCAN
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
int PC_IStartCAN (PInterface pInterface, UINT8 CANNum);

/*************************************************************************
**
** Function    : PC_IStopCAN
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
int PC_IStopCAN (PInterface pInterface, UINT8 CANNum);

/*************************************************************************
**
** Function    : PC_IResetCAN
**
** Description : reset CAN controller
** Parameters  : pInterface  (I/O)  CAN interface pointer
**               CANNum       (IN)  controller number
** Returnvalue : BCI_OK       - OK
**               BCI_BUSY     - command is busy
**               BCI_PARA_ERR - incorrect command
**               BCI_TIMEOUT  - timeout reached
**
*************************************************************************/
int PC_IResetCAN (PInterface pInterface, UINT8 CANNum);

/*************************************************************************
**
** Function: PC_IResetOverrun
**
** Description: reset CAN controller overruns
** Parameters:
**  *Interface  (I/O)  CAN interface handler
**  CANNum       (IN)  controller number
** Returnvalue: BCI_OK       - OK
**              BCI_BUSY     - command is busy
**              BCI_PARA_ERR - incorrect command
**              BCI_TIMEOUT  - timeout reached
**
*************************************************************************/
int PC_IResetOverrun (PInterface pInterface, UINT8 CANNum);

/*************************************************************************
**
** Function    : PC_ITestCAN
**
** Description : test CAN interface.
**               this function sends the command TEST to the board
**               and waits for a response. In the received message,
**               the data bytes have to be inverted.
** Parameters  : pInterface  (I/O)  CAN interface pointer
** Returnvalue : BCI_OK       - OK
**               BCI_BUSY     - command is busy
**               BCI_PARA_ERR - incorrect command
**               BCI_TIMEOUT  - timeout reached
**               BCI_TEST_ERR - testing error
**
*************************************************************************/
int PC_ITestCAN (PInterface pInterface);

/*************************************************************************
**
** Function:
**  PCIInitCAN
**
** Description:
**  init CAN controller
** Parameters:
**  *Interface  (I/O)  CAN interface handler
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
                 UINT8 mode);

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
int PC_IConfigRxQueue (PInterface pInterface, UINT8 CANNum, UINT8 IntMode);

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
                    UINT8 can_num, UINT8 MaskNum,
                    UINT8 type, UINT32 acc_code, UINT32 acc_mask);

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
**               BCI_CMND_ERR  - command buffer is busy
**               BCI_RESP_ERR  - no board response
**               BCI_SERV_ERR  - error in service
**               BCI_PARA_ERR  - Invalid can number
**               BCI_STATE_ERR - BCI is in wrong state
**
*************************************************************************/
int PC_IRegRxID (PInterface pInterface, UINT8 CANNum, UINT8 mff, UINT32 id);

/*************************************************************************
**
** Function    : PC_IUnregisterRxId
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
**               id      (IN) - identifier to be registered
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
int PC_IUnregRxID (PInterface pInterface, UINT8 CANNum, UINT8 mff, UINT32 id);

/*************************************************************************
**
** Function    : PC_IReceiveCANObj
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
int PC_IReceiveCANObj (PInterface pInterface, UINT8 CANNum,
                       BCI_ts_CanMsg * ReceivedCANMsg);

/*************************************************************************
**
** Function    : PC_ITransmitCANObj
**
** Description : transmit CAN object.
**               Try to find free slot in the transmit queue;
**               if there is no free slots, return BCI_BUSY
** Parameters  : pInterface          (I/O)  CAN interface pointer
**               CANNum               (IN)  controller number
**               *ToSendCANMsg     (IN)  received CAN object
** Returnvalue : BCI_OK          - OK
**               BCI_BUSY  - transmit queue is full
**
*************************************************************************/
int PC_ITransmitCANObj (PInterface pInterface, UINT8 CANNum,
                        BCI_ts_CanMsg * ToSendCANMsg);

/*************************************************************************
**
** Function    : PC_IGetBrdStatus
**
** Description : get CAN board status
** Parameters  : pInterface          (I/O)  CAN interface pointer
**               *BrdStatus           (IN)  current CAN board status
** Returnvalue : BCI_OK          - OK
**               BCI_BUSY        - semafor set failed
**
*************************************************************************/
int PC_IGetBrdStatus (PInterface pInterface, BCI_ts_BrdSts * BrdStatus);

/*************************************************************************
**
** Function    : PC_IWaitForData
**
** Description : wait for data. Only for
**               BCI_LATENCY_MODE (Interrupt after every received message) and
**               BCI_THROUGHPUT_MODE (Interrupt when queue is full) modes.
** Parameters  : pInterface  (I/O)  CAN interface pointer
**               CANNum               (IN)  controller number
** Returnvalue : TRUE : - data available
**               FALSE: - no data
**
*************************************************************************/
int PC_IWaitForData (PInterface pInterface, UINT8 CANNum, UINT32 Timeout_msec);

int PC_IGetBoardInfo (PInterface pInterface, BCI_ts_BrdInfo * BrdInfo);

/*************************************************************************
**    Board specific function prototypes
*************************************************************************/

#endif
