
/*************************************************************************
**    IXXAT Automation GmbH
**************************************************************************
**
**       File: ipci.h
**    Summary: Internal header file for iPCI boards support functions
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

#ifndef IPCI_H
#define IPCI_H

/*************************************************************************
**    constants and macros
*************************************************************************/
#define RESPONSE_TIMEOUT  3000              /* Wait for response to command (msec) */

#define MEMSET memset
#define MEMCPY memcpy
#define MEMCMP memcmp

#define DPRAM_OFFSET(INTERFACE,OFFSET) (INTERFACE->MemoryPtr+((Offsets*)(INTERFACE)->OffsetsPtr)->OFFSET)

#define SETBYTE(INTERFACE,OFFSET,VAL) MEMSET(INTERFACE->MemoryPtr+((Offsets*)(INTERFACE)->OffsetsPtr)->OFFSET,(VAL),1)

#define SETBYTE_OFFSET(INTERFACE,OFFSET,VAL) MEMSET((INTERFACE->MemoryPtr+OFFSET),(VAL),1)

#define GETBYTE(INTERFACE,OFFSET) (* (char *)(INTERFACE->MemoryPtr+((Offsets*)(INTERFACE)->OffsetsPtr)->OFFSET))

/*
** Defines for the Buffer commands
*/
#define CAN_CMD_ID                   1      /* Get identification string */
#define CAN_CMD_VERSION              2      /* Get version number string */
#define CAN_CMD_TEST                 3      /* Test the command buffer, invert data bytes */
#define CAN_CMD_INIT                 4      /* Initialization of the CAN controller */

#define CAN_CMD_START                6      /* Start the CAN controller */
#define CAN_CMD_STOP                 7      /* Stop the CAN controller */
#define CAN_CMD_RESET                8      /* Reset the CAN controller */

#define CAN_CMD_CONFIG_RX_QUEUE      11     /* Config receive queue mode */
#define CAN_CMD_GET_BOARD_INFO       12     /* Get board information */
#define CAN_CMD_START_TIMER          13     /* Start cyclic timer */
#define CAN_CMD_STOP_TIMER           14     /* Stop cyclic timer */
#define CAN_CMD_SET_ACC_MASK         15     /* Set acceptance mask */

//
//  Some definitions for HEX file processing
//

#define HEXFILE_MAX_LINE_LENGTH      256

#define HEX_DATA                     0
#define HEX_EOF                      1
#define HEX_8086_SEGMENT_ADDRESS     2
#define HEX_EXTENDED_LINEAR_ADDRESS  4

/*************************************************************************
**    data types
*************************************************************************/
typedef struct
{
  UINT16 LD_SYNC;
  UINT16 LD_CMND;
  UINT16 LD_NUM;
  UINT16 LD_ADDRESS;
  UINT16 LD_DATA;
  UINT16 BCI_SYNC;
  UINT16 BCI_NUM;
  UINT16 BCI_DATA;
  UINT16 MODE;
  UINT16 SEMA_0;
  UINT16 SEMA_1;
  UINT16 SEMA_2;
  UINT16 SEMA_3;  
  UINT16 RESET;
  UINT16 INTERRUPT;
}
BCI_PACK_STRUCT_ATTR Offsets;

/*************************************************************************
**    global variables
*************************************************************************/

/*************************************************************************
**    function prototypes
*************************************************************************/

/*************************************************************************
**
** Function    : iPCIOpen
**
** Description : Open device file, map DPRAM, allocate dynamical memory for queues,
**               initialize queues counter and service offsets
** Parameters  : pInterface (I/O)  CAN interface pointer
** Returnvalue : BCI_OK
**               BCI_BOARD_ERR  - device file not found
**
*************************************************************************/
int iPCIOpen (PInterface pInterface);

/*************************************************************************
**
** Function    : iPCIClose
**
** Description : Close device file, unmap DPRAM, free queues
**               dynamical memory
** Parameters  : pInterface   (I/O)  CAN interface pointer
** Returnvalue : BCI_OK          - OK
**
*************************************************************************/
int iPCIClose (PInterface pInterface);

