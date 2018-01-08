
/*************************************************************************
**    IXXAT Automation GmbH
**************************************************************************
**
**  $Workfile: BCI.H $
**    Summary: headerfile of the BCI (basic CAN interface), a generic
**             CAN/LIN library for IXXAT CAN/LIN boards.
**   $Archive: /database/produkte/treiber/BCI V4/common/src/BCI.H $
**  $Revision: 1.26 $
**      $Date: 2003/07/02 10:39:30 $
**     Author: A.Rothenhaeusler, J.Stolberg
**
**************************************************************************
**************************************************************************
**
**  Functions: BCI_Init
**             BCI_OpenBoard
**             BCI_CloseBoard
**             BCI_ResetCan
**             BCI_StartCan
**             BCI_InitCan
**             BCI_StopCan
**             BCI_SetExtFilterMask
**             BCI_RegFilterListEntry
**             BCI_ConfigRxQueue
**             BCI_TransmitCanMsg
**             BCI_ReceiveCanMsg
**             BCI_GetCanStatus
**             BCI_GetBoardStatus
**             BCI_GetBoardInfo
**             BCI_GetErrorString
**             BCI_LinControlStart
**             BCI_LinControlStop
**             BCI_LinControlReset
**             BCI_LinControlInitialize
**             BCI_LinGetStatus
**             BCI_LinSlaveUpdateTxBuffer
**             BCI_LinMasterSendMsg
**             BCI_LinMasterRequestId
**             BCI_LinRecvMsg
**
**   Compiler: Borland-C 3.1 for DOS
**             GNU-C 2.7 for VxWorks
**             GNU-C 2,3,4 for Linux
**    Remarks:
**
**
**************************************************************************
**    all rights reserved
*************************************************************************/

#ifndef BCI_H
#define BCI_H
#define LINUX
#include "integral.h"

/*************************************************************************
**    constants and macros
*************************************************************************/

/*
** maximum number of supported boards
*/
#define BCI_MAX_BOARDS      4

/*
** maximum number of supported CAN controllers pro board
*/
#define BCI_MAX_CAN_NUM     2

/*
** maximum number of supported LIN controllers pro board
*/
#define BCI_MAX_LIN_NUM     2

/*
** BCI revision
*/
#define BCI_MAJOR_REV 4
#define BCI_MINOR_REV 5
#define BCI_SUB_REV   8

/*
**  some baudrates
*/
#define BCI_10KB            0x67,0x2f
#define BCI_20KB            0x53,0x2f
#define BCI_50KB            0x47,0x2F
#define BCI_100KB           0x43,0x2F
#define BCI_125KB           0x03,0x1C
#define BCI_250KB           0x01,0x1C
#define BCI_500KB           0x00,0x1C
#define BCI_1000KB          0x00,0x14

/*
**  parameter definitions
*/

/* message frame format */
#define BCI_MFF_11_DAT           0          /* standard mode, data frame */
#define BCI_MFF_11_RMT           1          /* standard mode, remote frame */
#define BCI_MFF_29_DAT           2          /* extended mode, data frame */
#define BCI_MFF_29_RMT           3          /* extended mode, remote frame */
#define BCI_MFF_STS_MSG          8          /* CAN status message */
#define BCI_MFF_RMT_MASK         1          /* remote frame mff mask */
#define BCI_MFF_EXT_MASK         2          /* extended mode mff mask */

/* timeout conditions */
#define BCI_NO_WAIT         (0)
#define BCI_WAIT_FOREVER    (-1)

/* acceptance mask type */
#define BCI_11B_MASK            0
#define BCI_29B_MASK            1

/*
** return values:
*/
#define BCI_OK                (1)           /* successfully finished */
#define BCI_BUSY              (0)           /* no access at the moment */
#define BCI_NO                (0)           /* no message available/timeout */
#define BCI_HDL_ERR          (-1)           /* wrong or unknown board handle */
#define BCI_INST_ERR         (-2)           /* install error */
#define BCI_FW_ERR           (-3)           /* download firmware error */
#define BCI_ISR_ERR          (-4)           /* install ISR error */
#define BCI_PARA_ERR         (-5)           /* parameter error */
#define BCI_STATE_ERR        (-6)           /* BCI is in wrong state */
#define BCI_CMND_ERR         (-7)           /* no command request possible */
#define BCI_RESP_ERR         (-8)           /* no board response */
#define BCI_SERV_ERR         (-9)           /* error in service */
#define BCI_USER_ERR         (-10)          /* wrong handling by user */
#define BCI_TIMER_ERR        (-11)          /* wrong timer state */

