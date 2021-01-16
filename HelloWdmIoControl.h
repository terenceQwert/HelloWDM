#pragma once
#define IOCTL_TEST1 CTL_CODE(\
FILE_DEVICE_UNKNOWN,\
0x800,\
METHOD_BUFFERED,\
FILE_ANY_ACCESS)

#define IOCTL_TEST2 CTL_CODE(\
FILE_DEVICE_UNKNOWN,\
0x801,\
METHOD_BUFFERED,\
FILE_ANY_ACCESS)


#define READ_PORT CTL_CODE(\
			FILE_DEVICE_UNKNOWN, \
			0x802, \
			METHOD_BUFFERED, \
			FILE_ANY_ACCESS)

#define WRITE_PORT CTL_CODE(\
			FILE_DEVICE_UNKNOWN, \
			0x803, \
			METHOD_BUFFERED, \
			FILE_ANY_ACCESS)

#define PCI_CONFIG CTL_CODE(\
			FILE_DEVICE_UNKNOWN, \
			0x804, \
			METHOD_BUFFERED, \
			FILE_ANY_ACCESS)


#define BUFFER_IO 1


// {053C4C1A-A4F0-4F25-856D-2F739DCDDA00}
static GUID MyWDMDevice =
{ 0x53c4c1a, 0xa4f0, 0x4f25, { 0x85, 0x6d, 0x2f, 0x73, 0x9d, 0xcd, 0xda, 0x0 } };

// driver 
#define MYWDM_NAME L"\\Device\\MyWDMDevice"
// #define SMBOL_LNK_NAME L"\\DosDevices\\HelloWDM"
#define SMBOL_LNK_NAME L"\\??\\HelloWDM"

// application
#define DRV_NAME L"\\\\.\\HelloWDM"
