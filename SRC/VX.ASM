		page	60, 132
		title	Vortex
		subttl	Copyright (c) 1993 Brent Engelbrecht

;*****************************************************************************
;*  PROGRAM         :  Vortex                                                *
;*  FILE NAME       :  VX.ASM  -->  VX.COM                                   *
;*  AUTHOR          :  Brent Engelbrecht                                     *
;*  ASSEMBLER       :  A86  v3.22                                            *
;*  PROGRAM VERSION :  1.01b                                                 *
;*  DATE STARTED    :  18 June 1993                                          *
;*  DATE COMPLETED  :   5 September 1993                                     *
;*  COMMENTS        :  - Shell program for the DOS environment               *
;*                  :  - COM format                                          *
;*****************************************************************************

BiosData	segment	at	40h
		org	10h
Equipment	dw	?

		org	17h
ShiftStatus_1	db	?

		org	49h
InitialVideo	db	?
BiosData	ends

CodeSeg		segment	para	'Code'
		assume	cs:CodeSeg, ds:CodeSeg, ss:CodeSeg, es:CodeSeg
		org	0100h

Main		proc	far
		jmp	Begin
		even

;----------------------------------------------------------------------------
; *****************************  PROGRAM DATA  *****************************
;

STACK_SIZE	equ	50000
PATH_SIZE	equ	64
DIR_REC_SIZE	equ	43

DTA_DATA_OFS	equ	15h
DTA_DATA_LEN	equ	22
ENTRY_LENGTH	equ	25
MAX_COLS	equ	0004h
MAX_ROWS	equ	09h
NAME_EXT_SPACE	equ	07h

SCREEN_WIDTH	equ	80
CMD_LINE_SIZE	equ	128
ASCII_255	equ	255

FILENAME_OFFS	equ	09h
FILENAME_LEN	equ	0ch

END_OF_FLIST	equ	0ffffh
SELECTED	equ	01111111xb
UNSELECTED	equ	00000000xb
CURRENT		equ	10000000xb
NOT_CURRENT	equ	01111111xb

ANY_FILE	equ	00ffh
ATTR_NORM_FILE	equ	00011000xb
ATTR_NORMAL	equ	NULL
ATTR_VOL	equ	08h
ATTR_DIR	equ	10h
ATTR_NORM_DIR	equ	00100111xb

FILE_OPEN_FAILED	equ	0ffffh
FILE_READ_FAILED	equ	0ffffh
FILE_READ_OK		equ	0000h
FILE_WRITE_FAILED	equ	0ffffh
FILE_BAD_SEEK		equ	0ffffh
NOT_TRANSFERRED		equ	0f0f0h

FUNCTION_COPY	equ	20h
FUNCTION_MOVE	equ	80h

FUNC_MOVE_DIR	equ	100h

WRITE_ONLY	equ	01h
READ_WRITE	equ	02h
READ_ONLY	equ	NULL

BUF_HEADER_LEN	equ	12+2+2 ; filename + next_seg + byte_count
END_OF_LIST	equ	0ffffh

TINY_BUFFER	equ	4096
SMALL_BUFFER	equ	8192
MEDIUM_BUFFER	equ	16384
LARGE_BUFFER	equ	32768
HUGE_BUFFER	equ	65536

ERROR_MEM_ALLOC		equ	01h
ERROR_MEM_FREE		equ	02h
ERROR_MEM_BROKE		equ	07h
ERROR_MEM_OUT		equ	08h
ERROR_MEM_ADDR		equ	09h
ERROR_USE_ALLOC		equ	100h
ERROR_USE_DEALL		equ	101h
ERROR_FIL_INVAL		equ	102h
ERROR_FIL_PATH		equ	103h
ERROR_FIL_NMORE		equ	104h
ERROR_FIL_ACCDEN	equ	105h
ERROR_FIL_MODE		equ	106h
ERROR_FIL_NHAND		equ	107h
ERROR_FIL_FUNC		equ	108h
ERROR_FIL_BHAND		equ	109h
ERROR_FIL_LOST_ATTR	equ	10ah
ERROR_DIR_NEMPTY	equ	200h
ERROR_DSK_LOAD		equ	400h
ERROR_TOO_MANY_FILES	equ	500h

NULL		equ	00h
DEFAULT		equ	0ffffh
TRUE		equ	0ffh
FALSE		equ	not TRUE
CHAR_NOT_FOUND	equ	0ffffh

END_OF_RECORD	equ	NULL
END_OF_TABLE	equ	0ffh
SPACE		equ	20h
NOT_ALLOC_FLAG	equ	NULL
UPPERCASE_MASK	equ	11011111xb

SEARCH_SELECTED	equ	0002h
SEARCH_CURRENT	equ	0004h

UPD_NEWPATH	equ	00000001xb
UPD_RELOAD	equ	00000010xb
UPD_REDRAW	equ	00000100xb
UPD_RESET	equ	00001000xb
UPD_NEWDRV	equ	00010000xb

NUPD_NEWPATH	equ	not UPD_NEWPATH
NUPD_RELOAD	equ	not UPD_RELOAD
NUPD_REDRAW	equ	not UPD_REDRAW
NUPD_RESET	equ	not UPD_RESET
NUPD_NEWDRV	equ	not UPD_NEWDRV

DRV_WIN_HIGH	equ	05h

NUM_BUF_SIZE	equ	06h

TITLE_DIVIDER	equ	0cdh ;0c4h
SCREEN_SIZE	equ	0fa0h

FIRST_WINDOW	equ	00h
SECOND_WINDOW	equ	01h

STD_OPTION	equ	01h

LIST_1_PARMS	equ	02h
;LIST_2_PARMS	equ	0ah
LIST_2_PARMS	equ	14h

NORMAL_VIDEO	equ	07h
REVERSE_VIDEO	equ	70h
HIGH_VIDEO	equ	0fh

SCHEME_MONO	equ	NULL
SCHEME_COLOUR	equ	01h
SCHEME_DEFAULT	equ	0ffffh

BLACK		equ	0000xb	;	0	black
BLUE		equ	0001xb	;	1	blue
GREEN		equ	0010xb	;	2	green
CYAN		equ	0011xb	;	3	cyan
RED		equ	0100xb	;	4	red
MAGENTA		equ	0101xb	;	5	magenta
BROWN		equ	0110xb	;	6	brown
LIGHT_GRAY	equ	0111xb	;	7	white

GRAY		equ	1000xb	;	8	gray
LIGHT_BLUE	equ	1001xb	;	9	light blue
LIGHT_GREEN	equ	1010xb	;	a	light green
LIGHT_CYAN	equ	1011xb	;	b	light cyan
LIGHT_RED	equ	1100xb	;	c	light red
LIGHT_MAGENTA	equ	1101xb	;	d	light magenta
YELLOW		equ	1110xb	;	e	yellow
WHITE		equ	1111xb	;	f	bright white

BORD_SINGLE	equ	00h
BORD_DOUBLE	equ	01h

ACT_WIN_MASK	equ	00000100xb		; Left- and Right-Ctrl
INACT_WIN_MASK	equ	00000011xb		; Left- and Right-Shift

DOS_ROW		equ	23
DOS_LINE_MASK	equ	00001000xb		; Left- and Right-Alt

KEY_WAITING	equ	not NULL
NO_KEY		equ	NULL

ESC_KEY		equ	1bh
BACKSPACE_KEY	equ	08h

