; DISPLAYS A WINDOW ONSCREEN
;	- RETURNS : Nothing
;	- STACK : (val) x1, (val) y1, (val) columns, (val) rows,
;                 (val) attr_to_use, (val) video_segment, R_IP
;                 AX, CX, DX, BX, SP, BP, SI, DI, ES

DrawWindow	proc	near
		pusha			; Save all our registers
		push	es

		mov	bp, sp		; Use BP for stack-indexing

		mov	ax, [bp+14h]	; Get video segment
		mov	es, ax		;    into ES

		mov	dx, [bp+1eh]	; Load window x-coordinate
		push	dx
		mov	dx, [bp+1ch]	; Load window y-coordinate
		push	dx
		call	SetLocation	; Position DI for screen write

		mov	ax, [bp+16h]	; Load window attribute
		mov	ah, al		; Attr in MSB for direct write
		mov	al, SPACE	; Fill window with spaces
		mov	cx, [bp+18h]	; Load # rows to write
		inc	cx		; Allow additional 2 lines
		inc	cx		;    for a window title
		cld			; Strings left to right
		mov	dx, [bp+1ah]	; Load # columns to write
		shl	dx, 1		; Adjust for attribute bytes

VertLoop:
		push	cx		; Save curr row value

		mov	cx, [bp+1ah]	; Load # columns to write
		rep	stosw		; Do the deed

		add	di, 00a0h	; Move pointer to next screen row
		sub	di, dx		; Reposition pointer to window left-col

		pop	cx		; Get back curr # of rows
		loop	VertLoop	; Do the next row

		pop	es		; Restore all registers
		popa
		ret	12		; Take 2 parms off the stack
DrawWindow	endp


; DISPLAYS A TITLE ON A WINDOW
;	- RETURNS : Nothing
;	- STACK : (val) x_value, (val) y_value, (val) columns, (val) attr,
;                 (val) video_segment, (ref) title, R_IP
;                 AX, BX, BP

TitleWindow	proc	near
		push	ax			; Save registers we'll use
		push	bx
		push	bp

		mov	bp, sp			; Access parms via BP

		mov	ax, [bp+12h]		; Load window x-coord
		push	ax			; Pass value
		mov	ax, [bp+10h]		; Load window y-coord
		push	ax			; Pass value
		call	SetLocation		; Locate DI to proper offset

		mov	ax, [bp+08h]		; Load addr of title-string
		push	ax			; Pass reference
		call	StrLength		; Calculate length of string
		push	dx			; NB ! Preserve DX
		mov	dx, [bp+0eh]		; Get width of window (cols)
		sub	dx, ax			; Less length of string
		and	dx, 0feh		; Force answer to even value
		add	di, dx			; Start offs to centre str at
		pop	dx			; Reset DX
		
		mov	ax, [bp+08h]		; Load addr of str to display
		push	ax			; Pass reference
		mov	ax, [bp+0ch]		; Attribute to use
		push	ax			; Value pass
		mov	ax, [bp+0ah]		; Get video seg address
		push	ax			; Pass value
		call	WriteString		; Display the title string

		mov	ax, [bp+12h]		; Get window x-coord
		push	ax			; Pass value
		mov	ax, [bp+10h]		; Get window y-coord
		inc	ax			; Locate on 2nd line of win
		push	ax			; Pass value
		call	SetLocation		; Set DI to correct (X, Y) ofs

		mov	al, TITLE_DIVIDER	; Separate title from items
		mov	ah, [bp+0ch]		; Attr to write in
		push	ax			; Pass value
		mov	ax, [bp+0eh]		; Get width of window
		push	ax			; Value pass
		mov	ax, [bp+0ah]		; Segment of video buffer
		push	ax			; Pass by value
		call	WriteChar		; Write series of char

		pop	bp			; Restore registers
		pop	bx
		pop	ax
		ret	12			; Take 6 parms off the stack
