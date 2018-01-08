
/*************************************************************************
**    IXXAT Automation GmbH
**************************************************************************
**
**  $Workfile: $
**    Summary: BCI library API functions
**   $Archive: $
**  $Revision: 1.38 $
**      $Date: 2004/01/28 11:47:50 $
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
#include "ipci_lin.h"
#include "pc_i.h"
#include "can.h"
#include "can_ioctl.h"
#include "filterlist.h"

/*************************************************************************
**    global variables
*************************************************************************/
char BCIErrorString[ERROR_STRING_LEN];

#ifdef BCI_TEST

/* Testin version of BCI with firmware command trace */
FILE *trace_fd = NULL;
char TestMsg[TEST_MSG_LEN];
char TestMsgFull[TEST_MSG_LEN];
#endif

/*************************************************************************
**    static constants, types, macros, variables
*************************************************************************/
static t_Interface *pFirstInterface = NULL;
static t_Interface *pCurrentInterface = NULL;
static t_Interface *pLastInterface = NULL;

#ifdef PRINTD
#undef PRINTD

#if (BCI_DEBUG == 1)
#define PRINTD(format, args...) printf(format, ## args)
#else
#define PRINTD(format, args...)
#endif
#endif

#ifdef PRINTI
#undef PRINTI
#define PRINTI(format, args...) printf(format, ## args)
#endif

/*************************************************************************
**    static function-prototypes
*************************************************************************/
static int BCI_CheckCANState (t_Interface * pInterface, UINT8 CANNum,
                              UINT8 StateExpected);
static int BCI_SetCANState (t_Interface * pInterface, UINT8 CANNum, UINT8 State);

static int BCI_CheckLINState (t_Interface * pInterface, UINT8 LINNum,
                              UINT8 StateExpected);
static int BCI_SetLINState (t_Interface * pInterface, UINT8 LINNum, UINT8 State);

/*************************************************************************
**    global functions
*************************************************************************/

/*************************************************************************
**
**  Function:
**   ShowInterfacesList
**
**  Description:
**   Show current CAN interfaces list
**
**  Parameters:
**   none
**
**  Returnvalue:
**    BCI_OK          - OK
**
*************************************************************************/
int ShowInterfacesList (void)
{
  pCurrentInterface = pFirstInterface;
  if (pCurrentInterface == NULL)
  {

    PRINTI ("List is empty !\n");
    return BCI_OK;
  }
  PRINTD ("First %s Last %s\n", pFirstInterface->Name, pLastInterface->Name);

  while (1)
  {
    PRINTD ("[%s]>", (pCurrentInterface->Name));
    if (pCurrentInterface->Next != NULL)
      pCurrentInterface = pCurrentInterface->Next;
    else
      break;
  }
  PRINTD ("[NUll]\n");
  return BCI_OK;
}

int BCI_Status2Str (UINT16 Status, char *StatusStr)
{

  sprintf (StatusStr, "%s%s%s%s%s%s%s%s%s%s%c",
           (Status & BCI_CAN_INIT_MODE) ? "I" : "-",
           (Status & BCI_CAN_WARN_LEVEL) ? "W" : "-",
           (Status & BCI_CAN_BUS_OFF) ? "O" : "-",
           (Status & BCI_CAN_DATA_OVR) ? "D" : "-",
           (Status & BCI_CAN_RX_STAT) ? "R" : "-",
           (Status & BCI_CAN_TX_PEND) ? "T" : "-",
           (Status & BCI_CAN_LOWSPEED_ERR) ? "L" : "-",
           (Status & BCI_CAN_STT_STAT) ? "S" : "-",
           (Status & BCI_CAN_QUE_OVR) ? "Q" : "-",
           (Status & BCI_CAN_QUE_EMPTY) ? "E" : "-", 0x0);

  return 0;
}

/*************************************************************************
**
**  Function:
**   BCI_ShowCANMsg
**
**  Description:
**   Show CAN object
**
**  Parameters:
**   *BCI_ts_CanMsg  (OUT)   CAN object pointer
**
**  Returnvalue:
**    BCI_OK          - OK
**
*************************************************************************/


int BCI_ParseCANMsg (BCI_ts_CanMsg * CANMsg, int *ID)
{
  int i;
  char FrameFormatName[16];
  char Status[16];

  switch (CANMsg->mff)
  {
    case BCI_MFF_11_DAT:
      strncpy (FrameFormatName, "11 bit data\0", 12);
      break;
    case BCI_MFF_29_DAT:
      strncpy (FrameFormatName, "29 bit data\0", 12);
      break;
    case BCI_MFF_11_RMT:
      strncpy (FrameFormatName, "11 bit rmt \0", 12);
      break;
    case BCI_MFF_29_RMT:
      strncpy (FrameFormatName, "29 bit rmt \0", 12);
      break;
    case BCI_MFF_STS_MSG:
      strncpy (FrameFormatName, "Status msg \0", 12);
      break;
    default:
      strncpy (FrameFormatName, "???????????\0", 12);
  }

  switch (CANMsg->mff)
  {
    case BCI_MFF_11_DAT:
    case BCI_MFF_29_DAT:
    case BCI_MFF_11_RMT:
    case BCI_MFF_29_RMT:
    *ID = CANMsg->id;
      //PRINTI ("ID [%8x] Fmt [%s] DLC [%d] [",
              //CANMsg->id, FrameFormatName, CANMsg->dlc);
      //for (i = 0; i < CANMsg->dlc; i++)
        //PRINTI ("%02X ", CANMsg->a_data[i]);
      //PRINTI ("]");
      break;
    case BCI_MFF_STS_MSG:
      BCI_Status2Str (CANMsg->id, Status);
      //PRINTI ("Status [ %s ]", Status);
      break;

    default:
      PRINTI ("Unknown message frame format [ %d ]", CANMsg->mff);
  }
  PRINTI (" Time [%8d.%.8d]\n",
          (CANMsg->time_stamp / 8) / 1000, (CANMsg->time_stamp / 8) % 1000);

  return 0;
}










































