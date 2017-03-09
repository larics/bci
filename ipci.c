/*************************************************************************
**    IXXAT Automation GmbH
**************************************************************************
**
**  $Workfile: $
**    Summary: Active cards BCI support 
**   $Archive: $
**  $Revision: 1.44 $
**      $Date: 2004/02/03 12:33:06 $
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
#include "dpram.h"
#include "can.h"
#include "can_ioctl.h"
#include "ipci.h"
#include "ipci_lin.h"
#include "filterlist.h"

/* }}} */

/* {{{ Definitions */

static UINT8 Command[BAP_CMND_BUFFER_SIZE], *CommandPtr;
static UINT8 Answer[BAP_CMND_BUFFER_SIZE];
static UINT8 CommandSize = 0;
static INT32 AnswerSize;
static UINT8 CommandCode;

char *GetCmdDescr (UINT8 * Cmd);

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

static int iPCI_Trace = 0;
static char iPCI_Trace_Buf[1024 * 8];
static FILE *iPCI_Trace_fd = NULL;
static char iPCI_Trace_fname[] = "iPCItrace.log";

/* }}} */



/* {{{ iPCIOpen */

/*************************************************************************
**
** Function    : iPCIOpen
**
** Description : Open device file, map DPRAM, allocate dynamical memory for queues,
**               initialize queues counter and service offsets
** Parameters  : pInterface (I/O)  CAN interface pointer
** Returnvalue : BCI_OK
**               BCI_SERV_ERR  - device file not found
**
*************************************************************************/
int iPCIOpen (PInterface pInterface)
{

  if (iPCI_Trace > 0)
  {
    switch (iPCI_Trace)
    {
      case 1:
        iPCI_Trace_fd = stdout;
        break;
      case 2:
        if ((iPCI_Trace_fd = fopen (iPCI_Trace_fname, "wb+")) == NULL)
        {
          iPCI_Trace_fd = stdout;
          PRINTD ("Cannot open the iPCI trace file %s\n", iPCI_Trace_fname);
          break;
        }
        PRINTD ("iPCI trace file %s opened\n", iPCI_Trace_fname);
        break;
      default:
        PRINTD ("Unknown iPCI trace level %d\n", iPCI_Trace);
    }
  }

  if ((pInterface->DeviceFileDesc[0] =
       open (pInterface->DeviceFile[0], O_RDWR)) == -1)
  {
    PRINTD ("Device [%s] not found !\n", pInterface->DeviceFile[0]);
    return (BCI_SERV_ERR);
  }

  BCI_MDelay (100);

  pInterface->MemoryPtr = (DPRAM *) GetDPRAMPtr (pInterface);
  pInterface->QueuesPtr = (Queues *) malloc (sizeof (Queues));
  pInterface->OffsetsPtr = (Offsets *) malloc (sizeof (Offsets));

  PRINTD ("Memory alloc: DPRAM %p Queues %p Offsets %p\n",
          pInterface->MemoryPtr, pInterface->QueuesPtr, pInterface->OffsetsPtr);

  // init queues
  ((Queues *) pInterface->QueuesPtr)->CAN_TxQueue0 = 0;
  ((Queues *) pInterface->QueuesPtr)->CAN_RxQueue0 = 0;
  ((Queues *) pInterface->QueuesPtr)->CAN_TxQueue1 = 0;
  ((Queues *) pInterface->QueuesPtr)->CAN_RxQueue1 = 0;

  // define offests values
  ((Offsets *) pInterface->OffsetsPtr)->BCI_SYNC = OF_BCI_SYNC;
  ((Offsets *) pInterface->OffsetsPtr)->BCI_NUM = OF_BCI_NUM;
  ((Offsets *) pInterface->OffsetsPtr)->BCI_DATA = OF_BCI_DATA;

  ((Offsets *) pInterface->OffsetsPtr)->LD_SYNC = OF_LD_SYNC;
  ((Offsets *) pInterface->OffsetsPtr)->LD_CMND = OF_LD_CMND;
  ((Offsets *) pInterface->OffsetsPtr)->LD_NUM = OF_LD_NUM;
  ((Offsets *) pInterface->OffsetsPtr)->LD_ADDRESS = OF_LD_ADDRESS;
  ((Offsets *) pInterface->OffsetsPtr)->LD_DATA = OF_LD_DATA;

  switch (pInterface->Type)
  {
    case DEVICE_iPCI165_ISA:
    case DEVICE_iPCI165_PCI:
      pInterface->SemaNum  = 8;
      pInterface->SemaSize = 1;      
      ((Offsets *) pInterface->OffsetsPtr)->MODE = OF_iPCI165_MODE;
      ((Offsets *) pInterface->OffsetsPtr)->SEMA_0 = OF_iPCI165_SEMA_0;
      ((Offsets *) pInterface->OffsetsPtr)->SEMA_1 = OF_iPCI165_SEMA_1;
      ((Offsets *) pInterface->OffsetsPtr)->SEMA_2 = 0;
      ((Offsets *) pInterface->OffsetsPtr)->SEMA_3 = 0;      
      ((Offsets *) pInterface->OffsetsPtr)->RESET = OF_iPCI165_RESET;
      ((Offsets *) pInterface->OffsetsPtr)->INTERRUPT = OF_iPCI165_INTERRUPT;
      break;

    case DEVICE_iPCIXC161_PCI:
    case DEVICE_iPCIXC161_PCIE:
      pInterface->SemaNum  = 8;
      pInterface->SemaSize = 2;
      ((Offsets *) pInterface->OffsetsPtr)->MODE = OF_iPCIXC16PCI_MODE;
      ((Offsets *) pInterface->OffsetsPtr)->SEMA_0 = OF_iPCIXC16PCI_SEMA_0;
      ((Offsets *) pInterface->OffsetsPtr)->SEMA_1 = OF_iPCIXC16PCI_SEMA_1;
      ((Offsets *) pInterface->OffsetsPtr)->SEMA_2 = OF_iPCIXC16PCI_SEMA_2;
      ((Offsets *) pInterface->OffsetsPtr)->SEMA_3 = OF_iPCIXC16PCI_SEMA_3;      
      ((Offsets *) pInterface->OffsetsPtr)->RESET = OF_iPCIXC16PCI_RESET;
      ((Offsets *) pInterface->OffsetsPtr)->INTERRUPT = OF_iPCIXC16PCI_INTERRUPT;
      break;

    case DEVICE_iPCI320_ISA:
    case DEVICE_iPCI320_PCI:
      pInterface->SemaNum  = 8;
      pInterface->SemaSize = 1;
      ((Offsets *) pInterface->OffsetsPtr)->MODE = OF_iPCI320_MODE;
      ((Offsets *) pInterface->OffsetsPtr)->SEMA_0 = OF_iPCI320_SEMA_0;
      ((Offsets *) pInterface->OffsetsPtr)->SEMA_1 = OF_iPCI320_SEMA_1;
      ((Offsets *) pInterface->OffsetsPtr)->SEMA_2 = 0;
      ((Offsets *) pInterface->OffsetsPtr)->SEMA_3 = 0;
      ((Offsets *) pInterface->OffsetsPtr)->RESET = OF_iPCI320_RESET;
      ((Offsets *) pInterface->OffsetsPtr)->INTERRUPT = OF_iPCI320_INTERRUPT;
      break;

    default:
      PRINTD ("Unknown interface type [%d]\n", pInterface->Type);
      return (BCI_SERV_ERR);
  }

  return (BCI_OK);
}

/* }}} */

/* {{{ iPCIClose */

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
int iPCIClose (PInterface pInterface)
{

  free (pInterface->QueuesPtr);
  free (pInterface->OffsetsPtr);

  FreeDPRAMPtr (pInterface);
  close (pInterface->DeviceFileDesc[0]);

  return BCI_OK;
}

/* }}} */