QUIT_KEY	equ	ESC_KEY			; Esc
MENU_KEY	equ	3ch			; F2
COPY_KEY	equ	3dh			; F3
MOVE_KEY	equ	3eh			; F4
DELETE_KEY	equ	3fh			; F5
UP_ARROW	equ	48h
DOWN_ARROW	equ	50h
LEFT_ARROW	equ	4bh
RIGHT_ARROW	equ	4dh
ENTER_KEY	equ	0dh
TAB_KEY		equ	09h
SELECT_KEY	equ	52h			; Ins
PARENT_KEY	equ	49h			; PgUp
TOP_LIST_KEY	equ	47h			; Home
BOT_LIST_KEY	equ	4fh			; End

;CTRL_C_INT	equ	23h
;
;OldCtrlC_Seg	dw	?
;OldCtrlC_Ofs	dw	?

MaxAvail	dw	?			; Memory free
VideoSegment	dw	?			; Segment of video buffer
Keyb_Status	dw	?			; 00h if no key; else FFh
MenuChoice	dw	?			; # of option sel'd from menu

FileListControl	label	word
ActiveWindow	dw	?

FileChoice_X1	dw	NULL
FileChoice_Y1	dw	NULL
CurrColumn_W1	dw	NULL
FileCount_W1	dw	?
FilesSel_W1	dw	?
BytesTotal_W1	dd	?
BytesSel_W1	dd	?

FileChoice_X2	dw	NULL
FileChoice_Y2	dw	NULL
CurrColumn_W2	dw	NULL
FileCount_W2	dw	?
FilesSel_W2	dw	?
BytesTotal_W2	dd	?
BytesSel_W2	dd	?

FileLocation	label	word
StartList_W1	dw	NULL
StartOfs_W1	dw	?
StartList_W2	dw	NULL
StartOfs_W2	dw	?

ActiveListStart	label	word
Act_List	dw	NULL
Act_Ofs		dw	?

Update_Flags	label	byte
UpdateOne	db	0ffh
UpdateTwo	db	0ffh

		even
Configurable	label	byte
InsMovesDown	db	TRUE
		even
StartUpDrive	dw	03h			; DEFAULT sets drv to current
Wind_Act	dw	FIRST_WINDOW
Block_Size	dw	MEDIUM_BUFFER		; Default to 16K

		even
FileList	label	word
FL_X		dw	0000h
FL_Y		dw	0001h
FL_Width	dw	0050h
FL_Height	dw	000bh
FL_Attr		dw	?
FL_BarWidth	dw	FILENAME_LEN
FL_Columns	dw	MAX_COLS
FL_SpacingVal	dw	NAME_EXT_SPACE
FL_HighAttr	dw	?
FL_Selected	dw	?

ControlPanelLoc	label	word
CP_X		dw	03h			; Starts at 3rd column
CP_Y		dw	03h			; Starts on 3rd row
CP_Width	dw	14h			; Menu is 20 chars wide
CP_X_Offset	dw	02h			; Options displ x chars away
						;    from first window column
CP_ItemCount	dw	09h			; Number of items in menu
CP_Attr		dw	?			; Menu colour is black on white
CP_Divider	dw	TITLE_DIVIDER		; Menu 'divider' character
CP_YStart	dw	00h			; Ofs from window-top for items
CP_BarWidth	dw	12h			; Width of menu bar (in chars)
CP_HighAttr	dw	?			; Attr of highlight bar

UserScreenInfo	label	word
SaveSegment	dw	?
SaveCursorLoc	dw	?
SaveCursorSize	dw	?

DTA_Segment	dw	?
DTA_Offset	dw	?
ListOne		dw	NULL
ListTwo		dw	NULL

FileListDrives	label	word
DriveNumOne	dw	?
DriveNumTwo	dw	?

Error_Word	dw	?
Scratch_Word	dw	?

DOS_Parameters	label	word
Win_X		dw	?
Win_Y		dw	DOS_ROW
Win_Length	dw	?
Curr_Pos	dw	NULL
Win_Attr	dw	?
CursorPos	dw	NULL
Line_Length	dw	CMD_LINE_SIZE - 1
ExitKeyUsed	dw	?
Curr_Length	dw	NULL
Prompt_Length	dw	?
Start_Pos	dw	NULL
DeActive	dw	?

Buffer_Segment	dw	NOT_ALLOC_FLAG
Buffer_Size	dw	TINY_BUFFER
Buffer_Ptr	dw	?
Buffer_Entries	dw	?
Buffer_Max	dw	?


ColourScheme	dw	SCHEME_COLOUR
ClrTableSize	dw	09h

		even
MonoSchemeTable	label	byte
Mono_ProgHead	db	HIGH_VIDEO		; OK
Mono_FileList	db	NORMAL_VIDEO		; OK
Mono_ListBar	db	REVERSE_VIDEO		; OK
Mono_ListSel	db	HIGH_VIDEO		; OK
Mono_CmdLineAct	db	HIGH_VIDEO		; OK
Mono_CmdLineDe	db	NORMAL_VIDEO		; OK
Mono_Menu	db	REVERSE_VIDEO		; OK
Mono_MenuHigh	db	NORMAL_VIDEO		; OK
Mono_Template	db	REVERSE_VIDEO		; OK

ClrSchemeTable	label	byte
Col_ProgHead	db	WHITE					; OK
Col_FileList	db	(BLUE * 16 + LIGHT_GRAY) AND 7fh	; OK
Col_ListBar	db	(BLACK * 16 + RED) AND 7fh		; OK
Col_ListSel	db	(BLUE * 16 + YELLOW) AND 7fh		; OK
Col_CmdLineAct	db	YELLOW					; OK
Col_CmdLineDe	db	LIGHT_GRAY				; OK
Col_Menu	db	(GREEN * 16 + BLACK) AND 7fh		; OK
Col_MenuHigh	db	(BLACK * 16 + RED) AND 7fh		; OK
Col_Template	db	(GREEN * 16 + BLACK) AND 7fh		; OK

		even
ColourTable	label	word
Attr_ProgHead	dw	?
Attr_FileList	dw	?
Attr_ListBar	dw	?
Attr_ListSel	dw	?
Attr_CmdLineAct	dw	?
Attr_CmdLineDe	dw	?
Attr_Menu	dw	?
Attr_MenuHigh	dw	?
Attr_Template	dw	?

		even
FullDirOne	db	'\'
DirectoryOne	db	PATH_SIZE dup (NULL)
		db	END_OF_RECORD
		even
FullDirTwo	db	'\'
DirectoryTwo	db	PATH_SIZE dup (NULL)
		db	END_OF_RECORD

FileMask	db	'*.*', END_OF_RECORD

PathOne		db	PATH_SIZE dup (NULL)
PathTwo		db	PATH_SIZE dup (NULL)

FileEntry	db	ENTRY_LENGTH dup (NULL)

Scratch_Buffer	db	PATH_SIZE dup (NULL)

DOS_Line	db	CMD_LINE_SIZE dup (NULL)

NumConvert	db	NUM_BUF_SIZE dup (SPACE)
		db	SPACE, ' Files', END_OF_RECORD

Dir_Tag		db	'[DIR]', END_OF_RECORD
Parent_Tag	db	'..', END_OF_RECORD

LegalDrives	db	64 dup (NULL)

MenuNames	db	' CONTROL PANEL ', END_OF_RECORD
		db	END_OF_TABLE		; Terminates list of menu names

BorderChars	db	218, 196, 191, 179, 192, 217, END_OF_RECORD
		db	201, 205, 187, 186, 200, 188, END_OF_RECORD
		db	END_OF_TABLE

