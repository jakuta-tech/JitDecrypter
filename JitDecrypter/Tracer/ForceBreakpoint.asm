
IFNDEF RAX             ; check for x86 or x64

.386                   ; enable 32-bit instructions for x86
.model flat, c         ; use C calling convention and flat memory model

ENDIF

public ForceBreakpoint

.code

ForceBreakpoint PROC
   int 3
   nop
   ret
ForceBreakpoint ENDP

END