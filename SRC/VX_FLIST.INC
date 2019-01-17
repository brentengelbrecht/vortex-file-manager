; MOVES UP THE DIRECTORY TREE TO THE PREVIOUS SUB-DIRECTORY
;	- RETURNS : Nothing
;	- STACK : (val) active_window,
;                 (ref) update_flags, (ref) parent_directory_tag, R_IP
;                 AX, BP

GoToParentDir	proc	near
		push	ax
		push	bp

		mov	bp, sp

		mov	ax, [bp+0ah]
		push	ax
		mov	ax, [bp+08h]
		push	ax
		mov	ax, [bp+06h]
		push	ax
		call	ChangeDirectory

		pop	bp
		pop	ax
		ret	6
GoToParentDir	endp


; MOVES THE FILE CURSOR TO THE LAST ENTRY IN THE FILE LIST
;	- RETURNS : Nothing
;	- STACK : (ref) file_list_control, (ref) list_start, R_IP
;                 AX, BX, CX, BP

GoListBot	proc	near
		push	ax
		push	bx
		push	cx
		push	bp

		mov	bp, sp

		mov	bx, [bp+0ch]
		mov	ax, [bx]
		cmp	ax, FIRST_WINDOW
		je	short GoBotWin1

		add	bx, LIST_2_PARMS
		jmp	short GoBotWin2

GoBotWin1:
		add	bx, LIST_1_PARMS

GoBotWin2:
		mov	ax, [bx+06h]
		mov	bx, [bp+0ah]
		sub	ax, [bx]

		mov	cl, 03h
		shl	ax, cl
		inc	ax
		mov	cx, ax

GoBotAgain:
		push	cx

		mov	ax, [bp+0ch]
		push	ax
		mov	ax, [bp+0ah]
		push	ax
		call	FileListRight

		pop	cx
		loop	GoBotAgain

		pop	bp
		pop	cx
		pop	bx
		pop	ax
		ret	4
GoListBot	endp


; MOVES THE FILE CURSOR TO THE FIRST ENTRY IN THE FILE LIST
;	- RETURNS : Nothing
;	- STACK : (ref) file_list_control, (ref) list_start, R_IP
;                 AX, BX, CX, BP

GoListTop	proc	near
		push	ax
		push	bx
		push	cx
		push	bp

		mov	bp, sp

		mov	bx, [bp+0ch]
		mov	ax, [bx]
		cmp	ax, FIRST_WINDOW
		je	short GoTopWin1

		add	bx, LIST_2_PARMS
		jmp	short GoTopWin2

GoTopWin1:
		add	bx, LIST_1_PARMS

GoTopWin2:
		mov	ax, [bx+06h]

		mov	cl, 03h
		shl	ax, cl
		inc	ax
		mov	cx, ax

GoTopAgain:
		push	cx

		mov	ax, [bp+0ch]
		push	ax
		mov	ax, [bp+0ah]
		push	ax
		call	FileListLeft

		pop	cx
		loop	GoTopAgain

		pop	bp
		pop	cx
		pop	bx
		pop	ax
		ret	4
GoListTop	endp


; TOGGLES THE "SELECTED"-STATUS OF THE ENTRY UNDER THE FILE LIST POINTER
;	- RETURNS : Nothing
;	- STACK : (val) offset_in_list_block, (val) active_window,
;                 (val) list_one_segment, (val) list_two_segment, R_IP
;                 AX, BX, DX, ES, BP

ToggleSeleFile	proc	near
		push	ax
		push	bx
		push	dx
		push	es
		push	bp

		mov	bp, sp

		mov	ax, [bp+0eh]
		mov	dx, [bp+10h]
		cmp	dx, FIRST_WINDOW
		je	short TogWin1

		mov	ax, [bp+0ch]

TogWin1:
		mov	es, ax

		mov	bx, [bp+12h]

		mov	al, es:[bx+FILENAME_OFFS]
		cmp	al, '.'
		je	short SkipTog

		mov	al, byte ptr es:[bx+ENTRY_LENGTH-03h]
		and	al, NOT_CURRENT
		cmp	al, SELECTED
		mov	al, byte ptr es:[bx+ENTRY_LENGTH-03h]
		je	short TogTurnOff

		or	al, NOT_CURRENT
		jmp	short DoneTog

TogTurnOff:
		and	al, CURRENT

DoneTog:
		mov	byte ptr es:[bx+ENTRY_LENGTH-03h], al

SkipTog:
		pop	bp
		pop	es
		pop	dx
		pop	bx
		pop	ax
		ret	8
ToggleSeleFile	endp


; MARKS THE FILE UNDER THE FILE LIST POINTER AS "NOT-CURRENT"
;	- RETURNS : Nothing
;	- STACK : (val) offset_in_list_block, (val) active_window,
;                 (val) list_one_segment, (val) list_two_segment, R_IP
;                 AX, BX, DX, ES, BP

UnMarkCurrFile	proc	near
		push	ax
		push	bx
		push	dx
		push	es
		push	bp

		mov	bp, sp

		mov	ax, [bp+0eh]
		mov	dx, [bp+10h]
		cmp	dx, FIRST_WINDOW
		je	short UnMarkWin1

		mov	ax, [bp+0ch]

UnMarkWin1:
		mov	es, ax

		mov	bx, [bp+12h]
		mov	al, byte ptr es:[bx+ENTRY_LENGTH-03h]
		and	al, NOT_CURRENT
		mov	byte ptr es:[bx+ENTRY_LENGTH-03h], al

		pop	bp
		pop	es
		pop	dx
		pop	bx
		pop	ax
		ret	8
UnMarkCurrFile	endp


; MARKS THE FILE UNDER THE FILE LIST POINTER AS "CURRENT"
;	- RETURNS : Nothing
;	- STACK : (val) offset_in_list_block, (val) active_window,
;                 (val) list_one_segment, (val) list_two_segment, R_IP
;                 AX, BX, DX, ES, BP