; ControlOptions record format :
;
;		db	active_flag	; Indicates if this option is
;					;    selectable within the menu
;		db	option_name	; Name of option, as displayed on the
;					;    menu
;		db	0ffh		; Indicates to table-handling routines
;					;    that end-of-table was reached

ControlOptions	db	STD_OPTION, 'Copy', END_OF_RECORD
		db	STD_OPTION, 'Move', END_OF_RECORD
		db	STD_OPTION, 'Delete', END_OF_RECORD
		db	STD_OPTION, 'Rename', END_OF_RECORD
		db	STD_OPTION, 'Create', END_OF_RECORD
		db	STD_OPTION, 'View', END_OF_RECORD
		db	STD_OPTION, 'Attributes', END_OF_RECORD
		db	STD_OPTION, 'Print', END_OF_RECORD
		db	STD_OPTION, 'Quit', END_OF_RECORD
		db	END_OF_TABLE		; Terminates list of options

		even
KeyStroke	label	word			; Saves an input key
ASCII_Code	db	?			; Saves an input ascii code
Scan_Code	db	?			; Saves an input scan code

UnknownMsg	db	'An unknown error has occurred !', '$'
Err_Alloc_Msg	db	'Error occurred during memory allocation !', '$'
Err_Free_Msg	db	'Error occurred during memory release !', '$'

Err_MCB_Broke	db	'Memory Control Block destroyed !', '$'
Err_Out_Of_Mem	db	'Out of memory !', '$'
Err_Bad_Addr	db	'Invalid memory address !', '$'

Err_User_Scrn	db	'No memory for User Screen !', '$'
Err_User_Deall	db	'Cannot deallocate U/S memory !', '$'

Err_Not_Found	db	'Requested file could not be located !', '$'
Err_Bad_Path	db	'Requested path could not be located !', '$'
Err_Access	db	'Access denied on requested file !', '$'
Err_File_Mode	db	'Invalid file open mode !', '$'
Err_No_Handles	db	'No available handles !', '$'
Err_Bad_Func	db	'Bad function code for open file !', '$'
Err_Bad_Handle	db	'Invalid file handle !', '$'
Err_Lost_Attr	db	'File attribute cannot be determined !', '$'

Program_Header	db	'Vortex', SPACE, 240, SPACE
		db	'Version 1.01b', SPACE, 240, SPACE
		db	'Copyright (c) 1993', SPACE
		db	'Brent Engelbrecht'
		db	END_OF_RECORD

Tester_Header	db	'Vortex', SPACE, 240, SPACE
		db	'(', 34, 'KISS MY ASS, NC !', 34, ')', SPACE, 240, SPACE
		db	'14 Dec 1993', SPACE, 240, SPACE
		db	'Brent Engelbrecht'
		db	END_OF_RECORD

FKey_Template	db	'F1 Help', SPACE
		db	'F2 Menu', SPACE
		db	'F3 Copy', SPACE
		db	'F4 Move', SPACE
		db	'F5 Del', SPACE
		db	'F6 Ren', SPACE
		db	'F7 MkDir', SPACE
		db	'F8 View', SPACE
		db	'F9 Edit', SPACE
		db	'F10 User', SPACE
		db	END_OF_RECORD

DriveWinTitle	db	'SELECT A NEW DRIVE', END_OF_RECORD
;----------------------------------------------------------------------------

Begin:
		lea	ax, End_Of_Code		; Load addr of last opcode
		add	ax, STACK_SIZE + 1	; Leave some space for stack
		mov	sp, ax			; Reduce amount of stack
		xor	ax, ax			; Initialise top of stack
		push	ax			;    as DOS does

;		mov	ax, CTRL_C_INT
;		push	ax
;		lea	ax, OldCtrlC_Seg
;		push	ax
;		lea	ax, OldCtrlC_Ofs
;		push	ax
;		call	GetVector
;
;		mov	ax, CTRL_C_INT
;		push	ax
;		push	cs
;		lea	ax, NewCtrlC
;		push	ax
;		call	SetVector

		lea	ax, End_Of_Code		; Addr of last program opcode
		push	ax			; Pass by reference
		push	cs			; Send our code segment as well
		call	FreeAllMem		; Shrink memory

		lea	ax, MaxAvail		; Addr of "mem free" var
		push	ax			; Pass by reference
		call	GetMemFree		; Get max memory available

		int	09h
		call	FlushKeyb

		lea	ax, VideoSegment	; Variable to receive the
		push	ax			;    seg of video buffer
		call	GetVideoSetup		; Get video segment

		mov	ax, cs
		mov	DTA_Segment, ax
		mov	DTA_Offset, 80h

		lea	ax, UserScreenInfo
		push	ax
		mov	ax, VideoSegment
		push	ax
		call	SaveUserScreen

		mov	ax, ColourScheme
		push	ax
		mov	ax, ClrTableSize
		push	ax
		lea	ax, MonoSchemeTable
		push	ax
		lea	ax, ColourTable
		push	ax
		call	InitColours

		lea	ax, DOS_Parameters
		push	ax
		lea	ax, ControlPanelLoc
		push	ax
		lea	ax, FileList
		push	ax
		lea	ax, ColourTable
		push	ax
		call	SetupColours

		xor	ax, ax
		push	ax
		mov	ax, VideoSegment
		push	ax
		call	ClearLine

		xor	ax, ax
		push	ax
		push	ax
		mov	ax, SCREEN_WIDTH
		push	ax
		mov	ax, Attr_ProgHead
		push	ax
		mov	ax, VideoSegment
		push	ax
		call	AttributeLine

		xor	ax, ax
		push	ax
		mov	ax, SCREEN_WIDTH
		push	ax
		xor	ax, ax
		push	ax
		lea	ax, Tester_Header
;		lea	ax, Program_Header
		push	ax
		mov	ax, Attr_ProgHead
		push	ax
		mov	ax, VideoSegment
		push	ax
		call	WriteCentred		; Write prog title to screen

		lea	ax, FKey_Template
		push	ax
		mov	ax, Attr_Template
		push	ax
		mov	ax, VideoSegment
		push	ax
		call	DispTemplate		; Write function key template
						;    to the screen

		mov	ax, Wind_Act
		mov	ActiveWindow, ax

		lea	ax, LegalDrives
		push	ax
		call	GetLegalDrv

		lea	ax, DOS_Parameters
		push	ax
		call	AllocCmdBuffer

ReStartCmdr:
		lea	bx, FileListDrives
		mov	si, ActiveWindow
		mov	di, si
		not	di
		and	di, 01h

		shl	si, 1
		add	bx, si

		lea	dx, FileListDrives
		shl	di, 1
		add	di, dx

		mov	ax, bx
		push	ax
		call	GetDefaultDrv

		mov	ax, StartUpDrive
		mov	[di], ax
		cmp	ax, DEFAULT
		jne	short RefreshPath

		mov	ax, [bx]
		mov	[di], ax

RefreshPath:
		cmp	ActiveWindow, SECOND_WINDOW
		je	short NewDriveWin2

		mov	al, UpdateOne
		and	al, UPD_NEWDRV
		cmp	al, UPD_NEWDRV
		jne	short NewPath

		mov	al, UpdateOne
		and	al, NUPD_NEWDRV
		mov	UpdateOne, al

		mov	ax, DriveNumOne
		push	ax
		call	ChangeDrive

		jmp	short NewPath

