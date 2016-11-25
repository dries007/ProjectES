
#include <sys/stat.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

register char * stack_ptr asm("sp");

caddr_t _sbrk(int incr)
{
	extern char end asm("end");
	static char *heap_end;
	char *prev_heap_end;

	if (heap_end == 0)
		heap_end = &end;

	prev_heap_end = heap_end;
	if (heap_end + incr > stack_ptr)
	{
        // FATAL
        while (1);
	}

	heap_end += incr;

	return (caddr_t) prev_heap_end;
}

#pragma clang diagnostic pop