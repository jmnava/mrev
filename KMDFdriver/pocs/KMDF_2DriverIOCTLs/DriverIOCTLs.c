// -----------------------------------------------------------
// Name: KMDF2DriverIOCTLs
// Visual Studio Project: Template -> Kernel Mode Driver, Empty (KMDF)


// This header is used by kernel
#include <ntddk.h>                                                                      // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddk/
// This header contains reference material that includes specific details about the routines, structures, and data types that you will need to use to write kernel-mode drivers.
#include <wdm.h>                                                                        // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/


/**
    @brief      An I/O control code is a 32-bit value that consists of several fields (https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/defining-i-o-control-codes).

                If a new IOCTL will be available to user-mode software components, the IOCTL must be used with IRP_MJ_DEVICE_CONTROL requests. User-mode components send IRP_MJ_DEVICE_CONTROL requests by calling the DeviceIoControl, which is a Win32 function.
                If a new IOCTL will be available only to kernel-mode driver components, the IOCTL must be used with IRP_MJ_INTERNAL_DEVICE_CONTROL requests. Kernel-mode components create IRP_MJ_INTERNAL_DEVICE_CONTROL requests by calling IoBuildDeviceIoControlRequest.

                Use the system-supplied CTL_CODE macro, which is defined in Wdm.h and Ntddk.h, to define new I/O control codes. The definition of a new IOCTL code, whether intended for use with IRP_MJ_DEVICE_CONTROL or IRP_MJ_INTERNAL_DEVICE_CONTROL requests, uses the following format:

                #define IOCTL_Device_Function CTL_CODE(DeviceType, Function, Method, Access)

    @param      DeviceType          This value must match the value that is set in the DeviceType member of the driver's DEVICE_OBJECT structure (https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/specifying-device-types).
    @param      FunctionCode        Identifies the function to be performed by the driver. Values of less than 0x800 are reserved for Microsoft. Values of 0x800 and higher can be used by vendors.
    @param      TransferType        Indicates how the system will pass data between the caller of DeviceIoControl (or IoBuildDeviceIoControlRequest) and the driver that handles the IRP (METHOD_BUFFERED, METHOD_IN_DIRECT, METHOD_OUT_DIRECT, METHOD_NEITHER).
    @param      RequiredAccess      Indicates the type of access that a caller must request when opening the file object that represents the device (FILE_ANY_ACCESS, FILE_READ_DATA, FILE_READ_DATA, FILE_READ_DATA and FILE_WRITE_DATA).
**/

#define IOCTL_COMMAND_0 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_COMMAND_1 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_COMMAND_2 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)


// Device name and symbolic link
UNICODE_STRING G_DEVICE_NAME;
UNICODE_STRING G_DEVICE_SYMBOLIC_LINK;


/**
    @brief      Unloads a Windows kernel-mode driver.

                This function is called when the driver is being unloaded from memory. It is responsible for cleaning up resources and performing necessary cleanup tasks before the driver is removed from the system. For guidelines and implementation details, see the Microsoft documentation at: https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nc-wdm-driver_unload
    
    @param      pDriverObject       Pointer to a DRIVER_OBJECT structure representing the driver.
**/

VOID
DriverUnload(
    _In_    PDRIVER_OBJECT      pDriverObject                                           // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_driver_object
)
{
    // Remove symbolic link from system
    IoDeleteSymbolicLink(&G_DEVICE_SYMBOLIC_LINK);                                      // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-iodeletesymboliclink

    // Remove device object from system
    IoDeleteDevice(pDriverObject->DeviceObject);                                        // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-iodeletedevice

    // Print a debug message to indicate the driver has been unloaded
    DbgPrint("Rootkit POC: Unloading... Service has stopped");                          // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-dbgprint
}


/**
    @brief      A passthrough function for handling IRPs (I/O Request Packets).

    @param      pDeviceObject       Pointer to a DEVICE_OBJECT structure representing the device.
    @param      pIrp                Pointer to an IRP (I/O Request Packet) to be processed.

    @return     A NTSTATUS value indicating success or an error code if operation fails.
**/

NTSTATUS
DriverPassthrough(
    _In_    PDEVICE_OBJECT      pDeviceObject,                                          // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_device_object
    _In_    PIRP                pIrp                                                    // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_irp
)
{
    // Preventing compiler warnings for unused parameter
    UNREFERENCED_PARAMETER(pDeviceObject);

    // Set I/O status information
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;
    
    // Complete IRP processing and indicate no increment in stack location
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);                                           // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-iocompleterequest

    // Operation was completed successfully
    return STATUS_SUCCESS;
}


/**
    @brief      Handles IOCTLs (Input/Output Control) requests from userland.

    @param      pDeviceObject       Pointer to a DEVICE_OBJECT structure representing the device.
    @param      pIrp                Pointer to an IRP (I/O Request Packet) to be processed.

    @return     A NTSTATUS value indicating success or an error code if operation fails.
**/

