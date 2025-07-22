section .data
    hStdOut dq 0
    hStdIn  dq 0

section .bss
    tape resb 32768
    bytes_read resd 1

section .text
    global _main, _in, _out
    extern ExitProcess, GetStdHandle, WriteFile, ReadFile
    extern himom, 

_out:
    mov rcx, [rel hStdOut]
    mov rdx, rsi
    mov r8, 1
    xor r9, r9
    sub rsp, 40
    mov qword [rsp + 32], 0
    call WriteFile
    add rsp, 40
    ret

_in:
    mov rcx, [rel hStdIn]
    mov rdx, rsi
    mov r8, 1
    lea r9, [rel bytes_read]
    sub rsp, 40
    mov qword [rsp + 32], 0
    call ReadFile
    add rsp, 40
    ret

_main:
    lea rsi, [rel tape]
    mov rcx, -11
    sub rsp, 32
    call GetStdHandle
    add rsp, 32
    mov [rel hStdOut], rax
    mov rcx, -10
    sub rsp, 32
    call GetStdHandle
    add rsp, 32
    mov [rel hStdIn], rax

    sub rsp, 32
    call himom
    add rsp, 32
    sub rsp, 32
    call function
    add rsp, 32
    sub rsp, 32
    call fallthrough
    add rsp, 32
    sub rsp, 32
    call himom
    add rsp, 32
    sub rsp, 32
    call eof
    add rsp, 32

function:
    sub rsp, 32
    call himom
    add rsp, 32

fallthrough:
    sub rsp, 32
    call himom
    add rsp, 32
    ret

eof:
    xor rcx, rcx
    call ExitProcess