int BCI_ShowCANMsg (BCI_ts_CanMsg * CANMsg)
{
  int i;
  char FrameFormatName[16];
  char Status[16];

  switch (CANMsg->mff)
  {
    case BCI_MFF_11_DAT:
      strncpy (FrameFormatName, "11 bit data\0", 12);
      break;
    case BCI_MFF_29_DAT:
      strncpy (FrameFormatName, "29 bit data\0", 12);
      break;
    case BCI_MFF_11_RMT:
      strncpy (FrameFormatName, "11 bit rmt \0", 12);
      break;
    case BCI_MFF_29_RMT:
      strncpy (FrameFormatName, "29 bit rmt \0", 12);
      break;
    case BCI_MFF_STS_MSG:
      strncpy (FrameFormatName, "Status msg \0", 12);
      break;
    default:
      strncpy (FrameFormatName, "???????????\0", 12);
  }

  switch (CANMsg->mff)
  {
    case BCI_MFF_11_DAT:
    case BCI_MFF_29_DAT:
    case BCI_MFF_11_RMT:
    case BCI_MFF_29_RMT:
      PRINTI ("ID [%8x] Fmt [%s] DLC [%d] [",
              CANMsg->id, FrameFormatName, CANMsg->dlc);
      for (i = 0; i < CANMsg->dlc; i++)
        PRINTI ("%02X ", CANMsg->a_data[i]);
      PRINTI ("]");
      break;
    case BCI_MFF_STS_MSG:
      BCI_Status2Str (CANMsg->id, Status);
      PRINTI ("Status [ %s ]", Status);
      break;

    default:
      PRINTI ("Unknown message frame format [ %d ]", CANMsg->mff);
  }
  PRINTI (" Time [%8d.%.8d]\n",
          (CANMsg->time_stamp / 8) / 1000, (CANMsg->time_stamp / 8) % 1000);

  return 0;
}

/*************************************************************************
**
**  Function:
**   BCI_CreateCANMsg
**
**  Description:
**   Create CAN message.
**
**  Parameters:
**   *BCI_ts_CanMsg  (OUT)   CAN object pointer
**   ID           (IN)  Object identifier
**   *Data        (IN)  Databytes of the message
**   DLC          (IN)  data length code (Bit 0..3)
**   RTR          (IN)  remote frame indication (Bit 6)
**
**  Returnvalue:
**    BCI_OK          - OK
**
*************************************************************************/
int BCI_CreateCANMsg (BCI_ts_CanMsg * CANMsg, UINT32 ID,        // Object identifier
                      UINT8 * Data,     // Databytes of the message
                      UINT8 DLC,        // data length code (Bit 0..3)
                      UINT8 MFF)        // Message frame format
{
  CANMsg->id = ID;
  CANMsg->mff = MFF;
  CANMsg->dlc = DLC;
  CANMsg->time_stamp = 0;
  MEMCPY (CANMsg->a_data, Data, DLC);

  return 0;
}

/*************************************************************************
**
** Function:
**  BCI_ResetOverrun
**
** Description:
**  reset CAN controller data and queue overruns
**
** Parameters:
**  *Interface  (I/O)  CAN interface handler
**  CANNum       (IN)  controller number
**
** Returnvalue: BCI_OK       - OK
**              BCI_BUSY     - command is busy
**              BCI_PARA_ERR - incorrect command
**              BCI_TIMEOUT  - timeout reached
**
*************************************************************************/
int BCI_ResetOverrun (BCI_BRD_HDL Interface, UINT8 CANNum)
{
  int ret;
  t_Interface *pInterface = (t_Interface *) Interface;

  PRINTD ("Reset oveeruns for [%d] %s CANNum %d\n",
          pInterface->DeviceFileDesc[0], pInterface->Name, CANNum);

  ret = pInterface->CAN.CheckState (pInterface, CANNum, BCI_RUNNING);

  if (ret != BCI_OK)
  {
    return ret;
  }

  // reset overruns for this interface

  ret = pInterface->CAN.ResetOverrun (pInterface, CANNum);

  ret = BCI_OK;

  return ret;
}

static int InterfaceNumber = 0;

