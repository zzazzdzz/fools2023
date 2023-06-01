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
    mov r2, r3
    add r2, IntroBanner - Start
    push r3
    call PrintStr
    pop r3
    test r3, 0x2000
    jeq CorrectAddr
    mov r2, r3
    add r2, LoadMeText - Start
    jmp PrintStr

CorrectAddr:
    mov r2, PasswordBuffer
    mov r1, 18
    mov r0, 0
    call MemSet
    mov r2, EnterPasswordText
    call PrintStr
    mov r2, PasswordBuffer
    mov r3, 0xf
    call ReadStr
    mov r2, PasswordBuffer
    call StrTrim
    mov r2, PasswordBuffer
    mov r0, 0x9FC0
.keyDerivation:
    mov r1, word [r2]
    inc r2
    test r1, 0
    shl r0
    xor r0, r1
    shl r0
    add r0, r1
    jeq .done
    jmp .keyDerivation
.done:
    mov word [EncKey], r0
    mov r3, EncryptedDoc
    mov r2, DecryptionBuffer
    mov r1, 0x200
    call MemCpy
    mov r2, DecryptionBuffer
    mov r3, 0x100
.decryptLoop:
    call GetCryptWord
    mov r1, r0
    mov r0, word [r2]
    xor r0, r1
    mov word [r2], r0
    inc r2
    inc r2
    dec r3
    test r3, 0
    jne .decryptLoop
.finished:
    mov r2, HereBeDragonsText
    call PrintStr
    mov r2, DecryptionBuffer
    call PrintStr
    mov r2, HereBeDragonsText2
    call PrintStr
    ret

GetCryptWord:
    mov r0, word [EncKey]
    mul r0, 16807
    xor r0, 0x5555
    mov word [EncKey], r0
    ret

IntroBanner:
    #d "GLITCH RESEARCH LABORATORY SELF-CONTAINED ENCRYPTION TOOL", 0x0a, 0x00

LoadMeText:
    #d "PLEASE LOAD ME AT ADDRESS $2000... (EXITING)", 0x0a, 0x00

EnterPasswordText:
    #d "PLEASE ENTER ENCRYPTION PASSWORD: ", 0x00

HereBeDragonsText:
    #d "HERE IS THE ENCRYPTED DOCUMENT: "
HereBeDragonsText2:
    #d 0x0a, "--", 0x0a, 0x00

PasswordBuffer:
    #d "________________", 0x00, 0x00

EncKey:
    #d "__"
    
#d "33333333"

EncryptedDoc:
    #d incbin("report21enc.bin")

DecryptionBuffer:
