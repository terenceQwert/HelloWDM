#ifdef __cplusplus
extern "C"
{
#endif
#include <wdm.h>
#ifdef __cplusplus
}
#endif
#include "HelloWDMCommon.h"
#include "Enum.h"
#include "Feature_Flag.h"

extern "C" NTSTATUS DriverEntry(
  IN PDRIVER_OBJECT DriverObject,
  IN PUNICODE_STRING
);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
// #pragma alloc_text (INIT, EchoPrintDriverVersion)
// #pragma alloc_text (PAGE, EchoEvtDeviceAdd)
#endif
NTSTATUS HelloWDMAddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT PhysicalDeviceObject);
NTSTATUS HelloWDMPnp(IN PDEVICE_OBJECT fdo, IN PIRP pIrp);
NTSTATUS HelloWDMDispatch(IN PDEVICE_OBJECT fdo, IN PIRP pIrp);
VOID DumpDeviceStack(IN PDEVICE_OBJECT pdo);
VOID DisplayProcessName();
VOID LinkListTest();
VOID New_Test();
void CreateFileFromWDM();


void HelloWDMUnload(IN PDRIVER_OBJECT DriverObject);

#pragma INITCODE
extern "C"
NTSTATUS DriverEntry(
  IN PDRIVER_OBJECT DriverObject,
  IN PUNICODE_STRING RegistryEntry
)
{
  KdPrint(("Enter HelloWDM DriverEntry\n"));
  KdPrint(("Registry = %wZ\n", *RegistryEntry));
  DriverObject->DriverExtension->AddDevice = HelloWDMAddDevice;
  DriverObject->DriverStartIo = HelloWDMStartIO;
  DriverObject->MajorFunction[IRP_MJ_PNP] = HelloWDMPnp;
  DriverObject->MajorFunction[IRP_MJ_CREATE] = HelloWDMDispatchRoutine;
  DriverObject->MajorFunction[IRP_MJ_CLOSE] = DeviceClose;
#if BUFFER_IO
  DriverObject->MajorFunction[IRP_MJ_READ] = HelloWDMRead;
#else
  DriverObject->MajorFunction[IRP_MJ_READ] = HelloWDMDirectIoRead;
#endif
  DriverObject->MajorFunction[IRP_MJ_WRITE] = HelloWDMWrite;
  DriverObject->MajorFunction[IRP_MJ_CLEANUP] = HelloWDMDispatchRoutine;
  DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HelloWDMDeviceIoControl;
  DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION] = HelloWDMDispatch;
  DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = HelloWDMDispatchRoutine;
  DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = HelloWDMDispatch;
  DriverObject->DriverUnload = HelloWDMUnload;
  KdPrint(("Leave HelloWDM DriverEntry\n"));
  return STATUS_SUCCESS;
}