/*************************************************************************
**
**  Function:
**   BCI_OpenBoard
**
**  Description:
**   Open CAN interface and load firmware
**
**  Parameters:
**  *Interface   (I/O)  CAN interface handler
**  *DeviceFile  (IN)  device file name
**
**  Returnvalue:
**    BCI_OK          - OK
**    BCI_PARA_ERR    - incorrect parameters
**    BCI_CREATE_ERR  - interface creating error
**
*************************************************************************/
int BCI_OpenBoard (BCI_BRD_HDL * Interface, char *DeviceFile)
{

  int ret, iface;

  t_Interface *pInterface;

  int DeviceID = DEVICE_UNKNOWN;
  UINT32 FirmwareMode = FW_LOAD_DEFAULT;
  char HEXFileName[64] = "";

  PRINTD ("Ineterface create for %s\n", DeviceFile);

#ifdef BCI_TEST
  if (trace_fd == NULL)
  {
    trace_fd = stdout;
  }
#endif

  if (GetDeviceID (&DeviceID, DeviceFile) != BCI_OK)
  {
    PRINTD ("Error when open device file [%s]\nCheck if driver is loaded\n",
            DeviceFile);

    return (BCI_CREATE_ERR);
  }
  PRINTD ("Interface is : [%d]\n", DeviceID);

  pInterface = (t_Interface *) malloc (sizeof (t_Interface));
  *Interface = (unsigned long) pInterface;

  if (pFirstInterface == NULL)
  {
    pFirstInterface = pInterface;
  }
  else
  {
    pLastInterface->Next = pInterface;
  }

  pLastInterface = pInterface;

  PRINTD ("Interface = %p ptr %p nt ptr %lx\n",
          Interface, pInterface, (BCI_BRD_HDL) pInterface);
  PRINTD ("pFirstInterface = %p \n", pFirstInterface);

  // Common part of interface
  pInterface->InterfaceNumber = InterfaceNumber;
  pInterface->Next = NULL;
  pInterface->Type = DeviceID;
  strcpy (pInterface->DeviceFile[0], DeviceFile);
  //    strcpy(pInterface->Name, DeviceName);
  strcpy (pInterface->Name, "No Name");

  pInterface->CAN.StdMask[0][ACCEPTANCE_FILTER].Mask = 0x0;
  pInterface->CAN.ExtMask[0][ACCEPTANCE_FILTER].Code = 0x0;
  pInterface->CAN.StdMask[0][LIST_REDUCTION_FILTER].Mask = 0x0;
  pInterface->CAN.ExtMask[0][LIST_REDUCTION_FILTER].Code = 0x0;

  pInterface->CAN.StdMask[1][ACCEPTANCE_FILTER].Mask = 0x0;
  pInterface->CAN.ExtMask[1][ACCEPTANCE_FILTER].Code = 0x0;
  pInterface->CAN.StdMask[1][LIST_REDUCTION_FILTER].Mask = 0x0;
  pInterface->CAN.ExtMask[1][LIST_REDUCTION_FILTER].Code = 0x0;

  pInterface->CAN.CheckState = BCI_CheckCANState;
  pInterface->CAN.SetState = BCI_SetCANState;

  for (iface = 0; iface < BCI_MAX_CAN_NUM; iface++)
  {
    pInterface->CAN.BCIState[iface] = BCI_UNCONFIGURED;
  }
  for (iface = 0; iface < BCI_MAX_LIN_NUM; iface++)
  {
    pInterface->LIN.BCIState[iface] = BCI_UNCONFIGURED;
  }

  // Reset all LIN methonds here
  pInterface->LIN.Init = NULL;
  pInterface->LIN.Start = NULL;
  pInterface->LIN.Stop = NULL;
  pInterface->LIN.OpenInterface = NULL;
  pInterface->LIN.CloseInterface = NULL;        

  pInterface->LIN.CheckState = NULL;
  pInterface->LIN.SetState = NULL;

  switch (DeviceID)
  {
    case DEVICE_iPCI165_ISA:
    case DEVICE_iPCI165_PCI:
    case DEVICE_iPCI320_ISA:
    case DEVICE_iPCI320_PCI:
    case DEVICE_iPCIXC161_PCI:
    case DEVICE_iPCIXC161_PCIE:
      PRINTD ("Active board\n");

      pInterface->CAN.OpenInterface = iPCIOpen;
      pInterface->CAN.CloseInterface = iPCIClose;
      pInterface->CAN.Reset = iPCIResetCAN;
      pInterface->CAN.Test = iPCITestCAN;
      pInterface->CAN.Start = iPCIStartCAN;
      pInterface->CAN.Stop = iPCIStopCAN;
      pInterface->CAN.Init = iPCIInitCAN;
      pInterface->CAN.ConfigRxQueue = iPCIConfigRxQueue;
      pInterface->CAN.RegRxID = iPCIRegRxID;
      pInterface->CAN.UnregRxID = iPCIUnregRxID;
      pInterface->CAN.SetAccMask = iPCISetAccMask;
      pInterface->CAN.ReceiveCANObj = iPCIReceiveCANObj;
      pInterface->CAN.TransmitCANObj = iPCITransmitCANObj;
      pInterface->CAN.GetBrdStatus = iPCIGetBrdStatus;
      pInterface->CAN.DownloadFirmware = iPCIDownloadFirmware;
      pInterface->WaitForData = iPCIWaitForData;
      pInterface->CAN.GetBoardInfo = iPCIGetBoardInfo;
      pInterface->CAN.ResetOverrun = iPCIResetOverrun;
      break;
    case DEVICE_PCI03_ISA:
    case DEVICE_PCI04_ISA:
    case DEVICE_PCI04_PCI:
      PRINTD ("Passive board\n");

      pInterface->CAN.OpenInterface = PC_IOpen;
      pInterface->CAN.CloseInterface = PC_IClose;
      pInterface->CAN.Reset = PC_IResetCAN;
      pInterface->CAN.Test = PC_ITestCAN;
      pInterface->CAN.Start = PC_IStartCAN;
      pInterface->CAN.Stop = PC_IStopCAN;
      pInterface->CAN.Init = PC_IInitCAN;
      pInterface->CAN.ConfigRxQueue = PC_IConfigRxQueue;
      pInterface->CAN.RegRxID = PC_IRegRxID;
      pInterface->CAN.UnregRxID = PC_IUnregRxID;
      pInterface->CAN.SetAccMask = PC_ISetAccMask;
      pInterface->CAN.ReceiveCANObj = PC_IReceiveCANObj;
      pInterface->CAN.TransmitCANObj = PC_ITransmitCANObj;
      pInterface->CAN.GetBrdStatus = PC_IGetBrdStatus;
      pInterface->CAN.DownloadFirmware = PC_IDownloadFirmware;
      pInterface->WaitForData = PC_IWaitForData;
      pInterface->CAN.GetBoardInfo = PC_IGetBoardInfo;
      pInterface->StartFirmware = PC_IStartFirmware;
      pInterface->GenerateInterrupt = PC_IGenerateInterrupt;
      pInterface->BoardDPRAMSize = PC_I_DPRAM_SIZE;
      pInterface->BoardMemSize = PC_I_MEM_SIZE;
      strcpy (pInterface->CAN.FirmwareFile, "no_firmware");
      pInterface->CAN.FWArray = NULL;
      pInterface->CAN.ResetOverrun = PC_IResetOverrun;
      FirmwareMode = FW_DO_NOT_LOAD;
      break;
    default:
      PRINTD ("Unknown interface type (active or passive) [%d]\n", DeviceID);

      return (BCI_CREATE_ERR);
  }

  switch (DeviceID)
  {
    case DEVICE_iPCI165_ISA:
      PRINTD ("This is 165 ISA\n");

      pInterface->StartFirmware = iPCIStartFirmware_I165;
      pInterface->GenerateInterrupt = iPCIGenerateInterruptGeneric;

      strcpy (pInterface->CAN.FirmwareFile, "bcii165.H86");
      pInterface->BoardDPRAMSize = iPCI165_DPRAM_SIZE;
      pInterface->BoardMemSize = iPCI165_MEM_SIZE;
      pInterface->CAN.FWArray = BAP_a_Fw165;
      strcpy (pInterface->Name, "iPC-I 165 ISA");
      FirmwareMode = FW_LOAD_DEFAULT;
      break;

    case DEVICE_iPCI320_ISA:
      PRINTD ("This is 320 ISA\n");

      pInterface->StartFirmware = iPCIStartFirmware_I320;
      pInterface->GenerateInterrupt = iPCIGenerateInterruptGeneric;
      strcpy (pInterface->CAN.FirmwareFile, "bcii320.hex");
      pInterface->BoardDPRAMSize = iPCI320_DPRAM_SIZE;
      pInterface->BoardMemSize = iPCI320_MEM_SIZE;
      pInterface->CAN.FWArray = BAP_a_Fw320;
      strcpy (pInterface->Name, "iPC-I 320 ISA");
      FirmwareMode = FW_LOAD_DEFAULT;
      break;

    case DEVICE_iPCI165_PCI:
      PRINTD ("This is 165 PCI\n");

      pInterface->StartFirmware = iPCIStartFirmware_I165;
      pInterface->GenerateInterrupt = iPCIGenerateInterrupt165PCI;
      strcpy (pInterface->CAN.FirmwareFile, "bcii165.H86");
      pInterface->BoardDPRAMSize = iPCI165_DPRAM_SIZE;
      pInterface->BoardMemSize = iPCI165_MEM_SIZE;
      pInterface->CAN.FWArray = BAP_a_Fw165;
      strcpy (pInterface->Name, "iPC-I 165 PCI");
      FirmwareMode = FW_LOAD_DEFAULT;
      break;

    case DEVICE_iPCIXC161_PCI:
    case DEVICE_iPCIXC161_PCIE:
      PRINTD ("This is XC161 PCI\n");

      pInterface->StartFirmware = iPCIStartFirmware_IXC161;
      pInterface->GenerateInterrupt = iPCIGenerateInterruptXC161PCI;
      strcpy (pInterface->CAN.FirmwareFile, "empty");
      pInterface->BoardDPRAMSize = iPCIXC161_DPRAM_SIZE;
      pInterface->BoardMemSize = iPCIXC161_MEM_SIZE;
      pInterface->CAN.FWArray = BAP_a_Fw161XC;

      if (DeviceID == DEVICE_iPCIXC161_PCI)
      {
        strcpy (pInterface->Name, "iPC-I XC161 PCI");
      }
      else
      {        
        strcpy (pInterface->Name, "iPC-I XC161 PCIe");
      }
      
      FirmwareMode = FW_DO_NOT_LOAD;

      /* Set up LIN part */
      pInterface->LIN.CheckState = BCI_CheckLINState;
      pInterface->LIN.SetState = BCI_SetLINState;

      pInterface->LIN.OpenInterface = iPCI_LIN_Open;
      pInterface->LIN.CloseInterface = iPCI_LIN_Close;

      pInterface->LIN.Start = iPCI_LIN_Start;
      pInterface->LIN.Stop = iPCI_LIN_Stop;
      pInterface->LIN.Reset = iPCI_LIN_Reset;
      pInterface->LIN.GetBrdStatus = iPCI_LIN_GetStatusImage;
      pInterface->LIN.Init = iPCI_LIN_Init;
      pInterface->LIN.WriteMsg = iPCI_LIN_WriteMsg;
      pInterface->LIN.ReadMsg = iPCI_LIN_ReadMsg;
      break;

    case DEVICE_iPCI320_PCI:
      PRINTD ("This is 320 PCI\n");
      pInterface->StartFirmware = iPCIStartFirmware_I320;
      pInterface->GenerateInterrupt = iPCIGenerateInterruptGeneric;
      strcpy (pInterface->CAN.FirmwareFile, "bcii320.hex");
      pInterface->BoardDPRAMSize = iPCI320_DPRAM_SIZE;
      pInterface->BoardMemSize = iPCI320_MEM_SIZE;
      pInterface->CAN.FWArray = BAP_a_Fw320;
      strcpy (pInterface->Name, "iPC-I 320 PCI");
      FirmwareMode = FW_LOAD_DEFAULT;
      break;
    case DEVICE_PCI03_ISA:
      PRINTD ("This is PC-I 03 ISA\n");

      strcpy (pInterface->Name, "PC-I 03 ISA");
      break;
    case DEVICE_PCI04_ISA:
      PRINTD ("This is PC-I 04 ISA\n");

      strcpy (pInterface->Name, "PC-I 04 ISA");
      break;
    case DEVICE_PCI04_PCI:
      PRINTD ("This is PC-I 04 PCI\n");

      strcpy (pInterface->Name, "PC-I 04 PCI");
      break;

    default:
      PRINTD ("Unknown interface type [%d]\n", DeviceID);

      return (BCI_CREATE_ERR);
  }
  InterfaceNumber++;

  ret = pInterface->CAN.OpenInterface (pInterface);
  PRINTD ("Open for [%d] %s type %d , memory %p [%s]\n",
          pInterface->DeviceFileDesc[0],
          pInterface->Name,
          pInterface->Type, pInterface->MemoryPtr, (char *) pInterface->MemoryPtr);

  if (ret != BCI_OK)
    return (ret);
  else
  {
    switch (FirmwareMode)
    {
      case FW_DO_NOT_LOAD:
        ret = BCI_OK;
        break;
      case FW_LOAD_DEFAULT:
        ret = (pInterface->CAN.DownloadFirmware
               ((t_Interface *) (*Interface), FirmwareMode, (UINT8 *) ""));
        break;
      case FW_LOAD_DEFAULT_HEX:
        ret = (pInterface->CAN.DownloadFirmware
               ((t_Interface *) (*Interface), FirmwareMode,
                (UINT8 *) pInterface->CAN.FirmwareFile));
        break;
      case FW_LOAD_FROM_HEX:
        ret = (pInterface->CAN.DownloadFirmware
               ((t_Interface *) (*Interface), FirmwareMode, (UINT8 *) HEXFileName));
        break;

      default:
        return (BCI_PARA_ERR);
    }
  }
  PRINTD ("Firmware was downloaded with result [%d]\n", ret);

  if (ret == BCI_OK)
  {
    PRINTD ("Test for [%d] %s\n", pInterface->DeviceFileDesc[0], pInterface->Name);

    ret = (pInterface->CAN.Test (pInterface));
    if (ret == BCI_OK)
    {
      for (iface = 0; iface < BCI_MAX_CAN_NUM; iface++)
      {
        pInterface->CAN.SetState (pInterface, iface, BCI_BOARD_INITIALIZED);
      }
    }
  }

  if (pInterface->LIN.OpenInterface != NULL)
  {
    if (pInterface->LIN.OpenInterface (pInterface) == BCI_OK)
    {
      PRINTD ("LIN init Success !\n");
      for (iface = 0; iface < BCI_MAX_LIN_NUM; iface++)
      {
        pInterface->LIN.SetState (pInterface, iface, BCI_BOARD_INITIALIZED);
      }      
    }
    else
    {
      PRINTD ("LIN init Failed !\n");
    }
  }
      
  return ret;

}