MarkCurrFile	proc	near
		push	ax
		push	bx
		push	dx
		push	es
		push	bp

		mov	bp, sp

		mov	ax, [bp+0eh]
		mov	dx, [bp+10h]
		cmp	dx, FIRST_WINDOW
		je	short MarkWin1

		mov	ax, [bp+0ch]

MarkWin1:
		mov	es, ax

		mov	bx, [bp+12h]
		mov	al, byte ptr es:[bx+ENTRY_LENGTH-03h]
		or	al, CURRENT
		mov	byte ptr es:[bx+ENTRY_LENGTH-03h], al

		pop	bp
		pop	es
		pop	dx
		pop	bx
		pop	ax
		ret	8
MarkCurrFile	endp


; INITIALISES THE FILE LIST POINTER VARIABLES TO ZERO
;	- RETURNS : Nothing
;	- STACK : (val) window_to_clear, (ref) file_list_control,
;                 (ref) file_location, R_IP
;                 AX, BX, BP

FileListInit	proc	near
		push	ax
		push	bx
		push	bp

		mov	bp, sp

		mov	bx, [bp+0ah]
		mov	ax, [bp+0ch]
		cmp	ax, FIRST_WINDOW
		je	short ClearWin1Ptr

		add	bx, LIST_2_PARMS
		mov	ax, 04h
		jmp	short ClearWin2Ptr

ClearWin1Ptr:
		add	bx, LIST_1_PARMS
		xor	ax, ax

ClearWin2Ptr:
		mov	word ptr [bx], 00h
		mov	word ptr [bx+02h], 00h
		mov	word ptr [bx+04h], 00h
		mov	word ptr [bx+06h], 00h

		mov	bx, [bp+08h]
		add	bx, ax
		mov	word ptr [bx], 00h
		mov	word ptr [bx+02h], 00h

		pop	bp
		pop	bx
		pop	ax
		ret	4
FileListInit	endp


; WRITES THE PATH FOR EACH FILE LIST TO IT'S FILE LIST WINDOW
;	- RETURNS : Nothing
;	- STACK : (ref) dos_line_parms,
;                 (ref) dos_line, (ref) win_1_path,
;                 (ref) win_2_path, (val) active_win,
;                 (ref) file_list_data, (val) video_seg, R_IP
;                 AX, BX, CX, DX, SI, DI, ES, BP

FileListTitles	proc	near
		push	ax
		push	bx
		push	cx
		push	dx
		push	si
		push	di
		push	es
		push	bp

		mov	bp, sp

		mov	bx, [bp+14h]

		mov	ax, [bp+16h]
		cmp	ax, FIRST_WINDOW
		je	short OneActive

		mov	cx, [bx+08h]
		mov	dx, [bx+10h]
		or	cx, 08h		; Non-active pathname is intensified
		mov	si, [bp+18h]
		jmp	short ShowActiveList

OneActive:
		mov	cx, [bx+10h]
		mov	dx, [bx+08h]
		or	dx, 08h		; Non-active pathname is intensified
		mov	si, [bp+1ah]

ShowActiveList:
		mov	ax, [bx]
		push	ax
		mov	ax, [bx+04h]
		push	ax
		mov	ax, [bx+02h]
		push	ax
		mov	ax, [bp+1ah]
		push	ax
		mov	ax, cx
		push	ax
		mov	ax, [bp+12h]
		push	ax
		call	WriteCentred

		mov	ax, [bx]
		push	ax
		mov	ax, [bx+04h]
		push	ax
		mov	ax, [bx+02h]
		add	ax, [bx+06h]
		push	ax
		mov	ax, [bp+18h]
		push	ax
		mov	ax, dx
		push	ax
		mov	ax, [bp+12h]
		push	ax
		call	WriteCentred

		mov	ax, PATH_SIZE
		push	ax
		call	AllocMem	; Get memory to save the path in
		mov	cx, ax

		cmp	ax, NOT_ALLOC_FLAG
		jne	short DosLineGotMem

		mov	ax, ERROR_MEM_ALLOC
		push	ax
		call	ErrorProc
		jmp	DoneFileListTitles

DosLineGotMem:
		mov	ax, DOS_ROW
		push	ax
		mov	ax, [bp+12h]
		push	ax
		call	ClearLine	; Erase all text on DOS LINE row

		mov	bx, [bp+1eh]

		xor	ax, ax
		push	ax
		mov	ax, DOS_ROW
		push	ax
		mov	ax, SCREEN_WIDTH
		push	ax
		mov	ax, [bx+16h]
		push	ax
		mov	ax, [bp+12h]
		push	ax
		call	AttributeLine	; Colour the dos_row as de-active

		push	cx
		xor	ax, ax
		push	ax
		push	ds
		push	si
		mov	ax, PATH_SIZE
		push	ax
		call	CopyString	; Save the active-path in buffer

		inc	si		; Don't count leading SPACE
		push	si
		call	StrLength
		sub	ax, 06h		; Don't display '\*.* '
		mov	bx, ax		; Have length of the prompt

		cmp	byte ptr [si+bx-01h], ':'	; Are we in root dir ?
		jne	short NotInRtDir

		inc	bx		; Yes -- leave room for the '\'
		inc	ax		;    char in e.g. 'D:\>'

