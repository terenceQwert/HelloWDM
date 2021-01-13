// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <SetupAPI.h>
#include <winioctl.h>
#include "../HelloWdmIoControl.h"
#include "../PciCommon.h"
#if 0
void _cdecl operator delete (void *pointer)
{
  std::cout << "delete invoke";
  delete [] pointer;
  pointer = NULL;
}
#endif
typedef struct _PCI_SLOT_NUMBER {
  union {
    struct {
      ULONG   DeviceNumber : 5;
      ULONG   FunctionNumber : 3;
      ULONG   Reserved : 24;
    } bits;
    ULONG   AsULONG;
  } u;
} PCI_SLOT_NUMBER, *PPCI_SLOT_NUMBER;


using namespace std;
// {27056255-0FAC-42F8-BCED-E646141A5915}
static const GUID gGuid =
{ 0x27056255, 0xfac, 0x42f8, { 0xbc, 0xed, 0xe6, 0x46, 0x14, 0x1a, 0x59, 0x15 } };


#define WRITE_LENGTH sizeof(GUID)
void Read( HANDLE devHandler)
{
  UCHAR *buffer = new UCHAR[WRITE_LENGTH];
  memset(buffer, 0, WRITE_LENGTH);
  ULONG ulRead = 0;

  BOOL bResult = ReadFile(devHandler, buffer, WRITE_LENGTH, &ulRead, NULL);
  if (bResult)
  {
    printf("Read %d bytes", ulRead);
    for (ULONG i = 0; i < ulRead; i++)
    {
      printf("0x%02x ", buffer[i]);
    }
    printf("\n");
  }

}

void Write(HANDLE devHandler)
{
#if BUFFER_IO // current driver is with BUFFER_IO
  UCHAR *buffer = new UCHAR[WRITE_LENGTH];
  DWORD ulWrite = 0;
  memcpy_s(buffer, WRITE_LENGTH, &gGuid, WRITE_LENGTH);
  BOOL bResult = WriteFile(devHandler, buffer, WRITE_LENGTH, &ulWrite, NULL);
  if (bResult)
  {
    printf("write data success, length = %d\n", ulWrite);
  }
#endif
}
#pragma PAGEDCODE
void IOCTL( HANDLE devHandle)
{
  ULONG cbin = sizeof(GUID);
  UCHAR *InputBuffer = new UCHAR[sizeof(GUID)];
  ULONG cbout = sizeof(GUID);
  UCHAR *OutputBuffer = new UCHAR[sizeof(GUID)];
  DWORD bBytesReturn = 0;

  memset(InputBuffer, 0xEE, sizeof(GUID));
  memset(OutputBuffer, 0, sizeof(GUID));

  BOOL bResult = DeviceIoControl(
    devHandle,
    IOCTL_TEST1,
    InputBuffer,
    cbin,
    OutputBuffer,
    cbout,
    &bBytesReturn,
    NULL
  );
}

DWORD In_32(HANDLE hDevice, USHORT port)
{
  DWORD dwOutput;
  DWORD inputBuffer[2] = {
    port,
    4
  };
  DWORD dwResult;
  DeviceIoControl(hDevice, READ_PORT, inputBuffer, sizeof(inputBuffer), &dwResult, sizeof(DWORD), &dwOutput, NULL);
  return dwResult;
}

