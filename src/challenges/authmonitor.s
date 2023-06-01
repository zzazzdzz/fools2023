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

FancyTable = 0xc000

EntryPoint:
	mov sp, 0xff00
	mov r0, 0xfff0
	mov r1, 0x98
	mov byte [r0], r1
	inc r0
	mov r1, BreakIntoMonitor
	mov word [r0], r1
	mov r2, Text_Intro
	call PrintStr



.startAuth:
    mov r2, UsernameStr
	call PrintStr
    mov r3, 0x10
    mov r2, UsernameBuffer
    call ReadStr

    mov r2, UserDatabase
.findUser:
    mov r0, byte [r2]
    test r0, 0
    jeq .userNotFound
    mov r3, UsernameBuffer
    push r2
    call StrCmp
    pop r2
    test r0, 0
    jeq .userFound
    add r2, 16+64
    jmp .findUser
.userFound:
    add r2, 16
    push r2
    call AuthDelay
    mov r2, PasswordStr
	call PrintStr
    mov r3, 0x30
    mov r2, PasswordBuffer
    call ReadStr
    call AuthDelay
	mov r0, 1
	mov r1, FancyTable
	sys 4
    pop r2
    mov r3, PasswordBuffer
    ; r2 = correct pass, r3 = input buf
.passwordCheck:
    push r3
    mov r1, FancyTable
    mov r0, byte [r3]
    add r1, r0
    ; r1 is the base now
    mov r3, 0
    mov r0, byte [r1]
    add r3, r0
    add r1, 0x100
    mov r0, byte [r1]
    add r3, r0
    add r1, 0x100
    mov r0, byte [r1]
    add r3, r0
    add r1, 0x100
    mov r0, byte [r1]
    add r3, r0
    mov r0, r3
    and r0, 0x00ff
    ; r0 is the enc byte
    mov r1, byte [r2]
    test r0, r1
    jne .passwordWrong
    pop r3
    call RotateFancyTable
    inc r2
    inc r3
    mov r0, byte [r2]
    test r0, 0
    jeq .fin
    jmp .passwordCheck
.fin:
    mov r2, PassSuccessStr
    call PrintStr
	brk
	mov r2, Text_ContinueTooFar
	call PrintStr
	sys 3
.forever:
	jmp .forever
.userNotFound:
    call AuthDelay
    mov r2, UserFailStr
	call PrintStr
    jmp .startAuth
.passwordWrong:
    mov r2, PassFailStr
	call PrintStr
    jmp .startAuth

RotateFancyTable:
    mov r0, byte [FancyTable+0]
    mov byte [RotateFancyTableStorage+0], r0
    mov r0, byte [FancyTable+256]
    mov byte [RotateFancyTableStorage+1], r0
    mov r0, byte [FancyTable+512]
    mov byte [RotateFancyTableStorage+2], r0
    mov r0, byte [FancyTable+768]
    mov byte [RotateFancyTableStorage+3], r0
    mov r1, FancyTable
.rotate:
    inc r1
    mov r0, byte [r1]
    dec r1
    mov byte [r1], r0
    inc r1
    test r1, FancyTable+1023
    jne .rotate
.restore:
    mov r0, byte [RotateFancyTableStorage+0]
    mov byte [FancyTable+255], r0
    mov r0, byte [RotateFancyTableStorage+1]
    mov byte [FancyTable+256+255], r0
    mov r0, byte [RotateFancyTableStorage+2]
    mov byte [FancyTable+512+255], r0
    mov r0, byte [RotateFancyTableStorage+3]
    mov byte [FancyTable+768+255], r0
    ret

RotateFancyTableStorage:
    #d8 0,0,0,0

AuthDelay:
    push r2
    mov r2, WaitStr
	call PrintStr
    pop r2
    mov r0, 0x3fff
.loop:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    dec r0
    test r0, 0
    jne .loop
    ret