NotInRtDir:
		mov	byte ptr [bx+si], '>'
		mov	byte ptr [bx+si+01h], NULL

		mov	bx, [bp+1eh]
		mov	[bx+12h], ax	; Save length of prompt

		xor	ax, ax
		push	ax
		mov	ax, DOS_ROW
		push	ax
		call	SetLocation	; Position at second-last screen row

		push	si
		mov	ax, [bx+16h]
		push	ax
		mov	ax, [bp+12h]
		push	ax
		call	WriteString	; Write the prompt to the screen

		mov	ax, [bp+1ch]
		push	ax
		call	StrLength
		mov	[bx+10h], ax	; Determine length of input string

		mov	ax, [bx+12h]
		inc	ax
		mov	[bx], ax	; Determine window's starting column
					;    (SCREEN_WIDTH - length_of_prompt)

		mov	ax, [bx]	; X-coordinate of window
		add	ax, [bx+10h]	;    + current length of input string
		dec	ax
		mov	[bx+0ah], ax	; Store cursor position

		mov	ax, SCREEN_WIDTH
		sub	ax, [bx+12h]
		dec	ax
		mov	[bx+04h], ax	; Store width of the window
					;    (SCREEN_WIDTH - length of prompt)

		mov	word ptr [bx+14h], NULL
		mov	ax, [bx+04h]
		cmp	ax, [bx+10h]	; Is window wider than input-string ?
		ja	short NotWider	; Yes -- don't need to scroll it

		mov	ax, [bx+10h]
		sub	ax, [bx+04h]
		mov	[bx+14h], ax

		mov	ax, [bx]
		add	ax, [bx+04h]
		dec	ax
		mov	[bx+0ah], ax

NotWider:
		mov	ax, [bx]
		push	ax
		mov	ax, [bx+02h]
		push	ax
		call	SetLocation	; Locate the start of the window

		mov	ax, [bp+1ch]
		add	ax, [bx+14h]
		push	ax
		mov	ax, [bx+16h]
		push	ax
		mov	ax, [bp+12h]
		push	ax
		call	WriteString	; Write the input string to screen

		mov	ax, [bx+0ah]
		push	ax
		mov	ax, [bx+02h]
		push	ax
		call	SetCursor	; Position the onscreen-cursor

		push	ds
		push	si
		push	cx
		xor	ax, ax
		push	ax
		mov	ax, PATH_SIZE
		push	ax
		call	CopyString	; Restore the current pathname

		push	cx
		call	FreeMem		; Deallocate save-buffer memory

DoneFileListTitles:
		pop	bp		; Restore registers
		pop	es
		pop	di
		pop	si
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	14
FileListTitles	endp


; HIGHLIGHTS THE ENTRY UNDER THE FILE LIST CURSOR
;	- RETURNS : Nothing
;	- STACK : (ref) file_list_data, (ref) file_list_control,
;                 (val) video_segment, R_IP
;                 AX, BX, DI, SI, BP

FileListBarOn	proc	near
		push	ax
		push	bx
		push	di
		push	si
		push	bp

		mov	bp, sp

		mov	si, [bp+0eh]
		mov	di, si
		cmp	word ptr [si], FIRST_WINDOW

		je	short AttrWinOne
		add	di, LIST_2_PARMS
		jmp	short AttrWinTwo

AttrWinOne:
		add	di, LIST_1_PARMS

AttrWinTwo:
		mov	bx, [bp+10h]

		mov	ax, [bx]
		inc	ax
		add	ax, [di]
		push	ax
		mov	ax, [bx+02h]
		inc	ax
		cmp	word ptr [si], FIRST_WINDOW

		je	short BarFirstWin
		add	ax, [bx+06h]

BarFirstWin:
		add	ax, [di+02h]
		push	ax
		mov	ax, [bx+0ah]
		add	ax, NAME_EXT_SPACE
		push	ax
		mov	ax, [bx+10h]
		push	ax
		mov	ax, [bp+0ch]
		push	ax
		call	AttributeLine

		pop	bp
		pop	si
		pop	di
		pop	bx
		pop	ax
		ret	6
FileListBarOn	endp


; DE-HIGHLIGHTS THE ENTRY UNDER THE FILE LIST CURSOR
;	- RETURNS : Nothing
;	- STACK : (ref) file_list_data, (ref) file_list_control,
;                 (val) video_segment, R_IP
;                 AX, BX, DI, SI, BP

FileListBarOff	proc	near
		push	ax
		push	bx
		push	di
		push	si
		push	bp

		mov	bp, sp

		mov	si, [bp+0eh]
		mov	di, si
		cmp	word ptr [si], FIRST_WINDOW

		je	short DeAttrWinOne
		add	di, LIST_2_PARMS
		jmp	short DeAttrWinTwo

DeAttrWinOne:
		add	di, LIST_1_PARMS

DeAttrWinTwo:
		mov	bx, [bp+10h]

		mov	ax, [bx]
		inc	ax
		add	ax, [di]
		push	ax
		mov	ax, [bx+02h]
		inc	ax
		cmp	word ptr [si], FIRST_WINDOW

		je	short DeBarFirstWin
		add	ax, [bx+06h]

DeBarFirstWin:
		add	ax, [di+02h]
		push	ax
		mov	ax, [bx+0ah]
		add	ax, NAME_EXT_SPACE
		push	ax
		mov	ax, [bx+08h]
		push	ax
		mov	ax, [bp+0ch]
		push	ax
		call	AttributeLine

		pop	bp
		pop	si
		pop	di
		pop	bx
		pop	ax
		ret	6
FileListBarOff	endp


; MOVES THE FILE LIST POINTER ON THE SCREEN TO THE RIGHT
;	- RETURNS : Nothing
;	- STACK : (ref) file_list_control, (ref) start_file_list, R_IP
;                 AX, BX, DX, SI, BP

FileListRight	proc	near
		push	ax
		push	bx
		push	dx
		push	si
		push	bp

		mov	bp, sp

		mov	si, [bp+0eh]

		cmp	word ptr [si], FIRST_WINDOW
		mov	bx, si
		je	short RightFirstWin

		add	bx, LIST_2_PARMS
		jmp	short GotYCoordRight

RightFirstWin:
		add	bx, LIST_1_PARMS

GotYCoordRight:
		mov	ax, [bx+04h]
		inc	ax
		mov	dx, MAX_ROWS
		mul	dl
		add	ax, [bx+02h]
		mov	dx, [bx+06h]
		cmp	ax, dx
		jb	short GoRightOK

		mov	cx, MAX_ROWS

