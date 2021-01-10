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
#pragma PAGECODE
NTSTATUS HelloWDMDeviceIoControl(
  IN PDEVICE_OBJECT /* pDevObj */,
  IN PIRP pIrp
)
{
  NTSTATUS status = STATUS_SUCCESS;
  KdPrint(("HelloWDMDeviceIoControl Entry\n"));

  PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
  ULONG cbin = stack->Parameters.DeviceIoControl.InputBufferLength;
  ULONG cbout = stack->Parameters.DeviceIoControl.OutputBufferLength;
  ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;
  UCHAR * inputBuffer = (UCHAR*)pIrp->AssociatedIrp.SystemBuffer;
  UCHAR * OutputBuffer = (UCHAR*)pIrp->AssociatedIrp.SystemBuffer;
  ULONG info = 0;
  switch (code)
  {
  case IOCTL_TEST1:
    KdPrint(("IOCTL_TEST1\n"));
    for (ULONG i = 0; i < cbin; i++)
    {
      KdPrint(("%X\n", inputBuffer[i]));
    }
    memset(OutputBuffer, 0xAA, cbout);
    info = cbout;
    EnumDeviceStack(ACPI_PATH);
    break;
  default:
    status = STATUS_INVALID_VARIANT;
    break;
  }
  pIrp->IoStatus.Status = status;
  pIrp->IoStatus.Information = info;
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  KdPrint(("HelloWDMDeviceIoControl Exit\n"));
  return status;
}