/*************************************************************************
**
**  Function:
**   BCI_CloseBoard
**
**  Description:
**   Close CAN interface
**   Delete CAN interface object and free memory
**
**  Parameters:
**   *Interface   (I/O)  CAN interface handler
**
**  Returnvalue:
**    BCI_OK          - OK
**
*************************************************************************/
int BCI_CloseBoard (BCI_BRD_HDL Interface)
{

  t_Interface *PrevInterface = NULL;
  int ret;

  t_Interface *pInterface = (t_Interface *) Interface;

  PRINTD ("Close for [%d] %s, memory %p [%s]\n",
          pInterface->DeviceFileDesc[0],
          pInterface->Name, pInterface->MemoryPtr, (char *) pInterface->MemoryPtr);

  // Call close function for this interface
  ret = pInterface->CAN.CloseInterface (pInterface);

  if (ret != BCI_OK)
  {

    PRINTD ("Interface close error!\n");

    return ret;
  }

  pInterface->CAN.SetState (pInterface, 0, BCI_UNCONFIGURED);
  pInterface->CAN.SetState (pInterface, 1, BCI_UNCONFIGURED);

  PRINTD ("Interface delete for %s\n", pInterface->Name);

  // Now we need to delete this interface from list.
  // After that -> Next of previous interface should be equal to ->Next
  // of deleted interface

  pCurrentInterface = pFirstInterface;      // current interface is 1-st
  // intreface in the list
  PrevInterface = NULL;                     // previous board is NULL

  while (1)
  {
    PRINTD ("process : %s\n", (pCurrentInterface->Name));

    if (pInterface == pCurrentInterface)
    {
      // That's interface we need
      PRINTD ("It's Interface we need.\n");

      if ((PrevInterface == NULL) && (pCurrentInterface->Next == NULL))
      {
        PRINTD ("Only this interface left\n");

        pFirstInterface = NULL;
        pLastInterface = NULL;
        break;
      }

      if ((PrevInterface != NULL) && (pCurrentInterface->Next == NULL))
      {
        PRINTD ("This interface is last\n");

        PrevInterface->Next = NULL;         // Previous one should
        pLastInterface = PrevInterface;     // be last
        break;
      }

      if ((PrevInterface == NULL) && (pCurrentInterface->Next != NULL))
      {
        PRINTD ("This interface is first\n");

        pFirstInterface = pCurrentInterface->Next;      // It was 1-st
        // interface.
        // After deleting ->Next
        // should be 1-st.                        
        break;
      }

      if ((PrevInterface != NULL) && (pCurrentInterface->Next != NULL))
      {
        PRINTD ("This interface is inside list\n");

        PrevInterface->Next = pCurrentInterface->Next;  // Change
        // pointer
        break;
      }

    }

    else
    {                                       // It's another interface
      PRINTD ("pCurrentInterface->Next %p \n", pCurrentInterface->Next);

      if (pCurrentInterface->Next != NULL)
      {                                     // Is it last ?
        PRINTD ("Take Next.\n");

        PrevInterface = pCurrentInterface;  // Save previous interface
        pCurrentInterface = pCurrentInterface->Next;    // No, take Next
      }
      else
      {
        PRINTD ("Interface %p not found.\n", pCurrentInterface);

        return (BCI_LIST_ERR);
      }
    }
  }
  PRINTD ("Brd %p Cur %p Prev %p First %p Last %p\n",
          (t_Interface *) Interface, pCurrentInterface, PrevInterface,
          pFirstInterface, pLastInterface);

  free ((int *) Interface);
  PRINTD ("Deleting OK for %s\n", pInterface->Name);

  return (BCI_OK);
}

