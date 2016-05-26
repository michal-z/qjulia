format MS64 COFF

extrn '__imp_RtlFillMemory' as RtlFillMemory:qword

section '.text' code readable executable

public memset

memset:
    sub rsp,8
    mov r9d,edx
    mov rdx,r8
    mov r8d,r9d
    call [RtlFillMemory]
    add rsp,8
    ret