#ifdef LINUX
#define BCI_TEST_ERR          (-51)         /* BCI firmware test failed */
#define BCI_LOADER_TEST_ERR   (-52)         /* loader test failed */
#define BCI_INT_TEST_ERR      (-53)         /* interrupt to PC test failed */
#define BCI_SEMA_TEST_ERR     (-54)         /* semaphore test failed */
#define BCI_LIST_ERR          (-55)         /* interface list error */
#define BCI_CREATE_ERR        (-56)         /* interface create error */
#define BCI_LIN_NOT_SUPP      (-57)         /* LIN is not supported by this board */
#define BCI_LIN_WRONG_MODE    (-58)         /* LIN op mode is wrong */
#endif /* ifdef LINUX */

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

/*
** defines for the CAN Status
*/
#define BCI_CAN_INIT_MODE     0x0001
#define BCI_CAN_WARN_LEVEL    0x0002
#define BCI_CAN_BUS_OFF       0x0004
#define BCI_CAN_DATA_OVR      0x0008
#define BCI_CAN_RX_STAT       0x0010
#define BCI_CAN_TX_PEND       0x0020
#define BCI_CAN_LOWSPEED_ERR  0x0040
#define BCI_CAN_STT_STAT      0x0080
#define BCI_CAN_QUE_OVR       0x0100
#define BCI_CAN_QUE_EMPTY     0x0200

/*
** Defines for the parameter mode in the function BCI_RegFilterListEntry
*/
#define BCI_ACC_ALL         (~0x01)         /* Accept all CAN identifiers */
#define BCI_REJECT_ALL      (~0x02)         /* Reject all CAN identifiers */
#define BCI_MASK            0x04            /* Accept only specific CAN identifier */

/*
** Defines for parameter mode in the function BCI_ConfigRxQueue
*/
#define BCI_POLL_MODE       0x00            /* No interrupt from MC to PC */
#define BCI_LATENCY_MODE    0x01            /* Interrupt after every received
                                               message */
#define BCI_THROUGHPUT_MODE 0x02            /* Interrupt when queue is full */

/*
  BCI states and transitions:

  - UNCONFIGURED:       The board is unconfigured. No board or
                        CAN/LIN functionality is available.
                        Board firmware is not running.

  - BOARD-INITIALIZED:  The board is running and ready for
                        CAN/LIN controller initialization.
                        The CAN/LIN controller is unconfigured.

  - INITIALIZED:        controller is configured/initialized
                        but not running. No CAN/LIN communication is
                        possible.

  - RUNNING:            CAN/LIN controller is running and able to receive
                        and transmit CAN/LIN messages.

                                BCI_Init |
                                         v
                                  +--------------+
                                  | UNCONFIGURED |
                                  +--------------+
                                      |     ^
                        BCI_OpenBoard |     | BCI_CloseBoard
                                      v     |
                                +-------------------+
                                | BOARD-INITIALIZED |
                                +-------------------+
                                      |     ^
                          BCI_InitCan |     | BCI_ResetCan
                                      v     |
                                 +-----------------+
                                 |   INITIALIZED   |
                                 +-----------------+
                                      |     ^
                         BCI_StartCan |     | BCI_StopCan
                                      v     |
                                  +-------------+
                                  |   RUNNING   |
                                  +-------------+
*/

/*************************************************************************
**    data types
*************************************************************************/
typedef enum
{ BCI_IPCI320_ISA,              /* iPC-I 320 - ISA */
  BCI_IPCI320_PCI,              /* iPC-I 320 - PCI */
  BCI_IPCI165_ISA,              /* iPC-I 165 - ISA */
  BCI_IPCI165_PCI               /* iPC-I 165 - PCI */
}
BCI_t_BrdType;

typedef struct
{
  UINT32 id;                    /* message identifier */
  UINT8 dlc;                    /* Data Length Code (0 - 8) */
  UINT8 mff;                    /* message frame format (see defines) */
  UINT8 a_data[8];              /* CAN data */
  UINT32 time_stamp;            /* receive time stamp */
}
BCI_PACK_STRUCT_ATTR BCI_ts_CanMsg;

