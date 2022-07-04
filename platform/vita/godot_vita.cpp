#include <psp2/kernel/threadmgr.h>

#include <sys/syslimits.h>
#include <stdlib.h>
#include <unistd.h>

#include "main/main.h"
#include "os_vita.h"
#include <taihen.h>

#define MEMORY_SCELIBC_MB 20
#define MEMORY_NEWLIB_MB 120
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
    char title_id[0xA];
    char app_dir_path[0x100];
    char app_kernel_module_path[0x100];
    SceUID pid = -1;
    sceKernelLoadStartModule("vs0:sys/external/libfios2.suprx", 0, NULL, 0, NULL, NULL);
    sceKernelLoadStartModule("vs0:sys/external/libc.suprx", 0, NULL, 0, NULL, NULL);

    pid = sceKernelGetProcessId();
    sceAppMgrAppParamGetString(pid, 12, title_id, sizeof(title_id));
    snprintf(app_dir_path, sizeof(app_dir_path), "ux0:app/%s", title_id);
    snprintf(app_kernel_module_path, sizeof(app_kernel_module_path), "%s/module/libgpu_es4_kernel_ext.skprx", app_dir_path);

    SceUID res = taiLoadStartKernelModule(app_kernel_module_path, 0, NULL, 0);
    if (res < 0) {
        sceClibPrintf("Failed to load kernel module: %08x\n", res);
    }

    scePowerSetArmClockFrequency(444);
    scePowerSetBusClockFrequency(222);
    scePowerSetGpuClockFrequency(222);
    scePowerSetGpuXbarClockFrequency(166);

    sceClibPrintf("Showing the path now UwU: %d %s\n", argc, argv[0]);
	char* args[] = {"--path", "app0:/game_data", "--main-pack", "app0:/game_data/game.pck"};

	Error err = Main::setup("", sizeof(args)/sizeof(args[0]), args);
	if (err != OK) {
		return 255;
	}

	if (Main::start())
		os.run(); // it is actually the OS that decides how to run
	Main::cleanup();
	return 0;
}