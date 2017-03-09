/*************************************************************************
**    IXXAT Automation GmbH
**************************************************************************
**
**  $Workfile: $
**    Summary:
**   $Archive: $
**  $Revision: 1.3 $
**      $Date: 2003/03/19 13:11:19 $
**     Author: Alexander Kuzmich
**
**************************************************************************
**************************************************************************
**
**  Functions: Common CAN device driver definitions
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

#ifndef CAN_H
#define CAN_H

/*************************************************************************
**    constants and macros
*************************************************************************/
#define CAN_PAS_MAJOR_REV   4
#define CAN_PAS_MINOR_REV   1
#define CAN_ACT_MAJOR_REV   4
#define CAN_ACT_MINOR_REV   1

/* Device types */
#define DEVICE_UNKNOWN        0
#define DEVICE_iPCI165_ISA    1
#define DEVICE_iPCI320_ISA    2
#define DEVICE_iPCI165_PCI    3
#define DEVICE_iPCI320_PCI    4
#define DEVICE_PEP_CP350      5
#define DEVICE_PCI            6
#define DEVICE_iPCIXC161_PCI  7
#define DEVICE_iPCIXC161_PCIE 8
#define DEVICE_PCI03_ISA      10
#define DEVICE_PCI04_ISA      11
#define DEVICE_PCI04_PCI      12

/* Active cards parameters */
#define iPCI165_DPRAM_SIZE   8*1024
#define iPCI320_DPRAM_SIZE   4*1024
#define iPCIXC161_DPRAM_SIZE 8*1024

#define iPCI165_MEM_SIZE     16*1024
#define iPCI320_MEM_SIZE     8*1024
#define iPCIXC161_MEM_SIZE   16*1024

/* Passive cards parameters */
#define PC_I_DPRAM_SIZE 4*1024
#define PC_I_MEM_SIZE 4*1024

/* Maximal amount of the PCI CAN cards in PC */
#define MAX_PCI_CARD_NUM   3

/* Maximal amount of ports for board */
#define MAX_PORTS_ON_BOARD 2

/* Size of the PCI control memory */
#define PCI_MEMORY_SIZE           0x80
#define PCI_BRIDGE_MEMORY_SIZE    0x10000

/* Sizes of the internal strings */
#define NAME_LEN           64
#define MODE_LEN           14
#define TYPE_LEN           11

/* Loader control offsets */
#define OF_LD_SYNC      0x100
#define OF_LD_CMND      0x101
#define OF_LD_NUM       0x102
#define OF_LD_ADDRESS   0x104
#define OF_LD_DATA      0x108

/* Firmware command buffer */
#define OF_BCI_SYNC      0x00
#define OF_BCI_NUM       0x01
#define OF_BCI_DATA      0x04

/* Boot manager v2 offsets */
#define OF_BM_V2_SYNC_PC2MC     0x00
#define OF_BM_V2_SYNC_MC2PC     0x01
#define OF_BM_V2_PORT           0x02
#define OF_BM_V2_LEN            0x03
#define OF_BM_V2_DATA           0x04

#define OF_BM_V2_CMD            0x00
#define OF_BM_V2_CMD_STS        0x01
#define OF_BM_V2_CMD_DATA       0x02

/* Control offsets for 165 board */
#define OF_iPCI165_MODE      0x1FFA
#define OF_iPCI165_SEMA_0    0x2000
#define OF_iPCI165_SEMA_1    0x2001
#define OF_iPCI165_RESET     0x2400
#define OF_iPCI165_INTERRUPT 0x2C00

/* Control offsets for 320 board */
#define OF_iPCI320_MODE      0x0FFA
#define OF_iPCI320_SEMA_0    0x1000
#define OF_iPCI320_SEMA_1    0x1001
#define OF_iPCI320_RESET     0x1400
#define OF_iPCI320_INTERRUPT 0x1800

/* Control offsets for XC16 board */
#define OF_iPCIXC16PCI_MODE      0x1FFA
#define OF_iPCIXC16PCI_SEMA_0    0x2000
#define OF_iPCIXC16PCI_SEMA_1    0x2002
#define OF_iPCIXC16PCI_SEMA_2    0x2004
#define OF_iPCIXC16PCI_SEMA_3    0x2006
#define OF_iPCIXC16PCI_RESET     0x2400
#define OF_iPCIXC16PCI_INTERRUPT 0x1FFE

#define BCI_QUEUE_ITEM_SIZE 18              // Length of 1 item (18 bytes for BCI)

#ifdef CAN_MODULE_DEBUG

/* Info print */
#define PRINTI(format, args...) printk("KINF: " format, ## args)

/* Error print */
#define PRINTE(format, args...) printk("KERR: " format, ## args)

/* Debug print */
#define PRINTD(format, args...) printk("KDEB: " format, ## args)

#else

/* Info print */
#define PRINTI(format, args...) printk("KINF: " format, ## args)

/* Error print */
#define PRINTE(format, args...) printk("KERR: " format, ## args)

/* Debug print */
#define PRINTD(format, args...)

#endif

/*************************************************************************
**    data types
*************************************************************************/

/*************************************************************************
**    global variables
*************************************************************************/

/*************************************************************************
**    function prototypes
*************************************************************************/

#endif
