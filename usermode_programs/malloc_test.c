#include "userlib.h"


void _start (
    )
{
    int* my_tiny_array = (int*)malloc(sizeof(int) * 4);

    if (!my_tiny_array) {
        puts("Couldn't get the memory :(\n");
        puts("\n");
        puts("... but I'm going to try and use it anyway. Watch me page fault!\n");
    }

    my_tiny_array[0] = 256;

    _exit(0);
}