NewDriveWin2:
		mov	al, UpdateTwo
		and	al, UPD_NEWDRV
		cmp	al, UPD_NEWDRV
		jne	short NewPath

		mov	al, UpdateOne
		and	al, NUPD_NEWDRV
		mov	UpdateOne, al

		mov	ax, DriveNumTwo
		push	ax
		call	ChangeDrive

NewPath:
		mov	al, UpdateOne
		and	al, UPD_NEWPATH
		cmp	al, UPD_NEWPATH
		jne	short RebuildWin1

		mov	al, UpdateOne
		and	al, NUPD_NEWPATH
		or	al, UPD_RELOAD
		or	al, UPD_REDRAW
		or	al, UPD_RESET
		mov	UpdateOne, al

		mov	ax, FIRST_WINDOW
		push	ax
		lea	ax, FileListControl
		push	ax
		lea	ax, FileLocation
		push	ax
		call	FileListInit

		push	ds
		lea	ax, DirectoryOne
		push	ax
		mov	ax, DriveNumOne
		push	ax
		call	GetDefaultDir

RebuildWin1:
		lea	ax, PathOne
		push	ax
		lea	ax, DriveNumOne
		push	ax
		lea	ax, DirectoryOne
		push	ax
		lea	ax, FileMask
		push	ax
		call	BuildPathName

UpdateWin2:
		mov	al, UpdateTwo
		and	al, UPD_NEWPATH
		cmp	al, UPD_NEWPATH
		jne	short RebuildWin2

		mov	al, UpdateTwo
		and	al, NUPD_NEWPATH
		or	al, UPD_RELOAD
		or	al, UPD_REDRAW
		or	al, UPD_RESET
		mov	UpdateTwo, al

		mov	ax, SECOND_WINDOW
		push	ax
		lea	ax, FileListControl
		push	ax
		lea	ax, FileLocation
		push	ax
		call	FileListInit

		push	ds
		lea	ax, DirectoryTwo
		push	ax
		mov	ax, DriveNumTwo
		push	ax
		call	GetDefaultDir

RebuildWin2:
		lea	ax, PathTwo
		push	ax
		lea	ax, DriveNumTwo
		push	ax
		lea	ax, DirectoryTwo
		push	ax
		lea	ax, FileMask
		push	ax
		call	BuildPathName

RefreshLists:
		mov	al, UpdateOne
		and	al, UPD_RELOAD
		cmp	al, UPD_RELOAD
		jne	short ReloadWin2

		mov	al, UpdateOne
		or	al, UPD_REDRAW
		and	al, NUPD_RELOAD
		mov	UpdateOne, al

		lea	ax, Error_Word
		push	ax
		lea	ax, ListOne
		push	ax
		lea	ax, PathOne
		push	ax
		mov	ax, DTA_Segment
		push	ax
		mov	ax, DTA_Offset
		push	ax
		lea	ax, FileCount_W1
		push	ax
		call	LoadFileList

		mov	ax, StartList_W1
		push	ax
		lea	ax, StartOfs_W1
		push	ax
		call	FindFileEntry

ReloadWin2:
		mov	al, UpdateTwo
		and	al, UPD_RELOAD
		cmp	al, UPD_RELOAD
		jne	short RedrawLists

		mov	al, UpdateTwo
		or	al, UPD_REDRAW
		and	al, NUPD_RELOAD
		mov	UpdateTwo, al

		lea	ax, Error_Word
		push	ax
		lea	ax, ListTwo
		push	ax
		lea	ax, PathTwo
		push	ax
		mov	ax, DTA_Segment
		push	ax
		mov	ax, DTA_Offset
		push	ax
		lea	ax, FileCount_W2
		push	ax
		call	LoadFileList

		mov	ax, StartList_W2
		push	ax
		lea	ax, StartOfs_W2
		push	ax
		call	FindFileEntry

RedrawLists:
		mov	al, UpdateOne
		and	al, UPD_REDRAW
		cmp	al, UPD_REDRAW
		jne	short RedrawWin2

		lea	ax, FileList
		push	ax
		mov	ax, FIRST_WINDOW
		push	ax
		mov	ax, VideoSegment
		push	ax
		call	InitListWindow

RedrawWin2:
		mov	al, UpdateTwo
		and	al, UPD_REDRAW
		cmp	al, UPD_REDRAW
		jne	short ActivateList

		lea	ax, FileList
		push	ax
		mov	ax, SECOND_WINDOW
		push	ax
		mov	ax, VideoSegment
		push	ax
		call	InitListWindow

ActivateList:
		mov	al, UpdateOne
		mov	bl, UpdateTwo
		or	al, bl
		and	al, UPD_RESET
		cmp	al, UPD_RESET
		jne	short DisplayWin1

		mov	al, UpdateOne
		and	al, NUPD_RESET
		mov	UpdateOne, al
		mov	al, UpdateTwo
		and	al, NUPD_RESET
		mov	UpdateTwo, al

		mov	ax, ActiveWindow
		push	ax
		lea	ax, BorderChars
		push	ax
		lea	ax, FileList
		push	ax
		mov	ax, VideoSegment
		push	ax
		call	BorderLists

		lea	ax, DOS_Parameters
		push	ax
		lea	ax, DOS_Line
		push	ax
		lea	ax, PathOne
		push	ax
		lea	ax, PathTwo
		push	ax
		mov	ax, ActiveWindow
		push	ax
		lea	ax, FileList
		push	ax
		mov	ax, VideoSegment
		push	ax
		call	FileListTitles

;		lea	ax, FileListControl
;		push	ax
;		lea	ax, NumConvert
;		push	ax
;		call	Statistics

DisplayWin1:
		mov	al, UpdateOne
		and	al, NUPD_REDRAW
		mov	UpdateOne, al

		lea	ax, Dir_Tag
		push	ax
		mov	ax, FIRST_WINDOW
		push	ax
		mov	ax, StartList_W1
		push	ax
		mov	ax, StartOfs_W1
		push	ax
		mov	ax, FileCount_W1
		push	ax
		mov	ax, ListOne
		push	ax
		lea	ax, FileList
		push	ax
		mov	ax, VideoSegment
		push	ax
		call	DisplayFileList

Win2Redraw:
		mov	al, UpdateTwo
		and	al, NUPD_REDRAW
		mov	UpdateTwo, al

		lea	ax, Dir_Tag
		push	ax
		mov	ax, SECOND_WINDOW
		push	ax
		mov	ax, StartList_W2
		push	ax
		mov	ax, StartOfs_W2
		push	ax
		mov	ax, FileCount_W2
		push	ax
		mov	ax, ListTwo
		push	ax
		lea	ax, FileList
		push	ax
		mov	ax, VideoSegment
		push	ax
		call	DisplayFileList

BarOn:
		lea	ax, FileListControl
		push	ax
		lea	ax, Act_List
		push	ax
		call	ConvertCoords

		mov	ax, Act_List
		push	ax
		lea	ax, Act_Ofs
		push	ax
		call	FindFileEntry

		mov	ax, Act_Ofs
		push	ax
		mov	ax, ActiveWindow
		push	ax
		mov	ax, ListOne
		push	ax
		mov	ax, ListTwo
		push	ax
		call	MarkCurrFile

		lea	ax, FileList
		push	ax
		lea	ax, FileListControl
		push	ax
		mov	ax, VideoSegment
		push	ax
		call	FileListBarOn

