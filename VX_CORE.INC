; POSITIONS DI TO A (X, Y) LOCATION ONSCREEN
;	- RETURNS : DI = offset of (X, Y) in video buffer
;	- STACK : (val) x_value, (val) y_value, R_IP
;                 AX, BX, CX, DX, BP

SetLocation	proc	near
		push	ax			; Save registers
		push	bx
		push	cx
		push	dx
		push	bp

		mov	bp, sp			; Use BP to index stack

		xor	bx, bx			; Clear offset counter
		mov	cx, [bp+0ch]		; Load # rows
		cmp	cx, 00h			; Is it 0 ?
		je	short Skip_Y		; Yes -- only count columns

AddLines:
		add	bx, 00a0h		; Add 160 to ofs for every row
		loop	AddLines

Skip_Y:
		mov	dx, [bp+0eh]		; Load # columns
		shl	dx, 1			; Adjust for attributes
		add	bx, dx			; Add partial line to offset

		mov	di, bx			; Set up DI for return

		pop	bp			; Restore registers
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	4			; Take 2 parms off stack
SetLocation	endp


; DETERMINES THE LENGTH OF A NULL-DELIMITED STRING
;	- RETURN : AX = length of in_string
;	- STACK : (ref) in_string, R_IP
;                 CX, ES, DI, BP

StrLength	proc	near
		push	cx			; Save registers we'll use
		push	es
		push	di
		push	bp

		mov	bp, sp			; Use BP for stack-indexing

		mov	di, [bp+0ah]		; Load addr of string to check
		push	ds			; Scan needs data-segment of
		pop	es			;    string in ES
		mov	al, END_OF_RECORD	; Search for null-terminator
		cld				; Strings left to right
		mov	cx, 00a0h		; Only scan next 160 chars
		repnz	scasb			; Scan 'til cx=0/[si]=0

		mov	ax, 00a0h		; Max. length of scan
		sub	ax, cx			; Less # of chars we didn't do
						;    gives # of chars scanned

		pop	bp			; Restore used registers
		pop	di
		pop	es
		pop	cx
		ret	2			; Take 1 parm off the stack
StrLength	endp


; DETERMINES OFFSET OF NEXT ENTRY IN A NULL-DELIMITED TABLE
;	- RETURNS : AX = offset of next entry
;	- STACK : (ref) curr_position, R_IP
;                 CX, ES, DI, BP

FindNextEntry	proc	near
		push	cx			; Save registers to use
		push	es
		push	di
		push	bp

		mov	bp, sp			; Access parms through BP

		mov	di, [bp+0ah]		; Table offs to start search
		mov	al, END_OF_RECORD	; Look for end of this record
		mov	cx, 0050h		; Scan only the next 80 chars
		cld				; Scan left to right
		push	ds			; SCAN uses ES -- so set it
		pop	es			;    to our data segment
		repne	scasb			; Do the deed
		mov	ax, di			; Get new offset for return

		pop	bp			; Restore registers
		pop	di
		pop	es
		pop	cx
		ret	2			; Take 1 parm off the stack
FindNextEntry	endp


; SETS ATTRIBUTE OF A ROW OF CHARACTERS
;	- RETURNS : Nothing
;	- STACK : (val) x_pos, (val) y_pos, (val) bar_width,
;                 (val) attr_to_use, (val) video_seg, R_IP
;                 AX, BX, CX, ES, DI, BP

AttributeLine	proc	near
		push	ax			; Save registers we'll use
		push	bx
		push	cx
		push	es
		push	di
		push	bp

		mov	bp, sp			; Index stack with BP

		mov	ax, [bp+16h]		; Get x-coord off stack
		push	ax			; Pass value
		mov	ax, [bp+14h]		; Get y-coord off stack
		push	ax			; Value pass
		call	SetLocation		; Position DI at correct offs

		mov	ax, [bp+0eh]		; Load video seg from stack
		mov	es, ax			; Point to it
		mov	cx, [bp+12h]		; Load # of columns to attr
		mov	ax, [bp+10h]		; Get colour to use

