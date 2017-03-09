
/***************************************************************************
                          lindemo.c - basic LIN demo for XC161 IXXAT board 
                             -------------------
    begin                : Wed Mar 12 2008
    copyright            : (C) 2001 IXXAT Automation GmbH
    email                : kuzmich@ixxat.de
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bci.h"

#define LIN_DEMO_MODE_MASTER_BUF    0x0
#define LIN_DEMO_MODE_MASTER_TX     0x1
#define LIN_DEMO_MODE_SLAVE_BUF     0x2
#define LIN_DEMO_MODE_SLAVE_RX      0x3

void BCI_MDelay (unsigned int msec);

int LIN_ShowStatus (BCI_BRD_HDL iPCI_Board, UINT8 LIN_Controller);
int BCI_LinPrintMsg(BCI_ts_LinMsg *pslinMsg);


int main (int argc, char *argv[])
{
  BCI_BRD_HDL iPCI_Board;

  BCI_ts_BrdInfo BoardInfo;

  BCI_ts_LinMsg slinRxMsg;

  UINT8 LIN_Demo_Mode = LIN_DEMO_MODE_MASTER_TX;
  UINT16 LIN_Baudrate = 19200;

  UINT8 LIN_Controller = 0, bCRCType = BCI_LIN_CRC_CLASSIC;

  int ret, i;
  UINT32 dwRxCount;

  char DevFileName[64] = "/dev/can0";

  printf ("Parameters: %s %s %s\n", argv[0], argv[1], argv[3]);

  if (argv[1] != NULL)
  {
    LIN_Demo_Mode = strtol (argv[1], NULL, 10);
  }

  if (argv[2] != NULL)
  {
    strcpy (DevFileName, argv[2]);
  }

  if (argv[3] != NULL)
  {
    LIN_Controller = strtol (argv[3], NULL, 10);
  }

  printf ("Trying to open device file [%s]\n", DevFileName);
  printf ("Use LIN controller [%d]\n", LIN_Controller);

  printf ("Open board\n");
  ret = BCI_OpenBoard (&iPCI_Board, DevFileName);
  if (ret != BCI_OK)
  {
    printf (BCI_GetErrorString (ret));
    return -1;
  }

  printf ("Get Board Info\n");
  if ((ret = BCI_GetBoardInfo (iPCI_Board, &BoardInfo)) != BCI_OK)
  {
    printf (BCI_GetErrorString (ret));
    return -1;
  }

  printf ("Board info: FW version %d ContNum %d\n",
          BoardInfo.fw_version, BoardInfo.num_can);
  printf (" CAN_Controller 0: [%s]\n", BoardInfo.can_type[0]);
  printf (" CAN_Controller 1: [%s]\n", BoardInfo.can_type[1]);

  /*========================================================================*/

  printf ("LIN%d: Test API\n", LIN_Controller);

  if (LIN_Demo_Mode == LIN_DEMO_MODE_MASTER_BUF)
  {
    /* Configure the controller as Master and request data from Slave */
    UINT8 bReqId, bReqLen;
    UINT32 dwCount, dwRxCount;

    printf ("LIN Master requst buffer from slave mode\n");
    BCI_LinControlInitialize (iPCI_Board, LIN_Controller, BCI_LIN_OPMODE_MASTER,
                              BCI_LATENCY_MODE, LIN_Baudrate);
    BCI_LinControlStart (iPCI_Board, LIN_Controller);
    dwCount = 0;
    dwRxCount = 0;

    while (dwCount < 1000)
    {
      bReqId = dwCount % 63;
      bReqLen = (bReqId % 8) + 1;
      printf ("LIN%d: ReqId %d Len %d from Slave, RxCount %d\n",
              LIN_Controller, bReqId, bReqLen, dwRxCount);
      /* Send the requested Id */
      /* Warning: Id and Len should be exactly the same as Slave 
         buffers configuration ! */
      ret = BCI_LinMasterRequestId (iPCI_Board, LIN_Controller, bReqId, bCRCType, bReqLen);
      if (ret == BCI_OK)
      {
        dwCount++;
        /* Get the data from Slave */
        ret = BCI_LinRecvMsg (iPCI_Board, LIN_Controller, 1000, &slinRxMsg);
        if (ret == BCI_OK)
        {
          printf ("LIN%d: Recv ", LIN_Controller);

          BCI_LinPrintMsg(&slinRxMsg);
          dwRxCount++;
        }
        BCI_MDelay (1000);
      }
    }
    LIN_ShowStatus (iPCI_Board, LIN_Controller);
  }
  else if (LIN_Demo_Mode == LIN_DEMO_MODE_MASTER_TX)
  {
    static char strTx[64];
    int i;

    /* Configure the controller as Master and send data to Slave */
    printf ("LIN Master send message mode\n");
    BCI_LinControlInitialize (iPCI_Board, LIN_Controller, BCI_LIN_OPMODE_MASTER,
                              BCI_LATENCY_MODE, LIN_Baudrate);

    BCI_LinControlStart (iPCI_Board, LIN_Controller);
    BCI_MDelay (100);

    for (i=0; i< 20; i++)
    {
      memset(strTx, 0x0, sizeof(strTx));
      sprintf(strTx, "LT%06d", i);

      ret = BCI_LinMasterSendMsg (iPCI_Board, LIN_Controller, 0x1, bCRCType, (UINT8 *) strTx, 8);
      printf("Send Msg %s\n", strTx);
      LIN_ShowStatus (iPCI_Board, LIN_Controller);
      BCI_MDelay (1000);
    }
    
  }
  else if (LIN_Demo_Mode == LIN_DEMO_MODE_SLAVE_BUF)
  {
    UINT8 bReqId, bReqLen;

    /* Configure the controller as Slave and wait for the data request
       from Master */
    printf ("LIN Slave buffer mode\n");
    BCI_LinControlInitialize (iPCI_Board, LIN_Controller, BCI_LIN_OPMODE_SLAVE,
                              BCI_LATENCY_MODE, LIN_Baudrate);
    
    BCI_LinControlStart (iPCI_Board, LIN_Controller);

    for (bReqId = 0; bReqId < 64; bReqId ++) 
    {
      bReqLen = (bReqId % 8) + 1;
      ret = BCI_LinSlaveUpdateTxBuffer (iPCI_Board, LIN_Controller,
                                        bReqId, bCRCType, (UINT8 *) "12345678", bReqLen);
    }
    
    LIN_ShowStatus (iPCI_Board, LIN_Controller);
    BCI_MDelay (30000);
    
  }
  else if (LIN_Demo_Mode == LIN_DEMO_MODE_SLAVE_RX)
  {
    /* Configure the controller as Slave and wait for the data messages
       from Master */
    printf ("LIN Slave receiver mode\n");
    BCI_LinControlInitialize (iPCI_Board, LIN_Controller, BCI_LIN_OPMODE_SLAVE,
                              BCI_LATENCY_MODE, LIN_Baudrate);

    BCI_LinControlStart (iPCI_Board, LIN_Controller);
    LIN_ShowStatus (iPCI_Board, LIN_Controller);
  }

  printf ("Waiting for messages to receive ...\n");

  dwRxCount = 0;
  for (i = 0; i < 100000; i++)
  {
    ret = BCI_LinRecvMsg (iPCI_Board, LIN_Controller, 1000, &slinRxMsg);
    if (ret == BCI_OK)
    {
      printf ("%d: Received LIN message\n", dwRxCount);
      BCI_LinPrintMsg(&slinRxMsg);
      dwRxCount++;
    }
    else
    {
      printf ("%d: No LIN message received ...\n", dwRxCount);
    }
  }

  BCI_LinControlStop (iPCI_Board, LIN_Controller);
  BCI_MDelay (2000);
  BCI_LinControlReset (iPCI_Board, LIN_Controller);
  BCI_MDelay (2000);

  printf ("Close board\n");
  BCI_CloseBoard (iPCI_Board);

  return 0;
}