NextAction:
		lea	ax, Keyb_Status		; Var to receive status of
		push	ax			;    the keyboard
		call	KeyPressed		; Check keyb status

		push	ax
		push	bx
		push	es

		mov	ax, BiosData
		mov	es, ax
		lea	bx, ShiftStatus_1
		mov	al, es:[bx]
		and	al, INACT_WIN_MASK + ACT_WIN_MASK

		or	ah, al
		cmp	ah, ACT_WIN_MASK + INACT_WIN_MASK
		je	short NoDrvChange
		cmp	ah, NULL
		je	short NoDrvChange

		xor	al, al
		push	ax
		lea	ax, Update_Flags
		push	ax
		lea	ax, Keyb_Status
		push	ax
		lea	ax, KeyStroke
		push	ax
		mov	ax, ActiveWindow
		push	ax
		lea	ax, FileListDrives
		push	ax
		lea	ax, LegalDrives
		push	ax
		lea	ax, DriveWinTitle
		push	ax
		mov	ax, VideoSegment
		push	ax
		call	SetNewDrive

NoDrvChange:
		mov	al, es:[bx]
		and	al, DOS_LINE_MASK
		cmp	al, DOS_LINE_MASK

		pop	es
		pop	bx
		pop	ax

		jne	short NotACommand

		lea	ax, Update_Flags
		push	ax
		mov	ax, VideoSegment
		push	ax
		lea	ax, DOS_Line
		push	ax
		lea	ax, DOS_Parameters
		push	ax
		lea	ax, Keyb_Status
		push	ax
		lea	ax, KeyStroke
		push	ax
		call	GetDOSCommand

		jmp	RefreshPath

NotACommand:
		cmp	Keyb_Status, KEY_WAITING	; Is key in buffer ?
		je	short KeyAwaits
		jmp	NextAction		; No -- Wait til key is pressed

KeyAwaits:
		lea	ax, KeyStroke		; Save key pressed in this
		push	ax			;    var
		call	GetKey			; Get the keystroke from buf

		mov	ax, KeyStroke		; Load user's input
		cmp	al, QUIT_KEY		; Was QUIT key pressed ?
		jne	short NotQuitKey	; No -- check the others
		jmp	ExitHere		; Yes -- we're outta here !

NotQuitKey:
		cmp	al, TAB_KEY
		jne	short NotTabKey

		lea	ax, FileList
		push	ax
		lea	ax, FileListControl
		push	ax
		mov	ax, VideoSegment
		push	ax
		call	FileListBarOff

		mov	ax, ActiveWindow
		not	ax
		and	ax, 01h
		mov	ActiveWindow, ax

		cmp	ax, FIRST_WINDOW
		je	short TabWin1

		lea	ax, FullDirTwo
		mov	dx, DriveNumTwo
		and	UpdateOne, NULL
		jmp	short TabWin2

TabWin1:
		lea	ax, FullDirOne
		mov	dx, DriveNumOne
		and	UpdateTwo, NULL

TabWin2:
		push	dx
		call	ChangeDrive

		mov	si, ActiveWindow
		push	si
		lea	bx, Update_Flags
		push	bx
		push	ax
		call	ChangeDirectory

		and	byte ptr [bx][si], NUPD_NEWPATH
		or	byte ptr [bx][si], UPD_RESET
		not	si
		and	si, 01h
		and	byte ptr [bx][si], NUPD_NEWPATH

		lea	ax, FileList
		push	ax
		lea	ax, FileListControl
		push	ax
		mov	ax, VideoSegment
		push	ax
		call	FileListBarOn

		jmp	RefreshPath

NotTabKey:
		cmp	al, ENTER_KEY
		je	short EnterKey
		jmp	NotEnterKey

EnterKey:
		lea	ax, ActiveListStart
		push	ax
		lea	ax, FileLocation
		push	ax
		mov	ax, ActiveWindow
		push	ax
		call	GetActStart

		lea	ax, FileListControl
		push	ax
		lea	ax, Act_List
		push	ax
		call	ConvertCoords

		mov	ax, Act_List
		push	ax
		lea	ax, Act_Ofs
		push	ax
		call	FindFileEntry

		mov	ax, Act_Ofs
		push	ax
		mov	ax, ActiveWindow
		push	ax
		mov	ax, ListOne
		push	ax
		mov	ax, ListTwo
		push	ax
		call	MarkCurrFile

		cmp	ActiveWindow, FIRST_WINDOW
		je	short SrchWin1

		mov	ax, SEARCH_CURRENT
		push	ax
		lea	dx, FullDirTwo
		mov	ax, FileCount_W2
		push	ax
		lea	ax, Act_Ofs
		push	ax
		mov	bx, ListTwo
		push	bx
		jmp	short SrchWin2

SrchWin1:
		mov	ax, SEARCH_CURRENT
		push	ax
		lea	dx, FullDirOne
		mov	ax, FileCount_W1
		push	ax
		lea	ax, Act_Ofs
		push	ax
		mov	bx, ListOne
		push	bx

SrchWin2:
		call	SearchListBlock

		lea	ax, FileEntry
		push	ax
		push	bx
		mov	ax, Act_Ofs
		push	ax
		call	RetrieveEntry

		push	dx
		mov	ax, ActiveWindow
		push	ax
		lea	ax, Update_Flags
		push	ax
		lea	ax, FileEntry
		push	ax
		call	DecideOnEnter

		lea	bx, Update_Flags
		or	[bx][ActiveWindow], UPD_NEWPATH
		and	[bx][ActiveWindow], NUPD_NEWDRV

		mov	ax, Act_List
		push	ax
		lea	ax, Act_Ofs
		push	ax
		call	FindFileEntry

		mov	ax, Act_Ofs
		push	ax
		mov	ax, ActiveWindow
		push	ax
		mov	ax, ListOne
		push	ax
		mov	ax, ListTwo
		push	ax
		call	UnMarkCurrFile

		jmp	RefreshPath

NotEnterKey:
		push	ax

		mov	bx, Block_Size
		push	bx
		lea	bx, MaxAvail
		push	bx
		lea	bx, Error_Word
		push	bx
		lea	bx, Parent_Tag
		push	bx
		lea	bx, FileMask
		push	bx
		mov	bx, DTA_Segment
		push	bx
		mov	bx, DTA_Offset
		push	bx
		lea	bx, Update_Flags
		push	bx

		mov	bx, FileCount_W1
		push	bx
		mov	bx, FileCount_W2
		push	bx
		lea	bx, Scratch_Word
		push	bx
		mov	bx, ActiveWindow
		push	bx
		lea	bx, Scratch_Buffer
		push	bx
		lea	bx, FileEntry
		push	bx
		mov	bx, ListOne
		push	bx
		mov	bx, ListTwo
		push	bx
		lea	bx, DriveNumOne
		push	bx
		lea	bx, DriveNumTwo
		push	bx
		lea	bx, DirectoryOne
		push	bx
		lea	bx, DirectoryTwo
		push	bx

		lea	ax, KeyStroke
		push	ax
		lea	ax, MenuChoice
		push	ax
		lea	ax, ControlPanelLoc
		push	ax
		mov	ax, VideoSegment
		push	ax
		lea	ax, MenuNames
		push	ax
		lea	ax, ControlOptions
		push	ax

		call	DecideOperation

