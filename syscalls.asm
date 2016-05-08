align 16, db 0

db 'syscalls.asm', 0

align 16, db 0

system_call_handler:
    push ebp
    mov ebp, esp
    push edx
    push ecx
    push ebx
    push eax

    mov esi, string_system_call
    call println

    mov eax, [ebp - 0x10] ; function code
    cmp eax, 4 ; write
    jne done
write:
    mov ebx, [ebp - 0x0c] ; file handle
    cmp ebx, 1 ; stdout
    jne done
allocate_kernel_buffer:
    ; Figure out the buffer length and
    ; allocate space (on the stack for now
    ; because we're poor)
    mov ecx, [ebp - 0x04]
    mov eax, ecx
    add eax, 1 ; for null terminator, so we can use puts
    ; Ceiling to 4 and then divide by 4, to
    ; get the number of dwords we need to
    ; allocate
    add eax, 3
    shr eax, 2
.loop:
    cmp eax, 0
    je .done
    push dword 0x00000000
    dec eax
    jmp .loop
.done:
    mov edi, esp
copy_kernel_buffer:
    ; Copy the usermode buffer into the kernel
    ; space we just allocated
    mov esi, [ebp - 0x08]
    rep movsb
do_the_putc:
    ; Put those characters onto the screen!
    mov esi, esp
    call puts
done:
    mov esp, ebp
    pop ebp
    iret