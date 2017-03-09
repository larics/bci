
/***************************************************************************
      newdemo.c  -  demo application for 2-controller
              boards  

                             -------------------
    begin                : Mon Apr 2 2001
    copyright            : (C) 2001 IXXAT Automation GmbH
    email                : kuzmich@ixxat.de
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bci.h"

#define CAN0_ID 0x10
#define CAN1_ID 0x11
#define TEST_MSG_NUM 3

int BCI_Status2Str (UINT16 Status, char *StatusStr);
int BCI_ShowCANMsg (BCI_ts_CanMsg * CANMsg);
int BCI_CreateCANMsg (BCI_ts_CanMsg * CANMsg, UINT32 ID, UINT8 * Data,  // Databytes of the message
                      UINT8 DLC,        // data length code (Bit 0..3)
                      UINT8 MFF);       // Message frame format
void BCI_MDelay (unsigned int msec);

int main (int argc, char *argv[])
{
  BCI_BRD_HDL CANBoard;
  char DevFileName[64] = "/dev/can0";

  UINT8 CANSpeed, Controller = 0;

  UINT8 PacketData[8], Value0 = 0, Value1 = 0;

  int TxCounter0 = 0, RxCounter0 = 0, TxCounter1 = 0, RxCounter1 = 0, ret;

  BCI_ts_CanMsg ReceivedCANMsg0, ToSendCANMsg0, ReceivedCANMsg1, ToSendCANMsg1;

  BCI_ts_BrdSts BrdStatus;

  char CAN0TransmitOK = BCI_OK, CAN1TransmitOK = BCI_OK;

  char Status0[16], Status1[16];

  if (argv[1] != NULL)
  {
    strcpy (DevFileName, argv[1]);
  }

  printf ("Trying to open device file [%s]\n", DevFileName);

  printf ("Open board\n");
  ret = BCI_OpenBoard (&CANBoard, DevFileName);
  if (ret != BCI_OK)
  {
    printf (BCI_GetErrorString (ret));
    return -1;
  }

/*========================================================================*/

  printf ("Configure CAN0 controller and start it:\n");
  BCI_MDelay (1500);
  Controller = 0;
  CANSpeed = 0;

  printf ("Controller [%d] Init\n", Controller);
  BCI_InitCan (CANBoard, Controller, BCI_125KB, CANSpeed);
  printf ("Controller [%d] ConfigRxQueue\n", Controller);
  BCI_ConfigRxQueue (CANBoard, Controller, BCI_POLL_MODE);

  BCI_SetAccMask (CANBoard, Controller, BCI_11B_MASK, 0, BCI_ACC_ALL);
  BCI_SetAccMask (CANBoard, Controller, BCI_29B_MASK, 0, BCI_ACC_ALL);

  printf ("Controller [%d] Start\n", Controller);
  BCI_StartCan (CANBoard, Controller);

  printf ("Configure CAN1 controller and start it :\n");
  BCI_MDelay (1500);
  Controller = 1;
  CANSpeed = 0;

  printf ("Controller [%d] Init\n", Controller);
  BCI_InitCan (CANBoard, Controller, BCI_125KB, CANSpeed);
  printf ("Controller [%d] ConfigRxQueue\n", Controller);
  BCI_ConfigRxQueue (CANBoard, Controller, BCI_POLL_MODE);

  BCI_SetAccMask (CANBoard, Controller, BCI_11B_MASK, 0, BCI_ACC_ALL);
  BCI_SetAccMask (CANBoard, Controller, BCI_29B_MASK, 0, BCI_ACC_ALL);
  printf ("Controller [%d] Start\n", Controller);
  BCI_StartCan (CANBoard, Controller);

  printf ("Transmit / receive messages\n");

  while ((TxCounter0 < TEST_MSG_NUM) && (TxCounter1 < TEST_MSG_NUM))
  {

    printf ("CAN0 TX: ");
    if (CAN0TransmitOK == BCI_OK)
    {

      // C0 means CAN0 packages. For presentation purposes only.
      PacketData[0] = 0xC0;
      memset (PacketData + 1, Value0, 7);
      BCI_CreateCANMsg (&ToSendCANMsg0, CAN0_ID, PacketData, 8, BCI_MFF_11_DAT);
      BCI_ShowCANMsg (&ToSendCANMsg0);
      Value0 = (Value0 + 1) % 0xFF;
    }
    else
      printf ("Try again previous message \n");

    CAN0TransmitOK = BCI_TransmitCanMsg (CANBoard, 0, &ToSendCANMsg0);

    BCI_MDelay (750);

    if (CAN0TransmitOK == BCI_OK)
      TxCounter0++;

    printf ("CAN1 TX: ");
    if (CAN1TransmitOK == BCI_OK)
    {
      PacketData[0] = 0xC1;
      memset (PacketData + 1, Value1, 7);
      BCI_CreateCANMsg (&ToSendCANMsg1, CAN1_ID, PacketData, 8, BCI_MFF_11_DAT);
      BCI_ShowCANMsg (&ToSendCANMsg1);
      Value1 = (Value1 + 1) % 0xFF;
    }
    else
      printf ("Try again previous message \n");

    CAN1TransmitOK = BCI_TransmitCanMsg (CANBoard, 1, &ToSendCANMsg1);

    BCI_MDelay (750);

    if (CAN1TransmitOK == BCI_OK)
      TxCounter1++;

    printf ("CAN0 RX: ");
    if (BCI_ReceiveCanMsg (CANBoard, 0, &ReceivedCANMsg0, BCI_NO_WAIT) == BCI_OK)
    {
      BCI_ShowCANMsg (&ReceivedCANMsg0);
      RxCounter0++;
    }
    else
    {
      printf ("Nothing received !\n");
    }

    printf ("CAN1 RX: ");
    if (BCI_ReceiveCanMsg (CANBoard, 1, &ReceivedCANMsg1, BCI_NO_WAIT) == BCI_OK)
    {
      BCI_ShowCANMsg (&ReceivedCANMsg1);
      RxCounter1++;
    }
    else
    {
      printf ("Nothing received !\n");
    }

    BCI_GetBoardStatus (CANBoard, &BrdStatus);
    BCI_Status2Str (BrdStatus.can0_status, Status0);
    BCI_Status2Str (BrdStatus.can1_status, Status1);

    printf
      ("Status : CAN0 Ld[%d] St[%s] CAN1 Ld[%d] St[%s] CPU Load [%d] Counter [%d]\n",
       (UINT16) BrdStatus.can0_busload,
       Status0,
       (UINT16) BrdStatus.can1_busload,
       Status1, (UINT16) BrdStatus.cpu_load, (UINT16) BrdStatus.counter);

    printf ("--------------------------------------------------------\n");

  }

  printf ("CAN0: transmitted [%d] received [%d]\n", TxCounter0, RxCounter0);

  printf ("CAN1: transmitted [%d] received [%d]\n", TxCounter1, RxCounter1);

