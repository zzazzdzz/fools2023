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
    mov r3, 7
    call ReadStr
    mov r0, word [StringBuffer]
    test r0, 0x0059
    jeq DoTest
    test r0, 0x0079
    jeq DoTest
    test r0, 0x0a59
    jeq DoTest
    test r0, 0x0a79
    jeq DoTest
    mov r2, AbortedText
    call PrintStr
    ret

DoTest:
    sys 102
.loop:
    sys 100
    call 0x3009
    sys 101
    test r0, 2
    jeq .failed
    test r0, 1
    jeq .loop
.fin:
    ret
.failed:
    mov r2, FailedText
    call PrintStr
    ret

IntroBanner:
    #d "Glitch Research Laboratory Math Coprocessor\n"
    #d "Testing Software: Function SQRT\n\n"
    #d "This program will test the SQRT function of the math module.\n"
    #d "This function, executable with CALL 0x3009, should compute the\n"
    #d "integer part of sqrt(R0) and return it in R0 (preserving R1-R3).\n\n"
    #d "Math module results will be compared with the coprocessor.\n"
    #d "Note - if math module is not loaded, this test might crash the machine!\n"
    #d "Report any bugs to administrator: ax.arwen@glitchlabsresearch.internal\n\n"
    #d "Continue with running the test (Y/N): ", 0x00

AbortedText:
    #d "Aborted.\n", 0x00

FailedText:
    #d "Test was unsuccessful.\n", 0x00

LoadMeText:
    #d "Please load this program at address $2000.", 0x0a, 0x00

StringBuffer:
    #d "________", 0x00