typedef struct
{
  UINT16 can0_status;           /* CAN 0 status */
  UINT16 can0_busload;          /* CAN 0 busload */
  UINT16 can1_status;           /* CAN 1 status */
  UINT16 can1_busload;          /* CAN 1 busload */
  UINT16 cpu_load;              /* CPU load */
  UINT16 counter;               /* firmeware live sign counter */
}
BCI_PACK_STRUCT_ATTR BCI_ts_BrdSts;

typedef struct
{
  UINT16 fw_version;            /* firmware version (BCD coded, e.g. 100H = V1.00) */
  UINT16 num_can;               /* number of CAN controllers (1,2) */
  char can_type[2][10];         /* CAN type, e. g. "SJA1000", "82527" */
}
BCI_PACK_STRUCT_ATTR BCI_ts_BrdInfo;

typedef char BCI_t_ErrString[40];
typedef UINT32 BCI_t_BoardType;

/*************************************************************************
**    LIN definitions
*************************************************************************/

/* LIN operation mode, user API view */
#define BCI_LIN_OPMODE_UNKNOWN 0x00         /* Not configured */
#define BCI_LIN_OPMODE_SLAVE   0x01         /* Master op mode */
#define BCI_LIN_OPMODE_MASTER  0x02         /* Slave op mode */
#define BCI_LIN_OPMODE_ERRORS  0x04

#define LIN_MAX_DATA_LEN       8

#define BCI_LIN_BITRATE_MIN    1200         /* Minimal supported LIN bitrate */
#define BCI_LIN_BITRATE_MAX    20000        /* Maximal supported LIN bitrate */
#define BCI_LIN_BITRATE_AUTO   0xffffffff

#define BCI_LIN_WRITE_DIRECT    0x20        /* LIN direct send flag */
#define BCI_LIN_WRITE_TABLE     0x21        /* LIN update buffer flag */

#define BCI_LIN_MSGTYPE_DATA   0            /* Data frame */
#define BCI_LIN_MSGTYPE_INFO   1            /* Info frame (for Start, Stop, Reset) */
#define BCI_LIN_MSGTYPE_ERROR  2            /* Error frame */
#define BCI_LIN_MSGTYPE_WAKEUP 3            /* Wake Up frame */

#define BCI_LIN_INFOTYPE_NONE    0x00       /* No info */
#define BCI_LIN_INFOTYPE_OVR     0x01       /* Overrun */
#define BCI_LIN_INFOTYPE_SELF    0x02       /* Self receiption */
#define BCI_LIN_INFOTYPE_ID_ONLY 0x04       /* Only Id received */

#define BCI_LIN_SEND_NO_DATA     0x0        /* Send no data */
#define BCI_LIN_SEND_DATA        0x1        /* Send LIN message data */

/* LIN checksum model */
#define BCI_LIN_CRC_CLASSIC   0x0           /* Classic message checksum calculation */
#define BCI_LIN_CRC_ENHANCED  0x1           /* Enhanced message checksum calculation */
#define BCI_LIN_CRC_ERROR     0x2           /* Generate a false CRC for testing purpose */

/* LIN operation mode, firmware view */
#define BCI_LIN_FW_OPMODE_MODE_MASTER 0x01
#define BCI_LIN_FW_OPMODE_MODE_SLAVE  0x00

/* LIN interface error codes, used in BCI_ts_LinMsgErr */
#define BCI_LIN_NO_ERROR                          0x00
#define BCI_LIN_BIT_ERROR                         0x01
#define BCI_LIN_CHECKSUM_ERROR                    0x02
#define BCI_LIN_ID_PARITY_ERROR                   0x03
#define BCI_LIN_SLAVE_NOT_RESPONDING_ERROR        0x04
#define BCI_LIN_SYNCH_BREAK_ERROR                 0x05
#define BCI_LIN_INCONSISTENT_SYNCH_FIELD_ERROR    0x06
#define BCI_LIN_MORE_DATA_EXPECTED                0x07
#define BCI_LIN_TIME_OUT_AFTER_START_SYNCH_BREAK  0x08
#define BCI_LIN_TIME_OUT_AFTER_SYNCH_BREAK        0x09
#define BCI_LIN_TIME_OUT_AFTER_SYNCH_FIELD        0x0a
#define BCI_LIN_NOT_CONNECTED                     0x0b
#define BCI_LIN_PARAMETER_ERROR                   0x0c
#define BCI_LIN_BUS_NOT_FREE                      0x0d
#define BCI_LIN_UNKNOWN_ERROR                     0x0e