/*========================================================================*/
  RxCounter0 = 0;
  RxCounter1 = 0;
  TxCounter0 = 0;
  TxCounter1 = 0;
  printf ("Now send 29bit ID messages\n");

  printf ("Transmit / receive messages\n");

  while ((TxCounter0 < TEST_MSG_NUM) && (TxCounter1 < TEST_MSG_NUM))
  {

    printf ("CAN0 TX: ");
    if (CAN0TransmitOK == BCI_OK)
    {

      // C0 means CAN0 packages. For presentation purposes only.
      PacketData[0] = 0xC0;
      memset (PacketData + 1, Value0, 7);
      BCI_CreateCANMsg (&ToSendCANMsg0, CAN0_ID, PacketData, 8, BCI_MFF_29_DAT);
      BCI_ShowCANMsg (&ToSendCANMsg0);
      Value0 = (Value0 + 1) % 0xFF;
    }
    else
      printf ("Try again previous message \n");

    CAN0TransmitOK = BCI_TransmitCanMsg (CANBoard, 0, &ToSendCANMsg0);

    BCI_MDelay (750);

    if (CAN0TransmitOK == BCI_OK)
      TxCounter0++;

    printf ("CAN1 TX: ");
    if (CAN1TransmitOK == BCI_OK)
    {
      PacketData[0] = 0xC1;
      memset (PacketData + 1, Value1, 7);
      BCI_CreateCANMsg (&ToSendCANMsg1, CAN1_ID, PacketData, 8, BCI_MFF_29_DAT);
      BCI_ShowCANMsg (&ToSendCANMsg1);
      Value1 = (Value1 + 1) % 0xFF;
    }
    else
      printf ("Try again previous message \n");

    CAN1TransmitOK = BCI_TransmitCanMsg (CANBoard, 1, &ToSendCANMsg1);

    BCI_MDelay (750);

    if (CAN1TransmitOK == BCI_OK)
      TxCounter1++;

    printf ("CAN0 RX: ");
    if (BCI_ReceiveCanMsg (CANBoard, 0, &ReceivedCANMsg0, BCI_NO_WAIT) == BCI_OK)
    {
      BCI_ShowCANMsg (&ReceivedCANMsg0);
      RxCounter0++;
    }
    else
    {
      printf ("Nothing received !\n");
    }

    printf ("CAN1 RX: ");
    if (BCI_ReceiveCanMsg (CANBoard, 1, &ReceivedCANMsg1, BCI_NO_WAIT) == BCI_OK)
    {
      BCI_ShowCANMsg (&ReceivedCANMsg1);
      RxCounter1++;
    }
    else
    {
      printf ("Nothing received !\n");
    }

    BCI_GetBoardStatus (CANBoard, &BrdStatus);
    BCI_Status2Str (BrdStatus.can0_status, Status0);
    BCI_Status2Str (BrdStatus.can1_status, Status1);

    printf
      ("Status : CAN0 Ld[%d] St[%s] CAN1 Ld[%d] St[%s] CPU Load [%d] Counter [%d]\n",
       (UINT16) BrdStatus.can0_busload,
       Status0,
       (UINT16) BrdStatus.can1_busload,
       Status1, (UINT16) BrdStatus.cpu_load, (UINT16) BrdStatus.counter);

    printf ("--------------------------------------------------------\n");

  }

  printf ("CAN0: transmitted [%d] received [%d]\n", TxCounter0, RxCounter0);

  printf ("CAN1: transmitted [%d] received [%d]\n", TxCounter1, RxCounter1);