#pragma PAGED_CODE
NTSTATUS HelloWDMAddDevice(
  IN PDRIVER_OBJECT DriverObject, 
  IN PDEVICE_OBJECT PhysicalDeviceObject  /* PDO*/
)
{
  PAGED_CODE();
  KdPrint(("Enter HelloWDMAddDevice\n"));

  NTSTATUS ntStatus = STATUS_SUCCESS;
  PDEVICE_OBJECT fdo;
  UNICODE_STRING  devName;
  KdPrint(("HelloWDMAddDevice initialize unicode\n"));
  RtlInitUnicodeString(&devName, MYWDM_NAME);
  KdPrint(("HelloWDMAddDevice IoCreateDevice\n"));
  ntStatus = IoCreateDevice(
    DriverObject,
    sizeof(DEVICE_EXTENSION),
#if USE_NAME
    // &devName,
#else
    NULL,
#endif
    FILE_DEVICE_UNKNOWN,
    0,
    FALSE,
    &fdo
  );
  if (!NT_SUCCESS(ntStatus))
  {
    KdPrint(("HelloWDMAddDevice IoCreateDevice fail with status %r\n", ntStatus));
    return STATUS_SUCCESS;
//    return ntStatus; 
  }
  KdPrint(("HelloWDMAddDevice create name start\n"));
  PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;
  pdx->fdo = fdo;
  memset(pdx->buffer, 0, sizeof(pdx->buffer));
  pdx->NextStackDevice = IoAttachDeviceToDeviceStack(fdo, PhysicalDeviceObject);
  KdPrint(("Print Device Address 0x%x\n", pdx->NextStackDevice));

  // register device's interface
  ntStatus = IoRegisterDeviceInterface(PhysicalDeviceObject, &MyWDMDevice, NULL, &pdx->interfaceName);
  if (!NT_SUCCESS(ntStatus))
  {
    IoDeleteDevice(fdo);
    return ntStatus;
  }
#if USE_NAME
  UNICODE_STRING  symLinkName;
  RtlInitUnicodeString(&symLinkName, SMBOL_LNK_NAME);
  pdx->ustrDeviceName = devName;
  pdx->ustrSymLinkName = symLinkName;

  ntStatus = IoCreateSymbolicLink(&symLinkName, &devName);
  if (!NT_SUCCESS(ntStatus))
  {
    KdPrint(("HelloWDMAddDevice - create symbolic link fail with status = %d\n", ntStatus));
    IoDeleteSymbolicLink(&pdx->ustrDeviceName);
    ntStatus = IoCreateSymbolicLink(&symLinkName, &devName);

    if (!NT_SUCCESS(ntStatus))
    {
      return ntStatus;
    }
  }
#else
  KdPrint(("%wZ\n", &pdx->interfaceName));
  IoSetDeviceInterfaceState(&pdx->interfaceName, TRUE);
  if (!NT_SUCCESS(ntStatus))
  {
    return ntStatus;
  }
#endif
#if BUFFER_IO
  fdo->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;
#else
  fdo->Flags |= DO_DIRECT_IO;
#endif
  fdo->Flags &= ~DO_DEVICE_INITIALIZING;
#if 0
  IoInitializeTimer(fdo, OnTimer, NULL);
#else
  KeInitializeTimer(&pdx->pollingTimer);
  KeInitializeDpc(&pdx->pollingDPC, PollingTimerDpc, (PVOID)fdo);
#endif
  KdPrint(("Leave HelloWDMAddDevice\n"));
  DumpDeviceStack(PhysicalDeviceObject);
//  DisplayProcessName();
  CreateFileFromWDM();

  return STATUS_SUCCESS;
}

#pragma
NTSTATUS DefaultPnpHandler(PDEVICE_EXTENSION pdx, PIRP Irp)
{
  PAGED_CODE();
  KdPrint(("Enter DefaultHandler\n"));
  IoSkipCurrentIrpStackLocation(Irp);
  KdPrint(("Leave DefaultHandler\n"));
  return IoCallDriver(pdx->NextStackDevice, Irp);
}



#if USE_NAME
#pragma
NTSTATUS HandleRemoveDevice(PDEVICE_EXTENSION pdx, PIRP pIrp)
{
  PAGED_CODE();
  KdPrint(("Enter HanldeRemoveDevice\n"));
  pIrp->IoStatus.Status = DefaultPnpHandler(pdx, pIrp);
  IoDeleteSymbolicLink(&pdx->ustrSymLinkName);

  if (pdx->NextStackDevice)
    IoDetachDevice(pdx->NextStackDevice);
  IoDeleteDevice(pdx->fdo);
  KdPrint(("Leave HandleRemoveDevice\n"));
  return STATUS_SUCCESS;
}
#endif