GoToListEnd:
		push	cx

		mov	ax, [bp+0eh]
		push	ax
		mov	ax, [bp+0ch]
		push	ax
		call	FileListDown

		pop	cx
		loop	GoToListEnd

		jmp	short ListRightDone

GoRightOK:
		inc	word ptr [bx+04h]
		cmp	word ptr [bx+04h], MAX_COLS
		jb	short NoRightPrecede
		mov	si, [bp+0ch]
		add	word ptr [si], MAX_ROWS
		jmp	short ListRightDone

NoRightPrecede:
		add	word ptr [bx], FILENAME_LEN + NAME_EXT_SPACE

ListRightDone:
		pop	bp
		pop	si
		pop	dx
		pop	bx
		pop	ax
		ret	4
FileListRight	endp


; MOVES THE FILE LIST POINTER ON THE SCREEN TO THE LEFT
;	- RETURNS : Nothing
;	- STACK : (ref) file_list_control, (ref) start_file_list, R_IP
;                 AX, BX, DX, SI, BP

FileListLeft	proc	near
		push	ax
		push	bx
		push	dx
		push	si
		push	bp

		mov	bp, sp

		mov	si, [bp+0eh]

		cmp	word ptr [si], FIRST_WINDOW
		mov	bx, si
		je	short LeftFirstWin

		add	bx, LIST_2_PARMS
		jmp	short GotYCoordLeft

LeftFirstWin:
		add	bx, LIST_1_PARMS

GotYCoordLeft:
		cmp	word ptr [bx+04h], 00h
		jne	short FileLeft_X_OK

		mov	word ptr [bx+02h], 00h
		mov	word ptr [bx+04h], 00h
		jmp	short ListLeftDone

FileLeft_X_OK:
		dec	word ptr [bx+04h]
		cmp	word ptr [bx+04h], MAX_COLS - 1
		jb	short NoLeftAdvance
		mov	si, [bp+0ch]
		sub	word ptr [si], MAX_ROWS
		jmp	short ListLeftDone

NoLeftAdvance:
		sub	word ptr [bx], FILENAME_LEN + NAME_EXT_SPACE

ListLeftDone:
		pop	bp
		pop	si
		pop	dx
		pop	bx
		pop	ax
		ret	4
FileListLeft	endp


; MOVES THE FILE LIST POINTER UP THE SCREEN
;	- RETURNS : Nothing
;	- STACK : (ref) file_list_control, (ref) start_file_list, R_IP
;                 AX, BX, DX, SI, BP

FileListUp	proc	near
		push	ax
		push	bx
		push	dx
		push	si
		push	bp

		mov	bp, sp

		mov	si, [bp+0eh]

		cmp	word ptr [si], FIRST_WINDOW
		mov	bx, si
		je	short ThisIsFirst

		add	bx, LIST_2_PARMS
		jmp	short GotYCoordUp

ThisIsFirst:
		add	bx, LIST_1_PARMS

GotYCoordUp:
		xor	dx, dx
		cmp	word ptr [bx], 00h
		jnz	short FileUp_Y_OK

		cmp	word ptr [bx+02h], 00h
		jne	short FileUp_Y_OK
		or	dx, 01h

FileUp_Y_OK:
		or	dx, dx
		jnz	short ListUpDone

		cmp	word ptr [bx+02h], 00h
		je	short GoToPrevCol
		dec	word ptr [bx+02h]
		jmp	short ListUpDone

GoToPrevCol:
		mov	word ptr [bx+02h], MAX_ROWS - 1
		dec	word ptr [bx+04h]
		cmp	word ptr [bx+04h], MAX_COLS - 1
		jb	short NoPrecedeFile
		mov	si, [bp+0ch]
		sub	word ptr [si], MAX_ROWS
		jmp	short ListUpDone

NoPrecedeFile:
		sub	word ptr [bx], FILENAME_LEN + NAME_EXT_SPACE

ListUpDone:
		pop	bp
		pop	si
		pop	dx
		pop	bx
		pop	ax
		ret	4
FileListUp	endp


; MOVES THE FILE LIST POINTER DOWN THE SCREEN
;	- RETURNS : Nothing
;	- STACK : (ref) file_list_control, (ref) start_file_list, R_IP
;                 AX, BX, DX, SI, BP

FileListDown	proc	near
		push	ax
		push	bx
		push	dx
		push	si
		push	bp

		mov	bp, sp

		mov	si, [bp+0eh]

		cmp	word ptr [si], FIRST_WINDOW
		mov	bx, si
		je	short ThisWinFirst

		add	bx, LIST_2_PARMS
		jmp	short GotYCoord

ThisWinFirst:
		add	bx, LIST_1_PARMS

GotYCoord:
		mov	ax, [bx+04h]
		mov	dx, MAX_ROWS
		mul	dl
		add	ax, word ptr [bx+02h]
		mov	dx, [bx+06h]
		dec	dx
		cmp	ax, dx
		jae	short ListDownDone

		inc	word ptr [bx+02h]
		cmp	word ptr [bx+02h], MAX_ROWS
		jae	short GoToNextCol
		jmp	short ListDownDone

GoToNextCol:
		mov	word ptr [bx+02h], 00h
		inc	word ptr [bx+04h]
		cmp	word ptr [bx+04h], MAX_COLS
		jb	short NoAdvanceFile
		mov	si, [bp+0ch]
		add	word ptr [si], MAX_ROWS
		jmp	short ListDownDone

NoAdvanceFile:
		add	word ptr [bx], FILENAME_LEN + NAME_EXT_SPACE

ListDownDone:
		pop	bp
		pop	si
		pop	dx
		pop	bx
		pop	ax
		ret	4
FileListDown	endp


; READS THE DIRECTORY ENTRIES FOR THE CURRENT PATH INTO MEMORY
;	- RETURNS : list_block = segment address of loaded data
;	- STACK : (ref) error_code, (ref) list_block,
;                 (ref) pathname, (val) DTA_seg,
;                 (val) DTA_ofs, (ref) count, R_IP
;                 AX, BX, CX, DX, DS, ES, SI, DI, BP