/*========================================================================*/
  RxCounter0 = 0;
  RxCounter1 = 0;
  TxCounter0 = 0;
  TxCounter1 = 0;

  printf ("Now check the id filter list\n");

  for (Controller = 0; Controller < 2; Controller++)
  {
    printf ("Controller [%d] Stop\n", Controller);
    BCI_StopCan (CANBoard, Controller);
    printf ("Controller [%d] Reset\n", Controller);
    BCI_ResetCan (CANBoard, Controller);

    printf ("Controller [%d] Init\n", Controller);
    BCI_InitCan (CANBoard, Controller, BCI_125KB, CANSpeed);
    printf ("Controller [%d] ConfigRxQueue\n", Controller);
    BCI_ConfigRxQueue (CANBoard, Controller, BCI_POLL_MODE);

    printf ("Controller [%d] Disable Acceptance Code/Mask\n", Controller);
    BCI_SetAccMask (CANBoard, Controller, BCI_11B_MASK, 0, BCI_REJECT_ALL);
    BCI_SetAccMask (CANBoard, Controller, BCI_29B_MASK, 0, BCI_REJECT_ALL);

  }

  Controller = 0;
  printf ("Controller [%d] Configure Id Filter List\n", Controller);
  BCI_RegisterRxId (CANBoard, Controller, BCI_MFF_29_DAT, CAN1_ID + 1);
  BCI_RegisterRxId (CANBoard, Controller, BCI_MFF_29_DAT, CAN1_ID + 2);
  printf ("Controller [%d] Allowed Id's are %x %x\n", Controller, CAN1_ID + 1,
          CAN1_ID + 2);
  printf ("Controller [%d] Start\n", Controller);
  BCI_StartCan (CANBoard, Controller);

  Controller = 1;
  printf ("Controller [%d] Configure Id Filter List\n", Controller);
  BCI_RegisterRxId (CANBoard, Controller, BCI_MFF_29_DAT, CAN0_ID + 1);
  BCI_RegisterRxId (CANBoard, Controller, BCI_MFF_29_DAT, CAN0_ID + 2);
  printf ("Controller [%d] Allowed Id's are %x %x\n", Controller, CAN0_ID + 1,
          CAN0_ID + 2);
  printf ("Controller [%d] Start\n", Controller);
  BCI_StartCan (CANBoard, Controller);

  printf ("Transmit / receive messages\n");

  while ((TxCounter0 < TEST_MSG_NUM) && (TxCounter1 < TEST_MSG_NUM))
  {

    printf ("CAN0 TX: ");
    if (CAN0TransmitOK == BCI_OK)
    {

// C0 means CAN0 packages. For presentation purposes only.
      PacketData[0] = 0xC0;
      memset (PacketData + 1, Value0, 7);
      BCI_CreateCANMsg (&ToSendCANMsg0, CAN0_ID + TxCounter0, PacketData, 8,
                        BCI_MFF_29_DAT);
      BCI_ShowCANMsg (&ToSendCANMsg0);
      Value0 = (Value0 + 1) % 0xFF;
    }
    else
      printf ("Try again previous message \n");

    CAN0TransmitOK = BCI_TransmitCanMsg (CANBoard, 0, &ToSendCANMsg0);

    BCI_MDelay (750);

    if (CAN0TransmitOK == BCI_OK)
      TxCounter0++;

    printf ("CAN1 TX: ");
    if (CAN1TransmitOK == BCI_OK)
    {
      PacketData[0] = 0xC1;
      memset (PacketData + 1, Value1, 7);
      BCI_CreateCANMsg (&ToSendCANMsg1, CAN1_ID + TxCounter0, PacketData, 8,
                        BCI_MFF_29_DAT);
      BCI_ShowCANMsg (&ToSendCANMsg1);
      Value1 = (Value1 + 1) % 0xFF;
    }
    else
      printf ("Try again previous message \n");

    CAN1TransmitOK = BCI_TransmitCanMsg (CANBoard, 1, &ToSendCANMsg1);

    BCI_MDelay (750);

    if (CAN1TransmitOK == BCI_OK)
      TxCounter1++;

    printf ("CAN0 RX: ");
    if (BCI_ReceiveCanMsg (CANBoard, 0, &ReceivedCANMsg0, BCI_NO_WAIT) == BCI_OK)
    {
      BCI_ShowCANMsg (&ReceivedCANMsg0);
      RxCounter0++;
    }
    else
    {
      printf ("Nothing received !\n");
    }

    printf ("CAN1 RX: ");
    if (BCI_ReceiveCanMsg (CANBoard, 1, &ReceivedCANMsg1, BCI_NO_WAIT) == BCI_OK)
    {
      BCI_ShowCANMsg (&ReceivedCANMsg1);
      RxCounter1++;
    }
    else
    {
      printf ("Nothing received !\n");
    }

    BCI_GetBoardStatus (CANBoard, &BrdStatus);
    BCI_Status2Str (BrdStatus.can0_status, Status0);
    BCI_Status2Str (BrdStatus.can1_status, Status1);

    printf
      ("Status : CAN0 Ld[%d] St[%s] CAN1 Ld[%d] St[%s] CPU Load [%d] Counter [%d]\n",
       (UINT16) BrdStatus.can0_busload,
       Status0,
       (UINT16) BrdStatus.can1_busload,
       Status1, (UINT16) BrdStatus.cpu_load, (UINT16) BrdStatus.counter);

    printf ("--------------------------------------------------------\n");

  }

  printf ("CAN0: transmitted [%d] received [%d]\n", TxCounter0, RxCounter0);

  printf ("CAN1: transmitted [%d] received [%d]\n", TxCounter1, RxCounter1);