/* {{{ iPCISendCommand */

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
int iPCISendCommand (PInterface pInterface, UINT8 * Command, int CommandSize)
{
  UINT8 *CmndBuffer;
  Offsets *Offset;

  if (CommandSize > BAP_CMND_BUFFER_SIZE)
  {
    PRINTD ("command parameters error \n ");
    return (BCI_PARA_ERR);
  }

  CmndBuffer = ((DPRAM *) (pInterface->MemoryPtr))->CmndBuffer;
  Offset = (Offsets *) pInterface->OffsetsPtr;

  // check command buffer status
  if (MEMCMP (CmndBuffer, "\0", 1))
  {
    PRINTD ("SendCommand: access error \n ");
    return (BCI_CMND_ERR);                  /* command buffer is busy */
  }

  // prepare command
  MEMSET (CmndBuffer + 1, CommandSize, 1);

  MEMCPY (CmndBuffer + 4, Command, CommandSize);

  // set status
  MEMSET (CmndBuffer, 1, 1);

  if (iPCI_Trace > 0)
  {
    int i, len;

    len = 0;

    memset (iPCI_Trace_Buf, 0x0, sizeof (iPCI_Trace_Buf));

    len += sprintf (iPCI_Trace_Buf, "iPCI Cmd(%2d) [ ", CommandSize);

    for (i = 0; i < CommandSize; i++)
    {
      len += sprintf (iPCI_Trace_Buf + len, "%02x ", Command[i]);
    }

    len += sprintf (iPCI_Trace_Buf + len, "] %s\n", GetCmdDescr (Command));

    fprintf (iPCI_Trace_fd, "%s", iPCI_Trace_Buf);
    fflush (iPCI_Trace_fd);
  }

  // generate interrupt to microcontroller
  pInterface->GenerateInterrupt (pInterface);

  return (BCI_OK);
}

/* }}} */

/* {{{ iPCIGetBoardInfo */

int iPCIGetBoardInfo (PInterface pInterface, BCI_ts_BrdInfo * BrdInfo)
{

  int ret;
  UINT8 dummy;

  CommandCode = CAN_CMD_GET_BOARD_INFO;

  CommandPtr = Command;
  dummy = 0;
  CommandSize =
    PushCmdData ((char **) &CommandPtr, (char *) &CommandCode, sizeof (UINT8));
  CommandSize +=
    PushCmdData ((char **) &CommandPtr, (char *) &dummy, sizeof (UINT8));

  ret = iPCISendCommand (pInterface, Command, CommandSize);
  if (ret != BCI_OK)
  {
    return (ret);                           /* error  */
  }

  
  ret = iPCIGetResponse (pInterface, Answer, &AnswerSize);
  if (ret == BCI_OK)
  {
    MEMCPY (BrdInfo, Answer + 2,            // Skip command and dummy bytes
            sizeof (BCI_ts_BrdInfo));
  }

  return (ret);

}

/* }}} */

/* {{{ iPCIResetCAN */

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
int iPCIResetCAN (PInterface pInterface, UINT8 CANNum)
{

  int ret;

  CommandCode = CAN_CMD_RESET;

  (pInterface)->CAN.StdMask[CANNum][0].Mask = 0;
  (pInterface)->CAN.StdMask[CANNum][1].Mask = 0;
  (pInterface)->CAN.ExtMask[CANNum][0].Mask = 0;
  (pInterface)->CAN.ExtMask[CANNum][1].Mask = 0;
  (pInterface)->CAN.StdMask[CANNum][0].Code = 0;
  (pInterface)->CAN.StdMask[CANNum][1].Code = 0;
  (pInterface)->CAN.ExtMask[CANNum][0].Code = 0;
  (pInterface)->CAN.ExtMask[CANNum][1].Code = 0;
  FilterListInit (&(pInterface->CAN.FilterListStd[CANNum]));
  FilterListInit (&(pInterface->CAN.FilterListExt[CANNum]));

  CommandPtr = Command;
  CommandSize =
    PushCmdData ((char **) &CommandPtr, (char *) &CommandCode, sizeof (UINT8));
  CommandSize +=
    PushCmdData ((char **) &CommandPtr, (char *) &CANNum, sizeof (UINT8));

  if ((ret = iPCISendCommand (pInterface, Command, CommandSize)) != BCI_OK)
  {
    return (ret);                           /*
                                             * error 
                                             */
  }

  return (iPCIGetResponse (pInterface, Answer, &AnswerSize));
}

/* }}} */

/* {{{ iPCIResetOverrun */

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
int iPCIResetOverrun (PInterface pInterface, UINT8 CANNum)
{

  PRINTD ("Overrun reset is not supported for active cards!\n");

  return BCI_OK;
}

/* }}} */