TitleWindow	endp


; UPDATES THE MENU HIGHLIGHT BAR BASED ON KEYB INPUT
;	- RETURNS : menu_choice = option's number
;	- STACK : (ref) menu_choice, (ref) parm_list, (val) video_seg, R_IP
;                 AX, BX, DX, BP

GetMenuChoice	proc	near
		push	ax			; Save registers we use
		push	bx
		push	dx
		push	bp

		mov	bp, sp			; BP indexes the stack

		xor	dx, dx			; Clear selection counnter

HighlightBar:
		mov	bx, [bp+0ch]		; Get ptr to menu parm list
		mov	ax, [bx]		; Load window x-coord
		add	ax, [bx+06h]		; Add offset from left-margin
		dec	ax			; Position at 1 less than item
		push	ax			; Send by value
		mov	ax, [bx+02h]		; Load window y-coord
		add	ax, [bx+0eh]		; Add offset from top of win
		add	ax, 02h			; 2 for title
		add	ax, dx			; Offs from first item in list
		push	ax			; Value pass
		mov	ax, [bx+10h]		; Load width of menu bar
		push	ax			; Pass by value
		mov	ax, [bx+12h]		; Load highlight attribute
		push	ax			; Pass by value
		mov	ax, [bp+0ah]		; Load video segment address
		push	ax			; Pass by value
		call	AttributeLine		; Highlight first menu item

WaitForKey:
		lea	ax, Keyb_Status		; Load addr of status ret-var
		push	ax			; Pass reference
		call	KeyPressed		; Check if a key is waiting

		cmp	Keyb_Status, 00h	; Is a key waiting ?
		je	WaitForKey		; No -- keep checking

		lea	ax, KeyStroke		; Load addr of k/stroke ret-var
		push	ax			; Pass reference
		call	GetKey			; Get keystroke from keyb buf

		mov	ax, KeyStroke		; Retrieve keystroke
		or	al, al			; Is it ann extended key ?
		jz	short MenuExtendedKey	; Yes -- process arrow-keys
		cmp	al, ENTER_KEY		; Was Select key pressed ?
		jne	short NotEnter		; No -- check for ESC key
		jmp	short DoneGetting	; Yes -- finish menu

NotEnter:
		cmp	al, QUIT_KEY		; Was Quit key pressed ?
		jne	WaitForKey		; No -- wait for a valid key
		mov	dx, 0ffffh		; Indicate 'no-selection-made'
		jmp	short DoneGetting	; Finish menu

MenuExtendedKey:
		push	ax			; Save keystroke
		mov	bx, [bp+0ch]		; Get addr of menu parm list
		mov	ax, [bx]		; Load x-coord of window
		add	ax, [bx+06h]		; Add offs from left-margin
		dec	ax			; Start 1 col before item
		push	ax			; Send value
		mov	ax, [bx+02h]		; Load y-coord of window
		add	ax, [bx+0eh]		; Add offs from window-top
		add	ax, 02h			; 2 for window title
		add	ax, dx			; Offs from first item disp'ed
		push	ax			; Send value
		mov	ax, [bx+10h]		; Load width of menu bar
		push	ax			; Pass by value
		mov	ax, [bx+0ah]		; Load normal menu attr
		push	ax			; Send value
		mov	ax, [bp+0ah]		; Load video display segment
		push	ax			; Pass by value
		call	AttributeLine		; Reset line to normal video
		pop	ax			; Get back keystroke

		cmp	ah, DOWN_ARROW		; Was down-arrow key pressed ?
		jne	short NotDown		; No -- check for up arrow

		inc	dx			; Position at next item
		mov	ax, [bx+08h]		; Load item count from parms
		cmp	dx, ax			; Is DX >= # of items ?
		jae	short WrapUp		; Yes -- go to first item
		jmp	short NotUp		; Wait for next keystroke

