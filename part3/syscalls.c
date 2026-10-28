// Elevator system calls (548-550), compiled into the kernel.
// Each one forwards to a function pointer that the elevator module sets
// on insmod and clears on rmmod. With no module loaded they return -ENOSYS.

#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/export.h>

int (*STUB_start_elevator)(void) = NULL;
int (*STUB_issue_request)(int, int, int) = NULL;
int (*STUB_stop_elevator)(void) = NULL;
EXPORT_SYMBOL(STUB_start_elevator);
EXPORT_SYMBOL(STUB_issue_request);
EXPORT_SYMBOL(STUB_stop_elevator);

SYSCALL_DEFINE0(start_elevator)
{
	if (STUB_start_elevator != NULL)
		return STUB_start_elevator();
	return -ENOSYS;
}

SYSCALL_DEFINE3(issue_request, int, start_floor, int, destination_floor, int, type)
{
	if (STUB_issue_request != NULL)
		return STUB_issue_request(start_floor, destination_floor, type);
	return -ENOSYS;
}

SYSCALL_DEFINE0(stop_elevator)
{
	if (STUB_stop_elevator != NULL)
		return STUB_stop_elevator();
	return -ENOSYS;
}
