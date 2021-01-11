#ifdef __cplusplus
extern "C"
{
#endif
#include <wdm.h>
#ifdef __cplusplus
}
#endif
#include "HelloWDMCommon.h"

#pragma PAGEDCODE
NTSTATUS HelloWDMDirectIoRead(
  IN PDEVICE_OBJECT /* pDevObj */, 
  IN PIRP pIrp
)
{

  KdPrint(("HelloWDMDirectIoRead Entry\n"));
  NTSTATUS Status = STATUS_SUCCESS;
//  PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
  PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
  ULONG ulReadLength = stack->Parameters.Read.Length;
  KdPrint(("ulReadLength = %d\n", ulReadLength));
  ULONG mdl_length = MmGetMdlByteCount(pIrp->MdlAddress);
  PVOID mdl_address = MmGetMdlVirtualAddress(pIrp->MdlAddress);

  ULONG mdl_offset = MmGetMdlByteOffset(pIrp->MdlAddress);

  KdPrint(("mdl_address = 0x%08X", mdl_address));
  KdPrint(("mdl_length = %d", mdl_length));
  KdPrint(("mdl_offset = %d", mdl_offset));
  if (mdl_length != ulReadLength)
  {
    pIrp->IoStatus.Information = 0;
    Status = STATUS_UNSUCCESSFUL;
  }
  else {
    PVOID kernel_address = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
    KdPrint(("mdl_address = 0x%08X", kernel_address));
    memset(kernel_address, 0xAA, ulReadLength);
    pIrp->IoStatus.Information = ulReadLength;
  }
  pIrp->IoStatus.Status = Status;
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);
  KdPrint(("HelloWDMDirectIoRead Exit\n"));
  return Status;
}

#pragma PAGEDCODE
NTSTATUS OnRequestComplete(
  PDEVICE_OBJECT /* junk */, 
  PIRP /*pIrp*/, 
  PKEVENT pev
)
{
  KeSetEvent(pev, 0, FALSE);
  return STATUS_MORE_PROCESSING_REQUIRED;
}

#pragma PAGEDCODE
NTSTATUS ForwardAndWait(PDEVICE_EXTENSION pdx, PIRP pIrp)
{
  PAGED_CODE();
  KEVENT event;
  KeInitializeEvent(&event, NotificationEvent, FALSE);
  IoCopyCurrentIrpStackLocationToNext(pIrp);
  IoSetCompletionRoutine(pIrp, (PIO_COMPLETION_ROUTINE)OnRequestComplete, (PVOID)&event, TRUE, TRUE, TRUE);

  IoCallDriver(pdx->NextStackDevice, pIrp);
  // wait for PDO complete
  KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
  return pIrp->IoStatus.Status;
}


#pragma PAGEDCODE
VOID ShowResources(IN PCM_PARTIAL_RESOURCE_LIST list)
{
  PCM_PARTIAL_RESOURCE_DESCRIPTOR resource = list->PartialDescriptors;
  ULONG nres = list->Count;
  ULONG i;

  for (i = 0; i < nres; ++i, ++resource)
  {						// for each resource
    ULONG type = resource->Type;

    static char* name[] = {
      "CmResourceTypeNull",
      "CmResourceTypePort",
      "CmResourceTypeInterrupt",
      "CmResourceTypeMemory",
      "CmResourceTypeDma",
      "CmResourceTypeDeviceSpecific",
      "CmResourceTypeBusNumber",
      "CmResourceTypeDevicePrivate",
      "CmResourceTypeAssignedResource",
      "CmResourceTypeSubAllocateFrom",
    };

    KdPrint(("    type %s", type < arraysize(name) ? name[type] : "unknown"));

    switch (type)
    {					// select on resource type
    case CmResourceTypePort:
    case CmResourceTypeMemory:
      KdPrint((" start %8X%8.8lX length %X\n",
        resource->u.Port.Start.HighPart, resource->u.Port.Start.LowPart,
        resource->u.Port.Length));
      break;

    case CmResourceTypeInterrupt:
      KdPrint(("  level %X, vector %X, affinity %X\n",
        resource->u.Interrupt.Level, resource->u.Interrupt.Vector,
        resource->u.Interrupt.Affinity));
      break;

    case CmResourceTypeDma:
      KdPrint(("  channel %d, port %X\n",
        resource->u.Dma.Channel, resource->u.Dma.Port));
    }					// select on resource type
  }						// for each resource
}							// ShowResources


#pragma PAGEDCODE
NTSTATUS HandleStartDevice(

  PDEVICE_EXTENSION pdx, 
  PIRP pIrp
)
{
  PAGED_CODE();
  KdPrint(("Enter HandleStartDevcie\n"));
  NTSTATUS status = STATUS_SUCCESS;
  status = ForwardAndWait(pdx, pIrp);
  if (!NT_SUCCESS(status))
  {
    pIrp->IoStatus.Status = status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
  }

  PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
  PCM_PARTIAL_RESOURCE_LIST raw;
  if (stack->Parameters.StartDevice.AllocatedResources)
  {
    raw = &stack->Parameters.StartDevice.AllocatedResources->List[0].PartialResourceList;
  }
  else {
    KdPrint(("HandleStartDevcie - raw = NULL\n"));
    raw = NULL;
  }
  KdPrint(("Show raw resources\n"));

  // must verify resource if null.
  if( raw != NULL) {
  ShowResources(raw);
  }

  PCM_PARTIAL_RESOURCE_LIST translated;
  if (stack->Parameters.StartDevice.AllocatedResourcesTranslated)
  {
    translated = &stack->Parameters.StartDevice.AllocatedResourcesTranslated->List[0].PartialResourceList;
  }
  else {
    KdPrint(("HandleStartDevcie - translated = NULL\n"));
    translated = NULL;
  }

  KdPrint(("Show translated resources\n"));
  // must verify resource if null.
  if( translated != NULL) {
    ShowResources(translated);
  }

  KdPrint(("Show translated resources\n"));
  pIrp->IoStatus.Status = STATUS_SUCCESS;
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);
  KdPrint(("Leave HandleStartDevice\n"));
  return status;
}
