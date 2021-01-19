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

VOID PollingTimerDpc(
  IN PKDPC /* pDpc */,
  IN PVOID pContext,
  IN PVOID /* SysArg1 */,
  IN PVOID /* SysArg2 */
)
{
  PDEVICE_OBJECT pDevObj = (PDEVICE_OBJECT)pContext;
  PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;

  KeSetTimer(&pdx->pollingTimer,
    pdx->pollingInterval,
    &pdx->pollingDPC);
  KdPrint(("PollingTimerDpc\n"));

  PEPROCESS pEProcess = IoGetCurrentProcess();
  PTSTR ProcessName = (PTSTR)((ULONGLONG)pEProcess + 0x174);
  KdPrint(("Porcess Name = %s\n", ProcessName));
}

