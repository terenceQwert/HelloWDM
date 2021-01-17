
#ifdef __cplusplus
extern "C"
{
#endif
#include <wdm.h>
#ifdef __cplusplus
}
#endif
#include "HelloWDMCommon.h"
#include  "SimulateData.h"
#include "Feature_Flag.h"

#pragma PAGEDCODE
NTSTATUS HelloWDMRead(
  IN PDEVICE_OBJECT pDevObj,
  IN PIRP pIrp)
{

  KdPrint(("HelloWDMRead Entry\n"));
  NTSTATUS Status = STATUS_SUCCESS;
  UNICODE_STRING  DeviceName;
  PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
  OBJECT_ATTRIBUTES objAttributes;
  IO_STATUS_BLOCK   statusBlock;

  RtlInitUnicodeString(&DeviceName, L"\\Device\\00000020");
  // initialize objAttributes
  InitializeObjectAttributes(&objAttributes, &DeviceName, OBJ_CASE_INSENSITIVE, NULL, NULL);
  Status = ZwCreateFile(
    &pdx->hDevice,
    FILE_READ_ATTRIBUTES | SYNCHRONIZE,
    &objAttributes,
    &statusBlock,
    NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ,
    FILE_OPEN_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
  // acquire device extension
  if (NT_SUCCESS(Status))
  {
    KdPrint(("Read Entry -- Open another device success\n"));
    ZwClose(pdx->hDevice);
  }
#if USE_IRP_PENDING 
  IoSetCancelRoutine(pIrp, CancelReadIrp);
  // pending this irp 
  IoMarkIrpPending(pIrp);
#if DRIVER_START_IO
  // put it to queue for StartIo Serial handling 
  IoStartPacket(pDevObj, pIrp, 0, onCancelIrp);
#endif
  KdPrint(("HelloWDMRead Exit\n"));
  return (Status = STATUS_PENDING);
#else
  PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
  PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
  ULONG ulReaLength = stack->Parameters.Read.Length;
  pIrp->IoStatus.Status = Status;
  pIrp->IoStatus.Information = ulReaLength;
  //  memset(pIrp->AssociatedIrp.SystemBuffer, 0xaa, ulReaLength);
  memcpy(pIrp->AssociatedIrp.SystemBuffer, pDevExt->buffer, ulReaLength);
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  KdPrint(("HelloWDMRead Exit\n"));
  return Status;
#endif
}

