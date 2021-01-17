
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

  // acquire device extension
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