/* {{{ iPCITestCAN */

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
int iPCITestCAN (PInterface pInterface)
{
  int ret, semaCount, semaOffset, semaAlign, testCount;
  UINT8 Value;
  UINT8 TestValue, TestString[BAP_CMND_BUFFER_SIZE - 5];
  UINT8 *CmndBuffer;
  Offsets *Offset;

  semaAlign = 2;

  // First part: Loader mode. Interrupt and semaphores.

  ioctl (pInterface->DeviceFileDesc[0], IOCTL_RESET, 0);        // Go to loader mode
  BCI_MDelay (1000);

  SETBYTE (pInterface, LD_SYNC, 0);

  // Check the interrupt

  if (GETBYTE (pInterface, LD_SYNC) != '\0')
  {
    PRINTD ("Loader buffer not ready!");
    return BCI_LOADER_TEST_ERR;
  }

  /* Clear interrupt status */
  ret = (((PInterface) pInterface)->WaitForData (pInterface, 0, BCI_NO_WAIT));

/*  SETBYTE (pInterface, LD_CMND, 5);
  SETBYTE (pInterface, LD_SYNC, 1);

  if (((PInterface) pInterface)->
      WaitForData ((PInterface) pInterface, 0, 5000))
  {
    PRINTD ("Interrupt test OK!\n");
  }
  else
  {
    PRINTD ("Interrupt test failed!\n");
    return BCI_INT_TEST_ERR;
  }
*/
  /* Disable semaphore test for PCI boards */
  if ((pInterface->Type != DEVICE_iPCI320_ISA) &&
      (pInterface->Type != DEVICE_iPCI320_PCI) &&
      (pInterface->Type != DEVICE_iPCIXC161_PCI) &&
      (pInterface->Type != DEVICE_iPCI165_PCI))
  {
    // Semaphores check
    // Request semaphores
    if (pInterface->Type == DEVICE_iPCIXC161_PCIE)
    {
      semaAlign = 2;
    }
    else
    {
      semaAlign = 1;
    }

    for (semaCount = 0; semaCount < pInterface->SemaNum; semaCount += 1)
    {
      semaOffset = semaCount * semaAlign;
      Value = GETBYTE (pInterface, SEMA_0 + semaOffset);
      PRINTD ("Semaphore [0x%x/0x%x] (0x%x -> ", semaCount,
              ((Offsets *) (pInterface)->OffsetsPtr)->SEMA_0 + semaOffset, Value);
      BCI_MDelay (50);
      SETBYTE (pInterface, SEMA_0 + semaOffset, 0);
      BCI_MDelay (50);
      Value = GETBYTE (pInterface, SEMA_0 + semaOffset);

      if (Value != 0x0)
      {
        PRINTD ("0x%x) request error !\n", Value);
        return BCI_SEMA_TEST_ERR;
      }
      PRINTD ("0x%x)\n", Value);
    }

    /* Release semaphores */
    for (semaCount = 0; semaCount < pInterface->SemaNum; semaCount += 1)
    {
      semaOffset = semaCount * semaAlign;
      Value = GETBYTE (pInterface, SEMA_0 + semaOffset);
      PRINTD ("Release  Semaphore [0x%x/0x%x] (0x%x -> ", semaCount,
              ((Offsets *) (pInterface)->OffsetsPtr)->SEMA_0 + semaOffset, Value);
      SETBYTE (pInterface, SEMA_0 + semaOffset, 0x1);
      BCI_MDelay (50);
      Value = GETBYTE (pInterface, SEMA_0 + semaOffset);

      if (Value != 0xff)
      {
        PRINTD ("0x%x) release error !\n", Value);
        return BCI_SEMA_TEST_ERR;
      }
      PRINTD ("0x%x)\n", Value);
    }
  }                                         /* Semaphore test disabling for PCI boards */

  SETBYTE (pInterface, LD_SYNC, 0);

//    if (GETBYTE(pInterface, LD_CMND) != '\0') {
//      PRINTD("Loader buffer not ready!"));
//      return BCI_LOADER_TEST_ERR;
//    }

  if ((pInterface->Type == DEVICE_iPCIXC161_PCI) ||
      (pInterface->Type == DEVICE_iPCIXC161_PCIE))
  {

    typedef struct
    {
      UINT8 fw_version[4];
      UINT32 fw_attribute;
      UINT32 start_addr;
      UINT32 length;
      UINT32 entry_point;
      char fw_id[10];
    } FLASH_FW_INFO;

    FLASH_FW_INFO *fw_info;
    
    /* Clear the interrupt from the MC side using block read BM command */
    SETBYTE_OFFSET (pInterface, OF_BM_V2_SYNC_PC2MC, 0);
    SETBYTE_OFFSET (pInterface, OF_BM_V2_SYNC_MC2PC, 0);
    SETBYTE_OFFSET (pInterface, OF_BM_V2_PORT, 0);
    SETBYTE_OFFSET (pInterface, OF_BM_V2_LEN, 7);
    /* Command read block */
    SETBYTE_OFFSET (pInterface, OF_BM_V2_DATA + OF_BM_V2_CMD, 4);
    /* Dummy byte */
    SETBYTE_OFFSET (pInterface, OF_BM_V2_DATA + OF_BM_V2_CMD_STS, 0);
    /* Read address in MC address range, 0x00401FFE */
    SETBYTE_OFFSET (pInterface, OF_BM_V2_DATA + OF_BM_V2_CMD_DATA + 0, 0xfe);
    SETBYTE_OFFSET (pInterface, OF_BM_V2_DATA + OF_BM_V2_CMD_DATA + 1, 0x1f);
    SETBYTE_OFFSET (pInterface, OF_BM_V2_DATA + OF_BM_V2_CMD_DATA + 2, 0x40);        
    SETBYTE_OFFSET (pInterface, OF_BM_V2_DATA + OF_BM_V2_CMD_DATA + 3, 0x00);
    /* Read length, 1 status byte */
    SETBYTE_OFFSET (pInterface, OF_BM_V2_DATA + OF_BM_V2_CMD_DATA + 4, 0x01);    
    
    SETBYTE_OFFSET (pInterface, OF_BM_V2_SYNC_PC2MC, 1);
    BCI_MDelay (50);

    SETBYTE_OFFSET (pInterface, OF_BM_V2_SYNC_MC2PC, 0);
    BCI_MDelay (50);    

    /* Check firmware version */
    SETBYTE_OFFSET (pInterface, OF_BM_V2_SYNC_PC2MC, 0);
    SETBYTE_OFFSET (pInterface, OF_BM_V2_SYNC_MC2PC, 0);
    SETBYTE_OFFSET (pInterface, OF_BM_V2_PORT, 0);
    SETBYTE_OFFSET (pInterface, OF_BM_V2_LEN, 2);

    SETBYTE_OFFSET (pInterface, OF_BM_V2_DATA + OF_BM_V2_CMD, 7);
    SETBYTE_OFFSET (pInterface, OF_BM_V2_DATA + OF_BM_V2_CMD_STS, 0);
//      SETBYTE_OFFSET (pInterface, OF_BM_V2_DATA + OF_BM_V2_CMD_DATA, 1);

    SETBYTE_OFFSET (pInterface, OF_BM_V2_SYNC_PC2MC, 1);
    BCI_MDelay (50);

    SETBYTE_OFFSET (pInterface, OF_BM_V2_SYNC_MC2PC, 0);
    BCI_MDelay (50);

    fw_info = (FLASH_FW_INFO *) (((DPRAM *) (pInterface->MemoryPtr))->
                                 CmndBuffer + 6);

    if (strncmp (fw_info->fw_id, "BCI", 3))
    {
      /* Firmware type error */
      PRINTD ("libBCI: FW type is [%s], test failed !\n", fw_info->fw_id);
      return (BCI_TEST_ERR);
    }
    else
    {
      /* Firmware type is OK (BCI), but version is not correct */
      UINT8 Major, Minor;

      Major = (fw_info->fw_version[1] / 16) * 10 + (fw_info->fw_version[1] -
                                                    (fw_info->fw_version[1] /
                                                     16) * 16);
      Minor = (fw_info->fw_version[2] / 16) * 10 + (fw_info->fw_version[2] -
                                                    (fw_info->fw_version[2] /
                                                     16) * 16);
      if ((Major * 0xff + Minor) !=
          (BCI_iPCIXC161_FW_VERSION_MAJOR * 0xff + BCI_iPCIXC161_FW_VERSION_MINOR))
      {
        PRINTD
          ("libBCI: FW type is [%s], version is [%x %x %x %x], %d.%d, allowed =>%d.%d, test failed !\n",
           fw_info->fw_id, fw_info->fw_version[0], fw_info->fw_version[1],
           fw_info->fw_version[2], fw_info->fw_version[3], Major, Minor,
           BCI_iPCIXC161_FW_VERSION_MAJOR, BCI_iPCIXC161_FW_VERSION_MINOR);

        return (BCI_TEST_ERR);
      }
    }
  }

  pInterface->StartFirmware (pInterface);   // Start firmware again    

  // Second part: BCI firmware running. CAN_CMD_TEST command execution.

  CmndBuffer = ((DPRAM *) (pInterface->MemoryPtr))->CmndBuffer;
  Offset = (Offsets *) pInterface->OffsetsPtr;

  CommandCode = CAN_CMD_TEST;

  TestValue = 0;
  PRINTD ("Testing :");
  do
  {

    MEMSET (TestString, TestValue, sizeof (TestString));

    CommandPtr = Command;

    CommandSize =
      PushCmdData ((char **) &CommandPtr, (char *) &CommandCode,
                   sizeof (CommandCode));
    CommandSize +=
      PushCmdData ((char **) &CommandPtr, (char *) TestString, sizeof (TestString));

    if ((ret = iPCISendCommand (pInterface, Command, CommandSize)) != BCI_OK)
    {
      return (ret);                         /*
                                             * error 
                                             */
    }

    ret = iPCIGetResponse (pInterface, Answer, &AnswerSize);

    if (ret != BCI_OK)
      return (ret);                         /* error  */

    for (testCount = 1; testCount < CommandSize; testCount++)
    {
      if ((Command[testCount]) != (UINT8) (~(Answer[testCount])))
      {
        return (BCI_TEST_ERR);              /* service error */
      }

    }

    TestValue++;

  }
  while (TestValue < 0xFF);
  PRINTD (" OK\n");
  return (BCI_OK);
}

/* }}} */

/* {{{ iPCIInitCAN */

/*************************************************************************
**
** Function:
**  iPCIInitCAN
**
** Description:
**  init CAN controller
** Parameters:
**  *pInterface  (I/O)  CAN interface handler
**   CANNum      (IN)  controller number (0, 1)
**   bt0         (IN) - value for the Bus-Timing-0
**   bt1         (IN) - value for the Bus-Timing-1
**   mode        (IN) - CAN controller communication mode selection
**                      Physical bus transceiver:
**                      b0 = 0 - high speed (ISO/IS 11898-2) transceiver 
**                      b0 = 1 - low speed (ISO/IS 11898-3) transceiver 
**                      Transmission mode:
**                      b1 = 0 - Standard Communication
**                      b1 = 1 - Single Transmission Try (STT)
**
** Returnvalue: BCI_OK       - OK
**              BCI_BUSY     - command is busy
**              BCI_PARA_ERR - incorrect command
**              BCI_TIMEOUT  - timeout reached
**
***************************************************************************/
int iPCIInitCAN (PInterface pInterface, UINT8 CANNum, UINT8 bt0, UINT8 bt1,
                 UINT8 mode)
{
  int ret;

  if ((ret = iPCIResetCAN (pInterface, CANNum)) != BCI_OK)
  {
    return (ret);                           /*
                                             * error 
                                             */
  }

  CommandCode = CAN_CMD_INIT;

  CommandPtr = Command;
  CommandSize =
    PushCmdData ((char **) &CommandPtr, (char *) &CommandCode, sizeof (UINT8));
  CommandSize +=
    PushCmdData ((char **) &CommandPtr, (char *) &CANNum, sizeof (UINT8));
  CommandSize += PushCmdData ((char **) &CommandPtr, (char *) &bt0, sizeof (UINT8));
  CommandSize += PushCmdData ((char **) &CommandPtr, (char *) &bt1, sizeof (UINT8));
  CommandSize += PushCmdData ((char **) &CommandPtr, (char *) &mode, sizeof (UINT8));

  if ((ret = iPCISendCommand (pInterface, Command, CommandSize)) != BCI_OK)
  {
    return (ret);                           /*
                                             * error 
                                             */
  }

  return (iPCIGetResponse (pInterface, Answer, &AnswerSize));
}

/* }}} */

/* {{{ iPCIConfigRxQueue */

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
int iPCIConfigRxQueue (PInterface pInterface, UINT8 CANNum, UINT8 IntMode)
{
  int ret;

  CommandCode = CAN_CMD_CONFIG_RX_QUEUE;
  CommandPtr = Command;
  CommandSize =
    PushCmdData ((char **) &CommandPtr, (char *) &CommandCode, sizeof (UINT8));
  CommandSize +=
    PushCmdData ((char **) &CommandPtr, (char *) &CANNum, sizeof (UINT8));
  CommandSize +=
    PushCmdData ((char **) &CommandPtr, (char *) &IntMode, sizeof (UINT8));

  if ((ret = iPCISendCommand (pInterface, Command, CommandSize)) != BCI_OK)
  {
    return (ret);                           /*
                                             * error 
                                             */
  }

  return (iPCIGetResponse (pInterface, Answer, &AnswerSize));
}