LoadFileList	proc	near
		push	ax
		push	bx
		push	cx
		push	dx
		push	ds
		push	es
		push	si
		push	di
		push	bp

		mov	bp, sp

		mov	bx, [bp+1ch]
		mov	ax, [bx]
		cmp	ax, NOT_ALLOC_FLAG
		je	short FirstTime

		push	ax
		call	FreeMem
		cmp	ax, NOT_ALLOC_FLAG
		jne	short FirstTime

		mov	bx, [bp+1eh]
		mov	word ptr [bx], ERROR_DSK_LOAD
		jmp	DoneLoadList

FirstTime:
		mov	bx, [bp+1ah]
		mov	ax, TINY_BUFFER
		mov	si, ax
		push	ax
		call	AllocMem
		cmp	ax, NOT_ALLOC_FLAG
		jne	short GotListMemory
		jmp	DoneLoadList

GotListMemory:
		xor	di, di
		push	di
		push	ax
		push	di
		push	si
		call	InitBuffer

		mov	bx, [bp+1ch]
		mov	[bx], ax
		mov	es, ax
		xor	di, di
		mov	bx, si

		mov	si, [bp+16h]
		mov	dx, [bp+18h]

		mov	ax, [bp+1ah]
		inc	ax
		push	ax
		call	GetFirstFile
		xor	cx, cx

ListGetNext:
		cmp	ax, ERROR_FIL_NMORE
		je	short DoneLoadList

		inc	cx

		push	dx
		push	si
		push	es
		push	di
		call	CopyDTAEntry
		add	di, ENTRY_LENGTH
		mov	byte ptr es:[di-03h], NULL

		mov	ax, bx
		sub	ax, ENTRY_LENGTH
		cmp	di, ax
		jb	short EnoughMem

		push	bx

		mov	ax, [bp+1eh]
		push	ax
		push	es
		mov	ax, bx
		shl	ax, 1
		push	ax
		call	ResizeMem

		mov	bx, [bp+1eh]
		mov	ax, [bx]

		pop	bx

		cmp	ax, NULL
		je	short LoadResizeOK

		mov	ax, ERROR_TOO_MANY_FILES
		mov	bx, [bp+1eh]
		mov	[bx], ax
		jmp	short DoneLoadList

LoadResizeOK:
		shl	bx, 1

		xor	ax, ax
		push	ax
		push	es
		mov	ax, di
		push	ax
		mov	ax, bx
		sub	ax, di
		push	ax
		call	InitBuffer

EnoughMem:
		call	GetNextFile
		jmp	ListGetNext

DoneLoadList:
		mov	bx, ss:[bp+14h]
		mov	ss:[bx], cx

		pop	bp
		pop	di
		pop	si
		pop	es
		pop	ds
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	12
LoadFileList	endp


; INITIALISES A FILE LIST WINDOW ON THE SCREEN
;	- RETURNS : Nothing
;	- STACK : (ref) location_data, (val) window_number,
;                 (val) video_segment, R_IP
;                 AX, BX, DX, BP

InitListWindow	proc	near
		push	ax
		push	bx
		push	dx
		push	bp

		mov	bp, sp

		mov	bx, [bp+0eh]
		mov	ax, [bx]
		push	ax
		mov	ax, [bx+06h]
		mov	dx, [bp+0ch]
		mul	dl
		add	ax, [bx+02h]
		push	ax
		mov	ax, [bx+04h]
		push	ax
		mov	ax, [bx+06h]
		sub	ax, 02h
		push	ax
		mov	ax, [bx+08h]
		push	ax
		mov	ax, [bp+0ah]
		push	ax
		call	DrawWindow

		pop	bp
		pop	dx
		pop	bx
		pop	ax
		ret	6
InitListWindow	endp


; DRAWS BORDERS ON BOTH FILE LIST WINDOWS ON THE SCREEN
;	- RETURNS : Nothing
;	- STACK : (val) active_window, (ref) border_chars
;                 (ref) location_data, (val) video_segment, R_IP
;                 AX, BX, DX, BP

BorderLists	proc	near
		push	ax
		push	bx
		push	dx
		push	bp

		mov	bp, sp

		mov	dx, [bp+10h]

		mov	bx, [bp+0ch]
		mov	ax, [bx]
		push	ax
		mov	ax, [bx+02h]
		push	ax
		mov	ax, [bx+04h]
		push	ax
		mov	ax, [bx+06h]
		sub	ax, 02h
		push	ax
		mov	ax, [bp+0eh]
		push	ax
		cmp	dx, FIRST_WINDOW
		jne	short NextIsActive
		mov	ax, BORD_DOUBLE
		jmp	short ThisIsActive

NextIsActive:
		mov	ax, BORD_SINGLE

ThisIsActive:
		mov	dx, ax
		not	dx
		and	dx, 01h
		push	ax
		mov	ax, [bx+08h]
		push	ax
		mov	ax, [bp+0ah]
		push	ax
		call	DrawBorder

		mov	bx, [bp+0ch]
		mov	ax, [bx]
		push	ax
		mov	ax, [bx+06h]
		add	ax, [bx+02h]
		push	ax
		mov	ax, [bx+04h]
		push	ax
		mov	ax, [bx+06h]
		sub	ax, 02h
		push	ax
		mov	ax, [bp+0eh]
		push	ax
		push	dx
		mov	ax, [bx+08h]
		push	ax
		mov	ax, [bp+0ah]
		push	ax
		call	DrawBorder

		pop	bp
		pop	dx
		pop	bx
		pop	ax
		ret	8
BorderLists	endp


; CALCULATES A LINEAR OFFSET FROM A SET OF FILE LIST COORDINATES
;	- RETURNS : Nothing
;	- STACK : (ref) file_list_control, (ref) file_loc_linear, R_IP
;                 AX, BX, DX, SI, DI, BP