/*************************************************************************
**
** Function:
**  BCI_StartCan
**
** Description:
**  start CAN controller
**
** Parameters:
**  *Interface  (I/O)  CAN interface handler
**  CANNum       (IN)  controller number
**
** Returnvalue: BCI_OK       - OK
**              BCI_BUSY     - command is busy
**              BCI_PARA_ERR - incorrect command
**              BCI_TIMEOUT  - timeout reached
**
*************************************************************************/
int BCI_StartCan (BCI_BRD_HDL Interface, UINT8 CANNum)
{
  int ret;
  t_Interface *pInterface = (t_Interface *) Interface;

  PRINTD ("Start for [%d] %s CANNum %d\n",
          pInterface->DeviceFileDesc[0], pInterface->Name, CANNum);

  ret = pInterface->CAN.CheckState (pInterface, CANNum, BCI_INITIALIZED);

  if (ret != BCI_OK)
  {
    return ret;
  }

  // Call start function for this interface

  ret = (pInterface->CAN.Start ((t_Interface *) Interface, CANNum));

  if (ret == BCI_OK)
  {
    pInterface->CAN.SetState (pInterface, CANNum, BCI_RUNNING);
  }
  return ret;
}

/*************************************************************************
**
** Function:
**  BCI_StopCan
**
** Description:
**  stop CAN controller
**
** Parameters:
**  *Interface  (I/O)  CAN interface handler
**  CANNum       (IN)  controller number
**
** Returnvalue: BCI_OK       - OK
**              BCI_BUSY     - command is busy
**              BCI_PARA_ERR - incorrect command
**              BCI_TIMEOUT  - timeout reached
**
*************************************************************************/
int BCI_StopCan (BCI_BRD_HDL Interface, UINT8 CANNum)
{
  int ret;
  t_Interface *pInterface = (t_Interface *) Interface;

  PRINTD ("Stop for [%d] %s CANNum %d\n",
          pInterface->DeviceFileDesc[0], pInterface->Name, CANNum);

  ret = pInterface->CAN.CheckState (pInterface, CANNum, BCI_RUNNING);

  if (ret != BCI_OK)
  {
    return ret;
  }

  // Call stop function for this interface

  ret = (pInterface->CAN.Stop ((t_Interface *) Interface, CANNum));

  if (ret == BCI_OK)
  {
    pInterface->CAN.SetState (pInterface, CANNum, BCI_INITIALIZED);
  }
  return ret;
}

/*************************************************************************
**
** Function:
**  BCI_ResetCan
**
** Description:
**  reset CAN controller
**
** Parameters:
**  *Interface  (I/O)  CAN interface handler
**  CANNum       (IN)  controller number
**
** Returnvalue: BCI_OK       - OK
**              BCI_BUSY     - command is busy
**              BCI_PARA_ERR - incorrect command
**              BCI_TIMEOUT  - timeout reached
**
*************************************************************************/
int BCI_ResetCan (BCI_BRD_HDL Interface, UINT8 CANNum)
{

  int ret;
  t_Interface *pInterface = (t_Interface *) Interface;

  PRINTD ("Reset for [%d] %s CANNum %d\n",
          pInterface->DeviceFileDesc[0], pInterface->Name, CANNum);

  ret = pInterface->CAN.CheckState (pInterface, CANNum,
                                    (BCI_INITIALIZED | BCI_RUNNING |
                                     BCI_BOARD_INITIALIZED));

  if (ret != BCI_OK)
  {
    return ret;
  }

  // Call Reset function for this interface
  ret = (pInterface->CAN.Reset ((t_Interface *) Interface, CANNum));

  if (ret == BCI_OK)
  {
    pInterface->CAN.SetState (pInterface, CANNum, BCI_BOARD_INITIALIZED);
  }

  return ret;

}

/*************************************************************************
**
** Function:
**  BCI_InitCanCAN
**
** Description:
**  init CAN controller
** Parameters:
**  *Interface  (I/O)  CAN interface handler
**   CANNum      (IN)  controller number (0, 1)
**   mode        (IN) - 11-Bit Identifier (BCI_11BIT)
**                        29-Bit-Identifier (BCI_29BIT)
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
*************************************************************************/
int BCI_InitCan (BCI_BRD_HDL Interface, UINT8 CANNum, UINT8 bt0, UINT8 bt1,
                 UINT8 mode)
{
  int ret;
  t_Interface *pInterface = (t_Interface *) Interface;

  PRINTD ("Init for [%d] %s CANNum %d [%d %d]\n",
          pInterface->DeviceFileDesc[0], pInterface->Name, CANNum, bt0, bt1);

  ret = pInterface->CAN.CheckState (pInterface, CANNum, BCI_BOARD_INITIALIZED);

  if (ret != BCI_OK)
  {
    return ret;
  }

  // Call Init function for this interface
  ret = pInterface->CAN.Init ((t_Interface *) Interface, CANNum, bt0, bt1, mode);

  if (ret == BCI_OK)
  {
    pInterface->CAN.SetState (pInterface, CANNum, BCI_INITIALIZED);

  }

  return ret;

}

