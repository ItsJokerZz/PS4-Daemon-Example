#include <csignal>

#include <orbis/libkernel.h>

bool unloaded = false;

void wait_for_unload()
{
    while (!unloaded) // Will need to handle unloading if you wish.
        sceKernelSleep(1);

    raise(SIGKILL);
}

int main(int, char *[])
{
    // Will neeed to jailbreak as this daemon will be sandboxed.

    wait_for_unload();

    return 0;
}
