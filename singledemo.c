
/***************************************************************************
                          singledemo.c  -  this demo uses only first
                          controller on the board

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
#define MESSAGES_NUMBER 10

int BCI_Status2Str (UINT16 Status, char *StatusStr);
int BCI_ShowCANMsg (BCI_ts_CanMsg * CANMsg);
int BCI_CreateCANMsg (BCI_ts_CanMsg * CANMsg, UINT32 ID, UINT8 * Data,  // Databytes of the message
                      UINT8 DLC,        // data length code (Bit 0..3)
                      UINT8 MFF);       // Message frame format
void BCI_MDelay (unsigned int msec);

int main (int argc, char *argv[])
{
  BCI_BRD_HDL CANBoard;
  UINT8 Controller = 0, PacketData[8], Value0 = 0;

  int TxCounter0 = 0, RxCounter0 = 0, ret;

  BCI_ts_CanMsg ReceivedCANMsg0, ToSendCANMsg0;

  BCI_ts_BrdSts BrdStatus;

  char CAN0TransmitOK = BCI_OK;

  BCI_ts_BrdInfo BoardInfo;
  char Status0[16], Status1[16];
  char DevFileName[64] = "/dev/can0";

  if (argv[1] != NULL)
  {
    strcpy (DevFileName, argv[1]);
  }

  if (argv[2] != NULL)
  {
    Controller = strtol (argv[2], NULL, 10);
  }

  printf ("Trying to open device file [%s]\n", DevFileName);
  printf ("Use controller [%d]\n", Controller);

  printf ("Open board\n");
  ret = BCI_OpenBoard (&CANBoard, DevFileName);
  if (ret != BCI_OK)
  {
    printf (BCI_GetErrorString (ret));
    return -1;
  }

  printf ("Get Board Info\n");
  if ((ret = BCI_GetBoardInfo (CANBoard, &BoardInfo)) != BCI_OK)
  {
    printf (BCI_GetErrorString (ret));
    return -1;
  }

  printf ("Board info: FW version %d ContNum %d\n", BoardInfo.fw_version,
          BoardInfo.num_can);
  printf (" Controller 0: [%s]\n", BoardInfo.can_type[0]);
  printf (" Controller 1: [%s]\n", BoardInfo.can_type[1]);

  /*========================================================================*/

  printf ("Configure CAN0 controller and start it:\n");
  BCI_MDelay (1500);
  printf ("Controller [%d] Reset\n", Controller);
  BCI_ResetCan (CANBoard, Controller);
  printf ("Controller [%d] Init\n", Controller);
  BCI_InitCan (CANBoard, Controller, BCI_125KB, 0);
  printf ("Controller [%d] ConfigRxQueue\n", Controller);
  BCI_ConfigRxQueue (CANBoard, Controller, BCI_POLL_MODE);

  printf ("Controller [%d] Register filter ID\n", Controller);
  BCI_RegisterRxId (CANBoard, Controller, BCI_MFF_29_DAT, 10);
  BCI_RegisterRxId (CANBoard, Controller, BCI_MFF_29_DAT, 11);
  BCI_RegisterRxId (CANBoard, Controller, BCI_MFF_29_DAT, 12);
  BCI_SetAccMask (CANBoard, Controller, BCI_11B_MASK, 0, BCI_ACC_ALL);
  BCI_SetAccMask (CANBoard, Controller, BCI_29B_MASK, 0x0, BCI_ACC_ALL);
  printf ("Controller [%d] Start\n", Controller);
  BCI_StartCan (CANBoard, Controller);

  printf ("Transmit %d messages\n", MESSAGES_NUMBER);

  while (TxCounter0 < MESSAGES_NUMBER)
  {

    printf ("CAN%d TX: ", Controller);
    if (CAN0TransmitOK == BCI_OK)
    {

      // C0 means CAN0 packages. For presentation purposes only.
      PacketData[0] = (Controller == 0) ? 0xC0 : 0xC1;
      memset (PacketData + 1, Value0, 7);
      BCI_CreateCANMsg (&ToSendCANMsg0, CAN0_ID, PacketData, 8, BCI_MFF_11_DAT);
      BCI_ShowCANMsg (&ToSendCANMsg0);
      Value0 = (Value0 + 1) % 0xFF;
    }
    else
      printf ("Try again previous message \n");

    CAN0TransmitOK = BCI_TransmitCanMsg (CANBoard, Controller, &ToSendCANMsg0);

    BCI_MDelay (80);

    if (CAN0TransmitOK == BCI_OK)
      TxCounter0++;

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

  printf ("CAN%d: transmitted [%d]\n", Controller, TxCounter0);

  printf ("Receive %d messages\n", MESSAGES_NUMBER);

  while (RxCounter0 < MESSAGES_NUMBER)
  {

    printf ("CAN%d RX: ", Controller);
    if (BCI_ReceiveCanMsg
        (CANBoard, Controller, &ReceivedCANMsg0, BCI_NO_WAIT) == BCI_OK)
    {
      BCI_ShowCANMsg (&ReceivedCANMsg0);
      RxCounter0++;
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
    BCI_MDelay (500);
  }

  printf ("CAN%d: received [%d]\n", Controller, RxCounter0);

  printf ("Now reconfigure CAN%d for interrupt use :\n", Controller);

  printf ("Controller [%d] ConfigRxQueue\n", Controller);
  BCI_ConfigRxQueue (CANBoard, Controller, BCI_LATENCY_MODE);

  BCI_MDelay (2000);

  printf ("Waiting for %d messages\n", MESSAGES_NUMBER);
  RxCounter0 = 0;
  while (RxCounter0 < MESSAGES_NUMBER)
  {

    if (BCI_ReceiveCanMsg (CANBoard, Controller, &ReceivedCANMsg0, 5000) == BCI_OK)
    {
      printf ("CAN%d RX: ", Controller);
      BCI_ShowCANMsg (&ReceivedCANMsg0);
      RxCounter0++;
    }

  }
  printf ("--------------------------------------------------------\n");

  printf ("Controller [%d] Stop\n", Controller);
  BCI_StopCan (CANBoard, Controller);

  printf ("Close board\n");
  BCI_CloseBoard (CANBoard);

  return 0;
}
