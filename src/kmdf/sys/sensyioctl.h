/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name: 

    sensyioctl.h

Abstract:

    This module contains the public device path names and
    IOCTL definitions for the Sensy.

Environment:

Revision History:

--*/

#ifndef _SENSYIOCTL_H_
#define _SENSYIOCTL_H_

//
// Device path names
//

#define Sensy_NAME L"Sensy"

#define sensy_SYMBOLIC_NAME L"\\DosDevices\\" Sensy_NAME
#define sensy_USERMODE_PATH L"\\\\.\\" Sensy_NAME
#define sensy_USERMODE_PATH_SIZE sizeof(sensy_USERMODE_PATH)

//
// Priavte Sensy IOCTLs
//

#define FILE_DEVICE_SPB_PERIPHERAL 0x400

#define IOCTL_sensy_OPEN              CTL_CODE(FILE_DEVICE_SPB_PERIPHERAL, 0x700, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_sensy_CLOSE             CTL_CODE(FILE_DEVICE_SPB_PERIPHERAL, 0x701, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_sensy_LOCK              CTL_CODE(FILE_DEVICE_SPB_PERIPHERAL, 0x702, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_sensy_UNLOCK            CTL_CODE(FILE_DEVICE_SPB_PERIPHERAL, 0x703, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_sensy_WRITEREAD         CTL_CODE(FILE_DEVICE_SPB_PERIPHERAL, 0x704, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_sensy_LOCK_CONNECTION   CTL_CODE(FILE_DEVICE_SPB_PERIPHERAL, 0x705, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_sensy_UNLOCK_CONNECTION CTL_CODE(FILE_DEVICE_SPB_PERIPHERAL, 0x706, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_sensy_SIGNAL_INTERRUPT  CTL_CODE(FILE_DEVICE_SPB_PERIPHERAL, 0x707, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_sensy_WAIT_ON_INTERRUPT CTL_CODE(FILE_DEVICE_SPB_PERIPHERAL, 0x708, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_sensy_FULL_DUPLEX       CTL_CODE(FILE_DEVICE_SPB_PERIPHERAL, 0x709, METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif _SENSYIOCTL_H_