;;;;; ************** FOLLOWING CODE RELATES TO CURSOR MOVEMENT KEYS
;;;;; ******************** AND SPECIAL MOVEMENT KEYS

		lea	ax, FileList
		push	ax
		lea	ax, FileListControl
		push	ax
		mov	ax, VideoSegment
		push	ax
		call	FileListBarOff

		lea	ax, ActiveListStart
		push	ax
		lea	ax, FileLocation
		push	ax
		mov	ax, ActiveWindow
		push	ax
		call	GetActStart

		mov	ax, Act_List
		push	ax
		mov	ax, Act_Ofs
		push	ax

		lea	ax, FileListControl
		push	ax
		lea	ax, Act_List
		push	ax
		call	ConvertCoords

		mov	ax, Act_List
		push	ax
		lea	ax, Act_Ofs
		push	ax
		call	FindFileEntry

		mov	ax, Act_Ofs
		push	ax
		mov	ax, ActiveWindow
		push	ax
		mov	ax, ListOne
		push	ax
		mov	ax, ListTwo
		push	ax
		call	UnMarkCurrFile

		pop	ax
		mov	Act_Ofs, ax
		pop	ax
		mov	Act_List, ax

		pop	ax

		cmp	ah, DOWN_ARROW
		jne	short NotDownArrow

		lea	ax, FileListControl
		push	ax
		lea	ax, Act_List
		push	ax
		call	FileListDown
		jmp	LoopBack

NotDownArrow:
		cmp	ah, UP_ARROW
		jne	short NotUpArrow

		lea	ax, FileListControl
		push	ax
		lea	ax, Act_List
		push	ax
		call	FileListUp
		jmp	LoopBack

NotUpArrow:
		cmp	ah, LEFT_ARROW
		jne	short NotLeftArrow

		lea	ax, FileListControl
		push	ax
		lea	ax, Act_List
		push	ax
		call	FileListLeft
		jmp	LoopBack

NotLeftArrow:
		cmp	ah, RIGHT_ARROW
		jne	short NotRightArrow

		lea	ax, FileListControl
		push	ax
		lea	ax, Act_List
		push	ax
		call	FileListRight
		jmp	LoopBack

NotRightArrow:
		cmp	ah, TOP_LIST_KEY
		jne	short NotHomeKey

		lea	ax, FileListControl
		push	ax
		lea	ax, Act_List
		push	ax
		call	GoListTop
		jmp	LoopBack

NotHomeKey:
		cmp	ah, BOT_LIST_KEY
		jne	short NotBotKey

		lea	ax, FileListControl
		push	ax
		lea	ax, Act_List
		push	ax
		call	GoListBot
		jmp	LoopBack

NotBotKey:
		cmp	ah, PARENT_KEY
		jne	short NotParentKey

		cmp	ActiveWindow, FIRST_WINDOW
		je	short GoParWin1

		lea	bx, DirectoryTwo

		mov	al, [bx]
		cmp	al, END_OF_RECORD
		je	short LoopBack
		jmp	short GoParWin2
		
GoParWin1:
		lea	bx, DirectoryOne

		mov	al, [bx]
		cmp	al, END_OF_RECORD
		je	short LoopBack

GoParWin2:
		mov	ax, ActiveWindow
		push	ax
		lea	ax, Update_Flags
		push	ax
		lea	ax, Parent_Tag
		push	ax
		call	GoToParentDir
		jmp	short LoopBack

NotParentKey:
		cmp	ah, SELECT_KEY
		jne	short LoopBack

		mov	bx, Act_List
		push	bx
		mov	bx, Act_Ofs
		push	bx

		lea	ax, FileListControl
		push	ax
		lea	ax, Act_List
		push	ax
		call	ConvertCoords

		mov	ax, Act_List
		push	ax
		lea	ax, Act_Ofs
		push	ax
		call	FindFileEntry

		mov	ax, Act_Ofs
		push	ax
		mov	ax, ActiveWindow
		push	ax
		mov	ax, ListOne
		push	ax
		mov	ax, ListTwo
		push	ax
		call	ToggleSeleFile

		pop	bx
		mov	Act_Ofs, bx
		pop	bx
		mov	Act_List, bx

		mov	al, InsMovesDown
		cmp	al, TRUE
		jne	short LoopBack

		lea	ax, FileListControl
		push	ax
		lea	ax, Act_List
		push	ax
		call	FileListDown

LoopBack:
		mov	ax, Act_List
		push	ax
		lea	ax, Act_Ofs
		push	ax
		call	FindFileEntry

		lea	ax, ActiveListStart
		push	ax
		lea	ax, FileLocation
		push	ax
		mov	ax, ActiveWindow
		push	ax
		call	SetActStart

		lea	ax, FileList
		push	ax
		lea	ax, FileListControl
		push	ax
		mov	ax, VideoSegment
		push	ax
		call	FileListBarOn

		jmp	RefreshPath		; Loop back -- wait for
						;    next keypress

ExitHere:
		mov	ax, ListOne
		cmp	ax, NOT_ALLOC_FLAG
		je	short CheckBlockTwo

		push	ax
		call	FreeMem

CheckBlockTwo:
		mov	ax, ListTwo
		cmp	ax, NOT_ALLOC_FLAG
		je	short FreeUserScreen

		push	ax
		call	FreeMem

FreeUserScreen:
		lea	ax, UserScreenInfo
		push	ax
		mov	ax, VideoSegment
		push	ax
		call	RestUserScreen

		lea	ax, DOS_Parameters
		push	ax
		call	FreeCmdBuffer

;		mov	ax, CTRL_C_INT
;		push	ax
;		mov	ax, OldCtrlC_Seg
;		push	ax
;		mov	ax, OldCtrlC_Ofs
;		push	ax
;		call	SetVector

		call	BackToDOS		; End the program
Main		endp


; SETS THE COLOURS FOR THE VARIOUS ENTITIES WITHIN THE PROGRAM
;	- RETURNS : Nothing
;	- STACK : (ref) dos_line_parms, (ref) menu_system,
;                 (ref) file_list_parms, (ref) colour_table, R_IP
;                 AX, BX, SI, BP

SetupColours	proc	near
		push	ax
		push	bx
		push	si
		push	bp

		mov	bp, sp

		mov	si, [bp+0ah]

		mov	bx, [bp+0ch]
		mov	ax, [si+02h]
		mov	[bx+08h], ax
		mov	ax, [si+04h]
		mov	[bx+10h], ax
		mov	ax, [si+06h]
		mov	[bx+12h], ax

		mov	bx, [bp+0eh]
		mov	ax, [si+0ch]
		mov	[bx+0ah], ax
		mov	ax, [si+0eh]
		mov	[bx+12h], ax

		mov	bx, [bp+10h]
		mov	ax, [si+08h]
		mov	[bx+08h], ax
		mov	ax, [si+0ah]
		mov	[bx+16h], ax

		pop	bp
		pop	si
		pop	bx
		pop	ax
		ret	8
SetupColours	endp


; INITIALISES THE COLOUR TABLE USED BY MC FOR SCREEN DISPLAY
;	- RETURNS : Nothing
;	- STACK : (val) colour_scheme, (val) number_of_table_entries,
;                 (ref) mono_colour_table, (ref) program_colour_table, R_IP
;                 AX, BX, CX, DX, SI, DI, ES, BP

InitColours	proc	near
		push	ax
		push	bx
		push	cx
		push	dx
		push	si
		push	di
		push	es
		push	bp

		mov	bp, sp

		push	ds
		pop	es

		mov	cx, [bp+16h]
		mov	bx, [bp+14h]
		xor	si, si

		mov	ax, [bp+18h]
		cmp	ax, SCHEME_MONO
		je	short UseMonoScheme

		mov	ax, [bp+16h]
		add	bx, ax

