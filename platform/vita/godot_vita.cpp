#include <psp2/kernel/threadmgr.h>

#include <sys/syslimits.h>
#include <stdlib.h>
#include <unistd.h>

#include "main/main.h"
#include "os_vita.h"

#define MEMORY_SCELIBC_MB 180
#define MEMORY_NEWLIB_MB 100
# define SCE_NULL NULL

int _newlib_heap_size_user = MEMORY_NEWLIB_MB * 1024 * 1024;
unsigned int sceLibcHeapSize = MEMORY_SCELIBC_MB * 1024 * 1024;

extern "C"
{
    unsigned int sleep(unsigned int seconds)
    {
        sceKernelDelayThread(seconds*1000*1000);
        return 0;
    }

    int usleep(useconds_t usec)
    {
        sceKernelDelayThread(usec);
        return 0;
    }
}

int main(int argc, char *argv[]) {
	OS_Vita os;
    sceKernelLoadStartModule("vs0:sys/external/libfios2.suprx", 0, NULL, 0, NULL, NULL);
    sceKernelLoadStartModule("vs0:sys/external/libc.suprx", 0, NULL, 0, NULL, NULL);

    sceClibPrintf("Showing the path now UwU: %d %s\n", argc, argv[0]);
	char* args[] = {"--path", "app0:/game_data"};

	Error err = Main::setup("", sizeof(args)/sizeof(args[0]), args);
	if (err != OK) {
		return 255;
	}

	if (Main::start())
		os.run(); // it is actually the OS that decides how to run
	Main::cleanup();
	return 0;
}