/* LIN status (used by <BCI_ts_linBrdSts.dwStatus>) */
#define BCI_LIN_STATUS_OVRRUN    0x01       /* data overrun occurred */
#define BCI_LIN_STATUS_ININIT    0x10       /* init mode active */

typedef struct
{
  UINT8 bOpMode;                /* operating mode of the LIN interface, see BCI_LIN_FW_OPMODE_* */
  UINT8 bBusLoad;               /* average busload in procent (0..100) */
  UINT16 wBitrate;              /* bitrate of the LIN interface (1200..20000) */
  UINT32 dwStatus;              /* status of the LIN controller, see BCI_LIN_STATUS_* */
}
BCI_PACK_STRUCT_ATTR BCI_ts_linBrdSts;

/* LIN message header */
typedef struct
{

  UINT32 dwTimeStamp;           /* Message receive timestamp */
  UINT8 bType;                  /* Message type
                                   Bit 0..5: message type
                                   0x00 (data frame)
                                   0x01 (info frame)
                                   0x02 (error frame)
                                   0x03 (wakeup frame)
                                   0x04 (timer) */
  UINT8 bInfo;                  /* Message info
                                   Bit 0    overrun
                                   Bit 1    direction
                                   Bit 2    ID only */
}
BCI_PACK_STRUCT_ATTR BCI_ts_LinMsgHdr;

/* LIN Data message */
typedef struct
{

  UINT8 bId;                          /* Message identifier */
  UINT8 bModel;                       /* Message checksum model */
  UINT8 bCheckSum;                    /* Message checksum */
  UINT8 bSendData;                    /* Data send flag */
  UINT8 bLength;                      /* Message data length */
  UINT8 bRes;                         /* Reserved padding byte */
  UINT8 abDataByte[LIN_MAX_DATA_LEN]; /* Message data */
}
BCI_PACK_STRUCT_ATTR BCI_ts_LinMsgData;

/* LIN Error message */
typedef struct
{
  UINT16 wErrCode;              /* Error code, see LIN interface error codes definitions */
}
BCI_PACK_STRUCT_ATTR BCI_ts_LinMsgErr;

/* LIN status message */
typedef struct
{
  UINT16 wStatus;               /* LIN controller status, see BCI_LIN_STATUS_* definitions */
}
BCI_PACK_STRUCT_ATTR BCI_ts_LinMsgStat;

/* LIN message data part union, based on message type */
typedef union
{
  BCI_ts_LinMsgData Data;
  BCI_ts_LinMsgErr Err;
  BCI_ts_LinMsgStat Stat;
}
BCI_PACK_STRUCT_ATTR BCI_tu_LinMsg;

/* LIN message definition */
typedef struct
{
  BCI_ts_LinMsgHdr sHdr;
  BCI_tu_LinMsg uMsg;
}
BCI_PACK_STRUCT_ATTR BCI_ts_LinMsg;

/*************************************************************************
**    global variables
*************************************************************************/

/*************************************************************************
**    function prototypes
*************************************************************************/

/*************************************************************************
**
** Function    : BCI_Init
**
** Description : Initializes the BCI structures.
**               function must be called first. After that the
**               BCI is in the state UNCONFIGURED.
**               Call of this function clears all already distributed
**               board handles !!!
** Parameters  : -
** Returnvalue : BCI_OK         - OK
**
*************************************************************************/
int BCI_Init (void);

#ifndef LINUX

/*************************************************************************
**
** Function    : BCI_OpenBoard
**
** Description : The function:
**               - resets the board and tests the board communication
**               - loads and starts the firmware
**               - sets the CAN controller into the init mode
**               - delivers a handle for further board usage
**               After that the CAN controller can be initialized.
**               BCI state transition:  UNCONFIGURED -> BOARD-INITIALIZED
**
** Parameters  : p_board_hdl (OUT) - pointer to the distributed board handle
**               type        (IN)  - board type
**               location    (IN)  - ISA board: memory segment (e.g. 0xd800)
**                                   PCI board: board number (e.g. 0)
**               irq_num     (IN)  - board interrupt number (e.g. 5)
**                                   (Only relevant for ISA boards)
**
** Returnvalue : BCI_OK         - OK
**               BCI_PARA_ERR   - wrong or unknown parameter
**               BCI_INST_ERR   - no further board available
**               BCI_RESP_ERR   - no board response
**               BCI_FW_ERR     - firmware download error
**               BCI_ISR_ERR    - ISR installation error
**
*************************************************************************/
int BCI_OpenBoard (BCI_BRD_HDL * p_board_hdl,
                   BCI_t_BoardType type, UINT16 location, UINT8 irq);

