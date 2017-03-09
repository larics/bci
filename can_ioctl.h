
/*************************************************************************
**    IXXAT Automation GmbH
**************************************************************************
**
**  $Workfile: $
**    Summary: Definitions of the ioctl numbers
**   $Archive: $
**  $Revision: 1.4 $
**      $Date: 2003/03/20 19:04:41 $
**     Author: Alexander Kuzmich
**
**************************************************************************
**************************************************************************
**
**  Functions:  
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
**    constants and macros
*************************************************************************/

#ifndef CAN_IOCTL_H
#define CAN_IOCTL_H

#define CAN_DEVICE_LETTER 0x240

#define IOCTL_INTERRUPT_TO_MC   _IO(CAN_DEVICE_LETTER,  1)
#define IOCTL_GET_ID            _IOR(CAN_DEVICE_LETTER, 2, int)
#define IOCTL_RESET 		_IO(CAN_DEVICE_LETTER,  3)
#define IOCTL_SET_TIMING	_IO(CAN_DEVICE_LETTER, 22)
#define IOCTL_GET_TIMING	_IO(CAN_DEVICE_LETTER, 23)
#define IOCTL_SET_ACC_CODE_STD  _IO(CAN_DEVICE_LETTER, 24)
#define IOCTL_GET_ACC_CODE_STD	_IO(CAN_DEVICE_LETTER, 25)
#define IOCTL_SET_ACC_MASK_STD	_IO(CAN_DEVICE_LETTER, 26)
#define IOCTL_GET_ACC_MASK_STD	_IO(CAN_DEVICE_LETTER, 27)
#define IOCTL_SET_ACC_CODE_EXT	_IO(CAN_DEVICE_LETTER, 28)
#define IOCTL_GET_ACC_CODE_EXT	_IO(CAN_DEVICE_LETTER, 29)
#define IOCTL_SET_ACC_MASK_EXT	_IO(CAN_DEVICE_LETTER, 30)
#define IOCTL_GET_ACC_MASK_EXT	_IO(CAN_DEVICE_LETTER, 31)
#define IOCTL_GET_STATUS	_IOR(CAN_DEVICE_LETTER, 32, int)
#define IOCTL_REGISTER_STD_ID       _IO(CAN_DEVICE_LETTER, 33)
#define IOCTL_REGISTER_EXT_ID       _IO(CAN_DEVICE_LETTER, 34)
#define IOCTL_UNREGISTER_STD_ID     _IO(CAN_DEVICE_LETTER, 35)
#define IOCTL_UNREGISTER_EXT_ID     _IO(CAN_DEVICE_LETTER, 36)
#define IOCTL_LOCK_MC     		      _IO(CAN_DEVICE_LETTER, 37)
#define IOCTL_UNLOCK_MC     		    _IO(CAN_DEVICE_LETTER, 38)

#define IOCTL_PORT_START	_IO(CAN_DEVICE_LETTER, 41)
#define IOCTL_PORT_STOP		_IO(CAN_DEVICE_LETTER, 42)
#define IOCTL_RESET_OVERRUN	_IO(CAN_DEVICE_LETTER, 43)

#endif