UserDatabase:
    #d "ax.arwen", 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    #d8 198, 68, 153, 227, 233, 25, 13, 7, 13, 18, 121
    #d 0x00, 0x00, 0x00, 0x00, 0x00
    #d 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    #d 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    #d 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    #d 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    #d 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    #d 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    #d "sbw.shadow", 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    #d8 233, 34, 216, 124, 60, 7, 84, 45, 94, 83, 106, 255, 128, 94, 205, 200, 207, 255, 68, 116, 200, 216, 75
    #d 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    #d 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    #d 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    #d 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    #d 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    #d 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    #d 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    #d 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    #d 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

UsernameBuffer:
    #d "________________", 0x00

PasswordBuffer:
    #d "________________________________________________________________", 0x00

UsernameStr:
    #d "Username: ", 0x00

PasswordStr:
    #d ">> Username OK. Password required\nPassword: ", 0x00

WaitStr:
    #d ">> Please wait...\n", 0x00

UserFailStr:
    #d ">> User not found in database.\n", 0x00

PassFailStr:
    #d ">> Password is incorrect.\n", 0x00

PassSuccessStr:
    #d ">> Login successful.\n"
    #d "======================================================================\n"
    #d "Running machine language monitor now.\n"
    #d "======================================================================\n\n"
    #d 0x00

BreakIntoMonitor:
    ; save registers
    push sp
    mov word [SavedR0], r0
    mov word [SavedR1], r1
    mov word [SavedR2], r2
    mov word [SavedR3], r3
    pop r0
    mov word [SavedSP], r0
    ; get return address
    mov r0, sp
    mov r0, word [r0]
	add r0, 0xffff
    mov word [SavedIP], r0
    ; copy stack bytes
    mov r1, 12
    mov r2, SavedAtSP
    mov r3, word [SavedSP]
    call MemCpy
    ; print header
	mov r2, Text_BreakIntoMonitor
	call PrintStr
BreakIntoMonitorInputLoop:
.inputLoop:
	; terminate
	mov r0, 0
	mov byte [0xe002], r0
    ; get command
	mov r2, Text_ReadyPrompt
	call PrintStr
	mov r2, 0xe000
	mov r3, 8
    call ReadStr
	test r3, 0
	jeq .inputTooLong
	mov r2, 0xe000
	call StrTrim

	; try parsing
	mov r0, byte [0xe002]
	test r0, 0
	jne .invalidCommand
	mov r0, word [0xe000]
	; switch case with commands
	test r0, 0x0a68
	jeq Cmd_HelpText
	test r0, 0x0068
	jeq Cmd_HelpText
	test r0, 0x0072
	jeq Cmd_PrintHex
	test r0, 0x0a72
	jeq Cmd_PrintHex
	test r0, 0x0070
	jeq Cmd_PrintText
	test r0, 0x0a70
	jeq Cmd_PrintText
	test r0, 0x0078
	jeq Cmd_Exec
	test r0, 0x0a78
	jeq Cmd_Exec
	test r0, 0x0063
	jeq Cmd_Continue
	test r0, 0x0a63
	jeq Cmd_Continue
	test r0, 0x4355
	jeq Cmd_Undocumented
	test r0, 0x736c
	jeq Cmd_FileLS
	test r0, 0x6672
	jeq Cmd_FileRead
	test r0, 0x0077
	jeq Cmd_WriteHex
	test r0, 0x0a77
	jeq Cmd_WriteHex
.invalidCommand:
	mov r2, Text_BadCommandError
	call PrintStr
	jmp .inputLoop
.inputTooLong:
	mov r2, Text_InputTooLongError
	call PrintStr
	jmp .inputLoop

Cmd_HelpText:
	mov r2, Text_Help
	call PrintStr
	jmp BreakIntoMonitorInputLoop

