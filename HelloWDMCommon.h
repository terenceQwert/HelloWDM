#pragma once
//#include <wdm.h>
#include "HelloWdmIoControl.h"


#define PAGEDCODE code_seg("PAGE")
#define LOCKEDDATA data_seg()
#define LOCKEDCODE code_seg()
#define INITDATA  data_seg("INIT")


typedef struct _MYData
{
  ULONG data_1;
  ULONG data_2;
  LIST_ENTRY listEntry;
} MYDATASTRUCT, *PMYDATASTRUCT;

typedef struct _DEVICE_EXTENSION
{
  PDEVICE_OBJECT fdo;
  PDEVICE_OBJECT my_kbdDevice;
  PDEVICE_OBJECT NextStackDevice;
  UNICODE_STRING  ustrDeviceName;
  UNICODE_STRING  ustrSymLinkName;
  UCHAR           buffer[MAXIMUM_FILENAME_LENGTH];
  ULONG           file_length;
  UNICODE_STRING interfaceName;
  PDEVICE_OBJECT LowerDevice;
  LONG            lTimerCount;
  PIRP            currentPendingIrp;
  LARGE_INTEGER   pollingInterval;
  KTIMER         pollingTimer;
  KDPC           pollingDPC;
  HANDLE          hDevice;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

#define arraysize(p)  (sizeof(p)/sizeof((p)[0]))

NTSTATUS HelloWDMDispatchRoutine(IN PDEVICE_OBJECT fdo, IN PIRP Irp);
NTSTATUS DeviceClose(IN PDEVICE_OBJECT fdo, IN PIRP Irp);
NTSTATUS HelloWDMWrite(PDEVICE_OBJECT pDevObj, PIRP pIrp);
NTSTATUS HelloWDMRead(IN PDEVICE_OBJECT /*fdo*/, IN PIRP pIrp);
NTSTATUS HelloWDMDeviceIoControl(IN PDEVICE_OBJECT, IN PIRP);
NTSTATUS DefaultPnpHandler(PDEVICE_EXTENSION pdx, PIRP Irp);
VOID      HelloWDMStartIO(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp);

NTSTATUS ForwardAndWait(PDEVICE_EXTENSION pdx, PIRP pIrp);
VOID CancelReadIrp(IN PDEVICE_OBJECT DeviceObject, IN PIRP irp);
VOID
onCancelIrp(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS HandleStartDevice(PDEVICE_EXTENSION pdx, PIRP pIrp);
NTSTATUS HandleRemoveDevice(PDEVICE_EXTENSION pdx, PIRP Irp);
NTSTATUS PnpQueryCapabilitiesHandler(IN PDEVICE_EXTENSION pdx, IN PIRP irp);
NTSTATUS DeviceQueryDeviceRelation(PDEVICE_EXTENSION pdx, PIRP /* pIrp */);

VOID OnTimer(
  IN PDEVICE_OBJECT pDevObj,
  IN PVOID      /* Context */
);

VOID PollingTimerDpc(
  IN PKDPC pDpc,
  IN PVOID pContext,
  IN PVOID SysArg1,
  IN PVOID SysArg2
);


/// test method
VOID Start_Timer_Function(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);
VOID Stop_Timer_Function(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);


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