/* }}} */

/* {{{ iPCIStartCAN */

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
int iPCIStartCAN (PInterface pInterface, UINT8 CANNum)
{

  int ret;
  UINT32 AccCode, AccMask;

  /* Check if there is something in the standard filter list */
  if ((pInterface->CAN.FilterListStd[CANNum]).ItemNumber > 0)
  {
    /* Filter list is not empty. Sort it and calculate filter setting */
    FilterListSort (&(pInterface->CAN.FilterListStd[CANNum]));
    AccCode = (pInterface->CAN.FilterListStd[CANNum]).AccCode;
    AccMask = (pInterface->CAN.FilterListStd[CANNum]).AccMask;
  }
  else
  {
    /* Filter list is empty. Disable second filter */
    AccCode = 0;
    AccMask = BCI_REJECT_ALL;
  }

  if ((ret =
       iPCISetAccMask (pInterface, CANNum, LIST_REDUCTION_FILTER,
                       BCI_11B_MASK, AccCode, AccMask)) != BCI_OK)
  {
    return ret;
  }

  /* Check if there is something in the extended filter list */
  if ((pInterface->CAN.FilterListExt[CANNum]).ItemNumber > 0)
  {
    /* Filter list is not empty. Sort it and calculate filter setting */
    FilterListSort (&(pInterface->CAN.FilterListExt[CANNum]));
    AccCode = (pInterface->CAN.FilterListExt[CANNum]).AccCode;
    AccMask = (pInterface->CAN.FilterListExt[CANNum]).AccMask;
  }
  else
  {
    /* Filter list is empty. Disable second filter */
    AccCode = 0;
    AccMask = BCI_REJECT_ALL;
  }

  if ((ret =
       iPCISetAccMask (pInterface, CANNum, LIST_REDUCTION_FILTER,
                       BCI_29B_MASK, AccCode, AccMask)) != BCI_OK)
  {
    return ret;
  }
  /* Clear interrupt status */
  ret = (((PInterface) pInterface)->WaitForData (pInterface, 0, BCI_NO_WAIT));

  CommandCode = CAN_CMD_START;

  CommandPtr = Command;
  CommandSize =
    PushCmdData ((char **) &CommandPtr, (char *) &CommandCode, sizeof (UINT8));
  CommandSize +=
    PushCmdData ((char **) &CommandPtr, (char *) &CANNum, sizeof (UINT8));

  if ((ret = iPCISendCommand (pInterface, Command, CommandSize)) != BCI_OK)
  {
    return (ret);                           /*
                                             * error 
                                             */
  }

  return (iPCIGetResponse (pInterface, Answer, &AnswerSize));

}

/* }}} */

/* {{{ iPCIStopCAN */

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
int iPCIStopCAN (PInterface pInterface, UINT8 CANNum)
{

  int ret;

  CommandCode = CAN_CMD_STOP;

  CommandPtr = Command;
  CommandSize =
    PushCmdData ((char **) &CommandPtr, (char *) &CommandCode, sizeof (UINT8));
  CommandSize +=
    PushCmdData ((char **) &CommandPtr, (char *) &CANNum, sizeof (UINT8));

  if ((ret = iPCISendCommand (pInterface, Command, CommandSize)) != BCI_OK)
  {
    return (ret);                           /*
                                             * error 
                                             */
  }

  return (iPCIGetResponse (pInterface, Answer, &AnswerSize));
}

/* }}} */

/* {{{ iPCISetAccMask */

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
** Parameters  : pInterface  (IN) - board handle
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
                    UINT8 type, UINT32 acc_code, UINT32 acc_mask)
{
  UINT32 AccMask, AccCode;
  int ret;

  CommandCode = CAN_CMD_SET_ACC_MASK;
  switch (acc_mask)
  {
    case BCI_ACC_ALL:
      AccMask = 0;
      AccCode = 0;
      break;
    case BCI_REJECT_ALL:
      AccMask = 0;
      AccCode = 1;
      break;
    default:
      switch (type)
      {
        case (BCI_11B_MASK):
          (pInterface)->CAN.StdMask[CANNum][MaskNum].Mask = acc_mask;
          (pInterface)->CAN.StdMask[CANNum][MaskNum].Code = acc_code;
          AccMask = GenId11 ((pInterface)->CAN.StdMask[CANNum][MaskNum].Mask);
          AccCode = GenId11 ((pInterface)->CAN.StdMask[CANNum][MaskNum].Code);
          break;
        case (BCI_29B_MASK):
          (pInterface)->CAN.ExtMask[CANNum][MaskNum].Mask = acc_mask;
          (pInterface)->CAN.ExtMask[CANNum][MaskNum].Code = acc_code;
          AccMask = GenId29 ((pInterface)->CAN.ExtMask[CANNum][MaskNum].Mask);
          AccCode = GenId29 ((pInterface)->CAN.ExtMask[CANNum][MaskNum].Code);
          break;

        default:
          return (BCI_PARA_ERR);
      }
      AccCode &= AccMask;
  }

  PRINTD ("CAN%d Mask (%s %d): %x->%x Code %x->%x\n", CANNum,
          (type == BCI_11B_MASK) ? "Std" : "Ext", MaskNum, acc_mask, AccMask,
          acc_code, AccCode);

  CommandPtr = Command;
  CommandSize =
    PushCmdData ((char **) &CommandPtr, (char *) &CommandCode, sizeof (UINT8));
  CommandSize +=
    PushCmdData ((char **) &CommandPtr, (char *) &CANNum, sizeof (UINT8));
  CommandSize +=
    PushCmdData ((char **) &CommandPtr, (char *) &MaskNum, sizeof (UINT8));
  CommandSize += PushCmdData ((char **) &CommandPtr, (char *) &type, sizeof (UINT8));
  CommandSize +=
    PushCmdData ((char **) &CommandPtr, (char *) &AccMask, sizeof (UINT32));
  CommandSize +=
    PushCmdData ((char **) &CommandPtr, (char *) &AccCode, sizeof (UINT32));

  if ((ret = iPCISendCommand (pInterface, Command, CommandSize)) != BCI_OK)
  {
    return (ret);                           /*
                                             * error 
                                             */
  }

  return (iPCIGetResponse (pInterface, Answer, &AnswerSize));
}

/* }}} */

/* {{{ iPCIRegRxID */

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
int iPCIRegRxID (PInterface pInterface, UINT8 CANNum, UINT8 mff, UINT32 id)
{

  int ret;

  PRINTD ("CAN%d : mff %d id 0x%x\n", CANNum, mff, id);
  ret = BCI_OK;

  switch (mff)
  {
    case (BCI_MFF_11_DAT):
      if (FilterListAddID (&(pInterface->CAN.FilterListStd[CANNum]), id) < 0)
      {
        ret = BCI_SERV_ERR;
      }
      break;
    case (BCI_MFF_11_RMT):
      if (FilterListAddID
          (&(pInterface->CAN.FilterListStd[CANNum]), id | BCI_FILTER_RTR_MASK) < 0)
      {
        ret = BCI_SERV_ERR;
      }
      break;
    case (BCI_MFF_29_DAT):
      if (FilterListAddID (&(pInterface->CAN.FilterListExt[CANNum]), id) < 0)
      {
        ret = BCI_SERV_ERR;
      }
      break;
    case (BCI_MFF_29_RMT):
      if (FilterListAddID
          (&(pInterface->CAN.FilterListExt[CANNum]), id | BCI_FILTER_RTR_MASK) < 0)
      {
        ret = BCI_SERV_ERR;
      }
      break;

    default:
      return (BCI_PARA_ERR);
  }

  return (ret);
}

/* }}} */