ConvertCoords	proc	near
		push	ax
		push	bx
		push	dx
		push	si
		push	di
		push	bp

		mov	bp, sp

		mov	di, [bp+0eh]
		mov	si, [bp+10h]

		mov	bx, [bp+10h]
		mov	ax, [bx]
		cmp	ax, FIRST_WINDOW
		je	short CnvWin1

		add	si, LIST_2_PARMS
		jmp	short CnvWin2

CnvWin1:
		add	si, LIST_1_PARMS

CnvWin2:
		mov	dx, MAX_ROWS
		mov	ax, [si+04h]
		mul	dl
		add	ax, [si+02h]
		mov	[di], ax

		pop	bp
		pop	di
		pop	si
		pop	dx
		pop	bx
		pop	ax
		ret	4
ConvertCoords	endp


; COPIES A FILE ENTRY FROM THE LIST BLOCK TO A BUFFER IN THE DATA SEGMENT
;	- RETURNS : Nothing
;	- STACK : (ref) dest_buffer_offset,
;                 (val) list_block_segment, (ref) file_entry_offset, R_IP
;                 AX, BP

RetrieveEntry	proc	near
		push	ax
		push	bp

		mov	bp, sp

		xor	ax, ax
		push	ax
		push	ds
		mov	ax, [bp+0ah]
		push	ax
		mov	ax, ENTRY_LENGTH
		push	ax
		call	InitBuffer

		push	ds
		mov	ax, [bp+0ah]
		push	ax
		mov	ax, [bp+08h]
		push	ax
		mov	ax, [bp+06h]
		push	ax
		mov	ax, ENTRY_LENGTH
		push	ax
		call	CopyString

		pop	bp
		pop	ax
		ret	6
RetrieveEntry	endp


; SEARCHES A LIST BLOCK FOR THE NEXT ENTRY MARKED AS "SELECTED"
;	- RETURNS : new_offset = offset of first file in block marked "SEL";
;                                FFFFh if none marked as such
;	- STACK : (val) search_scope, (val) file_count,
;                 (ref) new_offset, (val) list_block, R_IP
;                 AX, BX, CX, DX, DI, ES, BP

SearchListBlock	proc	near
		push	ax
		push	bx
		push	cx
		push	dx
		push	di
		push	es
		push	bp

		mov	bp, sp

		mov	ax, [bp+10h]
		mov	es, ax
		xor	di, di
		mov	cx, [bp+14h]

		mov	ax, [bp+16h]
		cmp	ax, SEARCH_SELECTED
		jne	short CheckCurrSrch

		xor	dh, dh
		mov	dl, ENTRY_LENGTH-03h
		jmp	short DoSearch

CheckCurrSrch:
		cmp	ax, SEARCH_CURRENT
		jne	short NoMoreEntries

		xor	dh, dh
		mov	dl, ENTRY_LENGTH-03h

DoSearch:
		jcxz	short NoMoreEntries

		cmp	ax, SEARCH_SELECTED
		jne	short SearchCurr

		push	di
		add	di, dx

		push	ax
		mov	al, es:[di]
		and	al, NOT_CURRENT
		cmp	al, SELECTED
		pop	ax

		pop	di

		je	short EndSearch
		jmp	short SearchResume

SearchCurr:
		push	di
		push	ax
		add	di, dx
		mov	al, es:[di]
		and	al, CURRENT
		cmp	al, CURRENT
		pop	ax
		pop	di

		je	short EndSearch

SearchResume:
		add	di, ENTRY_LENGTH
		dec	cx
		jmp	short DoSearch

NoMoreEntries:
		mov	di, END_OF_FLIST

EndSearch:
		mov	bx, [bp+12h]
		mov	[bx], di

		pop	bp
		pop	es
		pop	di
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	8
SearchListBlock	endp


; SETS THE FILE DISPLAY OFFSET (IN DATA BUFFER) FOR ACTIVE LIST
;	- RETURNS : list_parm_block for active list = parms
;	- STACK : (ref) active_list_parm_block, (ref) list_parm_block,
;                 (val) active_window, R_IP
;                 AX, BX, CX, DX, BP

SetActStart	proc	near
		push	ax
		push	bx
		push	cx
		push	dx
		push	bp

		mov	bp, sp

		mov	bx, [bp+10h]
		mov	cx, [bx]
		mov	dx, [bx+02h]

		mov	bx, [bp+0eh]
		mov	ax, [bp+0ch]
		cmp	ax, FIRST_WINDOW
		je	short Wind1IsActive

		mov	[bx+04h], cx
		mov	[bx+06h], dx
		jmp	short DoneSetActive

Wind1IsActive:
		mov	[bx], cx
		mov	[bx+02h], dx

DoneSetActive:
		pop	bp
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	6
SetActStart	endp


; DETERMINES THE FILE DISPLAY OFFSET (IN DATA BUFFER) FOR ACTIVE LIST
;	- RETURNS : active_list_parm_block = parms for active list
;	- STACK : (ref) active_list_parm_block, (ref) list_parm_block,
;                 (val) active_window, R_IP
;                 AX, BX, CX, DX, BP

GetActStart	proc	near
		push	ax
		push	bx
		push	cx
		push	dx
		push	bp

		mov	bp, sp

		mov	bx, [bp+0eh]
		mov	ax, [bp+0ch]
		cmp	ax, FIRST_WINDOW
		je	short Win1IsActive

		mov	cx, [bx+04h]
		mov	dx, [bx+06h]
		jmp	short DoneGetActive

Win1IsActive:
		mov	cx, [bx]
		mov	dx, [bx+02h]

DoneGetActive:
		mov	bx, [bp+10h]
		mov	[bx], cx
		mov	[bx+02h], dx

		pop	bp
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	6
GetActStart	endp