#else /* ifndef LINUX */

/*************************************************************************
**
** Function    : BCI_OpenBoard
**
** Description : Variant for operating systems with driver model !!
**               The function:
**               - binds functions
**               - resets the board and tests the board communication
**               - loads and starts the firmware
**               - sets the CAN controller to the init mode
**               - delivers a handle for further board usage
**               After that the CAN controller can be initialized.
**               BCI state transition:  UNCONFIGURED -> BOARD-INITIALIZED
**
** Parameters  : p_board_hdl      (OUT) - pointer to the distributed board handle
**               device_file      (IN) - device file name
**
** Returnvalue : BCI_OK   - OK
**               BCI_ERR  - board open error
**
*************************************************************************/
int BCI_OpenBoard (BCI_BRD_HDL * p_board_hdl, char *device_file);

#endif /* ifndef LINUX */

/*************************************************************************
**
** Function    : BCI_CloseBoard
**
** Description : Resets and releases the board.
**               After that, the firmware and CAN controller is stopped.
**               This function can be called from every BCI state.
**               BCI state transition:  any state -> UNCONFIGURED
**
** Parameters  : board_hdl (IN)  - board handle
**
** Returnvalue : BCI_OK        - OK
**               BCI_HDL_ERR   - wrong or unknown board handle
**
*************************************************************************/
int BCI_CloseBoard (BCI_BRD_HDL board_hdl);

/*************************************************************************
**
** Function    : BCI_ResetCan
**
** Description : Set CAN controller to init mode by means of a hardware
**               reset (hard termination of a possible transmission).
**               All messages within the corresponding receive and
**               transmit queues are flushed.
**               The data overrun or error conditions within the CAN
**               controller are also cleared.
**               The CAN controller as well as the message
**               filter and receive queue must be initialized again.
**               BCI state transition:
**                 CAN-RUNNING/CAN-INITIALIZED -> BOARD-INITIALIZED
**
** Parameters  : board_hdl (IN) - board handle
**               can_num   (IN) - number of the CAN controller (0,1)
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
int BCI_ResetCan (BCI_BRD_HDL board_hdl, UINT8 can_num);

/**********************************************************************
**
** Function    : BCI_StartCan
**
** Description : Starts the CAN controller (set to running mode).
**               For this the corresponding CAN controller, message
**               filter and receive queue must be initialized.
**               All messages within the corresponding receive and
**               transmit queues are flushed.
**               The data overrun conditions within the CAN controller
**               or receive queue are also cleared.
**               The function returns after the bus-off bit is cleared
**               (if set) and the CAN controller is in running mode.
**               After that the CAN controller is able to receive and
**               transmit messages.
**               BCI state transition:  CAN-INITIALIZED -> CAN-RUNNING
**
** Parameters  : board_hdl (IN) - board handle
**               can_num   (IN) - number of the CAN controller (0,1)
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
int BCI_StartCan (BCI_BRD_HDL hdl, UINT8 can_num);

/*************************************************************************
**
** Function    : BCI_InitCan
**
** Description : Initialization of the CAN controller with the specified
**               bit timing parameters.
**               After that, the CAN controller is initialized and
**               further configuration of message filter and receive
**               queue is possible.
**               BCI state transition:  BOARD-INITIALIZED -> CAN-INITIALIZED
**
** Parameters  : board_hdl (IN) - board handle
**               can_num   (IN) - number of the CAN controller (0,1)
**               bt0       (IN) - value for the Bus-Timing-0
**               bt1       (IN) - value for the Bus-Timing-1
**               mode      (IN) - CAN controller communication mode selection
**                                Physical bus transceiver:
**                                b0 = 0 - high speed (ISO/IS 11898-2) transceiver
**                                b0 = 1 - low speed (ISO/IS 11898-3) transceiver
**                                Transmission mode:
**                                b1 = 0 - Standard Communication
**                                b1 = 1 - Single Transmission Try (STT)
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
int BCI_InitCan (BCI_BRD_HDL board_hdl, UINT8 CANNum, UINT8 bt0, UINT8 bt1,
                 UINT8 mode);

/*************************************************************************
**
** Function    : BCI_StopCan
**
** Description : Stops the CAN controller softly (a possible transmission
**               is finished first). The function returns after the CAN
**               has reached the init mode.
**               After that, the CAN controller configuration, received
**               messages as well as further configurations
**               (filter, queue) are still valid.
**               BCI state transition:  CAN-RUNNING -> CAN-INITIALIZED
**
** Parameters  : board_hdl (IN) - board handle
**               can_num   (IN) - number of the CAN controller (0,1)
**
** Returnvalue : BCI_OK        - OK
**               BCI_HDL_ERR   - wrong or unknown board handle
**               BCI_CMND_ERR  - command buffer is busy
**               BCI_RESP_ERR  - no board response
**               BCI_SERV_ERR  - error in service
**               BCI_PARA_ERR  - Invalid CAN number
**               BCI_STATE_ERR - BCI is in wrong state
**
*************************************************************************/
int BCI_StopCan (BCI_BRD_HDL board_hdl, UINT8 can_num);