#pragma
NTSTATUS HelloWDMDispatch(IN PDEVICE_OBJECT , IN PIRP irp)
{
  PAGED_CODE();
  KdPrint(("Enter HelloWDMDispatchRoutine\n"));
  irp->IoStatus.Status = STATUS_SUCCESS;
  irp->IoStatus.Information = 0;  // no bytes transferred.
  IoCompleteRequest(irp, IO_NO_INCREMENT);
  KdPrint(("Leave HelloWDMDispatchRoutine\n"));
  return STATUS_SUCCESS;
}

#pragma
void HelloWDMUnload(IN PDRIVER_OBJECT )
{
  PAGED_CODE();
  KdPrint(("Enter HelloWDMUnload\n"));
//  LinkListTest();
//  DisplayProcessName();
  KdPrint(("Leave HelloWDMUnload\n"));
}

#pragma PAGEDCODE
NTSTATUS HelloWDMPnp(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{

  PAGED_CODE();
#if DBG
  static char * fcnName[] = {
    "IRP_MN_START_DEVICE",
    "IRP_MN_QUERY_REMOVE_DEVICE",
    "IRP_MN_REMOVE_DEVICE",
    "IRP_MN_CANCEL_REMOVE_DEVICE",
    "IRP_MN_STOP_DEVICE",
    "IRP_MN_QUERY_STOP_DEVICE",
    "IRP_MN_CANCEL_STOP_DEVICE",
    "IRP_MN_QUERY_DEVICE_RELATIONS",
    "IRP_MN_QUERY_INTERFACE",
    "IRP_MN_QUERY_CAPABILITIES",
    "IRP_MN_QUERY_RESOURCES",
    "IRP_MN_QUERY_RESOURCE_REQUIREMENTS",
    "IRP_MN_QUERY_DEVICE_TEXT",
    "IRP_MN_FILTER_RESOURCE_REQUIREMENTS",
    "",
    "IRP_MN_READ_CONFIG",
    "IRP_MN_WRITE_CONFIG",
    "IRP_MN_EJECT",
    "IRP_MN_SET_LOCK",
    "IRP_MN_QUERY_ID",
    "IRP_MN_QUERY_PNP_DEVICE_STATE",
    "IRP_MN_QUERY_BUS_INFORMATION",
    "IRP_MN_DEVICE_USAGE_NOTIFICATION",
    "IRP_MN_SURPRISE_REMOVAL"
  };  
#endif // DBGS
  KdPrint(("Enter HelloWDMPnp\n"));
  NTSTATUS status = STATUS_SUCCESS;
  PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;
  PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
  static NTSTATUS(*fcntab[]) (PDEVICE_EXTENSION pdx, PIRP Irp) = {
#if 1
   HandleStartDevice, // // IRP_MN_START_DEVICE
#else
    DefaultPnpHandler,  // Start Device
#endif
    DefaultPnpHandler,  // IRP_MN_QUERY_REMOVE_DEVICE
    HandleRemoveDevice, // IRP_MN_REMOVE_DEVICE
    DefaultPnpHandler,  // IRP_MN_CANCEL_REMOVE_DEVICE
    DefaultPnpHandler,  // IRP_MN_STOP_DEVICE
    DefaultPnpHandler,  // IRP_MN_QUERY_STOP_DEVICE
    DefaultPnpHandler,  // IRP_MN_CANCEL_STOP_DEVICE
    DeviceQueryDeviceRelation,  // IRP_MN_QUERY_DEVICE_RELATIONS
    DefaultPnpHandler,  // IRP_MN_QUERY_INTERFACE
    PnpQueryCapabilitiesHandler,  // IRP_MN_QUERY_CAPABILITIES
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler
  };
  ULONG fcn = stack->MinorFunction;
  if (fcn >= arraysize(fcntab))
  {
    status = DefaultPnpHandler(pdx, Irp);
    return status;
  }
  KdPrint(("PNP  Request (%s)\n", fcnName[fcn]));
  status = (*fcntab[fcn])(pdx, Irp);
  KdPrint(("Leave HelloWDMPnp"));
  return status;
}


