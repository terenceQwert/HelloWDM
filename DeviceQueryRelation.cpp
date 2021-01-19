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
  KeCancelTimer(&pdx->pollingTimer);
  return STATUS_SUCCESS;
}
