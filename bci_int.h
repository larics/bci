/*************************************************************************
**    IXXAT Automation GmbH
**************************************************************************
**
**       File: bci_int.h
**    Summary: BCI library for Linux internal include file
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

#ifndef BCI_INT_H
#define BCI_INT_H
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include "integral.h"
#include "filterlist.h"

/*************************************************************************
**    constants and macros
*************************************************************************/
#define MAX_CONTROLLER_NUMBER       2

#define BCI_iPCIXC161_FW_VERSION_MAJOR 4
#define BCI_iPCIXC161_FW_VERSION_MINOR 25

/*
** maximal number of queues per direction
*/
#define BAP_MAX_PC2MC_QUEUES        2
#define BAP_MAX_MC2PC_QUEUES        2

/*
** Size of a queue-entry in the DPRAM
*/
#define CAN_QUEUE_ENTRY_SIZE  (2+18)        /* 2 bytes reserved for management */
#define LIN_QUEUE_ENTRY_SIZE  (2+20)        /* 2 bytes reserved for management */

/* entries per queue */
#define CAN_NUM_QUEUES_ENTRIES      50      /* Queuesize = 50 * 20 = 7F8H */
#define LIN_NUM_QUEUES_ENTRIES      50      /* Queuesize = 50 * 22 */

/*
** size of the command buffer in the DPRAM
*/
#define BAP_CMND_BUFFER_SIZE  76+4          /* 4 bytes reserved for management */

/*
** size of the immage buffer in the DPRAM
*/
#define CAN_IMAGE_BUF_SIZE       12
#define LIN_IMAGE_BUF_SIZE       8

#define LIN_TX_BUF_IMAGE_ENTRIES 64
#define LIN_STATUS_IMAGE_SEMA    1
#define LIN_TX_BUF_IMAGE0_SEMA   2
#define LIN_TX_BUF_IMAGE1_SEMA   3

/*
** Default extended acceptance mask
*/
#define DEF_EXT_ACC_MASK 0x1FFFFFFF         /* 0001 1111 1111 1111 - 29 bits set */

/*
** Number of supported filter set
*/
#define BCI_FILTER_NUM 2

#define ACCEPTANCE_FILTER 0
#define LIST_REDUCTION_FILTER 1

#define BCI_FILTER_RTR_MASK      0x80000000

#define BCI_SETBIT(n) (1<<n)

// BCI states
#define BCI_UNCONFIGURED      BCI_SETBIT(0)
#define BCI_BOARD_INITIALIZED BCI_SETBIT(1)
#define BCI_INITIALIZED       BCI_SETBIT(2)
#define BCI_RUNNING           BCI_SETBIT(3)

#define STATE_STR_SIZE        128
#define STATE_NUM             4

#define HEX_RET_OK                   1
#define HEX_RET_EOF                  -31
#define HEX_RET_ERR_EOF              -32
#define HEX_RET_ERR_CHECKSUM         -33
#define HEX_RET_WRONG_LEN            -34
#define HEX_RET_SYNTAX_ERR           -35
#define HEXFILE_FILE_OPEN_ERR        -36

//
// Firmware loading mode
//
#define FW_DO_NOT_LOAD         1            // Don't load
#define FW_LOAD_DEFAULT        2            // Load default firmware from C array
#define FW_LOAD_DEFAULT_HEX    3            // Load from default HEX file
#define FW_LOAD_FROM_HEX       4            // Load from specified HEX file

/*
** Controller mode
*/
#define BCI_11BIT_FF  0
#define BCI_29BIT_FF  1

#define BCI_DAT_FF  0
#define BCI_RTR_FF  1

#define ERROR_STRING_LEN 80


/*
** Read LowByte of a 16 Bit integer variable
*/

#ifndef  LOW8
#define  LOW8(wert)  (*( (UINT8 *) &(wert)))
#endif

/*
** Read HighByte of a 16 Bit integer variable
*/
#ifndef  HIGH8
#define  HIGH8(wert)  (*(((UINT8 *) &(wert)) + 1))
#endif

/*
** Read LowWord of a 32 Bit integer variable
*/
#ifndef  LOW16
#define  LOW16(wert)  (*( (UINT16 *) &(wert)))
#endif

