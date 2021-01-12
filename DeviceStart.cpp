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
    return status;
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