; DETERMINES OFFSET OF A FILE'S ENTRY WITHIN THE FILE LIST BLOCK
;	- RETURNS : entry_ofs = offset of file entry
;	- STACK : (val) position_in_list, (ref) entry_ofs, R_IP
;                 AX, BX, CX, BP

FindFileEntry	proc	near
		push	ax
		push	bx
		push	cx
		push	bp

		mov	bp, sp

		mov	ax, ENTRY_LENGTH
		mov	bx, [bp+0ch]
		mul	bl

		mov	bx, [bp+0ah]
		mov	[bx], ax

		pop	bp
		pop	cx
		pop	bx
		pop	ax
		ret	4
FindFileEntry	endp


; COPIES THE NEEDED FIELDS FROM THE DTA TO A FILE-LIST BUFFER
;	- RETURNS : Nothing
;	- STACK : (val) DTA_seg, (ref) DTA_ofs,
;                 (val) buffer_seg, (ref) buffer_ofs, R_IP
;                 AX, BP

CopyDTAEntry	proc	near
		push	ax
		push	bp

		mov	bp, sp

		mov	ax, [bp+08h]
		push	ax
		mov	ax, [bp+06h]
		push	ax
		mov	ax, [bp+0ch]
		push	ax
		mov	ax, [bp+0ah]
		add	ax, DTA_DATA_OFS
		push	ax
		mov	ax, DTA_DATA_LEN
		push	ax
		call	CopyString

		pop	bp
		pop	ax
		ret	8
CopyDTAEntry	endp


; DISPLAYS A FILE LIST ONSCREEN
;	- RETURNS : Nothing
;	- STACK : (ref) directory_tag, (val) which_window,
;                 (val) start_at_entry, (val) start_at_offset,
;                 (val) file_count, (val) list_block,
;                 (ref) list_window_parms, (val) video_segment, R_IP
;                 AX, BX, CX, DX, DS, ES, SI, DI, BP

DisplayFileList	proc	near
		push	ax		; Save registers
		push	bx
		push	cx
		push	dx
		push	ds
		push	es
		push	si
		push	di
		push	bp

		mov	bp, sp

		cld			; All strings left-to-right
		mov	bx, [bp+16h]	; File List Window-parameters

		mov	ax, [bx]
		inc	ax
		push	ax
		mov	ax, [bx+02h]
		inc	ax
		mov	dx, [bp+20h]		; Load active window
		cmp	dx, FIRST_WINDOW	; Is it window 1 ?
		je	short InitWin1		; Yes
		add	ax, [bx+06h]		; No -- add file list height
						;    to get to second window

InitWin1:
		push	ax
		call	SetLocation
		add	di, 02h			; Point DI to window we are
						;    updating

		mov	ax, [bp+14h]
		mov	es, ax			; Load the video segment

		mov	si, [bp+1ch]		; Load offset in list-block to
						;    start at
		mov	ax, [bp+18h]
		mov	ds, ax			; List-block segment becomes
						;    our DATA SEGMENT

		mov	cx, [bp+1ah]		; Total # of files in list
		sub	cx, [bp+1eh]		; Less previous files in block
						;    that aren't displayed
		add	cx, MAX_ROWS		; Add an extra 9 so that any
						;    space after the last file
						;    gets blanked out
		xor	ax, ax			; Clear row and column counter

NextFile:
		jcxz	short NoFilesLeft	; Jump if no more to show
		jmp	short DisplayMoreFiles	; Else, show the next file

NoFilesLeft:
		jmp	DoneDispFiles

DisplayMoreFiles:
		cmp	ah, MAX_COLS		; Have we filled all columns ?
		jae	short NoFilesLeft	; Yes -- finish routine

		push	cx			; Save # of files to do

		push	di
		push	si

		inc	al			; Add 1 to row counter
		add	si, FILENAME_OFFS	; Position at filename
		mov	cx, FILENAME_LEN	; Write max. 13 chars

		push	bx
		xor	bx, bx			; BX counts # of chars in name

		push	ax			; Save row / column counters
		push	dx			; Save active window

		push	bx			; Save char-counter
		mov	bx, [bp+16h]		; Load base of file-list parms
		mov	dx, cs:[bx+12h]		; Load attr for sel'd file
		xchg	dh, dl			; Make it MSB
		mov	ax, cs:[bx+08h]		; Load file list attr
		xchg	ah, al			; Attr has to be MSB
		pop	bx			; Restore char-counter

		push	ax			; Save attr for file list
		mov	al, [si+FILENAME_LEN+01h]	; Load SELECTION byte
		and	al, NOT_CURRENT		; Mask out the CURRENT bit
		cmp	al, UNSELECTED		; Is file marked as unsel'd ?
		pop	ax			; Restore attr for file list

		je	short FileDispName	; Yes -- normal (low) colours
		mov	ax, dx			; No -- use high colours

FileDispName:
		jcxz	short DoneNameDisp	; If 13 chars written, bug out
		cmp	byte ptr [si], END_OF_RECORD	; Reached a NULL ?
		je	short DoneNameDisp	; Yes -- end of filename

		lodsb			; Read a char
		stosw			; And write to screen with attribute
		dec	cx		; 1 less char to write
		inc	bx		; Counts # of chars in the filename
		jmp	FileDispName	; Write the next char

DoneNameDisp:
		pop	dx		; Restore active window
		pop	ax		; Restore row/column counters

		push	ax
		push	bx		; Save # of chars in the
					;    last filename displayed

		push	si
		push	bp
		mov	bp, sp
		mov	bx, [bp+0ah]		; Get offset of current
						;    entry in the list block
		pop	bp
		pop	si

		xor	dl, dl
		mov	al, ds:[bx]		; Load attr of file entry
		and	al, ATTR_DIR		; Mask out directory-bit
		cmp	al, ATTR_DIR		; Is this entry a dir ?

		pop	bx		; Restore # chars in last f/name disp
		je	short DispADir	; Yes

		mov	dl, 05h		; No -- instead of the '[DIR]' tag,
					;    write 5 blank spaces instead