/*
** Read HighWord of a 32 Bit integer variable
*/
#ifndef  HIGH16
#define  HIGH16(wert)  (*(((UINT16 *) &(wert)) + 1))
#endif

/*
** Swap LowByte and HighByteof a 16 Bit integer variable
** temp = High; High = Low; Low = temp
*/
#ifndef SWAP16
#define SWAP16(wert)       { UINT8 temp; temp = HIGH8(wert); HIGH8(wert) = (UINT8) wert; LOW8(wert) = temp; }
#endif

/*
** Swap LowWord and HighWord of a 32 Bit integer variable
** temp = HighWord; HighWord = LowWord; LowWord = temp
*/
#ifndef SWAP32
#define SWAP32(wert)       { UINT16 temp; temp = HIGH16(wert); HIGH16(wert) = (UINT16) wert; LOW16(wert) = temp; }
#endif

/*************************************************************************
**    data types
*************************************************************************/

typedef struct
{
  UINT32 Mask;
  UINT32 Code;
}
BCI_PACK_STRUCT_ATTR ts_AccMask;

/*
** struture for the BCI firmware
*/
typedef struct
{
  UINT8 len;                    /* number of data bytes */
  UINT32 addr;                  /* data destination address */
  UINT8 a_data[16];             /* data bytes */
}
BCI_PACK_STRUCT_ATTR BAP_ts_Firmware;

extern BAP_ts_Firmware BAP_a_Fw320[];
extern BAP_ts_Firmware BAP_a_Fw165[];
extern BAP_ts_Firmware BAP_a_Fw161XC[];

#define FAR

typedef struct tag_t_Interface *PInterface;

typedef struct
{
  int (*OpenInterface) (PInterface);
  int (*CloseInterface) (PInterface);
  int (*Reset) (PInterface, UINT8);
  int (*Start) (PInterface, UINT8);
  int (*Stop) (PInterface, UINT8);
  int (*GetBrdStatus) (PInterface, BCI_ts_linBrdSts *);
  int (*Init) (PInterface, UINT8, UINT8, UINT8, UINT16);
  int (*WriteMsg) (PInterface, UINT8, UINT8, BCI_ts_LinMsg *);
  int (*ReadMsg) (PInterface, UINT8, UINT32, BCI_ts_LinMsg *);

  int (*GetBoardInfo) (PInterface, BCI_ts_BrdInfo *);

  int (*SetState) (PInterface, UINT8, UINT8);
  int (*CheckState) (PInterface, UINT8, UINT8);
  UINT16 BCIState[BCI_MAX_LIN_NUM];
  UINT16 OpMode[BCI_MAX_LIN_NUM];

}
BCI_PACK_STRUCT_ATTR LINInterface;

typedef struct
{
  int (*OpenInterface) (PInterface);
  int (*CloseInterface) (PInterface);
  int (*Reset) (PInterface, UINT8);
  int (*Test) (PInterface);
  int (*Start) (PInterface, UINT8);
  int (*Stop) (PInterface, UINT8);
  int (*Init) (PInterface, UINT8, UINT8, UINT8, UINT8);
  int (*ConfigRxQueue) (PInterface, UINT8, UINT8);
  int (*SetAccMask) (PInterface, UINT8, UINT8, UINT8, UINT32, UINT32);
  int (*RegRxID) (PInterface, UINT8, UINT8, UINT32);
  int (*UnregRxID) (PInterface, UINT8, UINT8, UINT32);
  int (*TransmitCANObj) (PInterface, UINT8, BCI_ts_CanMsg *);
  int (*ReceiveCANObj) (PInterface, UINT8, BCI_ts_CanMsg *);
  int (*GetBrdStatus) (PInterface, BCI_ts_BrdSts *);
  int (*DownloadFirmware) (PInterface, UINT8, UINT8 *);

  int (*GetBoardInfo) (PInterface, BCI_ts_BrdInfo *);
  int (*ResetOverrun) (PInterface, UINT8);

  int (*SetState) (PInterface, UINT8, UINT8);
  int (*CheckState) (PInterface, UINT8, UINT8);
  UINT16 BCIState[BCI_MAX_CAN_NUM];

  char FirmwareFile[64];
  FilterList_t FilterListStd[2];
  FilterList_t FilterListExt[2];
  ts_AccMask StdMask[2][BCI_FILTER_NUM];
  ts_AccMask ExtMask[2][BCI_FILTER_NUM];
 
  BAP_ts_Firmware *FWArray;

}
BCI_PACK_STRUCT_ATTR CANInterface;

