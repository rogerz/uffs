/// uffs ThreadX port

#include "uffs/uffs_os.h"

int uffs_SemCreate(int n)
{
	return 1;
}

int uffs_SemWait(int sem)
{
	return 1;
}

int uffs_SemSignal(int sem)
{
	return 1;
}

int uffs_SemDelete(int *sem)
{
	return 1;
}

#if CONFIG_USE_NATIVE_MEMORY_ALLOCATOR > 0
void uffs_CriticalEnter(void)
{
	return;
}
void uffs_CriticalExit(void);
{
	return;
}
#endif

int uffs_OSGetTaskId(void)
{
	return 0;
}

unsigned int uffs_GetCurDateTime(void)
{
	return 0;
}
