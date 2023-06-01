#include "machine.s"

#bankdef org
{
    #addr 0xf000
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

Start:
	mov sp, 0xff00
    sys 5
    mov word [0xfe00], r0
    mov word [StackSmashReference], r0
    sys 5
    mov word [0xfe02], r0
    mov word [StackSmashReference+2], r0
    mov r2, IntroBanner
    call PrintStr
    mov r2, NameBuffer
    mov r3, 15
    call ReadStr
    mov r2, NameBuffer
    call StrTrim

    call InfCenterLoop

    mov r2, GoodbyeText
    call PrintStr
    sys 3
.forever:
    jmp .forever

InfCenterLoop:
.inpLoop:

    mov r2, IntroBanner2
    call PrintStr
    mov r2, NameBuffer
    call PrintStr
    mov r2, IntroBanner3
    call PrintStr
    
    mov r2, ChoiceBuffer
    ; oops, forgor the "mov r3, ..."!
    call ReadStrSafeguarded

    mov r0, byte [ChoiceBuffer]
    test r0, 0x51
    jeq .leave
    test r0, 0x71
    jeq .leave
    mov r0, byte [ChoiceBuffer+1]
    test r0, 0x0a
    jeq .ok
    test r0, 0
    jeq .ok
    jmp .tooMuch
.ok:
    mov r3, byte [ChoiceBuffer]
    add r3, 0xffd0
    test r3, 3
    jgt .tooMuch
    add r3, r3
    add r3, InfoPointers
    mov r2, word [r3]
    call PrintStr
    jmp .inpLoop
.tooMuch:
    mov r2, Info0
    call PrintStr
    mov r3, 0x1000
    jmp .inpLoop
.leave:
    mov r0, word [StackSmashReference]
    mov r1, word [0xfe00]
    test r0, r1
    jne StackSmash
    mov r0, word [StackSmashReference+2]
    mov r1, word [0xfe00+2]
    test r0, r1
    jne StackSmash
    ret

ReadStrSafeguarded:
	; read r3 bytes from input to r2
	; r3 is zero if overflowed
	sys 2
	mov byte [r2], r0
	test r0, 0x0a
	jeq .foundNl
	test r3, 0
	jeq .noIncrement
	inc r2
	dec r3
.noIncrement:
	jmp ReadStrSafeguarded
.foundNl:
	inc r2
	mov r0, 0
	mov byte [r2], r0
    mov r0, word [StackSmashReference]
    mov r1, word [0xfe00]
    test r0, r1
    jne StackSmash
    mov r0, word [StackSmashReference+2]
    mov r1, word [0xfe00+2]
    test r0, r1
    jne StackSmash
	ret

StackSmash:
    mov r2, StackSmashText
    call PrintStr
    sys 3
StackSmashText:
    #d "*** stack smashing detected *** "
    #d 0xf1
    #d16 dw(0xfe00)
    #d 0xf1
    #d16 dw(0xfe01)
    #d 0xf1
    #d16 dw(0xfe02)
    #d 0xf1
    #d16 dw(0xfe03)
    #d " != "
    #d 0xf1
    #d16 dw(StackSmashReference)
    #d 0xf1
    #d16 dw(StackSmashReference+1)
    #d 0xf1
    #d16 dw(StackSmashReference+2)
    #d 0xf1
    #d16 dw(StackSmashReference+3)
    #d " - halted\n", 0x00

IntroBanner:
    #d "Welcome to Glitch Research Lab Information Server (GRLINFSRV)\n"
    #d "Enter your name for our records (max 15 characters): ", 0x00

IntroBanner2:
    #d "\n-----\n\nWelcome, ", 0x00

IntroBanner3:
    #d "!\n\nSelection of topics:\n"
    #d "(1) About the Glitch Research Laboratory\n"
    #d "(2) About the GLVM test servers\n"
    #d "(3) About the Fight Simulation Program\n\n"
    #d "Enter the number of the topic you wish to view, or 'q' to leave: ", 0x00

GoodbyeText:
    #d "Thank you. Have a nice day!\n", 0x00

InfoPointers:
    #d16 dw(Info0)
    #d16 dw(Info1)
    #d16 dw(Info2)
    #d16 dw(Info3)

Info0:
    #d "Please type 1, 2 or 3, dummy!\n", 0x00
Info1:
    #d "\n-----\n\n"
    #d "Glitch Research Laboratory is the leading glitchology research\n"
    #d "facility in all of Glitch Islands. We specialize in research of\n"
    #d "various glitch phenomena existing in Glitch Islands and in all of\n"
    #d "the surrounding areas.\n", 0x00
Info2:
    #d "\n-----\n\n"
    #d "GLVM is our innovative, new machine architecture, designed to bring\n"
    #d "the ease of Z80 assembly programming to the modern world.\n"
    #d "The simplified instruction set of the GLVM enhances productivity\n"
    #d "of our employees and significantly speeds up development of software\n"
    #d "required for our research.\n"
    #d "Work on the GLVM architecture is still ongoing, and all implementation\n"
    #d "details are strictly confidential, until further notice.\n", 0x00
Info3:
    #d "\n-----\n\n"
    #d "Fight Simulation Program is the current ongoing project of Glitch\n"
    #d "Research Laboratory. It allows for examination of behavior of\n"
    #d "malicious glitch actors, such as Missingno., without running the risk\n"
    #d "of prolonged exposure to glitches or corruption.\n"
    #d "Participants are able to fight against opponents in an isolated\n"
    #d "environment, similar to typical turn-based strategy games.\n"
    #d "The program is still in development, and as such, all details are\n"
    #d "strictly confidential.\n", 0x00

InfoX:
    #d "FOOLS2023_{DoesThisCountAsFormatString}\n", 0x00

NameBuffer:
    #d "________________"

StackSmashReference:
    #d "____"

ChoiceBuffer:
    #d "________"