/*************************************************************************
**
** Function    : iPCIGetIdentificationString
**
** Description : get firmware identification string
** Parameters  : pInterface  (I/O)  CAN interface pointer
**               *ID         (OUT) (out) - string to store the firmware
**                              identification, maximal string length
**                              is 75 bytes
** Returnvalue : BCI_OK       - OK
**               BCI_BUSY     - command is busy
**               BCI_PARA_ERR - incorrect command
**               BCI_TIMEOUT  - timeout reached
**
*************************************************************************/
int iPCIGetIdentificationString (PInterface pInterface, char *ID);

/*************************************************************************
**
** Function    : iPCIGetVersionNumberString
**
** Description : get firmware version number string
** Parameters  :   pInterface  (I/O)  CAN interface pointer
**                 *Version    (OUT) (out) - string to store the firmware
**                                version number, maximal string length
**                                is 75 bytes
** Returnvalue : BCI_OK       - OK
**               BCI_BUSY     - command is busy
**               BCI_PARA_ERR - incorrect command
**               BCI_TIMEOUT  - timeout reached
**
*************************************************************************/
int iPCIGetVersionNumberString (PInterface pInterface, char *Version);

/*************************************************************************
**
** Function    : iPCIStartCAN
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
int iPCIStartCAN (PInterface pInterface, UINT8 CANNum);

/*************************************************************************
**
** Function    : iPCIStopCAN
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
int iPCIStopCAN (PInterface pInterface, UINT8 CANNum);

/*************************************************************************
**
** Function    : iPCIResetCAN
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
int iPCIResetCAN (PInterface pInterface, UINT8 CANNum);

/*************************************************************************
**
** Function    : iPCIResetOverrun
**
** Description : reset CAN controller overruns
** Parameters  : pInterface  (I/O)  CAN interface pointer
**               CANNum       (IN)  controller number
** Returnvalue : BCI_OK       - OK
**               BCI_BUSY     - command is busy
**               BCI_PARA_ERR - incorrect command
**               BCI_TIMEOUT  - timeout reached
**
*************************************************************************/
int iPCIResetOverrun (PInterface pInterface, UINT8 CANNum);

/*************************************************************************
**
** Function    : iPCITestCAN
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
int iPCITestCAN (PInterface pInterface);

/*************************************************************************
**
** Function:
**  iPCIInitCAN
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
int iPCIInitCAN (PInterface pInterface, UINT8 CANNum, UINT8 bt0, UINT8 bt1,
                 UINT8 mode);

/*************************************************************************
**
** Function    : iPCIConfigRxQueue
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
**                                iPCIReceiveCanObj() to get the
**                                received CAN messages. Note, that
**                                you must call the function
**                                iPCIReceiveCanObj() as long as
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
**                                iPCIReceiveCanObj() to get the
**                                received CAN messages. Note, that
**                                you must call the function
**                                iPCIReceiveCanObj() as long as
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
int iPCIConfigRxQueue (PInterface pInterface, UINT8 CANNum, UINT8 IntMode);

/*************************************************************************
**
** Function    : iPCIRegRxId
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
int iPCIRegRxID (PInterface pInterface, UINT8 CANNum, UINT8 mff, UINT32 id);

/*************************************************************************
**
** Function    : iPCIUnregRxID
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

int iPCIUnregRxID (PInterface pInterface, UINT8 CANNum, UINT8 mff, UINT32 id);

/*************************************************************************
**
** Function    : iPCISetAccMask
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
** Parameters  : Interface  (IN) - board handle
**               CANNum   (IN) - number of the CAN controller (0,1)
**               MaskNum  (IN) - mask/code pair number  
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
int iPCISetAccMask (PInterface pInterface, UINT8 CANNum, UINT8 MaskNum,
                    UINT8 type, UINT32 acc_code, UINT32 acc_mask);

/*************************************************************************
**
** Function    : iPCIReceiveCANObj
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
int iPCIReceiveCANObj (PInterface pInterface, UINT8 CANNum,
                       BCI_ts_CanMsg * ReceivedCANMsg);

/*************************************************************************
**
** Function    : iPCITransmitCANObj
**
** Description : transmit CAN object.
**               Try to find free slot in the transmit queue;
**               if there is no free slots, return BCI_BUSY
** Parameters  : pInterface          (I/O)  CAN interface pointer
**               CANNum              (IN)  controller number
**               *ToSendCANMsg       (IN)  received CAN object to transmit
** Returnvalue : BCI_OK          - OK
**               BCI_BUSY  - transmit queue is full
**
*************************************************************************/
int iPCITransmitCANObj (PInterface pInterface, UINT8 CANNum,
                        BCI_ts_CanMsg * ToSendCANMsg);