/*************************************************************************
**
** Function:
**  BCI_ConfigRxQueue
**
** Description:
**  init CAN controller
** Parameters:
**  *Interface  (I/O)  CAN interface handler
**   CANNum      (IN)  controller number (0, 1)
**   int_mode    (IN) - interrupt mode MC to PC
**
**                  - BCI_POLL_MODE:
**                    use this mode, when no interrupt
**                    service routine is installed, the
**                    function ReceiveCanObj() must
**                    be called cyclic to get the
**                    received CAN messages
**
**                  - BCI_LATENCY_MODE:
**                    use this mode, when a interrupt
**                    service routine is installed, the
**                    interrupt service routine must
**                    call the function
**                    iPCIReceiveCanObj() to get the
**                    received CAN messages. Note, that
**                    you must call the function
**                    iPCIReceiveCanObj() as long as
**                    CAN messages are received. After
**                    every received CAN message an
**                    interrupt to the PC is generated.
**                    This mode guaranted the best
**                    reactivity time.
**
**                  - BCI_THROUGHPUT_MODE:
**                    use this mode, when a interrupt
**                    service routine is installed, the
**                    interrupt service routine must
**                    call the function
**                    iPCIReceiveCanObj() to get the
**                    received CAN messages. Note, that
**                    you must call the function
**                    iPCIReceiveCanObj() as long as
**                    CAN messages are received. Only
**                    after a defined number of received
**                    CAN message an interrupt to the PC
**                    is generated.
**                    This mode guaranted the best
**                    data throughput.
**
** Returnvalue: BCI_OK       - OK
**              BCI_BUSY     - command is busy
**              BCI_PARA_ERR - incorrect command
**              BCI_TIMEOUT  - timeout reached
**
*************************************************************************/
int BCI_ConfigRxQueue (BCI_BRD_HDL Interface, UINT8 CANNum, UINT8 IntMode)
{
  int ret;
  t_Interface *pInterface = (t_Interface *) Interface;

  PRINTD ("ConfigRxQueue for [%d] %s CANNum %d IntMode %d\n",
          pInterface->DeviceFileDesc[0], pInterface->Name, CANNum, IntMode);

  ret = pInterface->CAN.CheckState (pInterface, CANNum,
                                    (BCI_INITIALIZED | BCI_RUNNING));
  if (ret != BCI_OK)
  {
    return ret;
  }
  // Call ConfigRxQueue function for this interface
  return (pInterface->
          CAN.ConfigRxQueue ((t_Interface *) Interface, CANNum, IntMode));

}

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
int BCI_RegisterRxId (BCI_BRD_HDL Interface, UINT8 CANNum, UINT8 mff, UINT32 ID)
{
  int ret;
  t_Interface *pInterface = (t_Interface *) Interface;

  PRINTD ("Register filter entry for [%d] %s CANNum %d [%d %d]\n",
          pInterface->DeviceFileDesc[0], pInterface->Name, CANNum, mff, ID);

  ret = pInterface->CAN.CheckState (pInterface, CANNum, BCI_INITIALIZED);
  if (ret != BCI_OK)
  {
    return ret;
  }

  // Call RegRxID function for this interface
  return (pInterface->CAN.RegRxID ((t_Interface *) Interface, CANNum, mff, ID));

}

/*************************************************************************
**
** Function    : BCI_UnregisterRxId
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
int BCI_UnregisterRxId (BCI_BRD_HDL Interface, UINT8 CANNum, UINT8 mff, UINT32 ID)
{
  int ret;
  t_Interface *pInterface = (t_Interface *) Interface;

  PRINTD ("Unregister filter entry for [%d] %s CANNum %d [%d %d]\n",
          pInterface->DeviceFileDesc[0], pInterface->Name, CANNum, mff, ID);

  ret = pInterface->CAN.CheckState (pInterface, CANNum, BCI_INITIALIZED);
  if (ret != BCI_OK)
  {
    return ret;
  }

  // Call UnregRxID function for this interface
  return (pInterface->CAN.UnregRxID ((t_Interface *) Interface, CANNum, mff, ID));

}

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
** Parameters  : brd_hdl  (IN) - board handle
**               can_num  (IN) - number of the CAN controller (0,1)
**               type     (IN) - type of filter mask (BCI_11B_MASK,
**                               BCI_29B_MASK)
**               acc_code (IN) - acceptance code value (left adjusted)
**               acc_mask (IN) - acceptance mask value (left adjusted)
**                               Two special masks are defined: 
**                               Accept all identifiers (BCI_ACC_ALL)
**                               Reject all identifier  (BCI_REJECT_ALL)
**                               When any of these masks is specified,
**                               the list of registered identifiers is ignored.   
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

int BCI_SetAccMask (BCI_BRD_HDL Interface,
                    UINT8 CANNum, UINT8 type, UINT32 acc_code, UINT32 acc_mask)
{
  int ret;
  t_Interface *pInterface = (t_Interface *) Interface;

  PRINTD ("Set global filter mask for [%d] %s CANNum %d [%d]\n",
          pInterface->DeviceFileDesc[0], pInterface->Name, CANNum, acc_mask);

  ret = pInterface->CAN.CheckState (pInterface, CANNum, BCI_INITIALIZED);
  if (ret != BCI_OK)
  {
    return ret;
  }

  // Call SetAccMask for this interface
  return (pInterface->CAN.SetAccMask
          ((t_Interface *) Interface, CANNum, ACCEPTANCE_FILTER, type,
           acc_code, acc_mask));

}

/*************************************************************************
**
** Function:
**  BCI_ReceiveCanMsg
**
** Description:
**  receive CAN object.
**  Try to find unprocessed object in the receive queue;
**  if there is no such objects, return BCI_NO
**
** Parameters:
**  *Interface          (I/O)  CAN interface handler
**  CANNum               (IN)  controller number
**  *ReceivedCANMsg     (OUT)   received CAN object
**
** Returnvalue: BCI_OK          - OK
**              BCI_NO - receive queue is empty
**
*************************************************************************/
int
BCI_ReceiveCanMsg (BCI_BRD_HDL Interface, UINT8 CANNum,
                   BCI_ts_CanMsg * ReceivedCANMsg, int Timeout)
{

  int ret = BCI_NO;
  t_Interface *pInterface = (t_Interface *) Interface;

  //PRINTD ("Receive CAN object for [%d] %s CANNum %d \n",
    //      pInterface->DeviceFileDesc[0], pInterface->Name, CANNum);

// Check queue first       
  ret = (pInterface->CAN.ReceiveCANObj
         ((t_Interface *) Interface, CANNum, ReceivedCANMsg));
  if (ret == BCI_OK)
  {
    return ret;
  }                                         // We have message in queue

  if (Timeout != BCI_NO_WAIT)
  {                                         // Queue is empty, lets wait for Timeout
    //PRINTD ("WaitForData for [%d] %s\n",
      //      pInterface->DeviceFileDesc[0], pInterface->Name);

    ret = (pInterface->WaitForData ((t_Interface *) Interface, CANNum, Timeout));
    if (ret == BCI_OK)
    {
      /* There was a message received during timeout */
      ret = (pInterface->CAN.ReceiveCANObj
             ((t_Interface *) Interface, CANNum, ReceivedCANMsg));
    }

  }

  return ret;
}

