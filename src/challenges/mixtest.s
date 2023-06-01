#include "machine.s"

#bankdef org
{
    #addr 0x2000
    #size 0xe00
    #outp 0
}
#bank org

PrintStr     = 0x0008
StrCmp       = 0x0010
FindIndex    = 0x0018
ConvertHex   = 0x0020
MemCpy       = 0x0028
ReadStr      = 0x0030
StrTrim      = 0x0038
MemSet       = 0x0040

Start:
    push ip
    pop r3
    test r3, 0x2000
    jeq CorrectAddr
    mov r2, r3
    add r2, LoadMeText - Start
    jmp PrintStr

CorrectAddr:
    mov r2, IntroBanner
    call PrintStr
    mov r2, StringBuffer
    mov r3, 16
    call ReadStr
    mov r2, StringBuffer
    call StrTrim
    mov r2, WaitText
    call PrintStr
    
    mov r3, 0
    mix

    #d8 0xa7 ; mov r1, imm
    #d16 dw(StringBuffer)
    #d8 0xfc ; inc r1
    #d8 0xf0 ; mov r0, word [r1]
    #d8 0xa7 ; mov r1, imm
    #d16 dw(0x5871)
    #d8 0x1a ; test r0, r1
    #d8 0x65 ; jne ...
    #d16 dw(.pos1)
    #d8 0xdf ; inc r3
.pos1:
    #d8 0x78 ; mix

    #d8 0x37 ; mov r0, word [...]
    #d16 dw(StringBuffer+3)
    #d8 0xfc ; xor r0, ...
    #d16 dw(0x54c2)
    #d8 0x5f ; test r0, ...
    #d16 dw(0x6da4)
    #d8 0xdd ; jne ...
    #d16 dw(.pos2)
    #d8 0x49 ; inc r3
.pos2:
    #d8 0x95 ; mix

    #d8 0x8b ; mov r1, ...
    #d16 dw(StringBuffer+1)
    #d8 0x67 ; dec r1
    #d8 0x2a ; mov r0, byte [r1]
    #d8 0xf2 ; add r0, r0
    #d8 0xf2 ; add r0, r0
    #d8 0x5b ; test r0, ...
    #d16 dw(0x11c)
    #d8 0x3b ; jne ...
    #d16 dw(.pos3)
    #d8 0x8a ; inc r3
.pos3:
    #d8 0x4d ; mov r0, ...
    #d16 dw(0xffff)
.pos4:
    #d8 0x64 ; dec r0
    #d8 0x5b ; test r0, ...
    #d16 0x0102
    #d8 0x3b ; jne ...
    #d16 dw(.pos4)
    #d8 0x5c ; unmix

    mov r0, byte [StringBuffer+5]
    test r0, 0
    jne .pos5
    inc r3
.pos5:
    test r3, 4
    jeq .correct
    mov r2, FailedText
    call PrintStr
    ret
.correct:
    mov r2, CorrectText
    call PrintStr
    ret


IntroBanner:
    #d "MIX/UNMIX opcodes - proof of concept\n"
    #d "Enter a password: ", 0x00

AbortedText:
    #d "Aborted.\n", 0x00

FailedText:
    #d "Nope, it's wrong.\n", 0x00

WaitText:
    #d "Validating...\n", 0x00

CorrectText:
    #d "Yes, it's correct! FOOLS2023_{*insert correct pass here*}\n", 0x00

LoadMeText:
    #d "Please load this program at address $2000.", 0x0a, 0x00

StringBuffer:
    #d "________________________", 0x00