Cmd_PrintHex:
	mov r2, Text_WhichAddr
	call PrintStr
	mov r2, 0xe000
	mov r3, 8
    call ReadStr
	mov r2, 0xe000
	call ConvertHex
	test r0, 0xffff
	jeq .wrongInput
	push r0
	mov r2, Text_WhichLength
	call PrintStr
	mov r2, 0xe000
	mov r3, 8
    call ReadStr
	mov r2, 0xe000
	call ConvertHex
	test r0, 0xffff
	jeq .wrongInput
	test r0, 0
	jeq .wrongInput
	mov r3, r0
	pop r2
	; read r3 lines from r2
.printOneLine:
	push r2
	push r3
	mov r1, 8
	mov r3, r2
	mov r2, SavedMemView
	call MemCpy
	pop r3
	pop r2
    mov word [SavedMemViewAddr], r2
	push r2
	push r3
	mov r2, Text_HexView
	call PrintStr
	pop r3
	pop r2
	add r2, 8
	dec r3
	test r3, 0
	jne .printOneLine
	jmp BreakIntoMonitorInputLoop
.wrongInput:
	mov r2, Text_WrongInput
	call PrintStr
	jmp BreakIntoMonitorInputLoop

Cmd_WriteHex:
	mov r2, Text_WhichAddr
	call PrintStr
	mov r2, 0xe000
	mov r3, 8
    call ReadStr
	mov r2, 0xe000
	call ConvertHex
	test r0, 0xffff
	jeq .wrongInput
	mov word [SavedMemViewAddr], r0
	mov r2, Text_EnterData
	call PrintStr
.readInput:
	mov r3, 0xe002
	mov r0, 0
	mov byte [r3], r0
	dec r3
	mov byte [r3], r0
	dec r3
.readFirstNibble:
	sys 2
	mov byte [r3], r0
	mov r2, r3
	test r0, 0x2e
	jeq .finished
	test r0, 0x0a
	jeq .readFirstNibble
	push r3
	call ConvertHex
	pop r3
	test r0, 0xffff
	jeq .readFirstNibble
.intermission:
	inc r3
.readSecondNibble:
	sys 2
	mov byte [r3], r0
	mov r2, r3
	test r0, 0x2e
	jeq .finished
	test r0, 0x0a
	jeq .readSecondNibble
	push r3
	call ConvertHex
	pop r3
	test r0, 0xffff
	jeq .readSecondNibble
.writeOneByte:
	dec r3
	mov r2, r3
	call ConvertHex
	mov r1, word [SavedMemViewAddr]
	mov byte [r1], r0
	inc r1
	mov word [SavedMemViewAddr], r1
	jmp .readInput
.finished:
	sys 2
	test r0, 0x0a
	jne .finished
	mov r2, Text_Loaded
	call PrintStr
	jmp BreakIntoMonitorInputLoop
.wrongInput:
	mov r2, Text_WrongInput
	call PrintStr
	jmp BreakIntoMonitorInputLoop

Cmd_PrintText:
	mov r2, Text_WhichAddr
	call PrintStr
	mov r2, 0xe000
	mov r3, 8
    call ReadStr
	mov r2, 0xe000
	call ConvertHex
	mov r2, r0
	call PrintStr
	jmp BreakIntoMonitorInputLoop

Cmd_Exec:
	mov r2, Text_WhichAddr
	call PrintStr
	mov r2, 0xe000
	mov r3, 8
    call ReadStr
	mov r2, 0xe000
	call ConvertHex
	push r0
	mov word [SavedMemViewAddr], r0
	mov r2, Text_ConfirmExec
	call PrintStr
	mov r2, 0xe000
	mov r3, 8
    call ReadStr
	mov r0, byte [0xe000]
	test r0, 0x59
	jeq Cmd_Exec_Run
	test r0, 0x79
	jeq Cmd_Exec_Run
	mov r2, Text_Cancelled
	call PrintStr
	pop r0
	jmp BreakIntoMonitorInputLoop
Cmd_Exec_Run:
	pop r3
	call r3
	jmp BreakIntoMonitorInputLoop

