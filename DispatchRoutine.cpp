#ifdef __cplusplus
extern "C"
{
#endif
#include <wdm.h>
#ifdef __cplusplus
}
#endif
#include "HelloWDMCommon.h"
#include "Feature_Flag.h"
#pragma PAGEDCODE
NTSTATUS HelloWDMDispatchRoutine(IN PDEVICE_OBJECT /*fdo*/, IN PIRP pIrp)
{

  PAGED_CODE();
  KdPrint(("Enter HelloWDMDispatchRoutine\n"));
  PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
#if 1
  static char * irpname[] = {
    "IRP_MJ_CREATE",
    "IRP_MJ_CREATE_NAMED_PIPE",
    "IRP_MJ_CLOSE",
    "IRP_MJ_READ",
    "IRP_MJ_WRITE",
    "IRP_MJ_QUERY_INFORMATION",
    "IRP_MJ_SET_INFORMATION",
    "IRP_MJ_QUERY_EA",
    "IRP_MJ_SET_EA",
    "IRP_MJ_FLUSH_BUFFERS",
    "IRP_MJ_QUERY_VOLUME_INFORMATION",
    "IRP_MJ_SET_VOLUME_INFORMATION",
    "IRP_MJ_DIRECTORY_CONTROL",
    "IRP_MJ_FILE_SYSTEM_CONTROL",
    "IRP_MJ_DEVICE_CONTROL",
    "IRP_MJ_INTERNAL_DEVICE_CONTROL",
    "IRP_MJ_SHUTDOWN",
    "IRP_MJ_LOCK_CONTROL",
    "IRP_MJ_CLEANUP",
    "IRP_MJ_CREATE_MAILSLOT",
    "IRP_MJ_QUERY_SECURITY",
    "IRP_MJ_SET_SECURITY",
    "IRP_MJ_POWER",
    "IRP_MJ_SYSTEM_CONTROL"
    "IRP_MJ_DEVICE_CHANGE"
    "IRP_MJ_QUERY_QUOTA"
    "IRP_MJ_SET_QUOTA"
    "IRP_MJ_PNP"
  };
#endif // DBGS
  UCHAR type = stack->MajorFunction;
  if( type >= arraysize(irpname))
  {
    KdPrint(("- unknown IRP, major type = %X\n", type));
  }
  else
  {
    KdPrint(("\t%s\n", irpname[type]));
  }

  NTSTATUS status = STATUS_SUCCESS;
  pIrp->IoStatus.Status = STATUS_SUCCESS;
  pIrp->IoStatus.Information = 0;
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);
  
  KdPrint(("Leave HelloWDMDispatchRoutine"));
  return status;
}


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

#pragma PAGEDCODE
NTSTATUS
HelloWDMWrite(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
  KdPrint(("HelloWDMWrite Entry\n"));
  NTSTATUS Status = STATUS_SUCCESS;
  PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
  PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
  ULONG ulWriteLength = stack->Parameters.Write.Length;
  ULONG ulWriteOffset = (ULONG)stack->Parameters.Write.ByteOffset.QuadPart;
  KdPrint(("Preparing write length = %d, offset = %d\n", ulWriteLength, ulWriteOffset));
  if (ulWriteOffset + ulWriteLength > MAXIMUM_FILENAME_LENGTH)
  {
    Status = STATUS_FILE_INVALID;
    ulWriteLength = 0;
  }
  else
  {
    memcpy(pDevExt->buffer + ulWriteOffset, pIrp->AssociatedIrp.SystemBuffer, ulWriteLength);
    Status = STATUS_SUCCESS;
    KdPrint(("============\n"));
    for (ULONG i = 0; i < ulWriteLength; i++)
    {
      KdPrint(("0x%02x ", pDevExt->buffer[i]));
    }
    KdPrint(("\n"));
    KdPrint(("============\n"));
    if(ulWriteLength + ulWriteOffset > pDevExt->file_length)
    {
      pDevExt->file_length = ulWriteLength + ulWriteOffset;
    }
  }
  pIrp->IoStatus.Status = Status;
  pIrp->IoStatus.Information = ulWriteLength;
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  KdPrint(("HelloWDMWrite Exit\n"));
  return Status;
}