typedef struct tag_t_Interface
{
  char DeviceFile[2][64];
  int DeviceFileDesc[2];

  int InterfaceNumber;
  int Type;
  int BoardDPRAMSize;
  int BoardMemSize;
  int SemaNum;
  int SemaSize;
  
  char Name[64];
  void *MemoryPtr;
  void *QueuesPtr;
  void *OffsetsPtr;

  CANInterface CAN;
  LINInterface LIN;
  


  int (*StartFirmware) (PInterface);
  int (*GenerateInterrupt) (PInterface);
  int (*WaitForData) (PInterface, UINT8, UINT32);

  struct tag_t_Interface *Next;

}
BCI_PACK_STRUCT_ATTR t_Interface;

typedef struct
{
  UINT8 CmndBuffer[BAP_CMND_BUFFER_SIZE];
  UINT8 CAN_TxQueue0[CAN_NUM_QUEUES_ENTRIES][CAN_QUEUE_ENTRY_SIZE];
  UINT8 CAN_TxQueue1[CAN_NUM_QUEUES_ENTRIES][CAN_QUEUE_ENTRY_SIZE];
  UINT8 CAN_RxQueue0[CAN_NUM_QUEUES_ENTRIES][CAN_QUEUE_ENTRY_SIZE];
  UINT8 CAN_RxQueue1[CAN_NUM_QUEUES_ENTRIES][CAN_QUEUE_ENTRY_SIZE];
  UINT8 CAN_ImageBuffer[CAN_IMAGE_BUF_SIZE];
  UINT16 wRes0;
  UINT16 wCounter;
  UINT8 LIN_TxQueue0[LIN_NUM_QUEUES_ENTRIES][LIN_QUEUE_ENTRY_SIZE];
  UINT8 LIN_RxQueue0[LIN_NUM_QUEUES_ENTRIES][LIN_QUEUE_ENTRY_SIZE];
  UINT8 LIN_ImageBuffer[LIN_IMAGE_BUF_SIZE];
  UINT8 abRes1[8];
  BCI_ts_LinMsgData LIN_TxBufImage0[LIN_TX_BUF_IMAGE_ENTRIES];
  BCI_ts_LinMsgData LIN_TxBufImage1[LIN_TX_BUF_IMAGE_ENTRIES];  
}
BCI_PACK_STRUCT_ATTR DPRAM;

typedef struct
{
  UINT8 CAN_TxQueue0;
  UINT8 CAN_RxQueue0;
  UINT8 CAN_TxQueue1;
  UINT8 CAN_RxQueue1;
  UINT8 LIN_TxQueue0;
  UINT8 LIN_RxQueue0;
}
BCI_PACK_STRUCT_ATTR Queues;

/*
**  Structures for Receive/Transmit Queue (SJA1000)
*/

typedef union
{
  struct
  {
    /* 11 Bit CAN-Identifier in */
    UINT16 ID;                  /* Motorola-Format (leftjustified) */
    UINT8 Data[8];              /* Databytes of the message */
  }
  Std;
  struct
  {
    /* 29 Bit CAN-Identifier in */
    UINT32 ID;                  /* Motorola-Format (leftjustified) */
    UINT8 Data[8];              /* Databytes of the message */
  }
  Ext;
}
BCI_PACK_STRUCT_ATTR SjaMsg;

typedef struct
{
  /* Time in timer-ticks, the message */
  /* has been received */
  UINT32 TimeStamp;
  UINT8 MsgType;                /* message type */
  UINT8 DLC:4;                  /* data length code (Bit 0..3) */
  UINT8 Reserved2:2;            /* reserved (Bit 4,5) */
  UINT8 RTR:1;                  /* remote frame indication (Bit 6) */
  UINT8 FrameFormat:1;          /* frame format (Bit 7) */
  SjaMsg Message;               /* CAN message */
}
BCI_PACK_STRUCT_ATTR RawCANMsg;