Cmd_Continue:
	mov r2, Text_Continuing
	call PrintStr
    mov r0, word [SavedR0]
    mov r1, word [SavedR1]
    mov r2, word [SavedR2]
    mov r3, word [SavedR3]
	ret

Cmd_FileLS:
	mov r2, Text_LSHeader
	call PrintStr
	mov r0, 0
	mov r1, 0xe000
	sys 4
	mov r3, 0xe000
.readFileEntries:
	mov r0, byte [r3]
	test r0, 0
	jeq .readNextEntry
	push r3
	mov byte [SavedMemView], r0
	inc r3
	mov r0, word [r3]
	push r3
	mov word [SavedMemViewAddr], r0
	mov r2, Test_LSHeader2
	call PrintStr
	pop r3
	inc r3
	inc r3
	mov r2, r3
	call PrintStr
	mov r0, 0x0a
	sys 1
	pop r3
.readNextEntry:
	add r3, 0x10
	test r3, 0xef00
	jne .readFileEntries
	jmp BreakIntoMonitorInputLoop

Cmd_Undocumented:
	mov r2, Text_MonitorUndocumented
	call PrintStr
	jmp BreakIntoMonitorInputLoop

Cmd_FileRead:
	mov r2, Text_WhichFile
	call PrintStr
	mov r0, 0
	mov r1, 0xe000
	sys 4
	mov r2, 0xefb0
	mov r3, 16
    call ReadStr
	test r3, 0
	jeq .inputTooLong
	mov r2, 0xefb0
	call StrTrim
	mov r3, 0xe003
.checkForFiles:
	mov r2, 0xefb0
	push r3
	call StrCmp
	pop r3
	test r0, 0
	jeq .fileFound
	add r3, 0x10
	test r3, 0xef03
	jeq .fileNotFound
	jmp .checkForFiles
.fileFound:
	dec r3
	dec r3
	dec r3
	mov r0, byte [r3]
	inc r3
	mov r1, word [r3]
	push r1
	mov r1, 0xe000
	sys 4
	mov r2, Text_WhichAddr
	call PrintStr
	mov r2, 0xefe0
	mov r3, 16
    call ReadStr
	mov r2, 0xefe0
	call ConvertHex
	pop r1
	test r0, 0xffff
	jeq .wrongInput
	mov r2, r0
	mov r3, 0xe000
	call MemCpy
	mov r2, Text_Loaded
	call PrintStr
	jmp BreakIntoMonitorInputLoop
.fileNotFound:
	mov r2, Text_FileNotFoundError
	call PrintStr
	jmp BreakIntoMonitorInputLoop
.inputTooLong:
	mov r2, Text_InputTooLongError
	call PrintStr
	jmp BreakIntoMonitorInputLoop
.wrongInput:
	mov r2, Text_WrongInput
	call PrintStr
	jmp BreakIntoMonitorInputLoop

Text_Intro:
	#d incbin("intro2.txt")
	#d 0x00

Text_InputTooLongError:
	#d "! Input too long error.\n", 0x00

Text_BadCommandError:
	#d "! Bad command error (h for help).\n", 0x00

Text_WrongInput:
	#d "! Invalid input error.\n", 0x00

Text_WhichAddr:
	#d "> Which address? ", 0x00

Text_WhichLength:
	#d "> How many lines? ", 0x00

Text_ConfirmExec:
	#d "> Really exec at ", 0xf0
	#d16 dw(SavedMemViewAddr)
	#d "? Type Y if so: ", 0x00

Text_Cancelled:
	#d "! Cancelled action error.\n", 0x00

Text_Continuing:
	#d "Continuing.\n", 0x00

Text_Loaded:
	#d "Loaded.\n", 0x00

Text_ContinueTooFar:
	#d "! Dry stack. Halting machine.\n", 0x00

Text_WhichFile:
	#d "> Filename? ", 0x00

Text_FileNotFoundError:
	#d "! File not found error.\n", 0x00

Text_EnterData:
	#d "> Enter hex data. End with dot ", 0x22, ".", 0x22, " + newline:\n", 0x00