/*************************************************************************
**
** Function    : BCI_SetAccMask
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
** Parameters  : board_hdl  (IN) - board handle
**               can_num    (IN) - number of the CAN controller (0,1)
**               type       (IN) - type of filter mask (BCI_11B_MASK,
**                                 BCI_29B_MASK)
**               acc_code   (IN) - acceptance code value (left adjusted)
**               acc_mask   (IN) - acceptance mask value (left adjusted)
**                                 Two special masks are defined:
**                                 Accept all identifiers (BCI_ACC_ALL)
**                                 Reject all identifier  (BCI_REJECT_ALL)
**                                 When BCI_ACC_ALL mask is specified,
**                                 the list of registered identifiers is ignored.
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
int BCI_SetAccMask (BCI_BRD_HDL board_hdl,
                    UINT8 can_num, UINT8 type, UINT32 acc_code, UINT32 acc_mask);

/*************************************************************************
**
** Function    : BCI_RegisterRxId
**
** Description : Registers an identifier for message reception.
**               This function must be called for all messages to
**               be received. Up to 4096 identifiers per CAN controller
**               can be registered (11- and 29-bit identifiers in all).
**               The CAN controller must be initialized, that means
**               the BCI must be in the state CAN-INITIALIZED.
**
** Parameters  : board_hdl (IN) - board handle
**               can_num   (IN) - number of the CAN controller (0,1)
**               mff       (IN) - message frame format:
**                                BCI_MFF_11_DAT: data frame with 11 bit identifier
**                                BCI_MFF_11_RMT: remote frame with 11 bit identifier
**                                BCI_MFF_29_DAT: data frame with 29 bit identifier
**                                BCI_MFF_29_RMT: remote frame with 29 bit identifier
**               id        (IN) - identifier to be registered
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
int BCI_RegisterRxId (BCI_BRD_HDL board_hdl, UINT8 can_num, UINT8 mff, UINT32 id);

/*************************************************************************
**
** Function    : BCI_UnregisterRxId
**
** Description : Unregisters an identifier for message reception.
**               The CAN controller must be initialized, that means
**               the BCI must be in the state CAN-INITIALIZED.
**
** Parameters  : board_hdl (IN) - board handle
**               can_num   (IN) - number of the CAN controller (0,1)
**               mff       (IN) - message frame format:
**                                BCI_MFF_11_DAT: data frame with 11 bit identifier
**                                BCI_MFF_11_RMT: remote frame with 11 bit identifier
**                                BCI_MFF_29_DAT: data frame with 29 bit identifier
**                                BCI_MFF_29_RMT: remote frame with 29 bit identifier
**               id        (IN) - identifier to be registered
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
int BCI_UnregisterRxId (BCI_BRD_HDL board_hdl, UINT8 can_num, UINT8 mff, UINT32 id);

/*************************************************************************
**
** Function    : BCI_ConfigRxQueue
**
** Description : Defines the event handling of the receive queues
**               and the behaviour of BCI_ReceiveCanMsg.
**               Depending on the parameter 'mode', the microcontroller
**               generates a receive interrupt:
**               - after every received message (BCI_LATENCY_MODE)
**               - only after several received messages (BCI_THROUGHPUT_MODE)
**               - never (BCI_POLL_MODE)
**               If the mode BCI_THROUGHPUT_MODE is used, the timeout
**               supervision of BCI_ReceiveCanMsg must be activated.
**               Otherwise message starvation in the receive queue cannot
**               be prevented.
**               The CAN controller must be initialized, that means
**               the BCI must be in the state CAN-INITIALIZED.
**
** Parameters  : board_hdl (IN) - board handle
**               can_num   (IN) - number of the CAN controller (0,1)
**               mode      (IN) - event handling mode:
**                              - BCI_LATENCY_MODE:
**                                After every received CAN message an
**                                interrupt/event is generated.
**                                This mode guarantees the best
**                                reactivity time on received messages.
**
**                              - BCI_THROUGHPUT_MODE:
**                                An interrupt/event
**                                is generated only after a predefined number of received
**                                CAN messages.
**                                This mode guarantees the best
**                                data throughput because of the reduced
**                                interrupt load for the PC.
**
**                              - BCI_POLL_MODE:
**                                Use this mode if your OS does not support
**                                interrupt handling or your application
**                                does not use the event handling for
**                                CAN message reception.
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
int BCI_ConfigRxQueue (BCI_BRD_HDL board_hdl, UINT8 can_num, UINT8 mode);

/*************************************************************************
**
** Function    : BCI_TransmitCanMsg
**
** Description : Transmits a CAN message
**               The CAN message is transmitted by means of the structure
**               BCI_ts_CanMsg.
**               The BCI must be in the state CAN-RUNNING.
**
** Parameters  : board_hdl (IN) - board handle
**               can_num   (IN) - number of the CAN controller (0,1)
**               can_msg   (IN) - pointer to the CAN message
**
** Returnvalue : BCI_OK        - OK
**               BCI_HDL_ERR   - wrong or unknown board handle
**               BCI_BUSY      - queue full, repeat later
**               BCI_PARA_ERR  - Invalid CAN number
**               BCI_STATE_ERR - BCI is in wrong state
**
*************************************************************************/
int BCI_TransmitCanMsg (BCI_BRD_HDL board_hdl, UINT8 can_num,
                        BCI_ts_CanMsg * can_msg);

