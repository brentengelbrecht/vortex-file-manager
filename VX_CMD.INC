; BUILDS UP A COMMAND-LINE FOR PROCESSING BY DOS
;	- RETURNS : Nothing
;	- STACK : (ref) update_flags, (val) video_segment,
;                 (ref) command_line_buffer, (ref) command_line_parms,
;                 (ref) key_pressed_flag, (ref) key_stroke, R_IP
;                 AX, BX, CX, DX, SI, BP

GetDOSCommand	proc	near
		push	ax		; Save registers
		push	bx
		push	cx
		push	dx
		push	si
		push	bp

		mov	bp, sp

		mov	bx, [bp+12h]

		xor	ax, ax
		push	ax
		mov	ax, DOS_ROW
		push	ax
		mov	ax, SCREEN_WIDTH
		push	ax
		mov	ax, [bx+08h]
		push	ax
		mov	ax, [bp+16h]
		push	ax
		call	AttributeLine	; Colour the dos_row as 'active'

GetCmdAgain:
		mov	ax, [bp+16h]
		push	ax
		mov	ax, [bp+14h]
		push	ax
		mov	ax, [bp+12h]
		push	ax
		mov	ax, [bp+10h]
		push	ax
		mov	ax, [bp+0eh]
		push	ax
		call	LineEdit		; Invoke the line editor

		mov	bx, [bp+12h]
		mov	ax, [bx+0eh]
		cmp	ah, UP_ARROW		; Was up-arrow used to exit ?
		jne	short GetCmdNotUpArr	; No

		mov	bx, [bp+12h]

		mov	ax, [bx+1eh]
		cmp	ax, NULL		; Are there 0 entries in buf ?
		je	short GetCmdAgain	; Yes -- nothing to scroll back
		mov	ax, [bx+1ch]
		cmp	ax, NULL		; Getback-ptr at beg of buf ?
		je	short GetCmdAgain	; Yes -- nothing more to return

		push	es
		mov	si, [bx+1ch]
		sub	si, CMD_LINE_SIZE
		mov	ax, [bx+18h]
		mov	es, ax
		mov	al, es:[si]
		cmp	al, ASCII_255
		pop	es
		je	short GetCmdAgain	; If buffer not filled, don't
						;    scroll back more entries
						;    than there are in the buf

		mov	ax, [bx+1ch]
		sub	ax, CMD_LINE_SIZE	; Scroll back 1 more line
		mov	[bx+1ch], ax		; Save the new getback-ptr

		push	ds
		mov	ax, [bp+14h]
		push	ax
		mov	ax, [bx+18h]
		push	ax
		mov	ax, [bx+1ch]
		push	ax
		mov	ax, CMD_LINE_SIZE
		push	ax
		call	CopyString		; Copy the line (128-chars) at
						;    the getback-ptr location
						;    to the input-string buf

		mov	word ptr [bx+14h], NULL	; Always clear the excess-val
						;    StrLength will check if
						;      its still necessary

		mov	ax, [bp+14h]		; Find length of the new string
		push	ax
		call	StrLength

		cmp	ax, [bx+04h]		; Is str > length of window ?
		jb	short UpArrWidthOK	; No

		push	ax
		sub	ax, [bx+04h]		; Yes -- by how much ?
		mov	[bx+14h], ax		; Store the excess value
		pop	ax

UpArrWidthOK:
		mov	[bx+10h], ax		; Save the length of the string

		dec	ax
		mov	[bx+06h], ax

		inc	ax			; Length of string + 1
		add	ax, [bx+12h]		; Add length of the prompt
		sub	ax, [bx+14h]		; Less the excess (is not disp)
		mov	[bx+0ah], ax		; Store the new cursor location

		jmp	GetCmdAgain

GetCmdNotUpArr:
		cmp	ah, DOWN_ARROW		; Was dn-arrow used to exit ?
		jne	short GetCmdNotDownArr	; No

		mov	ax, [bx+1ah]
		sub	ax, CMD_LINE_SIZE
		cmp	ax, [bx+1ch]		; Pointing to last buf entry ?
		ja	short GetCmdCallFwd	; No

		xor	ax, ax
		push	ax
		push	ds
		mov	ax, [bp+14h]
		push	ax
		mov	ax, CMD_LINE_SIZE
		push	ax
		call	InitBuffer		; Last entry in buffer is
						;    cleared-input_buffer

		mov	ax, [bx+1ah]
		mov	[bx+1ch], ax		; Reset the output-pointer

		jmp	GetCmdLastEntry

GetCmdCallFwd:
		mov	ax, [bx+1ch]
		add	ax, CMD_LINE_SIZE	; Scroll forward 1 more line
		mov	[bx+1ch], ax		; Save the new getback-ptr

		push	ds
		mov	ax, [bp+14h]
		push	ax
		mov	ax, [bx+18h]
		push	ax
		mov	ax, [bx+1ch]
		push	ax
		mov	ax, CMD_LINE_SIZE
		push	ax
		call	CopyString		; Copy the line (128-chars) at
						;    the getback-ptr location
						;    to the input-string buf

GetCmdLastEntry:
		mov	word ptr [bx+14h], NULL	; Clear the excess value

		mov	ax, [bp+14h]
		push	ax
		call	StrLength		; Find length of new input-str

		cmp	ax, [bx+04h]		; Str > width of window ?
		jb	short DnArrWidthOK	; No

		push	ax
		sub	ax, [bx+04h]		; Calculate # of excess chars
		mov	[bx+14h], ax		;    and store it
		pop	ax