AttrLoop:
		or	cx, cx			; Have we done all the cols ?
		je	short DoneAttr		; Yes -- break out of loop

		inc	di			; Move to next column
		stosb				; Set the colour

		dec	cx			; 1 less column to do
		jmp	AttrLoop		; Loop back

DoneAttr:
		pop	bp			; Restore used registers
		pop	di
		pop	es
		pop	cx
		pop	bx
		pop	ax
		ret	10			; Take 5 parms off the stack
AttributeLine	endp


; DETERMINES THE SEGMENT ADDRESS OF THE VIDEO BUFFER
;	- RETURNS : video_seg = segment addr of video buffer
;	- STACK : (ref) video_seg, R_IP
;                 AX, BX, ES, BP

GetVideoSetup	proc	near
		push	ax			; Save registers we'll use
		push	bx
		push	es
		push	bp

		mov	bp, sp			; Use BP to index stack

		mov	ax, BiosData		; Point to system settings seg
		mov	es, ax			;    with ES
		lea	bx, InitialVideo	; Load addr of startup-mode var

		mov	al, es:[bx]		; Get startup video mode

		mov	bx, [bp+0ah]		; Get addr of return-var
		cmp	al, 07h			; Is MDA / Herc present ?
		je	short MonoSetup		; Yes

		mov	word ptr [bx], 0b800h	; Save colour video seg addr
		jmp	short FinishedVideo	; Leave routine

MonoSetup:
		mov	word ptr [bx], 0b000h	; Save MONO video seg addr

FinishedVideo:
		pop	bp			; Restore registers
		pop	es
		pop	bx
		pop	ax
		ret	2			; Take 1 parm off stack
GetVideoSetup	endp


; CHECKS IF A CHARACTER IS AVAILABLE IN KEYB BUFFER
;	- RETURNS : keyb_status = 00h if no char
;                                 FFh if char is avail
;	- STACK : (ref) keyb_status, R_IP
;                 AX, BX, BP

KeyPressed	proc	near
		push	ax			; Save registers
		push	bx
		push	bp

		mov	bp, sp			; BP indexes stack

		mov	bx, [bp+08h]		; Get addr of return-var

		mov	ah, 01h			; Request check_keyb_status
		int	16h			; Call DOS

		mov	ax, NO_KEY
		jz	short IsNoKey

		mov	ax, KEY_WAITING

IsNoKey:
		mov	word ptr [bx], ax	; Save word in return-var

		pop	bp			; Restore registers
		pop	bx
		pop	ax
		ret	2			; Take 1 parm off stack
KeyPressed	endp


; GETS A KEY FROM THE KEYBOARD
;	- RETURNS : keystroke_var = scan-code/ascii code of input key
;	- STACK : (ref) keystroke_var, R_IP
;                 AX, BX, BP

GetKey		proc	near
		push	ax			; Save registers we'll use
		push	bx
		push	bp

		mov	bp, sp			; Index stack with BP

		mov	bx, word ptr [bp+08h]	; Get addr to store keystroke

		mov	ah, 00h			; Request read_keyboard
		int	16h			; Call BIOS

		mov	word ptr [bx], ax	; Save keystroke

		pop	bp			; Restore registers
		pop	bx
		pop	ax
		ret	2			; Take 1 parm off stack
GetKey		endp


; WRITES A COUNT OF A CERTAIN CHARACTER TO THE SCREEN
;	- RETURNS : Nothing
;	- STACK : (val) char_attr_to_write, (val) count, (val) video_seg, R_IP
;                 AX, CX, ES, DI, BP

WriteChar	proc	near
		push	ax			; Save registers to use
		push	cx
		push	es
		push	di
		push	bp

		mov	bp, sp			; Index stack with BP

		mov	ax, [bp+0ch]		; Load segment of video
		mov	es, ax			;    buffer and init'ise ES
		mov	cx, [bp+0eh]		; # of times to write char
		mov	ax, [bp+10h]		; char/attr to write
		cld				; Strings left to right
		repnz	stosw			; Do It !

		pop	bp			; Restore registers we used
		pop	di
		pop	es
		pop	cx
		pop	ax
		ret	6			; Take 3 parms off stack
WriteChar	endp