UseMonoScheme:
		mov	di, [bp+12h]
		xor	ah, ah

ChooseColours:
		mov	al, [bx][si]
		inc	si

		stosw
		loop	ChooseColours

		pop	bp
		pop	es
		pop	di
		pop	si
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	8
InitColours	endp


; ERASES ALL TEXT ON THE SPECIFIED ROW ONSCREEN
;	- RETURNS : Nothing
;	- STACK : (val) row_to_clear, (val) video_segment, R_IP
;                 AX, BP

ClearLine	proc	near
		push	ax
		push	bp

		mov	bp, sp

		xor	ax, ax
		push	ax
		mov	ax, [bp+08h]
		push	ax
		call	SetLocation

		mov	ah, NORMAL_VIDEO
		mov	al, SPACE
		push	ax
		mov	ax, SCREEN_WIDTH
		push	ax
		mov	ax, [bp+06h]
		push	ax
		call	WriteChar

		pop	bp
		pop	ax
		ret	4
ClearLine	endp


; DECIDE IF THE CURRENTLY-HIGHLIGHTED FILE ENTRY IS A DIRECTORY ENTRY,
; AND, IF SO, CHANGE TO IT
;	- RETURNS : Nothing
;	- STACK : (ref) current_full_directory, (val) active_window,
;                 (ref) update_flags, (ref) file_entry_buffer, R_IP
;                 AX, BX, BP

DecideOnEnter	proc	near
		push	ax
		push	bx
		push	bp

		mov	bp, sp

		mov	bx, [bp+08h]
		mov	al, [bx]
		and	al, ATTR_DIR
		cmp	al, ATTR_DIR
		jne	short DoneDirCheck

		mov	ax, [bp+0eh]
		push	ax
		call	StrLength

		mov	bx, [bp+0eh]
		add	bx, ax
		dec	bx
		cmp	byte ptr [bx-01h], '\'
		je	short RootDir

		mov	byte ptr [bx], '\'
		inc	bx

RootDir:
		push	ds
		push	bx
		push	ds
		mov	ax, [bp+08h]
		add	ax, FILENAME_OFFS
		push	ax
		xor	ax, ax
		push	ax
		call	CopyString

		mov	ax, [bp+0ch]
		push	ax
		mov	ax, [bp+0ah]
		push	ax
		mov	ax, [bp+0eh]
		push	ax
		call	ChangeDirectory

DoneDirCheck:
		pop	bp
		pop	bx
		pop	ax
		ret	8
DecideOnEnter	endp


; DISPLAYS A MESSAGE INDICATING AN ERROR CONDITION
;	- RETURN : Nothing
;	- STACK : (val) error_code, R_IP
;                 AX, DX, BP

ErrorProc	proc	near
		push	ax			; Save our registers
		push	dx
		push	bp

		mov	bp, sp			; Index for parms on stack

		mov	ax, [bp+08h]		; Get error-code from stack
		cmp	ax, ERROR_MEM_ALLOC	; Mem-allocation error ?
		jne	short Next_2		; No -- test next error
		lea	dx, Err_Alloc_Msg	; Load allocation error-msg
		jmp	DisplayMsg		; Write the message

Next_2:
		cmp	ax, ERROR_MEM_FREE	; Mem-freeing error ?
		jne	short Next_7		; No -- test for next error
		lea	dx, Err_Free_Msg	; Load mem-release error msg
		jmp	DisplayMsg		; Display the message

Next_7:
		cmp	ax, ERROR_MEM_BROKE	; MCB destroyed ?
		jne	short Next_8		; No -- test for next error
		lea	dx, Err_MCB_Broke	; Load proper message
		jmp	DisplayMsg		;    and display it

Next_8:
		cmp	ax, ERROR_MEM_OUT	; No more memory ?
		jne	short Next_9		; No -- test next error
		lea	dx, Err_Out_Of_Mem	; Load proper msg
		jmp	short DisplayMsg	;    and write it

Next_9:
		cmp	ax, ERROR_MEM_ADDR	; Invalid address ?
		jne	short Next_100		; No -- test next one
		lea	dx, Err_Bad_Addr	; Load bad-addr error msg
		jmp	short DisplayMsg	; Write the message

Next_100:
		cmp	ax, ERROR_USE_ALLOC	; No mem for user screen ?
		jne	short Next_101		; No -- test next
		lea	dx, Err_User_Scrn	; Load msg
		jmp	short DisplayMsg	; Write It

Next_101:
		cmp	ax, ERROR_USE_DEALL	; Error deallocating user mem ?
		jne	short Next_102		; No -- test next error
		lea	dx, Err_User_Deall	; Load message
		jmp	short DisplayMsg	; Display it

Next_102:
		cmp	ax, ERROR_FIL_INVAL
		jne	short Next_103
		lea	dx, Err_Not_Found
		jmp	short DisplayMsg

Next_103:
		cmp	ax, ERROR_FIL_PATH
		jne	short Next_105
		lea	dx, Err_Bad_Path
		jmp	short DisplayMsg

Next_105:
		cmp	ax, ERROR_FIL_ACCDEN
		jne	short Next_106
		lea	dx, Err_Access
		jmp	short DisplayMsg

Next_106:
		cmp	ax, ERROR_FIL_MODE
		jne	short Next_107
		lea	dx, Err_File_Mode
		jmp	short DisplayMsg

Next_107:
		cmp	ax, ERROR_FIL_NHAND
		jne	short Next_108
		lea	dx, Err_No_Handles
		jmp	short DisplayMsg

Next_108:
		cmp	ax, ERROR_FIL_FUNC
		jne	short Next_109
		lea	dx, Err_Bad_Func
		jmp	short DisplayMsg

Next_109:
		cmp	ax, ERROR_FIL_BHAND
		jne	short UnknownErr
		lea	dx, Err_Bad_Handle
		jmp	short DisplayMsg

Next_10a:
		cmp	ax, ERROR_FIL_LOST_ATTR
		jne	short UnknownErr
		lea	dx, Err_Lost_Attr
		jmp	short DisplayMsg

UnknownErr:
		lea	dx, UnknownMsg		; Load msg for unknown error

DisplayMsg:
		mov	ah, 09h			; Request display_string
		int	21h			; Call DOS

		lea	dx, KeyStroke		; Generates a loop to
		push	dx			;    wait for the user to
		call	GetKey			;       press a key

;	REMOVE THE ERROR MESSAGE FROM THE SCREEN

		pop	bp			; Restore registers
		pop	dx
		pop	ax
		ret	2			; Take 1 parm off the stack
ErrorProc	endp


; DETERMINES WHICH OPERATION TO PERFORM BASED ON THE INPUT KEY
;	- RETURNS : Nothing
;	- STACK : (val) block_size_in_bytes, (ref) memory_available,
;                 (ref) error_code, (ref) parent_tag_mask,
;                 (ref) mask_star_dot_star, (val) DTA_segment,
;                 (ref) DTA_offset, (ref) update_flags,
;
;                 (val) file_count_window_1, (val) file_count_window_2,
;                 (ref) scratch_offset, (val) active_window,
;
;                 (ref) scratch_buffer, (ref) file_entry_buffer,
;                 (val) list_block_one_segment, (val) list_block_two_segment,
;                 (ref) drive_one, (ref) drive_two,
;                 (ref) directory_one, (ref) directory_two,
;
;                 (ref) keystroke, (ref) menu_choice,
;                 (ref) parm_list, (val) video_seg,
;                 (ref) menu_names, (ref) menu_options, R_IP
;                 AX, BX, BP

