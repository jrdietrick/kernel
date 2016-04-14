%define KERNEL_TSS          0x0008
%define KERNEL_CODE_SEGMENT 0x0010
%define KERNEL_DATA_SEGMENT 0x0018

%define VGA_START 0x000b8000

global _start

; We lied a little with the name of this file -- at this point we are
; still in 16-bit real mode. But we had to get out of that
; claustrophobic MBR sector. Just need to set up segmentation and then
; we can be on our way...
use16

start:
_start:
    cli

    ; Set up the TSS entry in the GDT
    mov ecx, tss.end - tss - 1
    mov word [gdt.tss_entry], cx
    mov ecx, tss
    mov word [gdt.tss_entry + 2], cx
    shr ecx, 16
    mov byte [gdt.tss_entry + 4], cl
    shr ecx, 8
    mov byte [gdt.tss_entry + 7], cl

    ; Populate linear addresses for some structures
    ; we can't compute at compile time
    mov dword [gdt_desc + 2], gdt
    mov dword [idt_desc + 2], idt

    ; Turn on protected mode
    mov eax, cr0
    or eax, 0x01
    mov cr0, eax

    ; Switch to our own segments (load the GDT)
    lgdt [gdt_desc]
    jmp KERNEL_CODE_SEGMENT:start32

; Now we're really in 32-bit protected mode
use32

start32:
    ; Set stack to a known location
    mov esp, LOAD_LOCATION
    mov cx, KERNEL_DATA_SEGMENT
    mov ss, cx
    mov ds, cx
    mov es, cx
    mov fs, cx
    mov gs, cx

    mov cx, KERNEL_TSS
    ltr cx

setup_idt:
    mov ecx, 0

.loop:
    push ecx
    call fill_idt
    pop ecx

    inc ecx
    cmp ecx, 32
    jl .loop

    ; Load our interrupt vectors
    lidt [idt_desc]

    sti

disable_cursor:
    ; Disable the cursor
    xor eax, eax
    xor edx, edx
    mov dx, 0x3cc
    in ax, dx
    or ax, 0x1
    mov dx, 0x3c2
    out dx, ax
    mov ax, 0xa
    mov dx, 0x3d4
    out dx, ax
    mov dx, 0x3d5
    in ax, dx
    or ax, 0x20
    out dx, ax

    ; Clear the screen and print "OK"
    call clear_screen
    mov esi, string_ok
    call print_string

halt:
    cli
    hlt
    jmp halt

clear_screen:
    mov ecx, 0
.loop:
    mov word [VGA_START + ecx], 0x0720
    add ecx, 2
    cmp ecx, 80 * 25
    jne .loop
    ret

print_string:
    mov ecx, VGA_START
    mov eax, 0x0700

.loop:
    lodsb
    cmp al, 0
    je .done
    mov word [ecx], ax
    add ecx, 2
    jmp .loop

.done:
    ret

fill_idt:
    push ebp
    mov ebp, esp
    push ebx
    mov eax, [ebp + 8]

    ; Calculate the address of our IDT descriptor
    mov ebx, idt
    mov ecx, eax
    imul ecx, 8
    add ebx, ecx

    ; Fetch the entry point from the jump table
    mov eax, [exception_jump_table + (4 * eax)]

    ; Don't fill if there is no pointer
    cmp eax, 0
    jz .skip

    mov word [ebx + 2], KERNEL_CODE_SEGMENT
    mov [ebx], ax
    shr eax, 16
    mov [ebx + 6], ax
    mov word [ebx + 4], 0x8F00

.skip:
    pop ebx
    pop ebp
    ret

align 16, db 0

gdt:
    dq 0
.tss_entry:
    dq 0x0000890000000000 ; TSS entry, will be filled in further at start
    dq 0x00CF9A000000FFFF ; kernel stack segment
    dq 0x00CF92000000FFFF ; kernel data segment
.end:

align 16, db 0

gdt_desc:
    dw gdt.end - gdt - 1
    dd 0 ; will be filled in at start

align 16, db 0

tss:
    times 104 db 0
.end:

align 16, db 0

idt:
    times 32 dq 0
.end:

align 16, db 0

idt_desc:
    dw idt.end - idt - 1
    dd 0 ; will be filled in at start

align 16, db 0

string_ok: db 'OK', 0
string_exception: db 'EXCEPTION OCCURRED', 0
string_double_fault: db 'DOUBLE FAULT', 0
string_general_protection_fault: db 'GENERAL PROTECTION FAULT', 0

%include "exceptions.asm"
