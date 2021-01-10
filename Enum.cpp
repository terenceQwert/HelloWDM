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

VOID 
GetDeviceObjectInfo(PDEVICE_OBJECT DevObj)
{
  POBJECT_HEADER ObjectHeader;
  POBJECT_HEADER_NAME_INFO ObjectNameInfo;

  if (DevObj == NULL)
  {
    KdPrint(("DevObject is null\n"));
  }
  ObjectHeader = OBJECT_TO_OBJECT_HEADER(DevObj);
  if (ObjectHeader)
  {
    // query dvice name and print
    ObjectNameInfo = OBJECT_HEADER_TO_NAME_INFO(ObjectHeader);
    if (ObjectNameInfo && ObjectNameInfo->Name.Buffer)
    {
        KdPrint(("Driver Name: %wZ = Device Name:%wZ - Driver Addresss 0x%x - Device Addresss:0x%x\n",
        &DevObj->DriverObject->DriverName,
        &ObjectNameInfo->Name,
        DevObj->DriverObject,
        DevObj));

    }

    // if unmae device 
    else if( DevObj->DriverObject)
    {
      KdPrint(("Driver Name: %wZ = Device Name:%wZ - Driver Addresss 0x%x - Device Addresss:0x%x\n",
        &DevObj->DriverObject->DriverName,
        L"NULL",
        DevObj->DriverObject,
        DevObj));
    }
  }
}

VOID GetAttachdDeviceInfo(PDEVICE_OBJECT DevObj)
{
  PDEVICE_OBJECT DeviceObject;
  if (DevObj == NULL)
  {
    KdPrint(("DevObj is NULL \n"));
    return;
  }
  DeviceObject = DevObj->AttachedDevice;
  // enumerate each device from stack
  while (DeviceObject)
  {
    KdPrint(("Attached Driver Name:%wZ, Attached Driver Address:0x%x, Attached DeviceAddress:0x%x\n",
      &DeviceObject->DriverObject->DriverName,
      DeviceObject->DriverObject,
      DeviceObject));
    // Get DeviceOject from Device's Stack
    DeviceObject = DeviceObject->AttachedDevice;
  }
}


PDRIVER_OBJECT
EnumDeviceStack(PWSTR pwsDeviceName)
{
  UNICODE_STRING DriverName;
  PDRIVER_OBJECT DriverObject = NULL;
  PDEVICE_OBJECT DeviceObject = NULL;
  // initialize unicode string 
  RtlInitUnicodeString(&DriverName, pwsDeviceName);

  KdPrint(("Start enumeration driver object \n"));
  KdPrint(("DriverName = %wZ\n", DriverName));
  ObReferenceObjectByName(&DriverName, OBJ_CASE_INSENSITIVE, NULL, 0,
    (POBJECT_TYPE)IoDriverObjectType,
    KernelMode,
    NULL,
    (PVOID*)&DriverObject);

  if (NULL == DriverObject)
  {
    KdPrint(("DriverObject = NULL, Cannnot enum this driver object\n"));
    return NULL;
  }
  DeviceObject = DriverObject->DeviceObject;
  while (DeviceObject)
  {
    GetDeviceObjectInfo(DeviceObject);
    if (DeviceObject->AttachedDevice)
    {
      GetAttachdDeviceInfo(DeviceObject);
    }
    if (DeviceObject->Vpb && DeviceObject->Vpb->DeviceObject)
    {
      // grabe device objct
      GetDeviceObjectInfo(DeviceObject->Vpb->DeviceObject);
      if (DeviceObject->Vpb->DeviceObject->AttachedDevice)
      {
        GetAttachdDeviceInfo(DeviceObject->Vpb->DeviceObject);
      }
    }
    // get next devices
    DeviceObject = DeviceObject->NextDevice;
  }
  return DriverObject;
}