/* MsgType, former dummy */
#define BCI_MSGTYPE_DATA        0
#define BCI_MSGTYPE_STATUS      1

#ifdef BCI_TEST
extern FILE *trace_fd;

#define TEST_MSG_LEN 1024
extern char TestMsg[TEST_MSG_LEN];
extern char TestMsgFull[TEST_MSG_LEN];
int DecodeCommand (char *Command);
int SetTestLogFile (FILE * TestFile);
#endif

/* The following macro and corresponding calls are used to generate a type
   length check for all IXX types. For each type an array is defined with
   length of 1, if the size equals the IXXAT definition. Otherwise the array
   is defined with length -1. This leads directly into a compiler error and
   indicates, that the standard types do not match our specification. DO NOT
   EDIT this file for a project but INFORM the IXX developer about that
   issue. */

#ifndef IXX_TASSERT
  #define IXX_TASSERT(exp, name) typedef int dummy_##name [(exp) ? 1 : -1];
#endif

IXX_TASSERT(sizeof(UINT8) == 1, ixx_u8_1_byte)
IXX_TASSERT(sizeof(UINT16) == 2, ixx_u16_2_byte)
IXX_TASSERT(sizeof(UINT16) == 2, ixx_i16_2_byte)
IXX_TASSERT(sizeof(UINT32) == 4, ixx_u32_4_byte)
IXX_TASSERT(sizeof(UINT32) == 4, ixx_i32_4_byte)
IXX_TASSERT(sizeof(BCI_ts_BrdSts)    == CAN_IMAGE_BUF_SIZE, ixx_ts_canBrdSts)
IXX_TASSERT(sizeof(BCI_ts_linBrdSts) == LIN_IMAGE_BUF_SIZE, ixx_ts_linBrdSts)
IXX_TASSERT(sizeof(RawCANMsg)        == (CAN_QUEUE_ENTRY_SIZE-2), ixx_ts_canMsg)
IXX_TASSERT(sizeof(BCI_ts_LinMsg)    == (LIN_QUEUE_ENTRY_SIZE-2), ixx_ts_linMsg)  

/*************************************************************************
**
** Function    : State2Str
**
** Description : Describes BCI state 
**
** Parameters  : State                (IN)  BCI state 
**               StateStr             (OUT)  BCI state string representation

** Returnvalue : BCI_OK
**               
**
*************************************************************************/
int State2Str (int State, char *StateStr);

/*************************************************************************
**
** Function    : Timeout
**
** Description : windup timer (if msec > 0) and otherwise
**               return if timeout reached
** Parameters  : msecs (IN)
** Returnvalue : TRUE :
**               FALSE:
**
*************************************************************************/
UINT8 Timeout (unsigned int msecs);

/*************************************************************************

  Function:
   msec2timespec

  Description:
   Convert milliseconds to timespec 

  Parameters:
   msec (IN)

  Returnvalue:
   none
   
*************************************************************************/
void msec2timespec (UINT32 msec, struct timespec *conv_time);

/*************************************************************************

  Function:
   msec2timeval

  Description:
   Convert milliseconds to timeval format

  Parameters:
   msec (IN)
   conv_time (OUT) (out)

  Returnvalue:
   none
   
*************************************************************************/
void msec2timeval (UINT32 msec, struct timeval *conv_time);

void BCI_MDelay (unsigned int msec);

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
int BCI_LinControlWriteMsg (BCI_BRD_HDL board_hdl, UINT8 LINNum,
                            UINT8 fSend, BCI_ts_LinMsg * plinMsg);

/*************************************************************************

  Function:
   GetDeviceID

  Description:
   Gets device ID string

  Parameters:
   *InterfaceName (OUT) (out) device ID string
   *DeviceName    (IN)  (in) device file name

  Returnvalue:
    BCI_OK
    BCI_BOARD_ERR  - device file not found

*************************************************************************/
int GetDeviceID (int *ID, char *DeviceName);

#endif
