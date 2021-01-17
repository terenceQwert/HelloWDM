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


#pragma LOCKEDCODE
VOID OnTimer(
  IN PDEVICE_OBJECT pDevObj,
  IN PVOID      /* Context */
)
{
  // get device's extension
  PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
  KdPrint(("Enter OnTimer!\n"));
  InterlockedDecrement(&pDevExt->lTimerCount);
  LONG previousCount = InterlockedCompareExchange(&pDevExt->lTimerCount, TIMER_OUT, 0);
  // output log for each 3 seconds 
  if (previousCount == 0)
  {
    KdPrint(("%d seconds time out!\n", TIMER_OUT));
  }
  PEPROCESS pEProcess = IoGetCurrentProcess();
  PTSTR ProcessName = (PTSTR)((ULONGLONG)pEProcess + 0x174);  // Get current running process
  KdPrint(("The current process is %s\n", ProcessName));
  KdPrint(("Leave OnTimer!\n"));
}

VOID Start_Timer_Function(
  IN PDEVICE_OBJECT pDevObj,
  IN PIRP           /* pIrp */
)
{
  KdPrint(("IOCTL_START_TIMEOUT\n"));
#if 0
  PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
  ULONG cbIn = stack->Parameters.DeviceIoControl.InputBufferLength;
  ULONG cbOut = stack->Parameters.DeviceIoControl.OutputBufferLength;
#endif
  PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
  pDevExt->lTimerCount = TIMER_OUT;
  IoStartTimer(pDevObj);
}


VOID Stop_Timer_Function(
  IN PDEVICE_OBJECT pDevObj,
  IN PIRP           /* pIrp */
)
{
  KdPrint(("IOCTL_STOP_TIMEOUT\n"));
  IoStopTimer(pDevObj);
}