/*************************************************************************
**
** Function    : iPCIGetBrdStatus
**
** Description : get CAN board status
** Parameters  : pInterface          (I/O)  CAN interface pointer
**               *BrdStatus           (IN)  current CAN board status
** Returnvalue : BCI_OK          - OK
**               BCI_BUSY        - semafor set failed
**
*************************************************************************/
int iPCIGetBrdStatus (PInterface pInterface, BCI_ts_BrdSts * BrdStatus);

/*************************************************************************
**
** Function    : iPCIWaitForData
**
** Description : wait for data. Only for
**               BCI_LATENCY_MODE (Interrupt after every received message) and
**               BCI_THROUGHPUT_MODE (Interrupt when queue is full) modes.
** Parameters  : pInterface  (I/O)  CAN interface pointer
** Returnvalue : TRUE : - data available
**               FALSE: - no data
**
*************************************************************************/
int iPCIWaitForData (PInterface pInterface, UINT8 CANNum, UINT32 Timeout_msec);

/*************************************************************************
**
** Function    : iPCIDownloadFirmware
**
** Description : load firmware code into microcontroller
** Parameters  : pInterface  (I/O)  CAN interface pointer
** Returnvalue : BCI_OK                - OK
**               BCI_TIMEOUT           - timeout reached
**               BCI_FW_ERROR - firmware load error
**
*************************************************************************/
int iPCIDownloadFirmware (PInterface pInterface, UINT8 FirmwareMode,
                          UINT8 * HEXFileName);

/*************************************************************************
**
** Function    : GetCodeFromHEXFile
**
** Description : get code string for loading to MC
** Parameters  : pInterface  (I/O)  CAN interface pointer
**               *Code       (OUT)   constructed string, ready for loader processing
** Returnvalue : HEX_RET_OK            - OK
**               HEX_RET_EOF           - EOF reached
**               HEX_RET_ERR_EOF       - EOF not expected
**               HEX_RET_ERR_CHECKSUM  - checksum error
**               HEX_RET_WRONG_LEN     - wrong code line length
**               HEX_RET_SYNTAX_ERR    - syntax error
**               HEXFILE_FILE_OPEN_ERR - file open error
**
*************************************************************************/
int GetCodeFromHEXFile (FILE * fp, char *Code);

/*************************************************************************
**
** Function    : iPCIGetResponse
**
** Description : get response from command buffer
** Parameters  : pInterface  (I/O)  CAN interface pointer
**               *Answer     (OUT)  pointer to the answer string
**               *AnswerSize (OUT)  size of answer
** Returnvalue : BCI_OK       - OK
**               BCI_BUSY     - command is busy
**               BCI_PARA_ERR - incorrect command
**               BCI_TIMEOUT  - timeout reached
**
*************************************************************************/
int iPCIGetResponse (PInterface pInterface, UINT8 * Answer, INT32 * AnswerSize);

/*************************************************************************
**    Board specific function prototypes
*************************************************************************/

/*************************************************************************
**
** Function    : iPCIStartFirmware_I165
**
** Description : start firmware for I165 board type
**
** Parameters  : pInterface   (I/O)  CAN interface pointer
** Returnvalue : BCI_OK          - OK
**
*************************************************************************/
int iPCIStartFirmware_I165 (PInterface pInterface);

/*************************************************************************
**
** Function    : iPCIStartFirmware_IXC161
**
** Description : start firmware for IXC161 board type
**
** Parameters  : pInterface   (I/O)  CAN interface pointer
** Returnvalue : BCI_OK          - OK
**
*************************************************************************/
int iPCIStartFirmware_IXC161 (PInterface pInterface);

/*************************************************************************
**
** Function    : iPCIStartFirmware_I320
**
** Description : start firmware for I320 board type
**
** Parameters  : pInterface   (I/O)  CAN interface pointer
** Returnvalue : BCI_OK          - OK
**
*************************************************************************/
int iPCIStartFirmware_I320 (PInterface pInterface);