/* {{{ iPCIUnregRxID */

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
int iPCIUnregRxID (PInterface pInterface, UINT8 CANNum, UINT8 mff, UINT32 id)
{

  int ret;

  PRINTD ("CAN%d : mff %d id 0x%x\n", CANNum, mff, id);
  ret = BCI_OK;

  switch (mff)
  {
    case (BCI_MFF_11_DAT):
      if (FilterListRemoveID (&(pInterface->CAN.FilterListStd[CANNum]), id) < 0)
      {
        ret = BCI_SERV_ERR;
      }
      break;
    case (BCI_MFF_11_RMT):
      if (FilterListRemoveID
          (&(pInterface->CAN.FilterListStd[CANNum]), id | BCI_FILTER_RTR_MASK) < 0)
      {
        ret = BCI_SERV_ERR;
      }
      break;
    case (BCI_MFF_29_DAT):
      if (FilterListRemoveID (&(pInterface->CAN.FilterListExt[CANNum]), id) < 0)
      {
        ret = BCI_SERV_ERR;
      }
      break;
    case (BCI_MFF_29_RMT):
      if (FilterListRemoveID
          (&(pInterface->CAN.FilterListExt[CANNum]), id | BCI_FILTER_RTR_MASK) < 0)
      {
        ret = BCI_SERV_ERR;
      }
      break;

    default:
      return (BCI_PARA_ERR);
  }

  return (ret);

}

/* }}} */

/* {{{ iPCIGetResponse */

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
int iPCIGetResponse (PInterface pInterface, UINT8 * Answer, INT32 * AnswerSize)
{

  UINT8 *CmndBuffer;

  CmndBuffer = ((DPRAM *) (pInterface->MemoryPtr))->CmndBuffer;
  Timeout (RESPONSE_TIMEOUT);
  while (1)
  {
    // check command buffer status
    if (MEMCMP (CmndBuffer, "\2", 1))
    {
      if (Timeout (0))
      {
        return (BCI_NO);
      }
    }
    else
    {
      *AnswerSize = *(CmndBuffer + 1);
      MEMCPY (Answer, (CmndBuffer + 4), *AnswerSize);

      if (iPCI_Trace > 0)
      {
        int i, len;

        len = 0;
        memset (iPCI_Trace_Buf, 0x0, sizeof (iPCI_Trace_Buf));

        len += sprintf (iPCI_Trace_Buf, "iPCI Rsp(%2d) [ ", *AnswerSize);

        for (i = 0; i < *AnswerSize; i++)
        {
          len += sprintf (iPCI_Trace_Buf + len, "%02x ", Answer[i]);
        }

        len += sprintf (iPCI_Trace_Buf + len, "]\n");
                
        fprintf (iPCI_Trace_fd, "%s", iPCI_Trace_Buf);
        fflush (iPCI_Trace_fd);
      }

      MEMSET (CmndBuffer, 0, 1);
      return (BCI_OK);
    }
  }
}

/* }}} */

/* {{{ iPCIReceiveCANObj */

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
int
iPCIReceiveCANObj (PInterface pInterface, UINT8 CANNum,
                   BCI_ts_CanMsg * ReceivedCANMsg)
{
  UINT8 *QueuePtr;
  UINT8 *RxQueue;
  RawCANMsg ReceivedRawCANMsg;

  switch (CANNum)
  {
    case 0:
      RxQueue = &((Queues *) (pInterface->QueuesPtr))->CAN_RxQueue0;
      QueuePtr = ((DPRAM *) (pInterface->MemoryPtr))->CAN_RxQueue0[*RxQueue];
      break;

    case 1:
      RxQueue = &((Queues *) (pInterface->QueuesPtr))->CAN_RxQueue1;
      QueuePtr = ((DPRAM *) (pInterface->MemoryPtr))->CAN_RxQueue1[*RxQueue];
      break;

    default:
      return (BCI_PARA_ERR);
  }

  PRINTD ("CAN%d : RxQueue %d  Ptr 0x%p\n", CANNum, *RxQueue, QueuePtr);

  if (*QueuePtr != 0)
  {
    UINT8 b_msgSize;
    b_msgSize = sizeof(RawCANMsg);
    PRINTD ("CAN%d : QueuePtr != 0\n", CANNum);
    MEMCPY ((char *) &ReceivedRawCANMsg, (QueuePtr + 2), b_msgSize);
    MEMSET (QueuePtr, 0, 1);

    *RxQueue = (*RxQueue + 1) % CAN_NUM_QUEUES_ENTRIES;

    ReceivedCANMsg->dlc = ReceivedRawCANMsg.DLC;
    ReceivedCANMsg->time_stamp = ReceivedRawCANMsg.TimeStamp;

    // Check if we received a status message
    switch (ReceivedRawCANMsg.MsgType & 0x01)
    {
      case (BCI_MSGTYPE_STATUS):
        ReceivedCANMsg->mff = BCI_MFF_STS_MSG;
        ReceivedCANMsg->id = ReceivedRawCANMsg.Message.Std.ID;
        ReceivedCANMsg->dlc = 0;
        break;
      case (BCI_MSGTYPE_DATA):
        if (ReceivedRawCANMsg.FrameFormat == BCI_11BIT_FF)
        {
          /*
           * ** 11 Bit CAN identifier 
           */

          if (ReceivedRawCANMsg.RTR == BCI_RTR_FF)
          {                                 // Is RTR flag set ?
            ReceivedCANMsg->mff = BCI_MFF_11_RMT;
          }
          else
          {
            ReceivedCANMsg->mff = BCI_MFF_11_DAT;
          }

          ReceivedCANMsg->id = GetId11 (ReceivedRawCANMsg.Message.Std.ID);
          PRINTD ("CAN%d : received std id 0x%x type 0x%x\n",
                  CANNum, ReceivedCANMsg->id, ReceivedRawCANMsg.MsgType);
          if (ReceivedRawCANMsg.MsgType & 0x02)
          {
            /* Message has passed second filter. Check standard filter list */
            if (FilterListSearch (&(pInterface->CAN.FilterListStd[CANNum]),
                                  (ReceivedCANMsg->mff == BCI_MFF_11_RMT) ?
                                  ReceivedCANMsg->id | BCI_FILTER_RTR_MASK :
                                  ReceivedCANMsg->id) < 0)
            {
              PRINTD ("11-bit Message id 0x%x discarded\n", ReceivedCANMsg->id);
              /* ID is not in filter list. Discard message */
              break;
            }
          }
          MEMCPY (ReceivedCANMsg->a_data, ReceivedRawCANMsg.Message.Std.Data, 8);

          return (BCI_OK);
        }
        else
        {
          /*
           * ** 29 Bit CAN identifier 
           */

          if (ReceivedRawCANMsg.RTR == BCI_RTR_FF)
          {                                 
            /* Is RTR flag set ? */
            ReceivedCANMsg->mff = BCI_MFF_29_RMT;
          }
          else
          {
            ReceivedCANMsg->mff = BCI_MFF_29_DAT;
          }
          ReceivedCANMsg->id = GetId29 (ReceivedRawCANMsg.Message.Ext.ID);
          PRINTD ("CAN%d : received ext id 0x%x type 0x%x\n",
                  CANNum, ReceivedCANMsg->id, ReceivedRawCANMsg.MsgType);
          if (ReceivedRawCANMsg.MsgType & 0x02)
          {
            /* Message has passed second filter. Check extended filter list */
            if (FilterListSearch (&(pInterface->CAN.FilterListExt[CANNum]),
                                  (ReceivedCANMsg->mff == BCI_MFF_29_RMT) ?
                                  ReceivedCANMsg->id | BCI_FILTER_RTR_MASK :
                                  ReceivedCANMsg->id) < 0)
            {
              /* ID is not in filter list. Discard message */
              PRINTD ("29-bit Message id 0x%x discarded\n", ReceivedCANMsg->id);
              break;
            }
          }
          MEMCPY (ReceivedCANMsg->a_data, ReceivedRawCANMsg.Message.Ext.Data, 8);

          return (BCI_OK);
        }
        break;

      default:
        return (BCI_PARA_ERR);

    }

  }
  // else {
  // 
  // ret = BCI_BUSY;

  // }

  PRINTD ("Rx CAN%d : queue is empty\n", CANNum);
//      *RxQueue = (*RxQueue + 1) % NUM_QUEUES_ENTRIES;

  return (BCI_NO);

}

/* }}} */

/* {{{ iPCITransmitCANObj */