DispADir:
		push	ax

		mov	bh, FILENAME_LEN	; Determine # of spaces
		sub	bh, bl			;    necessary to make 12 chars

		push	cx
		push	ax

		push	si			; Save source pointer
		mov	si, [bp+0ah]		; Load offset of curr entry
		add	si, ENTRY_LENGTH-03h	; Point to SELECTION byte
		mov	al, ds:[si]		; Load SELECTION byte
		and	al, NOT_CURRENT		; Mask out CURRENT status-bit
		cmp	al, UNSELECTED		; Is this file unselected ?
		pop	si			; Restore source pointer
		jne	short Seled		; No -- it is sel'd

		push	bx			; Save whatever's there
		mov	bx, [bp+16h]		; Point to window parm block
		mov	ax, cs:[bx+08h]		; Load attr (low colour)
		pop	bx			; Restore back to original
		jmp	short UnSeled

Seled:
		push	bx			; Save whatever's there
		mov	bx, [bp+16h]		; Point to window parm block
		mov	ax, cs:[bx+08h]		; Load attr (high colour)
		pop	bx			; Restore original value

UnSeled:
		xchg	ah, al			; Attr has to be MSB
		mov	al, SPACE		; Blank out the gap

		mov	cl, bh		; Calculate # of spaces to 13 chars
		add	cl, dl		;    and add extra spaces
		inc	cl		;
		rep	stosw		; Blank the gap between the filename
					;    and the '[DIR]' tag

		pop	ax
		pop	cx

		or	dl, dl			; Is this entry for a dir ?
		jnz	short ResumeDisp	; No

		push	ax			; Yes -- write the '[DIR]'
		push	bx			;    tag next to the filename
		push	cx

		xchg	cx, bx

		push	bx			; Save original value
		mov	bx, [bp+16h]		; Offset of file-list parms
		mov	cx, cs:[bx+08h]		; Load normal video attr
		mov	ax, cs:[bx+12h]		; Load attr for sel'd files
		pop	bx			; Restore original value

		push	ax
		mov	al, byte ptr ds:[bx+si+01h]	; Load SELECTION byte
		and	al, NOT_CURRENT		; Mask out CURRENT status-bit
		cmp	al, UNSELECTED		; Is this file unselected ?
		pop	ax
		je	short DispFilUnSel	; Yes -- low video colours

		mov	cx, ax			; No -- set up with sel'd hue

DispFilUnSel:
		push	ds			; Save llist block segment

		push	cs			; Make DATA SEGMENT the same
		pop	ds			;    as our CODE SEGMENT

		mov	ax, [bp+22h]		; Offset of '[DIR]' tag-string
		push	ax
		mov	ax, cx			; Attribute
		push	ax
		mov	ax, [bp+14h]		; Video segment
		push	ax
		call	WriteString

		pop	ds			; Restore list block segment

		pop	cx
		pop	bx
		pop	ax

ResumeDisp:
		pop	ax

NotADir:
		pop	ax
		pop	bx

		pop	si
		pop	di

		add	di, 00a0h		; Move to next screen row
		add	si, ENTRY_LENGTH	; Position source at nxt entry

		mov	dx, cs:[bx+06h]		; Load height of list window
		sub	dl, 02h			; Less 2 for the upper-/lower-
						;    borders
		cmp	al, dl			; Have we done all the rows ?
		jb	short StillMoreRows	; No -- do next row

		push	ds		; Save list block segment

		push	cs		; Set DATA SEGMENT to our
		pop	ds		;    CODE SEGMENT

		push	cx		; Save # of files still to display

		inc	ah		; # of rows
		mov	cx, [bx]	; X-coordinate of file list
		inc	cx		; Add 1 'cause of border
		push	cx		; CX = X location to write at
		mov	cx, [bx+02h]	; Y-coordinate of file list
		inc	cx		; Add 1 'cause of the border

		push	dx			; Save height of list win
		mov	dx, [bp+20h]		; # of win we're busy with
		cmp	dx, FIRST_WINDOW	; Is it the first one ?
		je	short NextColWin1	; Yes
		add	cx, [bx+06h]		; No -- add height of the file
						;    list window

NextColWin1:
		pop	dx			; Restore height of list win

		push	cx		; CX = Y location to write at
		call	SetLocation	; Reposition DI

		mov	dx, ax		; Save the row and column counter
		mov	ax, [bx+0ah]	; Load width of file-list-pointer
		add	ax, [bx+0eh]	; Add space between adjacent entries
		mul	dh		; DH (AH) is the column counter
		shl	ax, 1		; Adjust for VIDEO ATTRIBUTES
		add	ax, 02h		; Extra gap bewteen adjacent entries
		add	di, ax		; Reposition output pointer
		mov	ax, dx		; Restore row and column counter
		xor	al, al		; Clear the row counter

		pop	cx		; Get back # of files still to display
		pop	ds		; Get back list block segment

StillMoreRows:
		pop	cx		; Get back # of files still to do
		dec	cx		; 1 less, though
		jmp	NextFile	; Do the file

DoneDispFiles:
		pop	bp		; Restore registers
		pop	di
		pop	si
		pop	es
		pop	ds
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	16
DisplayFileList	endp


; DISPLAYS THE FUNCTION KEY TEMPLATE ON THE LAST SCREEN ROW
;	- RETURNS : Nothing
;	- STACK : (ref) template_string, (val) attr, (val) video_seg, R_IP
;                 AX, BP

DispTemplate	proc	near
		push	ax
		push	bp

		mov	bp, sp

		xor	ax, ax
		push	ax
		mov	ax, 18h
		push	ax
		call	SetLocation

		mov	ax, [bp+0ah]
		push	ax
		mov	ax, [bp+08h]
		push	ax
		mov	ax, [bp+06h]
		push	ax
		call	WriteString

		pop	bp
		pop	ax
		ret	6
DispTemplate	endp
