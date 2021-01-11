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
#include "PciCommon.h"
VOID EnumeratePCI()
{
  ULONG bus=0;
  ULONG dev;
  ULONG func;
  PCI_COMMON_CONFIG PciConfig;
  PCI_SLOT_NUMBER   SlotNumber;

  KdPrint(("Bus \tDevice\tFunc\tVendor\tDevice\tBaseCls\tSubCls\tIRQ\tPIN\n"));
  for (dev = 0; dev < PDI_DEVICE_MAX; dev++)
  {
    for (func = 0; func < PDI_FUNCTION_MAX; func++)
    {
      SlotNumber.u.AsULONG = 0;
      SlotNumber.u.bits.DeviceNumber = dev;
      SlotNumber.u.bits.FunctionNumber = func;
      RtlZeroMemory(&PciConfig, sizeof(PCI_COMMON_CONFIG));
//      ULONG size = HalGetBusData()
    }
  }
}

#pragma PAGEDCODE
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
  PULONG dwIntputBuffer = (ULONG*)pIrp->AssociatedIrp.SystemBuffer;
  PULONG dwOutputBuffer = (PULONG)pIrp->AssociatedIrp.SystemBuffer;

  ULONG info = 0;
  UCHAR PORT = 0x70;
  UCHAR Data = 0x71;
  UCHAR Port_Offset = 0x50;
  UCHAR buf = 0;
  UCHAR index = 0;
  KSPIN_LOCK my_SpinLock;
  KIRQL       irql;

  // IO Action
  ULONG port = (ULONG)(*dwIntputBuffer);
  dwIntputBuffer++;
  UCHAR method = (UCHAR)(*dwIntputBuffer);
  dwIntputBuffer++;
  ULONG value = (ULONG)(*dwIntputBuffer);
  KeInitializeSpinLock(&my_SpinLock);
  switch (code)
  {
  case IOCTL_TEST1:
    KdPrint(("IOCTL_TEST1\n"));
    for (ULONG i = 0; i < cbin; i++)
    {
      KdPrint(("%X\n", inputBuffer[i]));
    }
    KeAcquireSpinLock(&my_SpinLock, &irql);
    for( index=0;index < 0x20;index++){
      WRITE_PORT_UCHAR((PUCHAR)PORT, Port_Offset+index);
      buf = READ_PORT_UCHAR((PUCHAR)Data);
      KdPrint(("Index_%0x =0x%0x \n", index, buf));
    }
    KeReleaseSpinLock(&my_SpinLock, irql);
    memset(OutputBuffer, 0xAA, cbout);
    info = cbout;
    EnumDeviceStack(ACPI_PATH);
    break;
  case READ_PORT:
    KdPrint(("READ_PORT\n"));
    KdPrint(("port:%x\n", port));
    KdPrint(("method:%x\n", method));

    if (method == 1)
    {
      *dwOutputBuffer = READ_PORT_UCHAR((PUCHAR)port);
    }
    else if (method == 2)
    {
      *dwOutputBuffer = READ_PORT_USHORT((PUSHORT)port);
    }
    else if (method == 4)
    {
      *dwOutputBuffer = READ_PORT_ULONG((PULONG)port);
    }
    info = 4;
    break;
  case WRITE_PORT:
    KdPrint(("WRITE_PORT\n"));
    KdPrint(("port:%x\n", port));
    KdPrint(("method:%x\n", method));
    KdPrint(("value:%x\n", value));
    if (1 == method)
    {
      WRITE_PORT_UCHAR((PUCHAR)port, (UCHAR)value);
    } else if(2 == method)
    {
      WRITE_PORT_USHORT((PUSHORT)port, (USHORT)value);
    }
    else if (4 == method)
    {
      WRITE_PORT_ULONG((PULONG)port, (ULONG)value);
    }
    info = 0;
    break;
  case Enum_PCI:

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