/*************************************************************************
**
** Function    : iPCIGenerateInterruptGeneric
**
** Description : generate interrupt for I165ISA, I320ISA, I320PCI boards
**
** Parameters  : pInterface   (I/O)  CAN interface pointer
** Returnvalue : BCI_OK          - OK
**
*************************************************************************/
int iPCIGenerateInterruptGeneric (PInterface pInterface);

/*************************************************************************
**
** Function    : iPCIGenerateInterrupt165PCI
**
** Description : generate interrupt for I165PCI board
**
** Parameters  : pInterface   (I/O)  CAN interface pointer
** Returnvalue : BCI_OK          - OK
**
*************************************************************************/
int iPCIGenerateInterrupt165PCI (PInterface pInterface);

/*************************************************************************
**
** Function    : iPCIGenerateInterruptXC161PCI
**
** Description : generate interrupt for IXC161PCI board
**
** Parameters  : pInterface   (I/O)  CAN interface pointer
** Returnvalue : BCI_OK          - OK
**
*************************************************************************/
int iPCIGenerateInterruptXC161PCI (PInterface pInterface);

/*************************************************************************
**     functions for driver/dpram access
*************************************************************************/

/*************************************************************************
**
** Function    : PushCmdData
**
** Description : constructs command string to microcontroller
** Parameters  : **Cmd  (I/O) pointer to command string
**               *Data  (IN)  pointer to current part
**               Num    (IN)  length of current part
** Returnvalue : number of bytes, added to command string
**
*************************************************************************/
int PushCmdData (char **Cmd, char *Data, int Num);

int iPCIStartTimer (PInterface pInterface, UINT16 ReloadValue);
int iPCIStopTimer (PInterface pInterface);
int iPCIGetBoardInfo (PInterface pInterface, BCI_ts_BrdInfo * BrdInfo);

int CalculateAccMask (PInterface pInterface, UINT8 can_num, UINT32 id, UINT8 mode);

int iPCIDownloadFirmwareFromFile (PInterface pInterface,
                                  UINT8 FirmwareMode, UINT8 * HEXFileName);
int iPCIDownloadFirmwareFromCArray (PInterface pInterface,
                                    BAP_ts_Firmware * FWArray);

/*************************************************************************
**
** Function: GenId11
**
** Description: Converts a numeric value in a 11 bit CAN identifier
**
** Parameters:
**  value (IN) - Numeric value
**
** Returnvalue: generated CAN identifier, in Motorola-Format and
**              leftjustified
**
*************************************************************************/
UINT32 GenId11 (UINT32 value);

/*************************************************************************
**
** Function: GenId29
**
** Description: Converts a numeric value in a 29 bit CAN identifier
**
** Parameters:
**  value (IN) - Numeric value
**
** Returnvalue: generated CAN identifier, in Motorola-Format and
**              leftjustified
**
*************************************************************************/
UINT32 GenId29 (UINT32 value);

/*************************************************************************
**
** Function:  GetId11
**
** Description: Converts a 11 bit CAN identifier in a numeric value
**
** Parameters:
**  id (IN) - CAN identifier, in Motorola-Format and
**               leftjustified
**
** Returnvalue: generated numeric value
**
*************************************************************************/
UINT32 GetId11 (UINT32 id);

/*************************************************************************
**
** Function:  GetId29
**
** Description: Converts a 29 bit CAN identifier in a numeric value
**
** Parameters:
**  id (IN) - CAN identifier, in Motorola-Format and
**            leftjustified
**
** Returnvalue:  generated numeric value
**
*************************************************************************/
UINT32 GetId29 (UINT32 id);

/*************************************************************************

 Function:
  iPCISendCommand

 Description:
  Send a command to the board via command buffer.
  after execution, the user have to call GetResponse,
  to clear the command buffer.

 Parameters  :
  *pInterface  (I/O)  CAN interface handler
  *Command     (IN) command string
   CommandSize (IN) size of command

 Returnvalues: BCI_OK       - OK
               BCI_BUSY     - command is busy
               BCI_PARA_ERR - incorrect command

*************************************************************************/
int iPCISendCommand (PInterface pInterface, UINT8 * Command, int CommandSize);

#endif