/*************************************************************************
**
** Function:
** BCI_TransmitCanMsg
**
** Description:
**  transmit CAN object.
**  Try to find free slot in the transmit queue;
**  if there is no free slots, return BCI_BUSY
**
** Parameters:
**  *Interface          (I/O)  CAN interface handler
**  CANNum               (IN)  controller number
**  *ToSendCANMsg        (IN)  received CAN object
**
** Returnvalue: BCI_OK          - OK
**              BCI_BUSY  - transmit queue is full
**
*************************************************************************/
int
BCI_TransmitCanMsg (BCI_BRD_HDL Interface, UINT8 CANNum,
                    BCI_ts_CanMsg * ToSendCANMsg)
{
  t_Interface *pInterface = (t_Interface *) Interface;

  return (pInterface->CAN.TransmitCANObj
          ((t_Interface *) Interface, CANNum, ToSendCANMsg));
}

/*************************************************************************
**
** Function:
**  BCI_GetBoardStatus
**
** Description:
**  get CAN board status
**
** Parameters:
**  *Interface          (I/O)  CAN interface handler
**  *BrdStatus           (IN)  current CAN board status
**
** Returnvalue: BCI_OK          - OK
**              BCI_BUSY        - semaphor set failed
**
*************************************************************************/
int BCI_GetBoardStatus (BCI_BRD_HDL Interface, BCI_ts_BrdSts * BrdStatus)
{
  t_Interface *pInterface = (t_Interface *) Interface;

  PRINTD ("Status for [%d] %s\n", pInterface->DeviceFileDesc[0], pInterface->Name);

  return (pInterface->CAN.GetBrdStatus ((t_Interface *) Interface, BrdStatus));

}

/*************************************************************************
**
** Function:
**  BCI_GetBoardInfo
**
** Description:
**  get CAN board info
**
** Parameters:
**  *Interface          (I/O)  CAN interface handler
**  *brd_info           (IN)   CAN board info
**
** Returnvalue: BCI_OK          - OK
**              BCI_BUSY        - semaphor set failed
**
*************************************************************************/
int BCI_GetBoardInfo (BCI_BRD_HDL Interface, BCI_ts_BrdInfo * brd_info)
{

  t_Interface *pInterface = (t_Interface *) Interface;

  PRINTD ("Get board info for [%d] %s, memory %p [%s]\n",
          pInterface->DeviceFileDesc[0],
          pInterface->Name, pInterface->MemoryPtr, (char *) pInterface->MemoryPtr);

  return (pInterface->CAN.GetBoardInfo ((t_Interface *) Interface, brd_info));
}

/*************************************************************************
**
** Function:
**  BCIError
**
** Description:
**  print error message according error code
**
** Parameters:
**  Error      (IN)  error number
**
** Returnvalue:
**               BCI_OK          - OK
**
*************************************************************************/
const char *BCI_GetErrorString (int Error)
{
  MEMSET (BCIErrorString, 0x0, ERROR_STRING_LEN);
  switch (Error)
  {
    case BCI_OK:
      strcpy (BCIErrorString, "OK");
      break;
    case BCI_BUSY:
      strcpy (BCIErrorString, "No access to the buffer or Receive queue is empty");
      break;
    case BCI_RESP_ERR:
      strcpy (BCIErrorString, "No response");
      break;
    case BCI_PARA_ERR:
      strcpy (BCIErrorString, "Parameter error");
      break;
    case BCI_USER_ERR:
      strcpy (BCIErrorString, "User error");
      break;
    case BCI_TIMER_ERR:
      strcpy (BCIErrorString, "Wrong timer state");
      break;
// Linux specific errors        
    case BCI_LIST_ERR:
      strcpy (BCIErrorString, "Interface list error");
      break;
    case BCI_CREATE_ERR:
      strcpy (BCIErrorString, "Interface create error");
      break;
    case BCI_FW_ERR:
      strcpy (BCIErrorString, "Firmware load error");
      break;
    case BCI_LOADER_TEST_ERR:
      strcpy (BCIErrorString, "Loader test failed");
      break;
    case BCI_INT_TEST_ERR:
      strcpy (BCIErrorString, "Interrupt MC->PC test failed");
      break;
    case BCI_SEMA_TEST_ERR:
      strcpy (BCIErrorString, "Semaphore test failed");
      break;

    default:
      sprintf (BCIErrorString, "Unknown error code %d", Error);
  }
  strcat (BCIErrorString, "\n");

  return BCIErrorString;
}