DecideOperation	proc	near
		push	ax
		push	bx
		push	bp

		mov	bp, sp

		mov	bx, [bp+12h]
		mov	ax, [bx]
		cmp	al, NULL
		je	short Extended_Key

		jmp	FinishedDecide

Extended_Key:
		mov	bx, ax

		cmp	bh, MENU_KEY
		jne	short NotMenuKey

		mov	ax, [bp+10h]
		push	ax
		mov	ax, [bp+0eh]
		push	ax
		mov	ax, [bp+0ch]
		push	ax
		mov	ax, [bp+0ah]
		push	ax
		mov	ax, [bp+08h]
		push	ax
		call	MakeMenu
		jmp	FinishedDecide

NotMenuKey:
		mov	ax, [bp+24h]
		mov	bl, al
		cmp	bl, FIRST_WINDOW
		je	short DecideWin1

		xor	ax, ax
		push	ax
		mov	ax, [bp+3ah]
		push	ax
		mov	ax, [bp+38h]
		push	ax
		mov	ax, [bp+1ah]
		push	ax
		mov	ax, [bp+16h]
		push	ax

		mov	ax, [bp+36h]
		push	ax
		mov	ax, [bp+34h]
		push	ax
		mov	ax, [bp+32h]
		push	ax
		mov	ax, [bp+30h]
		push	ax
		mov	ax, [bp+2eh]
		push	ax
		mov	ax, [bp+24h]
		push	ax
		mov	ax, [bp+2ch]
		push	ax
		mov	ax, [bp+28h]
		push	ax
		mov	ax, [bp+26h]
		push	ax
		mov	ax, [bp+18h]
		push	ax
		mov	ax, [bp+14h]
		push	ax
		mov	ax, [bp+1ch]
		push	ax
		mov	ax, [bp+20h]
		push	ax
		mov	ax, [bp+22h]
		push	ax
		jmp	short DecideWin2

DecideWin1:
		xor	ax, ax
		push	ax
		mov	ax, [bp+3ah]
		push	ax
		mov	ax, [bp+38h]
		push	ax
		mov	ax, [bp+18h]
		push	ax
		mov	ax, [bp+14h]
		push	ax

		mov	ax, [bp+36h]
		push	ax
		mov	ax, [bp+34h]
		push	ax
		mov	ax, [bp+32h]
		push	ax
		mov	ax, [bp+30h]
		push	ax
		mov	ax, [bp+2eh]
		push	ax
		mov	ax, [bp+24h]
		push	ax
		mov	ax, [bp+2ch]
		push	ax
		mov	ax, [bp+2ah]
		push	ax
		mov	ax, [bp+26h]
		push	ax
		mov	ax, [bp+1ah]
		push	ax
		mov	ax, [bp+16h]
		push	ax
		mov	ax, [bp+1eh]
		push	ax
		mov	ax, [bp+20h]
		push	ax
		mov	ax, [bp+22h]
		push	ax

DecideWin2:
		cmp	bh, DELETE_KEY
		jne	short NotDeleteKey

		call	Delete
		pop	ax
		pop	ax
		pop	ax
		pop	ax
		pop	ax
		jmp	short FinishedDecide

NotDeleteKey:
		cmp	bh, COPY_KEY
		jne	short NotCopyKey

		mov	bp, sp
		mov	word ptr [bp+24h], FUNCTION_COPY

		call	Copy
		jmp	short FinishedDecide

NotCopyKey:
		cmp	bh, MOVE_KEY
		jne	short NotMoveKey

		mov	bp, sp
		mov	word ptr [bp+24h], FUNCTION_MOVE

		call	Copy
		jmp	short FinishedDecide

NotMoveKey:
		mov	cx, 19

CleanUpStack:
		pop	ax
		loop	CleanUpStack

FinishedDecide:
		pop	bp
		pop	bx
		pop	ax
		ret	52
DecideOperation	endp


; SELECTS A FILE FOR VIEWING
;	- RETURNS : Nothing
;	- STACK : (ref) viewer_parameters,
;
;                 (ref) error_code, <marker>,
;                 <marker>, (val) DTA_segment,
;                 (ref) DTA_offset, (val) active_window,
;                 (ref) update_flags, (val) file_count,
;
;                 (ref) scratch_offset, (ref) drive_number,
;                 (ref) directory, (val) list_block_segment,
;                 (ref) file_entry_buffer, (ref) scratch_buffer, R_IP
;                 AX, BX, CX, DX, SI, DI, ES, BP

View		proc	near
		push	ax
		push	bx
		push	cx
		push	dx
		push	si
		push	di
		push	es
		push	bp

		mov	bp, sp

		mov	ax, SEARCH_CURRENT	; Search for files marked as
		push	ax			;    current
		mov	ax, [bp+1eh]		; # of files in the list block
		push	ax			;    (file_count)
		mov	ax, [bp+1ch]		; Offset of word to store offs
		push	ax			;    of entry in (scratch_word)
		mov	ax, [bp+16h]		; Segment value of list block
		push	ax			;    buffer (list_block)
		call	SearchListBlock		; Search for a file entry

		mov	bx, [bp+1ch]		; Get offset of search status
		mov	ax, [bx]		;    and load in
		cmp	ax, END_OF_FLIST	; Was file entry found ?
		jne	short ProcessView	; Yes -- view it
		jmp	DoneView		; No -- abort routine

ProcessView:
		mov	ax, [bp+14h]	; Offset of buffer with single file
		push	ax		;    entry (file_entry_buffer)
		mov	ax, [bp+16h]	; Segment value of current list data
		push	ax		;    (list_block)
		mov	bx, [bp+1ch]	; Addr of offset of entry in list block
		mov	ax, [bx]	; Offset of entry in list block
		push	ax		;    (scratch_word)
		call	RetrieveEntry	; Get file entry from list buffer

		mov	ax, [bp+12h]		; Buffer to accept pathname
		push	ax			;    (scratch_buffer)
		mov	ax, [bp+1ah]		; Drive number to use
		push	ax			;    (drive_number)
		mov	ax, [bp+18h]		; Directory to use
		push	ax			;    (directory)
		mov	ax, [bp+14h]		; Offset of file_entry_buffer
		add	ax, FILENAME_OFFS	; Position at NAME of entry
		push	ax			;    (file_entry_buffer+9)
		call	BuildPathName		; Make a full pathname

		mov	ax, [bp+16h]	; Load the segment value
		mov	es, ax		;    of the current list's block
		mov	bx, [bp+1ch]	; Addr of offset of entry in list block
		mov	ax, [bx]	; Offset of entry in list block
		mov	bx, ax		; Point to file entry being processed
		mov	al, byte ptr es:[bx]
		and	al, ATTR_DIR
		cmp	al, ATTR_DIR

		je	short DoneView

mov ax, 0
push ax
push ax
call setlocation

mov ax, [bp+12h]
push ax
mov ax, reverse_video
push ax
mov ax, videosegment
push ax
call writestring

DoneView:
		pop	bp
		pop	es
		pop	di
		pop	si
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	28
View		endp


; RETURNS CONTROL TO OPERATING SYSTEM
;	- STACK : Nothing

BackToDOS	proc	near
		mov	ah, 4ch			; Request terminate_program
		int	21h			; Call DOS
BackToDOS	endp

CodeSeg		ends
		end	Main