void Out_32(HANDLE hDevice, USHORT port, DWORD value)
{
  DWORD  dwOutput;
  DWORD inputBuffer[3] = { port, 4, value };
  DeviceIoControl(hDevice, WRITE_PORT, inputBuffer, sizeof(inputBuffer), NULL, 0, &dwOutput, NULL);
}
void DisplayPCIConfiguration(HANDLE hDevice, int bus, int dev, int fun)
{
  DWORD dwAdddr;
  DWORD dwData;
  PCI_COMMON_CONFIG pci_configuration;
  PCI_SLOT_NUMBER SlotNumber;
  SlotNumber.u.AsULONG = 0;
  SlotNumber.u.bits.DeviceNumber = dev;
  SlotNumber.u.bits.FunctionNumber = fun;
  dwAdddr = 0x80000000 | (bus << 16) | (SlotNumber.u.AsULONG << 8);
  for (int i = 0; i < 0x80; i += 4)
  {
    Out_32(hDevice, PCI_CONFIG_ADDRESS, dwAdddr | i);
    dwData = In_32(hDevice, PCI_CONFIG_DATA);
    memcpy(((PUCHAR)&pci_configuration) + i, &dwData, 4);
  }
  printf("bus:%d\tdev:%d\tfunc:%d\n", bus, dev, fun);
  printf("VendorID:%x\n", pci_configuration.VendorID);
  printf("DeviceID:%x\n", pci_configuration.DeviceID);
  printf("Command:%x\n", pci_configuration.Command);
  printf("Status:%x\n", pci_configuration.Status);
  printf("RevisionID:%x\n", pci_configuration.Status);
  printf("ProgIf:%x\n", pci_configuration.ProgIf);
  printf("SubClass:%x\n", pci_configuration.SubClass);
  printf("BaseClass:%x\n", pci_configuration.BaseClass);
  printf("CacheLineSize:%x\n", pci_configuration.CacheLineSize);
  printf("LatencyTimer:%x\n", pci_configuration.LatencyTimer);
  printf("HeaderType:%x\n", pci_configuration.HeaderType);
  printf("BIST:%x\n", pci_configuration.BIST);
  for (int i = 0; i < 6; i++)
  {
    printf("BaseAddress[%d]:0X%08X\n", i, pci_configuration.u.type0.BaseAddresses[i]);
  }
  printf("InterruptLine:%d\n", pci_configuration.u.type0.InterruptLine);
  printf("InterruptPin:%d\n", pci_configuration.u.type0.InterruptPin);
}

HANDLE GetDeviceViaInterfaces(GUID * pGuid, DWORD instances)
{
  HDEVINFO info = SetupDiGetClassDevs(pGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
  if (info == INVALID_HANDLE_VALUE)
  {
    printf("No HDEVINFO available for this GUID\n");
    return NULL;
  }

  SP_INTERFACE_DEVICE_DATA ifData;
  ifData.cbSize = sizeof(ifData);
  if (!SetupDiEnumDeviceInterfaces(info, NULL, pGuid, instances, &ifData))
  {
    printf("No SP_INTERFACE_DEVICE_DATA available for this GUID interface\n");
    SetupDiDestroyDeviceInfoList(info);
    return NULL;
  }
  DWORD RegLen;
  SetupDiGetDeviceInterfaceDetail(info, &ifData, NULL, 0, &RegLen, NULL);
  PSP_INTERFACE_DEVICE_DETAIL_DATA ifDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA) new TCHAR[RegLen];
  if (ifDetail == NULL)
  {
    SetupDiDestroyDeviceInfoList(info);
    return NULL;
  }
  // Get two symoblic link
  ifDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
  if (!SetupDiGetDeviceInterfaceDetail(info, &ifData, ifDetail, RegLen, NULL, NULL))
  {
    SetupDiDestroyDeviceInfoList(info);
    delete ifDetail;
    return NULL;
  }
  printf("Symbolic link is %ws\n", ifDetail->DevicePath);
  HANDLE rv = CreateFile(ifDetail->DevicePath,
    GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL);
  if (rv == INVALID_HANDLE_VALUE)
    rv = NULL;
  delete ifDetail;
  SetupDiDestroyDeviceInfoList(info);
  return rv;
}

int main()
{
  HANDLE hDevice = NULL;
#if USE_NAME
   hDevice = CreateFile(DRV_NAME, GENERIC_READ | GENERIC_WRITE,
    0, NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL);
  if (hDevice == INVALID_HANDLE_VALUE)
  {
    std::cout << "Fail TO OBTAIN FILE HANDLE TO DEVICE" << endl;
  }
#else
  hDevice = GetDeviceViaInterfaces(&MyWDMDevice, 0);
#endif
  DisplayPCIConfiguration(hDevice, 2, 0, 0);
  IOCTL(hDevice);
#if 0
  Read(hDevice);
  Write(hDevice);
  Read(hDevice);
#endif
  CloseHandle(hDevice);
  std::cout << "Hello World!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