/*************************************************************************
**
** Function    : BCI_GetCanStatus
**
** Description : This function reads CAN status register from the DPRAM,
**               which is cyclically updated.
**
** Parameters  : brd_hdl (IN)  - board handle
**               can_num (IN)  - number of the CAN controller (0,1)
**               can_sts (OUT) - pointer to store the status:
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
int BCI_GetCanStatus (BCI_BRD_HDL Interface, UINT8 CANNum, UINT16 * CANSts)
{

  BCI_ts_BrdSts BrdStatus;
  int ret;

  ret = BCI_GetBoardStatus (Interface, &BrdStatus);

  if (ret != BCI_OK)
  {
    return ret;
  }

  switch (CANNum)
  {
    case (0):
      *CANSts = BrdStatus.can0_status;
      return BCI_OK;
      break;
    case (1):
      *CANSts = BrdStatus.can1_status;
      return BCI_OK;
      break;

    default:
      return BCI_PARA_ERR;

  }
}

/*************************************************************************
**
** Function    : BCI_CheckCANState
**
** Description : Compare current BCI state with expected
**
** Parameters  : *Interface          (I/O)  CAN interface handler
**               CANNum               (IN)  controller number
**               StateExpected        (IN)  expected BCI state
** Returnvalue : BCI_OK
**               
**
*************************************************************************/
static int BCI_CheckCANState (t_Interface * pInterface, UINT8 CANNum,
                              UINT8 StateExpected)
{

  UINT16 *State;

#ifdef BCI_TEST
  char StateStr[STATE_STR_SIZE], StateExpStr[STATE_STR_SIZE];
#endif

  State = &(pInterface->CAN.BCIState[CANNum]);

#ifdef BCI_TEST

  State2Str (*State, StateStr);
  State2Str (StateExpected, StateExpStr);

  sprintf (TestMsgFull, "     State is %s, allowed %s\n", StateStr, StateExpStr);
#endif

  if (((*State) & StateExpected) == 0)
  {
#ifdef BCI_TEST
    sprintf (TestMsg, "     State error\n");
    strcat (TestMsgFull, TestMsg);
    fprintf (trace_fd, TestMsgFull);
#endif
    return BCI_STATE_ERR;
  }
  else
  {
#ifdef BCI_TEST
    sprintf (TestMsg, "     State OK\n");
    strcat (TestMsgFull, TestMsg);
    fprintf (trace_fd, TestMsgFull);
#endif
    return BCI_OK;
  }
}

/*************************************************************************
**
** Function    : BCI_SetCANState
**
** Description : Set BCI state
**
** Parameters  : *Interface          (I/O)  CAN interface handler
**               CANNum               (IN)  controller number
**               State                (IN)  new BCI state  
** Returnvalue : BCI_OK
**               
**
*************************************************************************/
static int BCI_SetCANState (t_Interface * pInterface, UINT8 CANNum, UINT8 State)
{
#ifdef BCI_TEST
  char StateFrom[STATE_STR_SIZE], StateTo[STATE_STR_SIZE];

  State2Str (pInterface->CAN.BCIState[CANNum], StateFrom);
  State2Str (State, StateTo);
  sprintf (TestMsgFull, "     State changed from  %s to  %s\n", StateFrom, StateTo);
  fprintf (trace_fd, TestMsgFull);
#endif

  pInterface->CAN.BCIState[CANNum] = State;

  return BCI_OK;
}

/*************************************************************************
**
** Function    : BCI_CheckLINState
**
** Description : Compare current BCI state with expected
**
** Parameters  : *Interface          (I/O)  interface handler
**               LINNum               (IN)  controller number
**               StateExpected        (IN)  expected BCI state
** Returnvalue : BCI_OK
**               
**
*************************************************************************/
static int BCI_CheckLINState (t_Interface * pInterface, UINT8 LINNum,
                              UINT8 StateExpected)
{

  UINT16 *State;

#ifdef BCI_TEST
  char StateStr[STATE_STR_SIZE], StateExpStr[STATE_STR_SIZE];
#endif

  State = &(pInterface->LIN.BCIState[LINNum]);

#ifdef BCI_TEST

  State2Str (*State, StateStr);
  State2Str (StateExpected, StateExpStr);

  sprintf (TestMsgFull, "     State is %s, allowed %s\n", StateStr, StateExpStr);
#endif

  if (((*State) & StateExpected) == 0)
  {
#ifdef BCI_TEST
    sprintf (TestMsg, "     State error\n");
    strcat (TestMsgFull, TestMsg);
    fprintf (trace_fd, TestMsgFull);
#endif
    return BCI_STATE_ERR;
  }
  else
  {
#ifdef BCI_TEST
    sprintf (TestMsg, "     State OK\n");
    strcat (TestMsgFull, TestMsg);
    fprintf (trace_fd, TestMsgFull);
#endif
    return BCI_OK;
  }
}

/*************************************************************************
**
** Function    : BCI_SetLINState
**
** Description : Set BCI state
**
** Parameters  : *Interface          (I/O)  interface handler
**               LINNum               (IN)  controller number
**               State                (IN)  new BCI state  
** Returnvalue : BCI_OK
**               
**
*************************************************************************/
static int BCI_SetLINState (t_Interface * pInterface, UINT8 LINNum, UINT8 State)
{
#ifdef BCI_TEST
  char StateFrom[STATE_STR_SIZE], StateTo[STATE_STR_SIZE];

  State2Str (pInterface->LIN.BCIState[LINNum], StateFrom);
  State2Str (State, StateTo);
  sprintf (TestMsgFull, "     State changed from  %s to  %s\n", StateFrom, StateTo);
  fprintf (trace_fd, TestMsgFull);
#endif

  pInterface->LIN.BCIState[LINNum] = State;

  return BCI_OK;
}

/*************************************************************************
**
** Function    : BCI_Init
**
** Description : Init the BCI
**
** Parameters  : none
** Returnvalue : BCI_OK
**               
**
*************************************************************************/
int BCI_Init (void)
{
  return BCI_OK;
}

#ifdef BCI_TEST

int SetTestLogFile (FILE * TestFile)
{
  trace_fd = TestFile;
  return BCI_OK;
}

#endif

/*************************************************************************
**
** Function    : BCI_Status2Str
**
** Description : Convert the CAN controller status byte to string.
**
** Parameters  : status     (IN)  - CAN controller status
**               status_str (OUT) - Pointer to the resulting status string
** Returnvalue : BCI_OK           - OK
**
*************************************************************************/
int State2Str (int State, char *StateStr)
{

  MEMSET (StateStr, 0x0, STATE_STR_SIZE);
  sprintf (StateStr, ((State) & BCI_UNCONFIGURED) ? "UNCONF " : "");
  strcat (StateStr, ((State) & BCI_BOARD_INITIALIZED) ? "BRDINI " : "");
  strcat (StateStr, ((State) & BCI_INITIALIZED) ? "CANINI " : "");
  strcat (StateStr, ((State) & BCI_RUNNING) ? "CANRUN " : "");

  return BCI_OK;
}

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
int GetDeviceID (int *ID, char *DeviceName)
{
  int ret, device_fd;           // Char device file descriptor

  if ((device_fd = open (DeviceName, O_RDWR)) == -1)
  {
    PRINTD ("Device [%s] not found !\n", DeviceName);
    return (BCI_SERV_ERR);
  }

  ret = ioctl (device_fd, IOCTL_RESET, 0);  // Reset device
  BCI_MDelay (200);
  ret = ioctl (device_fd, IOCTL_GET_ID, ID);    // Get device ID

  PRINTD ("Received [%d] from driver\n", *ID);

  close (device_fd);

  return BCI_OK;
}