/*************************************************************************
**
** Function    : BCI_ReceiveCanMsg
**
** Description : Reads a received CAN message from the receive queue (DPRAM).
**               The function blocks if no message is available and
**               the parameter timeout is not equal to BCI_NO_WAIT.
**               If the mode BCI_THROUGHPUT_MODE (BCI_ConfigRxQueue) is used,
**               the timeout supervision of BCI_ReceiveCanMsg must be
**               activated. Otherwise message starvation in the receive queue
**               cannot be prevented.
**               If the mode BCI_POLL_MODE (BCI_ConfigRxQueue) is used,
**               the function BCI_ReceiveCanMsg has to be executed
**               cyclically.
**               The BCI must be in the state CAN-RUNNING.
**
** Parameters  : board_hdl (IN)  - board handle
**               can_num   (IN)  - number of the CAN controller (0,1)
**               can_msg   (OUT) - structure, to store the received CAN
**                                 message
**               timeout   (IN)  - >0: timeout in msec if no message is
**                                     available
**                                  0: BCI_NO_WAIT for polling mode
**                                 -1: BCI_WAIT_FOREVER
**
** Returnvalue : BCI_OK        - CAN message received
**               BCI_HDL_ERR   - wrong or unknown board handle
**               BCI_NO        - no CAN message received/timeout
**               BCI_PARA_ERR  - invalid can number
**               BCI_STATE_ERR - BCI is in wrong state
**
*************************************************************************/
int BCI_ReceiveCanMsg (BCI_BRD_HDL board_hdl,
                       UINT8 can_num, BCI_ts_CanMsg * can_msg, int timeout);

/*************************************************************************
**
** Function    : BCI_GetCanStatus
**
** Description : This function reads CAN status register from the DPRAM,
**               which is cyclically updated.
**
** Parameters  : board_hdl (IN)  - board handle
**               can_num   (IN)  - number of the CAN controller (0,1)
**               can_sts   (OUT) - pointer to store the status:
**                               - BCI_CAN_INIT_MODE:
**                                 CAN controller must be in "init mode"
**                                 for configuration.
**                               - BCI_CAN_WARN_LEVEL:
**                                 The CAN controller switches to "warning
**                                 level" if communication problems occur.
**                                 (internal error counter >= 96)
**                                 State is automatically cleared by the
**                                 CAN controller after communication
**                                 recovery.
**                               - BCI_CAN_BUS_OFF:
**                                 The CAN controller switches to "bus off"
**                                 and additionally to "init mode"
**                                 if serious communication problems occur.
**                                 (internal error counter == 255) To restart
**                                 the CAN controller, call BCI_StartCan.
**                                 "bus off" state is then automatically
**                                 cleared by the CAN controller after
**                                 128 bus-idle sequences.
**                               - BCI_CAN_DATA_OVR:
**                                 CAN controller switches to "data overrun",
**                                 if one or more receive messages are lost.
**                                 Communication is still possible there.
**                                 State is only cleared by means of the
**                                 function BCI_StartCan.
**                               - BCI_CAN_RX_STAT:
**                                 Flag signals a incoming/receiving message.
**                               - BCI_CAN_TX_PEND:
**                                 CAN controller is sending a message at the
**                                 moment.
**                               - BCI_CAN_LOWSPEED_ERR:
**                                 status of the physical bus transceiver
**                                 (only available in lowspeed mode)
**                               - BCI_CAN_STT_STAT:
**                                 only applicable if in function BCI_InitCAN() mode
**                                 "Single Transmission Try" was initialized.
**                                 If cleared: message sent, acknowledgement
**                                 received.
**                                 If set: message sent, no acknowledgement received,
**                                 message discarded.
**                               - BCI_CAN_QUE_OVR:
**                                 One or more messages are lost because of an
**                                 overcrowded receive queue.
**                                 State is only cleared by means of the
**                                 function BCI_StartCan.
**                               - BCI_CAN_QUE_EMPTY:
**                                 There is no further message in the receive
**                                 queue.
**
** Returnvalue : BCI_OK       - OK
**               BCI_BUSY     - buffer busy at the moment,
**                              try again !
**               BCI_INST_ERR - board not open
**
*************************************************************************/
int BCI_GetCanStatus (BCI_BRD_HDL board_hdl, UINT8 can_num, UINT16 * can_sts);

/*************************************************************************
**
** Function    : BCI_GetBoardStatus
**
** Description : This function reads board information from the DPRAM,
**               which is updated cyclically. The following information is
**               available: CAN status 0, CAN status 1,
**               CAN Busload 0, CAN Busload 1, microcontroller CPU
**               load, counter.
**
** Parameters  : board_hdl (IN)  - board handle
**               brd_sts   (OUT) - structure to store information
**                                 about the board
**
** Returnvalue : BCI_OK       - OK
**               BCI_BUSY     - buffer busy at the moment,
**                              try again !
**               BCI_INST_ERR - board not open
**
*************************************************************************/
int BCI_GetBoardStatus (BCI_BRD_HDL board_hdl, BCI_ts_BrdSts * brd_sts);

/*************************************************************************
**
** Function    : BCI_GetBoardInfo
**
** Description : This function reads board information from the board.
**               The following information is available:
**               - firmware version
**               - number of available CAN controllers
**               - type of the CAN controllers
**
** Parameters  : board_hdl  (IN)  - board handle
**               brd_info   (OUT) - structure to store information
**                                  about the board
**
** Returnvalue : BCI_OK       - OK
**               BCI_BUSY     - buffer busy at the moment,
**                              try again !
**               BCI_INST_ERR - board not open
**
*************************************************************************/
int BCI_GetBoardInfo (BCI_BRD_HDL board_hdl, BCI_ts_BrdInfo * brd_info);

/*************************************************************************
**
** Function    : BCI_GetErrorString
**
** Description : Function delievers a zero-terminated error string
**               based on the specified error define.
**               The string has a maximum length of 80 characters.
**
** Parameters  : error (IN)  - BCI error define
**
** Returnvalue : pointer to constant error string
**
*************************************************************************/
const char *BCI_GetErrorString (int error);

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
int BCI_LinControlStart (BCI_BRD_HDL board_hdl, UINT8 LINNum);

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
int BCI_LinControlStop (BCI_BRD_HDL board_hdl, UINT8 LINNum);

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
int BCI_LinControlReset (BCI_BRD_HDL board_hdl, UINT8 LINNum);

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
int BCI_LinGetStatus (BCI_BRD_HDL board_hdl, UINT8 LINNum, BCI_ts_linBrdSts * LINSts);

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
                              UINT8 intmode, UINT16 bitrate);

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
                                UINT8 bId, UINT8 bCRCType, UINT8 * pbData, UINT8 bLen);

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
                          UINT8 bId, UINT8 bCRCType, UINT8 * pbData, UINT8 bLen);

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
                            UINT8 bId, UINT8 bCRCType, UINT8 bLen);


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
                    UINT32 dwMsTimeout, BCI_ts_LinMsg * plinMsg);

#endif /* BCI_H */