/*************************************************************************
 **
 ** Function    : iPCITransmitCANObj
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
iPCITransmitCANObj (PInterface pInterface, UINT8 CANNum,
                    BCI_ts_CanMsg * ToSendCANMsg)
{
  UINT8 *QueuePtr;
  UINT8 *TxQueue;
  RawCANMsg ToSendRawCANMsg;
  char CANMsgSize;

  // check command buffer status

  switch (CANNum)
  {
    case 0:
    {
      TxQueue = &((Queues *) (pInterface->QueuesPtr))->CAN_TxQueue0;
      QueuePtr = ((DPRAM *) (pInterface->MemoryPtr))->CAN_TxQueue0[*TxQueue];
    }
    break;

    case 1:
    {
      TxQueue = &((Queues *) (pInterface->QueuesPtr))->CAN_TxQueue1;
      QueuePtr = ((DPRAM *) (pInterface->MemoryPtr))->CAN_TxQueue1[*TxQueue];
    }
    break;

    default:
    {
      return (BCI_PARA_ERR);
    }
  }

  PRINTD ("CAN%d : TxQueue %d  Ptr 0x%p\n", CANNum, *TxQueue, QueuePtr);

  switch (ToSendCANMsg->mff)
  {
    case (BCI_MFF_11_DAT):
    {
      ToSendRawCANMsg.FrameFormat = BCI_11BIT_FF;
      ToSendRawCANMsg.RTR = BCI_DAT_FF;
    }
    break;

    case (BCI_MFF_11_RMT):
    {
      ToSendRawCANMsg.FrameFormat = BCI_11BIT_FF;
      ToSendRawCANMsg.RTR = BCI_RTR_FF;
    }
    break;

    case (BCI_MFF_29_DAT):
    {
      ToSendRawCANMsg.FrameFormat = BCI_29BIT_FF;
      ToSendRawCANMsg.RTR = BCI_DAT_FF;
    }
    break;

    case (BCI_MFF_29_RMT):
    {
      ToSendRawCANMsg.FrameFormat = BCI_29BIT_FF;
      ToSendRawCANMsg.RTR = BCI_RTR_FF;
    }
    break;

    default:
    {
      return (BCI_PARA_ERR);
    }
  }

  ToSendRawCANMsg.DLC = ToSendCANMsg->dlc;
  ToSendRawCANMsg.TimeStamp = 0;
  ToSendRawCANMsg.Reserved2 = 0;
  ToSendRawCANMsg.MsgType = BCI_MSGTYPE_DATA;

  if (ToSendRawCANMsg.FrameFormat == BCI_11BIT_FF)
  {
    /*
     * ** 11 Bit CAN identifier 
     */
    ToSendRawCANMsg.Message.Std.ID = GenId11 (ToSendCANMsg->id);
    MEMCPY (ToSendRawCANMsg.Message.Std.Data, ToSendCANMsg->a_data, 8);
  }
  else
  {
    /*
     * ** 29 Bit CAN identifier 
     */
    ToSendRawCANMsg.Message.Ext.ID = GenId29 (ToSendCANMsg->id);
    MEMCPY (ToSendRawCANMsg.Message.Ext.Data, ToSendCANMsg->a_data, 8);
  }

  if (*QueuePtr == 0)
  {                                         // Free entry
    CANMsgSize = sizeof (RawCANMsg);
    /* copy CAN message */
    MEMCPY ((QueuePtr + 2), (char *) &ToSendRawCANMsg, CANMsgSize);
    /* set message length */
    MEMSET (QueuePtr + 1, CANMsgSize, 1);
    /* set message status as valid */
    MEMSET (QueuePtr, 1, 1);

    *TxQueue = (*TxQueue + 1) % CAN_NUM_QUEUES_ENTRIES;
    pInterface->GenerateInterrupt (pInterface);

    PRINTD ("Tx CAN%d : item number %d, CANMode %d, Transmitted \n",
            CANNum, *TxQueue, ToSendRawCANMsg.FrameFormat);
    return (BCI_OK);
  }

  PRINTD ("Tx CAN%d : queue is full\n", CANNum);
  return (BCI_BUSY);

}

/* }}} */

/* {{{ iPCIGetBrdStatus */

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
int iPCIGetBrdStatus (PInterface pInterface, BCI_ts_BrdSts * BrdStatus)
{
  INT32 ret, i_sema_check_count;
  volatile UINT8 *pbSema = NULL;
  
  
  pbSema = ((UINT8 *) pInterface->MemoryPtr) + (((Offsets *) pInterface->OffsetsPtr)->SEMA_0);

  PRINTD("\nBrdStatus: Mem %p try to get sema at %p val 0x%x\n", (UINT8 *) pInterface->MemoryPtr, pbSema, *(pbSema));

  SETBYTE (pInterface, SEMA_0, 0);

  PRINTD("Sema set val 0x%x\n", *(pbSema));
  msync (pInterface->MemoryPtr, pInterface->BoardMemSize, MS_SYNC);

  i_sema_check_count = 0;
  
  while (TRUE)
  {
    if (*pbSema == 0x0)
    {
      MEMCPY (BrdStatus, ((DPRAM *) pInterface->MemoryPtr)->CAN_ImageBuffer,
              sizeof(BCI_ts_BrdSts));
      PRINTD("Get OK\n");
      ret = BCI_OK;
      break;
    }
    else
    {
      if ((i_sema_check_count++) < 100)
      {
        BCI_MDelay(1);
        PRINTD("Check failed, val 0x%x, continue\n", *(pbSema));
      }
      else
      {
        ret = BCI_BUSY;
        PRINTD("FAILED, val 0x%x\n", *(pbSema));
        break;
      }
    }
  }

  /* Clear the semaphore */
  SETBYTE (pInterface, SEMA_0, 1);

  if (ret == BCI_OK)
  {
    i_sema_check_count = 0;
  
    while (TRUE)
    {
      if (*pbSema != 0x0)
      {
        PRINTD("Free OK\n");
        ret = BCI_OK;
        break;
      }
      else
      {
        if ((i_sema_check_count++) < 100)
        {
          BCI_MDelay(1);
          PRINTD("Free check failed, val 0x%x, continue\n", *(pbSema));
        }
        else
        {
          ret = BCI_BUSY;
          PRINTD("Free FAILED, val 0x%x\n", *(pbSema));
          break;
        }
      }
    }
    PRINTD("BrdStatus:  free sema val 0x%x\n", *(pbSema));
  }
  return (ret);
}

/* }}} */

/* {{{ GetCodeFromHEXFile */

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
int GetCodeFromHEXFile (FILE * fp, char *Code)
{

  UINT8 Delim, CR;

  int ReadFieldNumber, j, ExtAddress;

  char *DataPtr, *CodePtr, Line[HEXFILE_MAX_LINE_LENGTH * 2],
    Rest[HEXFILE_MAX_LINE_LENGTH * 2], Data[HEXFILE_MAX_LINE_LENGTH];

  static unsigned short Address, Segment = 0x0, CurLine = 0;
  unsigned char Length, Type, Value, RecordCheckSum;

  unsigned char CalcCheckSum, FullLength;

  while (1)
  {

    CodePtr = Code;

    memset (Line, 0, HEXFILE_MAX_LINE_LENGTH);

    if (fscanf (fp, "%s%c", Line, &CR) == EOF)
    {
      fclose (fp);
      return HEX_RET_ERR_EOF;
    }

    CurLine++;

    DataPtr = Line + 1;
    CalcCheckSum = 0;
    FullLength = (strlen (Line) - 1) / 2 - 1;

    for (j = 0; j < FullLength; j++)
    {
      sscanf (DataPtr, "%2hhX", &Value);
      DataPtr += 2;
      CalcCheckSum += Value;
    }

    CalcCheckSum = (unsigned char) (~CalcCheckSum + 1);

    ReadFieldNumber =
      sscanf (Line, "%c%2hhX%4hX%2hhX%s", &Delim, &Length, &Address, &Type, Rest);

    if ((Delim != ':') || ReadFieldNumber != 5)
    {
      PRINTD ("Syntax error at line [%d] \n", CurLine);
      return HEX_RET_SYNTAX_ERR;
    }

    if (Length > FullLength)
    {
      PRINTD ("Wrong data length : [%d] line [%d] \n", Length, CurLine);
      return HEX_RET_WRONG_LEN;
    }
    DataPtr = Rest;

    PRINTD ("%d) Len = %d ,Adr = %x Type is %d Data[", CurLine, Length,
            Address, Type);
    for (j = 0; j < Length; j++)
    {
      sscanf (DataPtr, "%2hhX", &Data[j]);
      DataPtr += 2;

      PRINTD ("%02x ", Data[j]);
    }
    sscanf (DataPtr, "%2hhX", &RecordCheckSum);

    PRINTD ("], CheckSum : (%02X/%02X)\n", CalcCheckSum, RecordCheckSum);

    if (CalcCheckSum != RecordCheckSum)
    {
      PRINTD ("Checksum error, line %d\n", CurLine);

      return HEX_RET_ERR_CHECKSUM;

    }

    switch (Type)
    {
      case HEX_DATA:

        ExtAddress = (Segment << 16) | Address;
        memset (CodePtr, 1, 1);
        CodePtr++;
        memset (CodePtr, Length, 1);
        CodePtr++;
        memset (CodePtr, 0, 1);
        CodePtr++;
        memcpy (CodePtr, &ExtAddress, sizeof (ExtAddress));
        CodePtr += sizeof (ExtAddress);
        memcpy (CodePtr, Data, Length);
        CodePtr += Length;
        return HEX_RET_OK;

        break;
      case HEX_EOF:
        PRINTD ("%d lines processed\n", CurLine);
        CurLine = 0;
        return HEX_RET_EOF;
        break;
      case HEX_8086_SEGMENT_ADDRESS:
        sscanf (Data, "%4hX", &Segment);
        PRINTD ("HEX_8086_SEGMENT_ADDRESS %X\n", Segment);
        break;
      case HEX_EXTENDED_LINEAR_ADDRESS:
        sscanf (Data, "%4hX", &Segment);
        PRINTD ("HEX_EXTENDED_LINEAR_ADDRESS %X\n", Segment);
        break;
      default:
        PRINTD ("Unknown Type %d\n", Type);
        return (HEX_RET_SYNTAX_ERR);
    }
  }
}