WrapUp:
		xor	dx, dx			; Position at first item
		jmp	short NotUp		; Wait for next keystroke

NotDown:
		cmp	ah, UP_ARROW		; Was up-arrow pressed ?
		jne	short NotUp		; No -- wait for a valid key
		
		or	dx, dx			; Is location counter = 0 ?
		jz	short WrapDown		; Yes -- go to last item
		dec	dx			; Go up by 1 item
		jmp	short NotUp		; Wait for next keystroke

WrapDown:
		mov	dx, [bx+08h]		; Load # of items from parms
		dec	dx			; Go to previous item

NotUp:
		mov	bx, [bp+0ch]		; Get addr of menu parm list
		mov	ax, [bx]		; Load window x-coord
		add	ax, [bx+06h]		; Add offset from left-margin
		dec	ax			; Start 1 col before
		push	ax			; Pass by value
		mov	ax, [bx+02h]		; Load window y-coord
		add	ax, [bx+0eh]		; Add offs from top of window
		add	ax, 02h			; Skip 2 lines for title
		add	ax, dx			; Offs from first menu item
		push	ax			; Value pass
		mov	ax, [bx+10h]		; Load menu bar width (cols)
		push	ax			; Send value
		mov	ax, [bx+12h]		; Load menu highlight attr
		push	ax			; Value pass
		mov	ax, [bp+0ah]		; Load video display segment
		push	ax			; Pass by value
		call	AttributeLine		; Highlight the current item

		jmp	WaitForKey		; Wait for next keystroke

DoneGetting:
		mov	bx, [bp+0eh]		; Get addr of sel'ion ret-var
		mov	[bx], dx		; Save item number

		pop	bp			; Restore saved registers
		pop	dx
		pop	bx
		pop	ax
		ret	6			; Take 3 parms off the stack
GetMenuChoice	endp


; SELECTS AN ACTION TO TAKE BY USING A MENU
;	- RETURNS : menu_choice = a number corresponding to an action to take
;	- STACK : (ref) menu_choice, (ref) parm_list,
;                 (val) video_segment, (ref) title_list,
;                 (ref) options_list, R_IP
;                 AX, BX, DX, BP

MakeMenu	proc	near
		push	ax			; Save registers
		push	bx
		push	dx
		push	bp

		mov	bp, sp			; Use BP to index the stack

		mov	bx, [bp+10h]		; Get addr of menu parms
		mov	ax, [bx+04h]		; Get width of menu
		shl	ax, 1			; Multiply by 2 (chars/attr)
		mov	dh, [bx+08h]		; Get # of menu rows
		add	dh, 03h			; Add 2 lines (for title) and
						;    1 (for blank line at end)
		mul	dh			; bytes-wide * # of rows

		push	ax			; # bytes needed to save window
		call	AllocMem		; Get dynamic memory
		mov	dx, ax			; Save buffer segment in DX

		cmp	dx, NOT_ALLOC_FLAG	; Error occur during alloc ?
		jne	GoodMemAlloc		; No -- proceed with menu
		jmp	BadMemAlloc		; Yes -- leave routine quickly