DnArrWidthOK:
		mov	[bx+10h], ax		; Set length of new string

		dec	ax
		mov	[bx+06h], ax		; Set the current_position to
						;    the end of the string

		inc	ax
		add	ax, [bx+12h]
		sub	ax, [bx+14h]
		mov	[bx+0ah], ax		; Cursor is located at
						;    prompt_length + str_length
						;       - chars_not_displayed
		jmp	GetCmdAgain

GetCmdNotDownArr:
		cmp	al, ENTER_KEY		; Was ENTER used to exit ?
		jne	short GetCmdNotEnter	; No

		mov	bx, [bp+12h]

		mov	ax, [bx+18h]
		push	ax
		xor	ax, ax
		push	ax
		mov	ax, [bx+18h]
		push	ax
		mov	ax, CMD_LINE_SIZE
		push	ax
		mov	ax, [bx+1ah]
		sub	ax, CMD_LINE_SIZE
		push	ax
		call	CopyString		; Shift data in buffer UP
						;    by 1 cmd_line_size

		mov	ax, [bx+18h]
		push	ax
		mov	ax, [bx+1ah]
		sub	ax, CMD_LINE_SIZE
		push	ax
		push	ds
		mov	ax, [bp+14h]
		push	ax
		mov	ax, CMD_LINE_SIZE
		push	ax
		call	CopyString		; Copy data from input-buffer
						;    into last slot in data-buf

		mov	cx, [bx+20h]
		mov	ax, [bx+1eh]
		cmp	ax, cx
		jae	short CmdBufFull

		inc	word ptr [bx+1eh]

CmdBufFull:
		mov	bx, [bp+12h]
		mov	word ptr [bx+06h], NULL
		mov	ax, [bx+12h]
		inc	ax
		mov	[bx+0ah], ax
		mov	word ptr [bx+10h], NULL
		mov	word ptr [bx+14h], NULL

		mov	ax, [bx+1ah]
		mov	[bx+1ch], ax

		xor	ax, ax
		push	ax
		push	ds
		mov	ax, [bp+14h]
		push	ax
		mov	ax, CMD_LINE_SIZE
		push	ax
		call	InitBuffer		; Clear the input buffer

GetCmdNotEnter:
		cmp	al, ESC_KEY
		jne	short GetCmdNotEsc

GetCmdNotEsc:
		mov	bx, [bp+12h]

		xor	ax, ax
		push	ax
		mov	ax, DOS_ROW
		push	ax
		mov	ax, SCREEN_WIDTH
		push	ax
		mov	ax, [bx+16h]
		push	ax
		mov	ax, [bp+16h]
		push	ax
		call	AttributeLine	; Colour the dos_row as 'de-active'

		int	09h		; Reset keyboard flags
		call	FlushKeyb	; Clear the keyboard buffer

		mov	bx, [bp+18h]
		mov	al, [bx]
		and	al, NUPD_NEWPATH
		and	al, NUPD_NEWDRV
		or	al, UPD_RESET
		mov	byte ptr [bx], al
		mov	al, [bx+01h]
		and	al, NUPD_NEWPATH
		and	al, NUPD_NEWDRV
		or	al, UPD_RESET
		mov	byte ptr [bx+01h], al

		pop	bp		; Restore registers
		pop	si
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	12
GetDOSCommand	endp


; ALLOCS A BUFFER FOR THE COMMAND-LINE HISTORY FEATURE
;	- RETURNS : Nothing
;	- STACK : (ref) base_of_dos_parms_structure, R_IP
;                 AX, BX, CX, BP

AllocCmdBuffer	proc	near
		push	ax
		push	bx
		push	cx
		push	bp

		mov	bp, sp

		mov	bx, [bp+0ah]
		mov	ax, [bx+18h]
		cmp	ax, NOT_ALLOC_FLAG
		je	short AllocCmdBufOK

		push	ax
		call	FreeMem

AllocCmdBufOK:
		mov	ax, [bx+1ah]
		push	ax
		call	AllocMem

		cmp	ax, NOT_ALLOC_FLAG
		jne	short AllocCmdOK

		mov	ax, ERROR_MEM_ALLOC
		push	ax
		call	ErrorProc
		mov	ax, NOT_ALLOC_FLAG

AllocCmdOK:
		mov	word ptr [bx+18h], ax
		mov	ax, [bx+1ah]
		mov	word ptr [bx+1ch], ax
		mov	word ptr [bx+1eh], NULL

		mov	ax, [bx+1ah]
		mov	cl, 7
		shr	ax, cl
		mov	[bx+20h], ax

		mov	ax, ASCII_255
		push	ax
		mov	ax, [bx+18h]
		push	ax
		xor	ax, ax
		push	ax
		mov	ax, [bx+1ah]
		push	ax
		call	InitBuffer

		pop	bp
		pop	cx
		pop	bx
		pop	ax
		ret	2
AllocCmdBuffer	endp


; DEALLOCATES THE BUFFER USED FOR COMMAND-LINE HISTORY
;	- RETURNS : Nothing
;	- STACK : (ref) base_of_dos_parms_structure, R_IP
;                 AX, BX, BP

FreeCmdBuffer	proc	near
		push	ax
		push	bx
		push	bp

		mov	bp, sp

		mov	bx, [bp+08h]
		mov	ax, [bx+18h]
		cmp	ax, NOT_ALLOC_FLAG
		je	short DoneFreeCmd

		push	ax
		call	FreeMem

DoneFreeCmd:
		pop	bp
		pop	bx
		pop	ax
		ret	2
FreeCmdBuffer	endp