NTSTATUS
DriverHandleIOCTL(
    _In_    PDEVICE_OBJECT      pDeviceObject,                                          // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_device_object
    _In_    PIRP                pIrp                                                    // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_irp
)
{
    // Preventing compiler warnings for unused parameter
    UNREFERENCED_PARAMETER(pDeviceObject);

    // Get current stack location in IRP
    // IO_STACK_LOCATION structure defines an I/O stack location, which is an entry in the I/O stack that is associated with each IRP. Each I/O stack location in an IRP has some common members and some request-type-specific members.
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);                      // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_io_stack_location, https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-iogetcurrentirpstacklocation

    // Get IOCTL code from IRP
    ULONG ControlCode = stack->Parameters.DeviceIoControl.IoControlCode;

    // Initialize a message variable with the greeting message intended for userland communication
    CHAR* message = "Hello User";

    // Handle different IOCTL codes
    switch (ControlCode)
    {

        // Handle IOCTL_COMMAND_0
        case IOCTL_COMMAND_0:
            DbgPrint("Rootkit POC: Received IOCTL_COMMAND_0\n");
            pIrp->IoStatus.Information = 0;
            break;

        // Handle IOCTL_COMMAND_1
        case IOCTL_COMMAND_1:
            DbgPrint("Rootkit POC: Received IOCTL_COMMAND_1\n");
            pIrp->IoStatus.Information = strlen(message);

            // Copy the content of a source memory block to a destination memory block
            RtlCopyMemory(pIrp->AssociatedIrp.SystemBuffer, message, strlen(message));  // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-rtlcopymemory
            break;

        // Handle IOCTL_COMMAND_2
        case IOCTL_COMMAND_2:
            DbgPrint("Rootkit POC: Received IOCTL_COMMAND_2\n");
            DbgPrint("Rootkit POC: Input received from userland -> %s", (char*)pIrp->AssociatedIrp.SystemBuffer);
            pIrp->IoStatus.Information = strlen(message);

            // Copy the content of a source memory block to a destination memory block
            RtlCopyMemory(pIrp->AssociatedIrp.SystemBuffer, message, strlen(message));  // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-rtlcopymemory
            break;

        // Handle invalid IOCTL requests
        default:
            DbgPrint("Rootkit POC: Invalid IOCTL\n");
            break;
    }

    // Set I/O status information
    pIrp->IoStatus.Status = STATUS_SUCCESS;

    // Complete IRP processing and indicate no increment in stack location
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);                                           // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-iocompleterequest

    // Operation was completed successfully
    return STATUS_SUCCESS;
}


/**
    @brief      Entry point for a Windows kernel-mode driver.
    
                This function is called when the driver is loaded into memory. It initializes the driver and performs necessary setup tasks. For guidelines and implementation details, see the Microsoft documentation at: https://learn.microsoft.com/en-us/windows-hardware/drivers/wdf/driverentry-for-kmdf-drivers
    
    @param      pDriverObject       Pointer to a DRIVER_OBJECT structure representing the driver's image in the operating system kernel.
    @param      pRegistryPath       Pointer to a UNICODE_STRING structure, containing the driver's registry path as a Unicode string, indicating the driver's location in the Windows registry.

    @return     A NTSTATUS value indicating success or an error code if initialization fails.
**/

NTSTATUS
DriverEntry(
    _In_    PDRIVER_OBJECT      pDriverObject,                                          // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_driver_object
    _In_    PUNICODE_STRING     pRegistryPath                                           // https://learn.microsoft.com/en-us/windows/win32/api/ntdef/ns-ntdef-_unicode_string
)
{
    // Preventing compiler warnings for unused parameter
    UNREFERENCED_PARAMETER(pRegistryPath);

    // Variables
    NTSTATUS Status;

    // Initialize device name and symbolic link
    RtlInitUnicodeString(&G_DEVICE_NAME, L"\\Device\\MyKernelDriver");                  // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-rtlinitunicodestring
    RtlInitUnicodeString(&G_DEVICE_SYMBOLIC_LINK, L"\\DosDevices\\MyKernelDriver");

    // Set DriverUnload routine
    pDriverObject->DriverUnload = DriverUnload;                                         // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nc-wdm-driver_unload

    // Set MajorFunction for different IRP types
    // Each driver-specific I/O stack location (IO_STACK_LOCATION) for every IRP contains a major function code (IRP_MJ_XXX), which tells the driver what operation it or the underlying device driver should carry out to satisfy the I/O request. Each kernel-mode driver must provide dispatch routines for the major function codes that it must support (https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/irp-major-function-codes).
    pDriverObject->MajorFunction[IRP_MJ_CREATE] = DriverPassthrough;                    //https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/irp-mj-create
    pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverPassthrough;                     //https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/irp-mj-close
    pDriverObject->MajorFunction[IRP_MJ_READ] = DriverPassthrough;                      //https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/irp-mj-read
    pDriverObject->MajorFunction[IRP_MJ_WRITE] = DriverPassthrough;                     //https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/irp-mj-write
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverHandleIOCTL;            //https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/irp-mj-device-control

    // Create a device object
    // The IoCreateDevice routine creates a device object for use by a driver.
    Status = IoCreateDevice(pDriverObject, 0, &G_DEVICE_NAME, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDriverObject->DeviceObject); // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-iocreatedevice
    if (!NT_SUCCESS(Status))
    {
        DbgPrint("Rootkit POC Failed: Error creating device -> Status: 0x%08X\n", Status);
        return Status;
    }

    // Create a symbolic link for the device
    // The IoCreateSymbolicLink routine sets up a symbolic link between a device object name and a user-visible name for the device.
    Status = IoCreateSymbolicLink(&G_DEVICE_SYMBOLIC_LINK, &G_DEVICE_NAME);             // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-iocreatesymboliclink
    if (!NT_SUCCESS(Status))
    {
        DbgPrint("Rootkit POC Failed: Error creating symbolic link -> Status: 0x%08X\n", Status);
        return Status;
    }

    // Print a debug message to indicate the driver has been loaded
    DbgPrint("Rootkit POC: Loading... Hello World");                                    // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-dbgprint

    // Driver initialization was completed successfully
    return STATUS_SUCCESS;
}


// -----------------------------------------------------------