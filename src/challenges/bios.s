#include "machine.s"

#bankdef org
{
    #addr 0x0000
    #size 0xe00
    #outp 0
}
#bank org

Vectors:
    #d 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
.vec08:
    jmp PrintStr
    #d8 0,0,0,0,0
.vec10:
    jmp StrCmp
    #d8 0,0,0,0,0
.vec18:
    jmp FindIndex
    #d8 0,0,0,0,0
.vec20:
    jmp ConvertHex
    #d8 0,0,0,0,0
.vec28:
    jmp MemCpy
    #d8 0,0,0,0,0
.vec30:
    jmp ReadStr
    #d8 0,0,0,0,0
.vec38:
    jmp StrTrim
    #d8 0,0,0,0,0
.vec40:
    jmp MemSet
    #d8 0,0,0,0,0

FailureCondition:
    ret

PrintStr:
	; print string in r2. has some extra control codes as well
    test r2, 0x1000
    jlt FailureCondition
	mov r0, byte [r2]
	test r0, 0
	jeq .done
    test r0, 0xF0
    jeq .print16
    test r0, 0xF1
    jeq .print8
	sys 1
	inc r2
	jmp PrintStr
.done:
	ret
.print16:
    inc r2
    mov r0, word [r2]
    test r0, 0x1000
    jlt FailureCondition
    mov r0, word [r0]
    call PrintHexNumber16
    inc r2
    inc r2
    jmp PrintStr
.print8:
    inc r2
    mov r0, word [r2]
    test r0, 0x1000
    jlt FailureCondition
    mov r0, byte [r0]
    call PrintHexNumber8
    inc r2
    inc r2
    jmp PrintStr

StrCmp:
	; compare r2 to r3, r0=0 if same
    test r2, 0x1000
    jlt FailureCondition
    test r3, 0x1000
    jlt FailureCondition
	mov r0, byte [r2]
	mov r1, byte [r3]
	test r0, r1
	jne .bail
	test r0, 0
	jeq .bail
	inc r2
	inc r3
	jmp StrCmp
.bail:
	ret

MemSet:
    ; set r1 bytes at r2 to r0
    mov byte [r2], r0
    inc r2
    dec r1
    test r1, 0
    jne MemSet

PrintHexNumber16:
    bswap r0
    call PlaceHalfHexDigit
    bswap r0
PrintHexNumber8:
    call PlaceHalfHexDigit
    ret

PlaceHalfHexDigit:
    push r0
    shr r0
    shr r0
    shr r0
    shr r0
    and r0, 0x000f
    call .placeHexDigit
    pop r0
    push r0
    and r0, 0x000f
    call .placeHexDigit
    pop r0
    ret
.placeHexDigit:
    add r0, .placeHexDigitChars
    mov r0, byte [r0]
    sys 1
    ret
.placeHexDigitChars:
    #d "0123456789ABCDEF"

FindIndex:
	mov r1, 0xffff
.testIndex:
	inc r1
    test r2, 0x1000
    jlt FailureCondition
	mov r0, byte [r2]
	inc r2
	test r0, 0xff
	jeq .endOfList
	test r0, r3
	jne .testIndex
.correct:
	mov r0, r1
	ret
.endOfList:
	mov r0, 0xffff
	ret

FindIndexUnsafe:
	mov r1, 0xffff
.testIndex:
	inc r1
	mov r0, byte [r2]
	inc r2
	test r0, 0xff
	jeq .endOfList
	test r0, r3
	jne .testIndex
.correct:
	mov r0, r1
	ret
.endOfList:
	mov r0, 0xffff
	ret

FlagSection:
    #d8 0,0,0
    #d "Glitch Research Laboratory GLVM BIOS version 1.3 / Do not distribute! / FOOLS2023_{DumpingWasAnInsideJob}"
    #d8 0,0,0

ConvertHex:
	; string in r2, result in r0
	mov r3, 0
.convertChr:
    test r2, 0x1000
    jlt FailureCondition
	; read byte
	mov r0, byte [r2]
	inc r2
	; bail out if string finished
	test r0, 0
	jeq .done
	test r0, 0xa
	jeq .done
	; shift result to left
	add r3, r3
	add r3, r3
	add r3, r3
	add r3, r3
	; find offset in table1
	push r2
	push r3
	mov r2, .convTable1
	mov r3, r0
	call FindIndexUnsafe
	pop r3
	pop r2
	; check if failed
	test r0, 0xffff
	jeq .failure
	; use offset in table2
	mov r1, .convTable2
	add r1, r0
	; add entry to r3
	mov r1, byte [r1]
	add r3, r1
	; check next char
	jmp .convertChr
.done:
	mov r0, r3
.failure:
	ret

.convTable1:
	#d "0123456789abcdefABCDEF", 0xff
.convTable2:
	#d 0x00,0x01,0x02,0x03,0x04,0x05,0x06
	#d 0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d
	#d 0x0e,0x0f,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0xff
	
MemCpy:
    ; copy r1 bytes from r3 to r2
    test r3, 0x1000
    jlt FailureCondition
    test r2, 0x1000
    jlt FailureCondition
    mov r0, byte [r3]
    mov byte [r2], r0
    inc r3
    inc r2
    dec r1
    test r1, 0
    jne MemCpy
    ret

ReadStr:
	; read r3 bytes from input to r2
	; r3 is zero if overflowed
	sys 2
    test r2, 0x1000
    jlt FailureCondition
	mov byte [r2], r0
	test r0, 0x0a
	jeq .foundNl
	test r3, 0
	jeq .noIncrement
	inc r2
	dec r3
.noIncrement:
	jmp ReadStr
.foundNl:
	inc r2
	mov r0, 0
	mov byte [r2], r0
	ret

StrTrim:
	; remove trailing 0x0a from string at r2
	mov r0, byte [r2]
	test r0, 0x0a
	jeq .quit
	test r0, 0x00
	jeq .quit
	inc r2
	jmp StrTrim
.quit:
	mov r0, 0
    test r2, 0x1000
    jlt FailureCondition
	mov byte [r2], r0
	ret