/* }}} */

/* {{{ iPCIStartFirmware_I165 */

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
int iPCIStartFirmware_I165 (PInterface pInterface)
{

  SETBYTE (pInterface, LD_CMND, 2);
  SETBYTE (pInterface, LD_CMND + 1, 0);
  SETBYTE (pInterface, LD_SYNC, 1);
  BCI_MDelay (500);

  return (BCI_OK);
}

/* }}} */

/* {{{ iPCIStartFirmware_I165 */

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
int iPCIStartFirmware_IXC161 (PInterface pInterface)
{

  SETBYTE_OFFSET (pInterface, OF_BM_V2_SYNC_PC2MC, 0);
  SETBYTE_OFFSET (pInterface, OF_BM_V2_SYNC_MC2PC, 0);
  SETBYTE_OFFSET (pInterface, OF_BM_V2_PORT, 0);
  SETBYTE_OFFSET (pInterface, OF_BM_V2_LEN, 3);

  SETBYTE_OFFSET (pInterface, OF_BM_V2_DATA + OF_BM_V2_CMD, 2);
  SETBYTE_OFFSET (pInterface, OF_BM_V2_DATA + OF_BM_V2_CMD_STS, 0);
  SETBYTE_OFFSET (pInterface, OF_BM_V2_DATA + OF_BM_V2_CMD_DATA, 1);

  SETBYTE_OFFSET (pInterface, OF_BM_V2_SYNC_PC2MC, 1);
  BCI_MDelay (50);

  SETBYTE_OFFSET (pInterface, OF_BM_V2_SYNC_MC2PC, 0);
  BCI_MDelay (50);

  return (BCI_OK);
}

/* }}} */

/* {{{ iPCIStartFirmware_I320 */

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
int iPCIStartFirmware_I320 (PInterface pInterface)
{

  SETBYTE (pInterface, RESET, 1);
  BCI_MDelay (500);

  return (BCI_OK);
}

/* }}} */

/* {{{ iPCIGenerateInterrupt165PCI */

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
int iPCIGenerateInterrupt165PCI (PInterface pInterface)
{

  ioctl (pInterface->DeviceFileDesc[0], IOCTL_INTERRUPT_TO_MC, 0);

  return (BCI_OK);
}

/* }}} */

/* {{{ iPCIGenerateInterrupt165PCI */

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
int iPCIGenerateInterruptXC161PCI (PInterface pInterface)
{

  ioctl (pInterface->DeviceFileDesc[0], IOCTL_INTERRUPT_TO_MC, 0);

  return (BCI_OK);
}

/* }}} */

/* {{{ iPCIGenerateInterruptGeneric */

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
int iPCIGenerateInterruptGeneric (PInterface pInterface)
{

  SETBYTE (pInterface, INTERRUPT, 0x00);

  return (BCI_OK);
}

/* }}} */

/* {{{ iPCIWaitForData */

/*************************************************************************
**
** Function: iPCIWaitForData
**
** Description: wait for data. Only for
**              BCI_LATENCY_MODE (Interrupt after every received message) and
**              BCI_THROUGHPUT_MODE (Interrupt when queue is full) modes.
** Parameters:
**  *pInterface  (I/O)  CAN interface handler
** Returnvalue:
**   TRUE : - data available
**   FALSE: - no data
**
*************************************************************************/
int iPCIWaitForData (PInterface pInterface, UINT8 CANNum, UINT32 Timeout_msec)
{
  struct timeval timeout;
  fd_set read_fds;

  FD_ZERO (&read_fds);
  FD_SET (pInterface->DeviceFileDesc[0], &read_fds);

  msec2timeval (Timeout_msec, &timeout);

  return (select
          (pInterface->DeviceFileDesc[0] + 1, &read_fds, NULL, NULL, &timeout));
}

/* }}} */

/* {{{ iPCIDownloadFirmware */

/*************************************************************************
**
** Function    : iPCIDownloadFirmware
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
iPCIDownloadFirmware (PInterface pInterface, UINT8 FirmwareMode, UINT8 * HEXFileName)
{

//  ioctl (pInterface->DeviceFileDesc[0], IOCTL_RESET, 0); // Go to loader mode
//  BCI_MDelay (1000);

  switch (FirmwareMode)
  {

    case FW_DO_NOT_LOAD:
      return BCI_OK;
      break;
    case FW_LOAD_DEFAULT:
      return (iPCIDownloadFirmwareFromCArray (pInterface, pInterface->CAN.FWArray));
      break;
    case FW_LOAD_DEFAULT_HEX:
    case FW_LOAD_FROM_HEX:
      return (iPCIDownloadFirmwareFromFile (pInterface, FirmwareMode, HEXFileName));
      break;

    default:
      return (BCI_PARA_ERR);
  }

  return BCI_OK;
}

/* }}} */

/* {{{ iPCIDownloadFirmwareFromFile */

int
iPCIDownloadFirmwareFromFile (PInterface pInterface, UINT8 FirmwareMode,
                              UINT8 * HEXFileName)
{

  char Code[HEXFILE_MAX_LINE_LENGTH];
  int ret;
  Offsets *Offset = (Offsets *) pInterface->OffsetsPtr;
  FILE *fp = NULL;

  if (FirmwareMode != FW_LOAD_DEFAULT)
  {
    PRINTD ("Loading firmware from [%s]\n", HEXFileName);

    if ((fp = fopen ((char *) HEXFileName, "r")) == NULL)
    {
      PRINTD ("File open error !\n");
      return HEXFILE_FILE_OPEN_ERR;
    }
  }
  PRINTD ("Downloading :\n");
  do
  {
    memset (Code, 0, 128);
    ret = GetCodeFromHEXFile (fp, Code);

    if (ret != HEX_RET_OK)
      break;

    MEMCPY (pInterface->MemoryPtr + Offset->LD_CMND, Code, Code[1] + 7);

    SETBYTE (pInterface, LD_SYNC, 1);

    Timeout (RESPONSE_TIMEOUT);
    while (1)
    {
      if (MEMCMP (pInterface->MemoryPtr + Offset->LD_SYNC, "\0", 1))
      {
        if (Timeout (0))
        {

          if (FirmwareMode != FW_LOAD_DEFAULT)
          {
            fclose (fp);
          }

          return (BCI_NO);
        }

      }
      else
        break;
    }
    PRINTD (".");
  }
  while (ret == HEX_RET_OK);

  if (FirmwareMode != FW_LOAD_DEFAULT)
  {
    fclose (fp);
  }

  if (ret != HEX_RET_EOF)
  {
    PRINTD ("Download error !\n");
    return (BCI_FW_ERR);
  }
  PRINTD (" OK\n");

  pInterface->StartFirmware (pInterface);

  BCI_MDelay (500);

  return (BCI_OK);
}

/* }}} */

/* {{{ iPCIDownloadFirmwareFromCArray */