int BCI_LinFormatMsg(BCI_ts_LinMsg *pslinMsg, char *strBuf)
{
  int len;
  
  if (strBuf == NULL)
  {
    return -1;
  }

  if (pslinMsg == NULL)
  {
    return -1;
  }

  len = 0;
  
  switch (pslinMsg->sHdr.bType)
  {
    case BCI_LIN_MSGTYPE_DATA:
    {
      int byte;

      pslinMsg->uMsg.Data.bId &= 0x3f;
      strBuf[0] = 0;

      len += sprintf (strBuf + len, "DATA ts: %d id: 0x%02x len: %d data: ",
                     pslinMsg->sHdr.dwTimeStamp,
                     pslinMsg->uMsg.Data.bId, pslinMsg->uMsg.Data.bLength);

      for (byte = 0; byte < pslinMsg->uMsg.Data.bLength; byte++)
      {
        len +=
          sprintf (strBuf + len, "%02x ",
                   pslinMsg->uMsg.Data.abDataByte[byte]);
      }


    }
    break;
    case BCI_LIN_MSGTYPE_INFO:
      len += sprintf (strBuf + len, "INFO ts %d status 0x%x\n",
                      pslinMsg->sHdr.dwTimeStamp,
                      pslinMsg->uMsg.Stat.wStatus);
      break;
    case BCI_LIN_MSGTYPE_ERROR:
      len += sprintf (strBuf + len, "ERR ts %d code 0x%x\n",
                      pslinMsg->sHdr.dwTimeStamp,
                      pslinMsg->uMsg.Err.wErrCode);
      break;
    case BCI_LIN_MSGTYPE_WAKEUP:
      len += sprintf (strBuf + len, "WAKE UP\n");
      break;
    default:
      len += sprintf (strBuf + len, "UNKNOWN type 0x%x\n", pslinMsg->sHdr.bType);
  }
  return len;
}


int BCI_LinPrintMsg(BCI_ts_LinMsg *pslinMsg)
{
  static char strBuf[128];
  
  int ret;
  ret = BCI_LinFormatMsg(pslinMsg, strBuf);
  
  if (ret > 0)
  {
    printf("%s\n", strBuf);
  }

  return ret;
}



int LIN_ShowStatus (BCI_BRD_HDL iPCI_Board, UINT8 LIN_Controller)
{
  int ret;
  BCI_ts_linBrdSts slinSts;

  ret = BCI_LinGetStatus (iPCI_Board, LIN_Controller, &slinSts);
  if (ret == BCI_OK)
  {
    printf ("LIN%d: OpMode [%s] 0x%x Load %d%% Bitrate %d Status 0x%x\n",
            LIN_Controller,
            (slinSts.bOpMode == BCI_LIN_FW_OPMODE_MODE_MASTER) ? "Master" : "Slave",
            slinSts.bOpMode, slinSts.bBusLoad, slinSts.wBitrate, slinSts.dwStatus);
  }
  else
  {
    printf ("LIN%d: Cannot get status\n", LIN_Controller);
  }

  return ret;
}