GoodMemAlloc:
		push	dx			; Save buffer seg (for FreeMem)

		mov	bx, [bp+10h]		; Get addr of menu parms list
		mov	ax, [bx]		; Load upper-left x-coord
		push	ax			; Pass by value
		mov	ax, [bx+02h]		; Load upper-left y-coord
		push	ax			; Pass by value
		mov	ax, [bx+04h]		; Load width of menu
		push	ax			; Pass by value
		mov	ax, [bx+08h]		; Load # of rows
		add	ax, 03h			; Adjust for title, etc
		push	ax			; Save it
		push	dx			; Segment of data buffer
		mov	ax, [bp+0eh]		; Load video segment
		push	ax			; Pass by value
		call	SaveWindow		; Save window in data buffer

		mov	ax, [bx]		; Get x-coord for window
		push	ax			; Send by value
		mov	ax, [bx+02h]		; Get y-coord for window
		push	ax			; Pass by value
		mov	ax, [bx+04h]		; Load menu width
		push	ax			; Pass by value
		mov	ax, [bx+08h]		; Load # of items in menu
		inc	ax			; Adjust # of rows
		push	ax			; Pass by value
		mov	ax, [bx+0ah]		; Get window attr
		push	ax			; Pass by value
		mov	ax, [bp+0eh]		; Get video segment address
		push	ax			; Pass by value
		call	DrawWindow		; Draw the menu window

		mov	ax, [bx]		; Load y-coord of menu
		push	ax			; Send by value
		mov	ax, [bx+02h]		; Load x-coord of menu
		push	ax			; Send by value
		mov	ax, [bx+04h]		; Load menu # of columns
		push	ax			; Pass by value
		mov	ax, [bx+0ah]		; Load menu attribute
		push	ax			; Pass by value
		mov	ax, [bp+0eh]		; Load video segment addr
		push	ax			; Pass by value
		mov	ax, [bp+0ch]		; Load addr of window title
		push	ax			; Send to procedure
		call	TitleWindow		; Write title to window

		mov	ax, [bp+10h]		; Load menu parameter list
		push	ax			; Sennd address
		mov	ax, [bp+0ah]		; Load list of items addr
		push	ax			; Pass it to procedure
		mov	ax, [bp+0eh]		; Load video segment addr
		push	ax			; Pass by value
		call	DisplayItems		; Write sel'able items to menu

		mov	ax, [bp+12h]		; Load addr of return var
		push	ax			; Send it
		mov	ax, [bp+10h]		; Load addr of menu parm list
		push	ax			; Pass by reference
		mov	ax, [bp+0eh]		; Load video segment
		push	ax			; Pass to procedure
		call	GetMenuChoice		; Choose a option

		mov	bx, [bp+10h]		; Offs of parm list to index
		mov	ax, [bx]		; Load x-coord of window
		push	ax			; Value pass
		mov	ax, [bx+02h]		; Load y-coord of window
		push	ax			; Value pass
		mov	ax, [bx+04h]		; Load window # of columns
		push	ax			; Value pass
		mov	ax, [bx+08h]		; Load window # of items
		add	ax, 03h			; 2 for title, 1 for footer
		push	ax			; Send by value
		push	dx			; Segment of saved window
		mov	ax, [bp+0eh]		; Load video segment
		push	ax			; Send by value
		call	RestoreWindow		; Restore window to prev state

		call	FreeMem			; Deallocate memory for buffer

BadMemAlloc:
		pop	bp			; Restore saved registers
		pop	dx
		pop	bx
		pop	ax
		ret	10
MakeMenu	endp


; DRAWS A RECTANGULAR FRAME
;	- RETURNS : Nothing
;	- STACK : (val) x_value, (val) y_value, (val) width,
;                 (val) height, (ref) borders, (val) border_type,
;                 (val) window_attribute, (val) video_segment, R_IP
;                 AX, BX, CX, DX, DS, ES, SI, DI, BP

DrawBorder	proc	near
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

		mov	ax, [bp+22h]
		push	ax
		mov	ax, [bp+20h]
		push	ax
		call	SetLocation

		mov	ax, [bp+14h]
		mov	es, ax

		mov	cx, [bp+18h]
		mov	si, [bp+1ah]
		cld

FindBorder:
		jcxz	short GotBorder
		push	si
		call	FindNextEntry
		mov	si, ax
		loop	FindBorder