; WRITES A NULL-DELIMITED STRING TO THE SCREEN
;	- RETURNS : Nothing
;	- STACK : (ref) string_to_write, (val) text_attr, (val) video_seg, R_IP
;                 AX, BX, CX, ES, SI, BP

WriteString	proc	near
		push	ax			; Save registers
		push	bx
		push	cx
		push	es
		push	si
		push	bp

		mov	bp, sp			; Access parms via BP

		mov	si, [bp+12h]		; Get addr of str to write
		push	si			; Pass reference
		call	StrLength		; Get length of string
		mov	cx, ax			; Write that many chars
		dec	cx			; Don't write null-terminator

		mov	si, [bp+12h]		; Get addr of string again
		mov	ax, [bp+0eh]		; Load video seg to use
		mov	es, ax			;    and init'ise pointer
		mov	ah, [bp+10h]		; Get attr to use with str

WriteAgain:
		or	cx, cx			; Any more chars to write ?
		jz	short WriteDone		; No -- leave routine

		lodsb				; Get 1 char from string
		stosw				; Write char to video memory
		dec	cx			; 1 less char to write
		jmp	WriteAgain		; Loop back for more

WriteDone:
		pop	bp			; Restore registers
		pop	si
		pop	es
		pop	cx
		pop	bx
		pop	ax
		ret	6			; Take 3 parms off the stack
WriteString	endp


; WRITES A STRING TO THE SCREEN CENTRED BETWEEN TWO COLUMNS
;	- RETURNS : Nothing
;	- STACK : (val) x_value, (val) columns, (val) row
;                 (ref) string_to_write, (val) attr, (val) video_seg, R_IP
;                 AX, DX, BP

WriteCentred	proc	near
		push	ax
		push	dx
		push	bp

		mov	bp, sp

		mov	ax, [bp+0ch]
		push	ax
		call	StrLength
		mov	dx, ax
		and	dx, 0feh

		mov	ax, [bp+12h]
		push	ax
		mov	ax, [bp+0eh]
		push	ax
		call	SetLocation

		mov	ax, [bp+10h]
		sub	ax, dx
		add	di, ax

		mov	ax, [bp+0ch]
		push	ax
		mov	ax, [bp+0ah]
		push	ax
		mov	ax, [bp+08h]
		push	ax
		call	WriteString

		pop	bp
		pop	dx
		pop	ax
		ret	12
WriteCentred	endp


; MAKES A DUPLICATE OF AN INPUT STRING AT ANOTHER LOCATION
;	- RETURNS : Nothing
;	- STACK : (val) new_seg, (ref) new_ofs,
;                 (val) old_seg, (ref) old_ofs, (val) count, R_IP
;                 AX, CX, DS, ES, SI, DI, BP
; NB ! If count = 0 then old string is copied until null-terminator

CopyString	proc	near
		push	ax
		push	cx
		push	ds
		push	es
		push	si
		push	di
		push	bp

		mov	bp, sp

		cld
		mov	ax, [bp+18h]
		mov	es, ax
		mov	di, [bp+16h]

		mov	si, [bp+12h]
		mov	cx, [bp+10h]
		or	cx, cx
		jz	short NullCopyStr
		mov	ax, [bp+14h]
		mov	ds, ax
		rep	movsb
		jmp	short DoneCopyStr

NullCopyStr:
		cmp	byte ptr ds:[si], END_OF_RECORD
		je	short DoneNullCopyStr

		lodsb
		stosb
		jmp	short NullCopyStr

DoneNullCopyStr:
		mov	al, 00h
		stosb

DoneCopyStr:
		pop	bp
		pop	di
		pop	si
		pop	es
		pop	ds
		pop	cx
		pop	ax
		ret	10
CopyString	endp


; INITIALISES A BUFFER TO A STRING OF NULL CHARACTERS
;	- RETURNS : Nothing
;	- STACK : (val) char_to_use, (val) block_seg,
;                 (ref) block_ofs, (val) block_length_in_bytes, R_IP
;                 AX, CX, DI, ES, BP

