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

NTSTATUS DeviceQueryDeviceRelation(PDEVICE_EXTENSION pdx, PIRP /* pIrp */)
{
  PAGED_CODE();
  KdPrint(("DeviceQueryDeviceRelation Start \n"));
  KeCancelTimer(&pdx->pollingTimer);
  KdPrint(("DeviceQueryDeviceRelation Exit \n"));
  return STATUS_SUCCESS;
}