GotBorder:
		mov	dx, [bp+1eh]
		dec	dx
		shl	dx, 1

		mov	ah, [bp+16h]
		mov	al, [si]
		push	si
		stosw
		pop	si
		inc	si
		push	si
		mov	ah, [bp+16h]
		mov	al, [si]
		push	ax
		mov	ax, [bp+1eh]
		dec	ax
		dec	ax
		push	ax
		mov	ax, [bp+14h]
		push	ax
		call	WriteChar
		pop	si
		inc	si
		sub	di, 02h
		add	di, dx
		mov	ah, [bp+16h]
		mov	al, [si]
		push	si
		stosw
		sub	di, 02h
		pop	si
		inc	si

		add	di, 00a0h
		sub	di, dx

		mov	cx, [bp+1ch]

		mov	ah, [bp+16h]
		mov	al, [si]
		push	si

Bord_NextRow:
		push	cx

		stosw
		sub	di, 02h
		add	di, dx
		stosw
		sub	di, 02h
		add	di, 00a0h
		sub	di, dx

		pop	cx
		loop	Bord_NextRow
		pop	si

		inc	si
		mov	ah, [bp+16h]
		mov	al, [si]
		push	si
		stosw
		pop	si
		sub	si, 03h
		push	si
		mov	ah, [bp+16h]
		mov	al, [si]
		push	ax
		mov	ax, [bp+1eh]
		dec	ax
		dec	ax
		push	ax
		mov	ax, [bp+14h]
		push	ax
		call	WriteChar
		pop	si
		add	si, 04h
		sub	di, 02h
		add	di, dx
		mov	ah, [bp+16h]
		mov	al, [si]
		stosw

		pop	bp
		pop	di
		pop	si
		pop	es
		pop	ds
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	16
DrawBorder	endp


; COPIES A SAVED WINDOW BACK TO THE SCREEN
;	- RETURNS : Nothing
;	- STACK : (val) x_value, (val) y_value, (val) columns, (val) rows,
;                 (val) buffer_segment, (val) video_segment, R_IP
;                 AX, BX, CX, DS, ES, SI, DI, BP

RestoreWindow	proc	near
		push	ax		; Preserve the registers we'll use
		push	bx
		push	cx
		push	ds
		push	es
		push	si
		push	di
		push	bp

		mov	bp, sp		; Index the stack with BP

		mov	ax, [bp+1ch]	; Load x-coord of window to restore
		push	ax		; Send by value
		mov	ax, [bp+1ah]	; Load y-coord of window to restore
		push	ax		; Send by value
		call	SetLocation	; Position DI to point to (X, Y)
		xor	si, si		; Point SI to beginning of data buffer

		mov	ax, [bp+12h]	; Get video segment from stack
		mov	es, ax		; Init'ise to segment to write to

		mov	cx, [bp+16h]	; Load # of rows to restore
		mov	ax, [bp+14h]	; Get segment of data buffer from stack
		mov	ds, ax		; Init'ise to segment to read from
		cld			; Use strings from left to right

RestoreNextRow:
		push	cx		; Save # of rows still to do
		push	di		; Save DI at beginning of row

		mov	cx, [bp+18h]	; Get # of columns to restore

		repnz	movsw
;RestoreNextCol:
;		lodsw			; Read a char/attr pair from data buf
;		stosw			; Write it to the screen
;		loop	RestoreNextCol	; Loop till this row is done

		pop	di		; Get back DI offset at start of row
		add	di, 00a0h	; Position DI at start of next row
		pop	cx		; Get back # rows still to do
		loop	RestoreNextRow	; Loop until all rows are done

		pop	bp		; Restore registers we used
		pop	di
		pop	si
		pop	es
		pop	ds
		pop	cx
		pop	bx
		pop	ax
		ret	12		; Take 6 parms off the stack
RestoreWindow	endp


; SAVES A SECTION OF THE SCREEN IN A BUFFER
;	- RETURNS : Nothing
;	- STACK : (val) x_value, (val) y_value, (val) columns, (val) rows,
;                 (val) buffer_segment, (val) video_segment, R_IP
;                 AX, BX, CX, DS, ES, SI, DI, BP