Text_MonitorUndocumented:
	#d "Wow, undocumented monitor command! FOOLS2023_{Secret", 0xf0
    #d16 dw(0xe000)
	#d "x", 0xf0
    #d16 dw(0xe001)
	#d "Command}\n", 0x00

Text_Help:
	#d "Available commands:\n"
	#d "r   :: print memory as hex\n"
	#d "p   :: print memory as text\n"
	#d "w   :: write hex data to memory\n"
	#d "x   :: execute memory\n"
	#d "rf  :: load memory from file\n"
	#d "ls  :: print file index\n"
	#d "h   :: print this help message\n"
	#d "c   :: exit monitor and continue\n"
	#d "Please enter hex numbers when prompted.\n"
	#d "If necessary for debugging, you can break into monitor\n"
	#d "with instruction BRK (0x00) and continue with 'c'\n"
	#d "Note: memory region E000-FFFF is used by monitor\n"
	#d 0x00

Text_LSHeader:
	#d "BLK  SIZE  NAME\n"
	#d "=======================\n"
	#d 0x00

Test_LSHeader2:
	#d 0xf1
    #d16 dw(SavedMemView)
	#d "   ", 0xf0
    #d16 dw(SavedMemViewAddr)
	#d "  ", 0x00

Text_ReadyPrompt:
    #d "Ready.\n> ", 0x00

Text_BreakIntoMonitor:
	#d "*** BREAK INTO MONITOR AT $", 0xf0
    #d16 dw(SavedIP)
    #d " ***\n"
    #d "R0=$", 0xf0
    #d16 dw(SavedR0)
    #d " R1=$", 0xf0
    #d16 dw(SavedR1)
    #d " R2=$", 0xf0
    #d16 dw(SavedR2)
    #d " R3=$", 0xf0
    #d16 dw(SavedR3)
    #d "\nSP=$", 0xf0
    #d16 dw(SavedSP)
    #d " ["
    #d 0xf1
    #d16 dw(SavedAtSP)
    #d 0xf1
    #d16 dw(SavedAtSP+1)
    #d 0xf1
    #d16 dw(SavedAtSP+2)
    #d 0xf1
    #d16 dw(SavedAtSP+3)
    #d 0xf1
    #d16 dw(SavedAtSP+4)
    #d 0xf1
    #d16 dw(SavedAtSP+5)
    #d 0xf1
    #d16 dw(SavedAtSP+6)
    #d 0xf1
    #d16 dw(SavedAtSP+7)
    #d 0xf1
    #d16 dw(SavedAtSP+8)
    #d 0xf1
    #d16 dw(SavedAtSP+9)
    #d 0xf1
    #d16 dw(SavedAtSP+10)
    #d 0xf1
    #d16 dw(SavedAtSP+11)
    #d "]\n"
    #d 0x00

Text_HexView:
	#d 0xf0
	#d16 dw(SavedMemViewAddr)
	#d " | ", 0xf1
	#d16 dw(SavedMemView)
	#d " ", 0xf1
	#d16 dw(SavedMemView+1)
	#d " ", 0xf1
	#d16 dw(SavedMemView+2)
	#d " ", 0xf1
	#d16 dw(SavedMemView+3)
	#d " ", 0xf1
	#d16 dw(SavedMemView+4)
	#d " ", 0xf1
	#d16 dw(SavedMemView+5)
	#d " ", 0xf1
	#d16 dw(SavedMemView+6)
	#d " ", 0xf1
	#d16 dw(SavedMemView+7)
	#d "\n", 0x00

SavedR0:
    #d16 0
SavedR1:
    #d16 0
SavedR2:
    #d16 0
SavedR3:
    #d16 0
SavedSP:
    #d16 0
SavedIP:
    #d16 0
SavedAtSP:
    #d "XXXXXXXXXXXX"

SavedMemView:
	#d "XXXXXXXX"
SavedMemViewAddr:
	#d "XX"