/*========================================================================*/

  printf ("Now reconfigure CAN1 for interrupt use:\n");
  BCI_MDelay (2000);

  Controller = 1;

  for (Controller = 0; Controller < 2; Controller++)
  {
    printf ("Controller [%d] Stop\n", Controller);
    BCI_StopCan (CANBoard, Controller);
    printf ("Controller [%d] Reset\n", Controller);
    BCI_ResetCan (CANBoard, Controller);

    printf ("Controller [%d] Init\n", Controller);
    BCI_InitCan (CANBoard, Controller, BCI_125KB, CANSpeed);
    printf ("Controller [%d] ConfigRxQueue\n", Controller);
    BCI_ConfigRxQueue (CANBoard, Controller, BCI_LATENCY_MODE);

    BCI_SetAccMask (CANBoard, Controller, BCI_11B_MASK, 0, BCI_ACC_ALL);
    BCI_SetAccMask (CANBoard, Controller, BCI_29B_MASK, 0, BCI_ACC_ALL);

    printf ("Controller [%d] Start\n", Controller);
    BCI_StartCan (CANBoard, Controller);
  }

  RxCounter1 = TxCounter0 = 0;
  PacketData[0] = 0xC0;
  memset (PacketData + 1, 0xFF, 7);

  while (1)
  {
    BCI_CreateCANMsg (&ToSendCANMsg0, TxCounter0, PacketData, 8, BCI_MFF_29_DAT);

    if (BCI_TransmitCanMsg (CANBoard, 0, &ToSendCANMsg0) == BCI_OK)
    {
      printf ("CAN0 TX: ");
      BCI_ShowCANMsg (&ToSendCANMsg0);
      TxCounter0++;
      memset (PacketData + 1, TxCounter0 % 0xff, 7);        
    }

//   BCI_MDelay(2000);

// If there was interrupt generated (that means controller CAN1 has received
// message) call BCI_ReceiveCanMsg and show received CAN object.

    if (BCI_ReceiveCanMsg (CANBoard, 1, &ReceivedCANMsg1, 1000) == BCI_OK)
    {
      printf ("CAN1 RX: ");
      BCI_ShowCANMsg (&ReceivedCANMsg1);
      RxCounter1++;
    }
    else
    {
      printf ("No data\n");
      BCI_GetBoardStatus (CANBoard, &BrdStatus);
      BCI_Status2Str (BrdStatus.can0_status, Status0);
      BCI_Status2Str (BrdStatus.can1_status, Status1);

      printf
        ("Status : CAN0 Ld[%d] St[%s] CAN1 Ld[%d] St[%s] CPU Load [%d] Counter [%d]\n",
         (UINT16) BrdStatus.can0_busload,
         Status0,
         (UINT16) BrdStatus.can1_busload,
         Status1, (UINT16) BrdStatus.cpu_load, (UINT16) BrdStatus.counter);
    }

    printf ("--------------------------------------------------------\n");
  }
  Controller = 0;
  printf ("Controller [%d] Stop\n", Controller);
  BCI_StopCan (CANBoard, Controller);
  printf ("Controller [%d] Reset\n", Controller);
  BCI_ResetCan (CANBoard, Controller);

  Controller = 1;
  printf ("Controller [%d] Stop\n", Controller);
  BCI_StopCan (CANBoard, Controller);
  printf ("Controller [%d] Reset\n", Controller);
  BCI_ResetCan (CANBoard, Controller);

  printf ("Close board\n");
  BCI_CloseBoard (CANBoard);

  return 0;
}