InitBuffer	proc	near
		push	ax
		push	cx
		push	di
		push	es
		push	bp

		mov	bp, sp

		mov	ax, [bp+10h]
		mov	es, ax
		mov	di, [bp+0eh]
		mov	cx, [bp+0ch]
		mov	ax, [bp+12h]
		rep	stosb

		pop	bp
		pop	es
		pop	di
		pop	cx
		pop	ax
		ret	8
InitBuffer	endp


; SCANS A STRING FOR AN OCCURRANCE OF A SPECIFIC CHARACTER
;	- RETURNS : AX = CHAR_NOT_FOUND if the character was not found, else
;                        offset of char in string
;	- STACK : (val) LO_char_to_scan_for, (val) string_buffer_seg,
;                 (val) string_length, (ref) string_to_scan, R_IP
;                 CX, DI, ES, BP

FindChar	proc	near
		push	cx
		push	di
		push	es
		push	bp

		mov	bp, sp

		mov	ax, [bp+0eh]
		mov	es, ax
		mov	cx, [bp+0ch]
		mov	di, [bp+0ah]
		mov	ax, [bp+10h]
		cld
		repne	scasb

		dec	di
		mov	ax, di
		cmp	cx, END_OF_RECORD
		jne	short DoneFindChar

		mov	ax, CHAR_NOT_FOUND

DoneFindChar:
		pop	bp
		pop	es
		pop	di
		pop	cx
		ret	8
FindChar	endp


; SETS THE CURSOR TO THE SPECIFIED LOCATION
;	- RETURNS : Nothing
;	- STACK : (val) column, (val) row, R_IP
;                 AX, BX, DX, SI, DI, BP

SetCursor	proc	near
		push	ax
		push	bx
		push	dx
		push	si
		push	di
		push	bp

		mov	bp, sp

		mov	ax, [bp+0eh]
		mov	dh, al
		mov	ax, [bp+10h]
		mov	dl, al
		xor	bx, bx
		mov	ah, 02h
		int	10h

		pop	bp
		pop	di
		pop	si
		pop	dx
		pop	bx
		pop	ax
		ret	4
SetCursor	endp


; GETS THE SEGMENT:OFFSET ADDRESS OF AN INTERRUPT ROUTINE
;	- RETURNS : Nothing
;	- STACK : (val) int_number,
;                 (ref) save_segment, (ref) save_offset, R_IP
;                 AX, BX, SI, ES, BP

GetVector	proc	near
		push	ax
		push	bx
		push	si
		push	es
		push	bp

		mov	bp, sp

		mov	ax, [bp+10h]
		mov	ah, 35h
		int	21h

		mov	si, [bp+0eh]
		mov	[si], es
		mov	si, [bp+0ch]
		mov	[si], bx

		pop	bp
		pop	es
		pop	si
		pop	bx
		pop	ax
		ret	6
GetVector	endp


; SETS THE SEGMENT:OFFSET ADDRESS OF AN INTERRUPT ROUTINE
;	- RETURNS : Nothing
;	- STACK : (val) int_number,
;                 (val) new_segment, (val) new_offset, R_IP
;                 AX, DX, SI, DS, BP

SetVector	proc	near
		push	ax
		push	dx
		push	si
		push	ds
		push	bp

		mov	bp, sp

		mov	dx, [bp+0ch]
		mov	si, [bp+0eh]
		mov	ds, si
		mov	ax, [bp+10h]
		mov	ah, 25h
		int	21h

		pop	bp
		pop	ds
		pop	si
		pop	dx
		pop	ax
		ret	6
SetVector	endp


; FLUSHES THE KEYBOARD BUFFER
;	- RETURNS : Nothing
;	- STACK : R_IP
;                 AX, DX

FlushKeyb	proc	near
		push	ax
		push	dx

		mov	dl, 0ffh
		mov	al, 06h
		mov	ah, 0ch
		int	21h

		pop	dx
		pop	ax
		ret
FlushKeyb	endp


; GETS A LINE OF INPUT FROM THE USER
;	- RETURNS : Nothing
;	- STACK : (val) video_segment,
;                 (ref) buffer, (ref) parameters,
;                 (ref) key_status, (ref) key_stroke, R_IP
;                 AX, BX, CX, DX, SI, DI, BP