int iPCIDownloadFirmwareFromCArray (PInterface pInterface, BAP_ts_Firmware * FWArray)
{
  Offsets *Offset;

  Offset = (Offsets *) pInterface->OffsetsPtr;
  while (FWArray->len)
  {
    /*
     ** copy one data block to DPRAM and mark entry as busy
     */

    SETBYTE (pInterface, LD_NUM, FWArray->len);
    SETBYTE (pInterface, LD_NUM + 1, 0);

    MEMCPY (pInterface->MemoryPtr + Offset->LD_ADDRESS, &(FWArray->addr), 4);

    MEMCPY (pInterface->MemoryPtr + Offset->LD_DATA, FWArray->a_data, FWArray->len);
    SETBYTE (pInterface, LD_CMND, 1);

    SETBYTE (pInterface, LD_SYNC, 1);
    FWArray++;

    /* wind up timer for timeout supervision */
    Timeout (RESPONSE_TIMEOUT);
    while (1)
    {
      /* check for microcontroller reaction */
      if (MEMCMP (pInterface->MemoryPtr + Offset->LD_SYNC, "\0", 1))
      {
        if (Timeout (0))
        {

          return (BCI_NO);
        }

      }
      else
        break;
    }
  }

  pInterface->StartFirmware (pInterface);

  BCI_MDelay (500);
  return BCI_OK;
};

/* }}} */

/* {{{ BCI_MDelay */

/*************************************************************************

  Function:
   BCI_MDelay

  Description:
   Delay in milliseconds

  Parameters:
   msec (IN)

  Returnvalue:
   none

*************************************************************************/
void BCI_MDelay (unsigned int msec)
{

  struct timespec requested, remaining;

  msec2timespec (msec, &requested);

////    PRINTD("Sleep : msec %d sec %ld nsec %ld\n", msec,
//                  requested.tv_sec, requested.tv_nsec);
//
  remaining.tv_sec = 0;
  remaining.tv_nsec = 0;
  nanosleep (&requested, &remaining);
}

/* }}} */

/* {{{ msec2timespec */

/*************************************************************************

  Function:
   msec2timespec

  Description:
   Convert milliseconds to timespec format

  Parameters:
   msec (IN)
   conv_time (OUT) (out)

  Returnvalue:
   none
   
*************************************************************************/
void msec2timespec (UINT32 msec, struct timespec *conv_time)
{

  unsigned long int nsec = 0, sec = 0;

  if (msec > 999)
  {

    nsec = ((unsigned long int) msec) % 1000;
    sec = (msec - nsec) / 1000;
    nsec = (msec - sec * 1000) * 1000000;
  }
  else
    nsec = (unsigned long int) (msec * 1000000);

  conv_time->tv_sec = sec;
  conv_time->tv_nsec = nsec;                // nanoseconds
}

/* }}} */

/* {{{ msec2timeval */

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
void msec2timeval (UINT32 msec, struct timeval *conv_time)
{

  unsigned long int usec = 0, sec = 0;

  if (msec > 999)
  {

    usec = ((unsigned long int) msec) % 1000;
    sec = (msec - usec) / 1000;
    usec = (msec - sec * 1000) * 1000;
  }
  else
    usec = (unsigned long int) (msec * 1000);
  PRINTD ("Time : msec %d sec %ld nsec %ld\n", msec, sec, usec);
  conv_time->tv_sec = sec;
  conv_time->tv_usec = usec;                // microseconds
}

/* }}} */

/* {{{ Timeout */

/*************************************************************************

  Function:
   Timeout

  Description:
   windup timer (if msec > 0) and otherwise
   return if timeout reached

  Parameters:
   msecs (IN)

  Returnvalue:
   TRUE :
   FALSE:

*************************************************************************/
UINT8 Timeout (unsigned int msec)
{
  static struct timeval nominal_time, sys_time;
  long sec, usec, delta;

  gettimeofday (&sys_time, NULL);

  if (msec)
  {
    usec = (msec * 1000 + sys_time.tv_usec) % 1000000;
    sec = (msec * 1000 + sys_time.tv_usec - usec) / 1000000;

    nominal_time.tv_sec = sys_time.tv_sec + sec;
    nominal_time.tv_usec = usec;
    return 0;
  }
  else
  {
    delta = (nominal_time.tv_sec - sys_time.tv_sec) * 1000000 +
      (nominal_time.tv_usec - sys_time.tv_usec);

    return (delta <= 0);
  }
}

/* }}} */

/* {{{ PushCmdData */

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
int PushCmdData (char **Cmd, char *Data, int Num)
{
  int i;

  for (i = 0; i < Num; i++)
  {
    *((*Cmd)++) = *Data++;
  }
  return Num;
}

/* }}} */

/* {{{ Gen/Get ID  */

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

UINT32 GenId11 (UINT32 value)
{
  value = value << 21;

  SWAP16 (HIGH16 (value));
  SWAP16 (LOW16 (value));
  SWAP32 (value);

  return (value);
}

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

UINT32 GenId29 (UINT32 value)
{
  value = value << 3;

  SWAP16 (HIGH16 (value));
  SWAP16 (LOW16 (value));
  SWAP32 (value);

  return (value);
}

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

UINT32 GetId11 (UINT32 id)
{
  SWAP16 (HIGH16 (id));
  SWAP16 (LOW16 (id));
  SWAP32 (id);

  id = id >> 21;
  return id;
}

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
UINT32 GetId29 (UINT32 id)
{
  SWAP16 (HIGH16 (id));
  SWAP16 (LOW16 (id));
  SWAP32 (id);

  id = id >> 3;
  return id;
}

/* }}} */

char *GetCmdCodeDescr (UINT8 Cmd)
{
  char *psDescr;

  switch (Cmd)
  {
    case CAN_CMD_ID:
      psDescr = "CAN_CMD_ID";
      break;
    case CAN_CMD_VERSION:
      psDescr = "CAN_CMD_VERSION";
      break;
    case CAN_CMD_TEST:
      psDescr = "CAN_CMD_TEST";
      break;
    case CAN_CMD_INIT:
      psDescr = "CAN_CMD_INIT";
      break;

    case CAN_CMD_START:
      psDescr = "CAN_CMD_START";
      break;
    case CAN_CMD_STOP:
      psDescr = "CAN_CMD_STOP";
      break;
    case CAN_CMD_RESET:
      psDescr = "CAN_CMD_RESET";
      break;

    case CAN_CMD_CONFIG_RX_QUEUE:
      psDescr = "CAN_CMD_CONFIG_RX_QUEUE";
      break;
    case CAN_CMD_GET_BOARD_INFO:
      psDescr = "CAN_CMD_GET_BOARD_INFO";
      break;
    case CAN_CMD_START_TIMER:
      psDescr = "CAN_CMD_START_TIMER";
      break;
    case CAN_CMD_STOP_TIMER:
      psDescr = "CAN_CMD_STOP_TIMER";
      break;
    case CAN_CMD_SET_ACC_MASK:
      psDescr = "CAN_CMD_SET_ACC_MASK";
      break;

    case LIN_CMD_GET_STAT:
      psDescr = "LIN_CMD_GET_STAT";
      break;
    case LIN_CMD_START:
      psDescr = "LIN_CMD_START";
      break;
    case LIN_CMD_STOP:
      psDescr = "LIN_CMD_STOP";
      break;
    case LIN_CMD_RESET:
      psDescr = "LIN_CMD_RESET";
      break;
    case LIN_CMD_INIT:
      psDescr = "LIN_CMD_INIT";
      break;
    case LIN_CMD_UPDATE_BUF:
      psDescr = "LIN_CMD_UPDATE_BUF";
      break;

    default:
      psDescr = "iPCI_CMD_UNKNOWN";
  }
  return psDescr;
}

char *GetCmdDescr (UINT8 * Cmd)
{
  static char buf[1024 * 8];

  char *psDescr;

  switch (Cmd[0])
  {
    case LIN_CMD_INIT:
      {
        UINT8 mode = Cmd[2];
        UINT16 bitrate = *((UINT16 *) (Cmd + 4));
        char *smode;

        if (mode == BCI_LIN_FW_OPMODE_MODE_SLAVE)
        {
          smode = "LIN_OPMODE_SLAVE";
        }
        else if (mode == BCI_LIN_FW_OPMODE_MODE_MASTER)
        {
          smode = "LIN_OPMODE_MASTER";
        }
        else
        {
          smode = "LIN_OPMODE_UNKNOWN";
        }

        sprintf (buf, "%s mode %s (0x%x) bitrate %d",
                 GetCmdCodeDescr (Cmd[0]), smode, mode, bitrate);
        psDescr = buf;
      }
      break;
    case LIN_CMD_UPDATE_BUF:
      {
        sprintf (buf, "%s id 0x%x Model %d Checksum 0x%x SD %d Len %d",
                 GetCmdCodeDescr (Cmd[0]), (Cmd[2] & 0x3f), Cmd[3], Cmd[4],
                 Cmd[5], Cmd[6]);
        psDescr = buf;
      }
      break;

    default:
      psDescr = GetCmdCodeDescr (Cmd[0]);
  }
  return psDescr;
}
