use32

global _start

start:
_start:
    push dword string_to_print
    call puts
    add esp, 4

    int 0xd

spin:
    jmp spin

string_to_print: db 'this is a string from userspace!', 0x0a, 0x0a, 0

%include "userlib.asm"