LineEdit	proc	near
		push	ax
		push	bx
		push	cx
		push	dx
		push	si
		push	di
		push	bp

		mov	bp, sp

		mov	si, [bp+14h]

NeedInput:
		push	di
		push	si

		mov	ax, [si]
		push	ax
		mov	ax, [si+02h]
		push	ax
		call	SetLocation	; Locate at beginning of DOS window

		mov	ax, [bp+16h]
		add	ax, [si+14h]
		push	ax
		mov	ax, [si+08h]
		push	ax
		mov	ax, [bp+18h]
		push	ax
		call	WriteString	; Write the input string to screen

		mov	ax, [si+14h]
		cmp	ax, NULL
		ja	short NoBlanks

		mov	cx, [si+04h]
		sub	cx, [si+10h]

		mov	ah, [si+08h]
		mov	al, SPACE
		push	ax
		push	cx
		mov	ax, [bp+18h]
		push	ax
		call	WriteChar	; Clear to end-of-line

NoBlanks:
		mov	ax, [si+0ah]
		push	ax
		mov	ax, [si+02h]
		push	ax
		call	SetCursor	; Position cursor after input-data

		pop	si
		pop	di

		mov	ax, [bp+12h]
		push	ax
		call	KeyPressed

		mov	bx, [bp+12h]
		mov	ax, [bx]
		cmp	ax, KEY_WAITING
		jne	short NeedInput

		mov	ax, [bp+10h]
		push	ax
		call	GetKey

		mov	bx, [bp+10h]
		mov	ax, [bx]

LookASCII:
		cmp	al, 'z'			; AL <= 'z' ?
		jbe	short CheckLowerCase	; No -- take as input

		jmp	FoundExitKey

CheckLowerCase:
		cmp	al, 'a'			; AL < 'a' ?
		jb	short AlreadyUpper	; Yes -- already in uppercase

		and	al, UPPERCASE_MASK	; No -- convert to uppercase

AlreadyUpper:
		cmp	al, BACKSPACE_KEY
		je	short BkSpcGoBack

		cmp	al, SPACE		; AL >= SPACE ?
		jae	short DoLineEdit	; Yes -- accept char

		jmp	FoundExitKey

DoLineEdit:
		mov	dl, al

		mov	ax, [si+10h]
		cmp	ax, [si+0ch]
		jae	short AtMaxLength

		push	di

		mov	di, [bp+16h]
		mov	bx, [si+06h]

		mov	byte ptr [di][bx], dl

		inc	word ptr [si+06h]
		inc	word ptr [si+0ah]
		inc	word ptr [si+10h]

		mov	ax, [si+04h]
		cmp	ax, [si+10h]
		jae	short WindowNotFull

		inc	word ptr [si+14h]
		dec	word ptr [si+0ah]

WindowNotFull:
		pop	di

		jmp	NeedInput

AtMaxLength:
		cmp	dl, BACKSPACE_KEY
		je	short BkSpcGoBack
		jmp	NeedInput

BkSpcGoBack:
		mov	ax, [si+10h]
		cmp	ax, NULL + 01h
		ja	short DoBackSpc
		jmp	NeedInput

DoBackSpc:
		push	ds
		mov	ax, [bp+16h]
		add	ax, [si+06h]
		dec	ax
		push	ax
		push	ds
		mov	ax, [bp+16h]
		add	ax, [si+06h]
		push	ax
		xor	ax, ax
		push	ax
		call	CopyString

		mov	ax, [si+14h]
		cmp	ax, NULL
		je	short WindowNotScrolled

		dec	word ptr [si+06h]	; Current_position in string
		dec	word ptr [si+10h]	; Current_length of string
		dec	word ptr [si+14h]	; Starting_position

		jmp	NeedInput

WindowNotScrolled:
		dec	word ptr [si+06h]	; Current_position
		dec	word ptr [si+10h]	; Current_length of string
		dec	word ptr [si+0ah]	; Cursor_position

		jmp	NeedInput

FoundExitKey:
		mov	[si+0eh], ax

DoneLineEdit:
		pop	bp
		pop	di
		pop	si
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	10
LineEdit	endp
