
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

NTSTATUS DeviceClose(IN PDEVICE_OBJECT , IN PIRP /* Irp */)
{
  PAGED_CODE();
  KdPrint(("Enter DeviceClose\n"));

  KdPrint(("Leave DeviceClose\n"));
  return STATUS_SUCCESS;
}