SaveWindow	proc	near
		push	ax		; Save registers we'll be modifying
		push	bx
		push	cx
		push	ds
		push	es
		push	si
		push	di
		push	bp

		mov	bp, sp		; Index the stack using BP

		mov	ax, [bp+1ch]	; Get x-coord from stack
		push	ax		; Pass by value
		mov	ax, [bp+1ah]	; Get y-coord from stack
		push	ax		; Pass by value
		call	SetLocation	; Set DI to offset of (X, Y)
		mov	si, di		; Reading from screen -- so set SI
		xor	di, di		; Init'ise DI to start of data buffer

		mov	ax, [bp+14h]	; Get segment addr of data buffer
		mov	es, ax		;    and init'ise write pointer

		mov	cx, [bp+16h]	; Get # of rows to save from stack
		mov	ax, [bp+12h]	; Get segment of screen
		mov	ds, ax		;    and init'ise read pointer
		cld

SaveNextRow:
		push	cx		; Save # of rows to go
		push	si		; Save offset at start of this row

		mov	cx, ss:[bp+18h]	; Get # of columns to save from stack

SaveNextCol:
		lodsw			; Read a char/attr pair from screen
		stosw			; Save it in data buffer
		loop	SaveNextCol	; Loop until entire row is saved

		pop	si		; Get back offset at start of row
		add	si, 00a0h	; Position at start of next line
		pop	cx		; Get back # of rows still to go
		loop	SaveNextRow	; Loop until every row is done

		pop	bp		; Restore registers we used
		pop	di
		pop	si
		pop	es
		pop	ds
		pop	cx
		pop	bx
		pop	ax
		ret	12		; Take 6 parms off the stack
SaveWindow	endp


; WRITES A SET OF MENU ITEMS TO A MENU WINDOW
;	- RETURNS : Nothing
;	- STACK : (ref) parm_list, (ref) options_list, (val) video_seg, R_IP
;                 AX, BX, CX, DX, DI, BP

DisplayItems	proc	near
		push	ax			; Save registers we'll use
		push	bx
		push	cx
		push	dx
		push	di
		push	bp

		mov	bp, sp			; Index stack with BP

		mov	bx, [bp+12h]		; Get addr of parm list
		mov	ax, [bx]		; Load window x-coord
		push	ax			; Value pass
		mov	ax, [bx+02h]		; Load window y-coord
		add	ax, [bx+0eh]		; Add offset from win-top
		add	ax, 02h			; Adjust for win title
		push	ax			; Pass value
		call	SetLocation		; Set DI to correct offset

		mov	cx, [bx+08h]		; Load # of items to display
		mov	si, [bp+10h]		; Load addr of item-list
		mov	ax, [bx+0ah]		; Get attr of menu to use
		mov	dx, [bx+06h]		; Load offset from left-margin
		shl	dx, 1			; Adjust for attributes (* 2)
		add	di, dx			; Go to proper (X, Y)
		mov	bx, [bp+0eh]		; Get video segment address

ItemLoop:
		cmp	byte ptr [si], END_OF_TABLE   ; End of options-list ?
		je	short NoMoreItems	; Yes -- blank rest of win

		inc	si			; Skip record-id byte
		push	di			; Save offset at start of item

		push	si			; Addr of string to write
		push	ax			; Attr to use
		push	bx			; Video segment value
		call	WriteString		; Display the string

		pop	di			; Get back offs of item-start
		add	di, 00a0h		; Locate at next row onscreen

		push	ax			; Save attribute of menu
		push	si			; Pass ptr to current item
		call	FindNextEntry		; Get offset of next item
		mov	si, ax			; Make next item the curr one
		pop	ax			; Get back menu's attribute

		dec	cx			; 1 less item to display
		jmp	ItemLoop		; Loop back for more

NoMoreItems:
		pop	bp			; Restore saved registers
		pop	di
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	6			; Take 3 parms off the stack
DisplayItems	endp
