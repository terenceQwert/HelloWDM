#pragma once
//#include <wdm.h>
#include "HelloWdmIoControl.h"
typedef struct _MYData
{
  ULONG data_1;
  ULONG data_2;
  LIST_ENTRY listEntry;
} MYDATASTRUCT, *PMYDATASTRUCT;

typedef struct _DEVICE_EXTENSION
{
  PDEVICE_OBJECT fdo;
  PDEVICE_OBJECT NextStackDevice;
  UNICODE_STRING  ustrDeviceName;
  UNICODE_STRING  ustrSymLinkName;
  UCHAR           buffer[MAXIMUM_FILENAME_LENGTH];
  ULONG           file_length;
  UNICODE_STRING interfaceName;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

#define arraysize(p)  (sizeof(p)/sizeof((p)[0]))

NTSTATUS HelloWDMDispatchRoutine(IN PDEVICE_OBJECT fdo, IN PIRP Irp);
NTSTATUS HelloWDMWrite(PDEVICE_OBJECT pDevObj, PIRP pIrp);
NTSTATUS HelloWDMRead(IN PDEVICE_OBJECT /*fdo*/, IN PIRP pIrp);
NTSTATUS HelloWDMDeviceIoControl(IN PDEVICE_OBJECT, IN PIRP);
NTSTATUS DefaultPnpHandler(PDEVICE_EXTENSION pdx, PIRP Irp);

NTSTATUS ForwardAndWait(PDEVICE_EXTENSION pdx, PIRP pIrp);
VOID CancelReadIrp(IN PDEVICE_OBJECT DeviceObject, IN PIRP irp);

NTSTATUS HandleStartDevice(PDEVICE_EXTENSION pdx, PIRP pIrp);
NTSTATUS HandleRemoveDevice(PDEVICE_EXTENSION pdx, PIRP Irp);
NTSTATUS PnpQueryCapabilitiesHandler(IN PDEVICE_EXTENSION pdx, IN PIRP irp);

NTSTATUS ReadWriteConfigSpace(
  IN PDEVICE_OBJECT DeviceObj,
  IN ULONG ReadOrWrite,
  IN PVOID Buffer,
  IN ULONG Offset,
  IN ULONG Length
);

VOID ShowResources(IN PCM_PARTIAL_RESOURCE_LIST list);


#define SetMostPoweredState( SystemState, OurDeviceState)	\
	dps = deviceCapabilities->DeviceState[SystemState];		\
	if( dps==PowerDeviceUnspecified || dps>OurDeviceState)	\
		deviceCapabilities->DeviceState[SystemState] = OurDeviceState


// Direct I/O method
NTSTATUS HelloWDMDirectIoRead(IN PDEVICE_OBJECT, IN PIRP);
