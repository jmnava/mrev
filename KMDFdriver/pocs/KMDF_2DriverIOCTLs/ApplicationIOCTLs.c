// -----------------------------------------------------------
// Name: KMDF2ApplicationIOCTLs
// Visual Studio Project: Template -> Empty Project


// The primary C header file for accessing the Win32 API is the <windows.h> header file. To make a Win32 executable, the first step is to include this header file in your source code. The windows.h header file should be included before any other library include, even the C standard library files such as stdio.h or stdlib.h. This is because the windows.h file includes macros and other components that may modify, extend, or replace things in these libraries. This is especially true when dealing with UNICODE, because windows.h will cause all the string functions to use UNICODE instead. Also, because many of the standard C library functions are already included in the Windows kernel, many of these functions will be available to the programmer without needing to load the standard libraries. For example, the function sprintf is included in windows.h automatically. 
#include <windows.h>                                                // https://en.wikibooks.org/wiki/Windows_Programming/windows.h
// The C programming language provides many standard library functions for file input and output. These functions make up the bulk of the C standard library header <stdio.h>. The I/O functionality of C is fairly low-level by modern standards; C abstracts all file operations into operations on streams of bytes, which may be "input streams" or "output streams". Unlike some earlier programming languages, C has no direct support for random-access data files; to read from a record in the middle of a file, the programmer must create a stream, seek to the middle of the file, and then read bytes in sequence from the stream. 
#include <stdio.h>                                                  // https://en.wikibooks.org/wiki/C_Programming/stdio.h
// String.h is the header in the C standard library for the C programming language which contains macro definitions, constants and declarations of functions and types used not only for string handling but also various memory handling functions; the name is thus something of a misnomer. 
#include <string.h>                                                 // https://en.wikibooks.org/wiki/C_Programming/string.h


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


/**
    Main entry point
**/

int main()
{

    // Variables
    HANDLE hDevice;                                                 // Handle for interacting with the driver
    DWORD bytesReturned;                                            // Stores the number of bytes returned
    BOOL success;                                                   // Indicates the success of an operation
    int option;                                                     // Stores the user's menu selection
    char inbuffer[15] = { 0 };                                      // Input buffer for data to be sent to the driver
    char outbuffer[15] = { 0 };                                     // Output buffer for data received from the driver


    // Open I/O device
    // The function returns a handle that can be used to access the file or device for various types of I/O depending on the file or device and the flags and attributes specified.
    hDevice = CreateFile(L"\\\\.\\MyKernelDriver", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL); // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea
    
    // Error
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        printf("Error: Failed to access the device (MyKernelDriver). Please make sure the driver is installed.\n");
        return 1;
    }

    // Loop
    while (1)
    {

        // Menu
        printf("\nMenu:\n");
        printf("0. Send IOCTL_COMMAND_0\n");
        printf("1. Send IOCTL_COMMAND_1\n");
        printf("2. Send IOCTL_COMMAND_2\n");
        printf("3. Exit\n");
        printf("Select an option: ");

        // Ask user
        scanf_s("%d", &option);                                     // https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/scanf-s-scanf-s-l-wscanf-s-wscanf-s-l

        // Handle different options
        switch (option)
        {
        
            // Send IOCTL_COMMAND_0
            case 0:
                // Sends a control code directly to a specified device driver, causing the corresponding device to perform the corresponding operation.
                success = DeviceIoControl(hDevice, IOCTL_COMMAND_0, NULL, 0, NULL, 0, &bytesReturned, NULL); // https://learn.microsoft.com/en-us/windows/win32/api/ioapiset/nf-ioapiset-deviceiocontrol
                if (success)
                {
                    printf("IOCTL_COMMAND_0 sent successfully\n");
                }
                else
                {
                    printf("Error sending IOCTL_COMMAND_0\n");
                }
                break;
            
            // Send IOCTL_COMMAND_1 and receive a message from the kernel driver
            case 1:
                // Sends a control code directly to a specified device driver, causing the corresponding device to perform the corresponding operation.
                success = DeviceIoControl(hDevice, IOCTL_COMMAND_1, NULL, 0, outbuffer, sizeof(outbuffer), &bytesReturned, NULL); // https://learn.microsoft.com/en-us/windows/win32/api/ioapiset/nf-ioapiset-deviceiocontrol
                if (success)
                {
                    printf("IOCTL_COMMAND_1 sent successfully\n");
                    printf("Message received from kernel driver: %s\n", outbuffer);
                }
                else
                {
                    printf("Error sending IOCTL_COMMAND_1\n");
                }
                break;
            
            // Send IOCTL_COMMAND_2 with a message to the kernel driver and receive a response
            case 2:
                // Initialize input buffer
                strcpy_s(inbuffer, 13, "Hello Kernel");             // https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strcpy-s-wcscpy-s-mbscpy-s?view=msvc-170
                // Sends a control code directly to a specified device driver, causing the corresponding device to perform the corresponding operation.
                success = DeviceIoControl(hDevice, IOCTL_COMMAND_2, inbuffer, sizeof(inbuffer), outbuffer, sizeof(outbuffer), &bytesReturned, NULL); // https://learn.microsoft.com/en-us/windows/win32/api/ioapiset/nf-ioapiset-deviceiocontrol
                if (success)
                {
                    printf("IOCTL_COMMAND_2 sent successfully\n");
                    printf("Message sent to kernel driver: %s\n", inbuffer);
                    printf("Message received from kernel driver: %s\n", outbuffer);
                }
                else
                {
                    printf("Error sending IOCTL_COMMAND_2\n");
                }
                break;
            
            // Exit
            case 3:
                // Closes an open object handle
                CloseHandle(hDevice);                               // https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle
                return 0;
            
            // Invalid menu option
            default:
                printf("Invalid option\n");
                break;
        }
    }

    // Closes an open object handle
    CloseHandle(hDevice);                                           // https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle

    // Exit
    return 0;
}


// -----------------------------------------------------------