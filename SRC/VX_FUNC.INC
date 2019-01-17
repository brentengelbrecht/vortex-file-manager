; ALLOWS THE USER TO SELECT A NEW DEFAULT DRIVE
;	- RETURNS : Nothing
;	- STACK : (val) shift_mask,
;                 (ref) update_flags, (ref) keyboard_status,
;                 (ref) keystroke, (val) active_window,
;                 (ref) drive_location, (ref) legal_drives,
;                 (ref) drive_window_title, (val) video_segment, R_IP
;                 AX, BX, CX, DX, ES, BP

SetNewDrive	proc	near
		push	ax
		push	bx
		push	cx
		push	dx
		push	es
		push	bp

		mov	bp, sp

		mov	ax, [bp+12h]
		push	ax
		call	StrLength
		mov	cx, ax
		and	cx, 0feh
		add	cx, 08h

		mov	ax, [bp+10h]
		push	ax
		call	StrLength

		or	ax, cx
		mov	cx, ax

		mov	bl, 05h
		mul	bl
		shl	ax, 1

		push	ax
		call	AllocMem

		cmp	ax, NOT_ALLOC_FLAG
		jne	short ReadySetDrv
		jmp	DoneSetDrv

ReadySetDrv:
		mov	es, ax

		mov	bh, 03h
		mov	bl, MAX_ROWS + 5
		mov	dx, [bp+16h]
		cmp	dl, FIRST_WINDOW
		je	short ChgDrvWin1

		mov	dl, bh
		mov	dh, bl
		jmp	short ChgDrvWin2

ChgDrvWin1:
		mov	dl, bl
		mov	dh, bh

ChgDrvWin2:
		mov	ax, [bp+1eh]
		cmp	ah, ACT_WIN_MASK
		jne	short InActDrvFlags

		xchg	dl, dh

		push	bx
		push	si
		mov	bx, [bp+1ch]
		mov	si, [bp+16h]
		or	byte ptr [bx][si], UPD_NEWPATH
		or	byte ptr [bx][si], UPD_NEWDRV
		not	si
		and	si, 01h
		and	byte ptr [bx][si], NUPD_NEWPATH
		and	byte ptr [bx][si], NUPD_NEWDRV
		pop	si
		pop	bx
		jmp	short DoInactiveDrv

InActDrvFlags:
		push	bx
		push	si
		mov	bx, [bp+1ch]
		mov	si, [bp+16h]
		and	byte ptr [bx][si], NUPD_NEWPATH
		and	byte ptr [bx][si], NUPD_NEWDRV
		not	si
		and	si, 01h
		or	byte ptr [bx][si], UPD_NEWPATH
		or	byte ptr [bx][si], UPD_NEWDRV
		pop	si
		pop	bx

DoInactiveDrv:
		mov	ax, 80
		sub	ax, cx
		shr	ax, 1
		mov	bx, ax

		push	bx
		xor	dh, dh
		push	dx
		push	cx
		mov	ax, DRV_WIN_HIGH
		push	ax
		push	es
		mov	ax, [bp+0eh]
		push	ax
		call	SaveWindow

		push	bx
		push	dx
		push	cx

		push	bx
		push	dx
		push	cx
		mov	ax, 03h
		push	ax
		mov	ax, REVERSE_VIDEO
		push	ax
		mov	ax, [bp+0eh]
		push	ax
		call	DrawWindow

		push	bx
		push	dx
		push	cx
		mov	ax, REVERSE_VIDEO
		push	ax
		mov	ax, [bp+0eh]
		push	ax
		mov	ax, [bp+10h]
		push	ax
		call	TitleWindow

		push	bx
		inc	cx
		push	cx
		add	dx, 03h
		push	dx
		mov	ax, [bp+12h]
		push	ax
		mov	ax, REVERSE_VIDEO
		push	ax
		mov	ax, [bp+0eh]
		push	ax
		call	WriteCentred

GetDrvAgain:
		mov	bx, [bp+1ah]
		push	bx
		call	KeyPressed

		push	ax
		push	bx
		push	es

		mov	ax, BiosData
		mov	es, ax
		lea	bx, ShiftStatus_1
		mov	al, es:[bx]
		and	al, INACT_WIN_MASK + ACT_WIN_MASK
		cmp	al, NULL

		pop	es
		pop	bx
		pop	ax

		jne	short DoDrvSel

AbortDrvSel:
		push	bx
		push	si
		mov	bx, [bp+1ch]
		mov	si, [bp+16h]
		and	byte ptr [bx][si], NUPD_NEWPATH
		and	byte ptr [bx][si], NUPD_NEWDRV
		not	si
		and	si, 01h
		and	byte ptr [bx][si], NUPD_NEWPATH
		and	byte ptr [bx][si], NUPD_NEWDRV
		pop	si
		pop	bx

		jmp	FinDrvSel

DoDrvSel:
		mov	ax, [bx]
		cmp	ax, KEY_WAITING
		jne	short GetDrvAgain

		mov	bx, [bp+18h]
		push	bx
		call	GetKey

		mov	ax, [bx]
		and	al, UPPERCASE_MASK
		cmp	al, 5ah
		ja	short AbortDrvSel
		cmp	al, 41h
		jb	short CheckCtrlDrv

		sub	al, 40h
		mov	cx, [bp+16h]
		not	cx
		and	cx, 01h
		jmp	short SetGotDrv

CheckCtrlDrv:
		cmp	al, 1ah
		ja	short AbortDrvSel

		mov	cx, [bp+16h]

SetGotDrv:
		add	al, 40h
		push	ax

		mov	dx, ax
		push	dx
		push	ds

		push	ax
		mov	dx, [bp+12h]
		push	dx
		call	StrLength
		mov	dx, ax
		pop	ax

		push	dx
		mov	dx, [bp+12h]
		push	dx
		call	FindChar

		mov	dx, ax
		pop	ax

		cmp	dx, CHAR_NOT_FOUND
		jne	short GotDrvNow
		jmp	GetDrvAgain

GotDrvNow:
		push	ax
		push	bx
		push	es

		push	ax
		mov	ax, BiosData
		mov	es, ax
		lea	bx, ShiftStatus_1
		mov	al, es:[bx]
		and	al, not INACT_WIN_MASK + ACT_WIN_MASK
		mov	es:[bx], al
		pop	ax

		int	09h

		push	si
		mov	bx, [bp+14h]
		mov	si, cx
		shl	si, 1
		dec	al, 40h
		mov	word ptr [bx][si], ax
		pop	si

		pop	es
		pop	bx
		pop	ax

; ******* CHECK ***** 1. Check if al is in the valid drive string
; ******* CHECK ***** 2. If so, store that letter in appro. drive-num
; ******* CHECK ***** 3. Clear shift-status flags
; ******* CHECK ***** 4. Set update-flags

FinDrvSel:
		pop	cx
		pop	dx
		pop	bx

		push	bx
		push	dx
		push	cx
		mov	ax, DRV_WIN_HIGH
		push	ax
		push	es
		mov	ax, [bp+0eh]
		push	ax
		call	RestoreWindow

		push	es
		call	FreeMem
		
DoneSetDrv:
		pop	bp
		pop	es
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	18
SetNewDrive	endp


; DETERMINES THE VALID DRIVE LETTERS FOR A SYSTEM
;	- RETURNS : Nothing
;	- STACK : (ref) empty_drive_string, R_IP
;                 AX, BX, CX, DX, DI, ES, BP

GetLegalDrv	proc	near
		push	ax			; Save registers
		push	bx
		push	cx
		push	dx
		push	di
		push	es
		push	bp

		mov	bp, sp

		mov	di, [bp+10h]

		mov	ax, BiosData
		mov	es, ax
		lea	bx, Equipment
		mov	ax, es:[bx]		; Get equipment word
		mov	cl, 6			; Make '# of floppies' the
		shr	ax, cl			;    first two bits
		and	ax, 0000000000000011xb	; Mask them out
		cmp	ax, 01h			; 2 drives in the system ?
		jne	short OneFloppy		; No -- is it A: or B:

		mov	cx, 01h			; Start check at A:
		jmp	short CheckDrives

OneFloppy:
		xor	bx, bx			; Check A: drive
		mov	al, 08h			; Request
		mov	ah, 44h			;    does_drive_use_removable
		int	21h			; Call DOS

		xor	dh, dh			; Assume drive A: is present
		cmp	ax, 000fh		; Invalid drive ?
		jne	short GotCorrectFloppy	; No -- we've got an A:

		mov	dh, 01h			; Yes -- we must have a B:

GotCorrectFloppy:
		inc	dh			; Value is zero-based, so +1
		add	dh, 64			; Format drive letter
		mov	dl, SPACE		;    for output
		mov	[di], dx
		add	di, 02h
		mov	dh, SPACE
		mov	dl, ':'
		mov	[di], dx
		add	di, 02h

		mov	cx, 03h			; Start check at C:

CheckDrives:
		mov	bx, cx			; Current drive to check
		mov	al, 08h			; Request
		mov	ah, 44h			;    does_drive_use_removable
		int	21h			; Call DOS

		cmp	ax, 000fh		; Is the drive # invalid ?
		je	short ContinueDrvChk	; Yes -- don't include

		mov	dh, cl			; No --
		add	dh, 64			;    format drive number for
		mov	dl, SPACE		;       output
		mov	[di], dx
		add	di, 02h
		mov	dh, SPACE
		mov	dl, ':'
		mov	[di], dx
		add	di, 02h

ContinueDrvChk:
		cmp	cx, 26			; Done all possible drives ?
		je	short DoneChkDrv	; Yes -- finish up

		inc	cx			; No --
		jmp	short CheckDrives	;    we're hungry for more

DoneChkDrv:
		pop	bp			; Restore registers
		pop	es
		pop	di
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	2
GetLegalDrv	endp


; FORMS A PATHNAME FROM A DRIVE-NUMBER, DIRECTORY AND FILE-MASK
;	- RETURNS : path_dest = null-terminated path string
;	- STACK : (ref) path_dest, (ref) drive_num,
;                 (ref) dir_string, (ref) file_mask, R_IP
;                 AX, BX, DI, ES, BP

BuildPathName	proc	near
		push	ax
		push	bx
		push	di
		push	es
		push	bp

		mov	bp, sp

		xor	ax, ax
		push	ax
		push	ds
		mov	ax, [bp+12h]
		push	ax
		mov	ax, PATH_SIZE
		push	ax
		call	InitBuffer

		push	ds
		pop	es
		mov	di, [bp+12h]

		mov	al, SPACE
		stosb

		mov	bx, [bp+10h]
		mov	ax, [bx]
		add	al, 64
		stosb
		mov	ax, '\:'
		stosw

		mov	bx, [bp+0eh]
		push	bx
		call	StrLength

		push	ax

		push	ds
		push	di
		push	ds
		mov	bx, [bp+0eh]
		push	bx
		push	ax
		call	CopyString
		add	di, ax
		dec	di

		pop	ax
		cmp	ax, 01h
		je	short InRootDir

		mov	al, '\'
		stosb

InRootDir:
		mov	bx, [bp+0ch]
		push	bx
		call	StrLength

		push	ds
		push	di
		push	ds
		mov	bx, [bp+0ch]
		push	bx
		push	ax
		call	CopyString
		add	di, ax
		dec	di

		mov	al, SPACE
		stosb

		pop	bp
		pop	es
		pop	di
		pop	bx
		pop	ax
		ret	8
BuildPathName	endp


; COPIES THE "SELECTED" FILE ENTRIES FROM THE LOGGED PATH TO THE DEST PATH
;	- RETURNS : Nothing
;	- STACK : (val) subfunction_flag,
;                 (val) block_size_in_bytes, (ref) memory_avail,
;                 (ref) non_active_drive, (ref) non_active_directory,
;
;                 (ref) error_code, (ref) parent_dir_tag,
;                 (ref) mask_star_dot_star, (val) DTA_segment,
;                 (ref) DTA_offset, (val) active_window,
;                 (ref) update_flags, (val) file_count,
;
;                 (ref) scratch_word, (ref) drive_number,
;                 (ref) directory, (val) list_block_segment,
;                 (ref) file_entry_buffer, (ref) scratch_buffer, R_IP
;                 AX, BX, CX, DX, SI, DI, ES, BP

Copy		proc	near
		push	ax
		push	bx
		push	cx
		push	dx
		push	si
		push	di
		push	es
		push	bp

		mov	bp, sp
		xor	dx, dx

CopyLoop:
		mov	ax, SEARCH_SELECTED	; Search for files marked as

CopyDefault:
		push	ax			;    selected (not current)
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
		jne	short ProcessCopy	; Yes -- copy it
		jmp	CopyCurrent		; No -- copy current entry

ProcessCopy:
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

;mov ax, 0
;push ax
;push ax
;call setlocation
;
;mov ax, [bp+12h]
;push ax
;mov ax, reverse_video
;push ax
;mov ax, videosegment
;push ax
;call writestring

		inc	dx

		push	es		; Save whatever's there
		mov	ax, [bp+16h]	; Load the segment value
		mov	es, ax		;    of the current list's block
		mov	bx, [bp+1ch]	; Addr of offset of entry in list block
		mov	ax, [bx]	; Offset of entry in list block
		mov	bx, ax		; Point to file entry being processed
		mov	al, byte ptr es:[bx]
		and	al, ATTR_DIR
		cmp	al, ATTR_DIR

		je	short CopyDirEntry

		mov	ax, [bp+26h]
		push	ax
		mov	ax, [bp+24h]
		add	ax, DTA_DATA_OFS + FILENAME_OFFS
		push	ax
		push	ds
		mov	ax, [bp+14h]
		add	ax, FILENAME_OFFS
		push	ax
		mov	ax, DTA_DATA_LEN
		push	ax
		call	CopyString

		mov	ax, [bp+1ch]
		push	ax
		mov	ax, [bp+2ch]
		push	ax
		mov	ax, [bp+34h]
		push	ax
		mov	ax, [bp+20h]
		push	ax
		mov	ax, [bp+32h]
		push	ax
		mov	ax, [bp+22h]
		push	ax
		mov	ax, [bp+30h]
		push	ax
		mov	ax, [bp+2eh]
		push	ax
		mov	ax, [bp+12h]
		push	ax
		mov	ax, [bp+26h]
		push	ax
		mov	ax, [bp+24h]
		push	ax

		mov	ax, [bp+36h]
		cmp	ax, FUNCTION_COPY
		jne	short DoMoveRename

		call	CopyFile
		jmp	CopyFilEntry

DoMoveRename:
		call	MoveFile
		jmp	CopyFilEntry

CopyDirEntry:
		mov	al, es:[bx+FILENAME_OFFS]
		cmp	al, '.'
		jne	short CopyDirIsValid
		jmp	CopyFilEntry

CopyDirIsValid:
		mov	ax, PATH_SIZE
		push	ax
		call	AllocMem
		mov	si, ax

		push	si
		xor	ax, ax
		push	ax
		push	ds
		mov	ax, [bp+2eh]
		push	ax
		mov	ax, PATH_SIZE
		push	ax
		call	CopyString

		xor	ax, ax
		push	ax
		mov	ax, [bp+32h]
		push	ax
		mov	ax, [bp+1ch]
		push	ax
		mov	ax, [bp+34h]
		push	ax
		mov	ax, [bp+30h]
		push	ax
		mov	ax, [bp+2eh]
		push	ax
		mov	ax, [bp+12h]
		push	ax

		mov	ax, [bp+2ch]
		push	ax
		mov	ax, [bp+2ah]
		push	ax
		mov	ax, [bp+28h]
		push	ax
		mov	ax, [bp+22h]
		push	ax
		mov	ax, [bp+20h]
		push	ax
		mov	ax, [bp+14h]
		push	ax
		mov	ax, [bp+26h]
		push	ax
		mov	ax, [bp+24h]
		push	ax

		mov	ax, [bp+36h]
		cmp	ax, FUNCTION_COPY
		jne	short DoMoveRenameDir

		call	CopyDir

		pop	ax		; Move takes an extra parm not used
					;    by COPY

		jmp	short ResetNonActive

DoMoveRenameDir:
		mov	ax, FUNC_MOVE_DIR
		mov	bx, sp
		mov	[bx+1ch], ax

		call	MoveDir

ResetNonActive:
		push	ds
		mov	ax, [bp+2eh]
		push	ax
		push	si
		xor	ax, ax
		push	ax
		mov	ax, PATH_SIZE
		push	ax
		call	CopyString

		push	si
		call	FreeMem

CopyFilEntry:
		mov	byte ptr es:[bx+ENTRY_LENGTH-03h], NULL
					; Reset the SELECTION byte
					;    (So that this file is not "hit"
					;        by the search routine again)

		pop	es		; Get back original value

CopyResume:
		jmp	short CopyLoop	; Check for more selections

CopyCurrent:
		cmp	dx, NULL
		jne	short DoneCopy

		mov	ax, SEARCH_CURRENT
		jmp	short CopyDefault

DoneCopy:
		pop	bp
		pop	es
		pop	di
		pop	si
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	38
Copy		endp


; COPIES PART OF A DIRECTORY TREE
;	- RETURNS : Nothing
;	- STACK : (ref) memory_available, (ref) scratch_word,
;                 (val) block_size_in_bytes, (ref) non_active_drive,
;                 (ref) non_active_directory, (ref) scratch_buffer,
;
;                 (ref) error_code, (ref) parent_directory_tag,
;                 (ref) mask_star_dot_star, (val) active_window,
;                 (ref) update_flags, (ref) base_source_path_(file_entry_buf),
;                 (val) current_DTA_segment, (ref) current_DTA_offset, R_IP
;                 AX, BX, CX, SI, DI, ES, BP

CopyDir		proc	near
		push	ax		; Save registers
		push	bx
		push	cx
		push	si
		push	di
		push	es
		push	bp

		mov	bp, sp

		mov	ax, PATH_SIZE
		push	ax
		call	AllocMem
		mov	es, ax		; Allocate memory to save the
					;    non_active_directory

		push	es
		xor	ax, ax
		push	ax
		push	ds
		mov	ax, [bp+22h]
		push	ax
		mov	ax, PATH_SIZE
		push	ax
		call	CopyString	; Save non_active_directory

		push	es

		mov	ax, PATH_SIZE
		push	ax
		call	AllocMem
		mov	es, ax		; Get memory to save scratch_buffer

		push	es
		xor	ax, ax
		push	ax
		push	ds
		mov	ax, [bp+20h]
		push	ax
		mov	ax, PATH_SIZE
		push	ax
		call	CopyString	; Save original scratch_buffer

;mov ax, 40
;push ax
;mov ax, 0
;push ax
;call setlocation
;
;mov ax, [bp+14h]
;add ax, filename_offs
;push ax
;mov ax, reverse_video
;push ax
;mov ax, videosegment
;push ax
;call writestring
;
;mov ax, 0
;push ax
;mov ax, 24
;push ax
;call setlocation
;
;mov ax, [bp+22h]
;push ax
;mov ax, reverse_video
;push ax
;mov ax, videosegment
;push ax
;call writestring
;pop es

		mov	ax, [bp+18h]
		push	ax
		mov	ax, [bp+16h]
		push	ax
		mov	ax, [bp+14h]
		add	ax, FILENAME_OFFS
		push	ax
		call	ChangeDirectory		; Change directory on
						;    source path to the name
						;    in file_entry_buffer

		mov	ax, [bp+20h]
		push	ax
		mov	ax, [bp+24h]
		push	ax
		mov	ax, [bp+22h]
		push	ax
		mov	ax, [bp+14h]
		add	ax, FILENAME_OFFS
		push	ax
		call	BuildPathName	; Build the non-active target path

;mov ax, 0
;push ax
;mov ax, 0
;push ax
;call setlocation
;
;mov ax, [bp+20h]
;push ax
;mov ax, reverse_video
;push ax
;mov ax, videosegment
;push ax
;call writestring

		mov	ax, [bp+1eh]
		push	ax
		xor	ax, ax
		push	ax
		push	ds
		mov	ax, [bp+20h]
		inc	ax
		push	ax
		call	MakeDirectory	; Create the directory on the target
					;    drive

		push	ds
		mov	ax, [bp+22h]
		push	ax
		push	ds
		mov	ax, [bp+20h]
		add	ax, 04h		; Don't copy drive-letter & backslash
		push	ax
		mov	ax, PATH_SIZE
		push	ax
		call	CopyString	; Copy the target pathname to the non-
					;    active directory, in case we get
					;    involved in a little recursion

		mov	bx, [bp+22h]
		push	bx
		call	StrLength
		add	bx, ax
		sub	bx, 02h
		mov	byte ptr [bx], NULL	; Remove trailing SPACE char

		push	ds
		mov	ax, [bp+20h]
		push	ax
		push	es
		xor	ax, ax
		push	ax
		mov	ax, PATH_SIZE
		push	ax
		call	CopyString	; Restore the previous contents of
					;    the scratch buffer

		pop	es
		push	es

		mov	ax, [bp+12h]
		mov	es, ax
		mov	bx, [bp+10h]

		mov	ax, [bp+1ah]
		push	ax
		call	GetFirstFile	; Get the first directory entry

Reset_Copy:
		cmp	ax, ERROR_FIL_NMORE	; Any more file entries ?
		jne	short LookForDir	; No -- continue
		jmp	DoneCopyDir		; Yes -- return to caller

LookForDir:
		mov	al, es:[bx+DTA_DATA_OFS]
		and	al, ATTR_DIR
		cmp	al, ATTR_DIR		; Is this entry for a
						;    subdirectory ?
		je	short CopyDirectory	; Yes

		jmp	CopyDirFile

CopyDirectory:
		mov	al, es:[bx+DTA_DATA_OFS+FILENAME_OFFS]
		cmp	al, '.'			; Is this file '.' or '..'
						;    (BAD NEWS !) ?
		jne	short IsValidDir	; No (whew !)

		jmp	CopyDirNextEnt		; Yes -- so skip it !

IsValidDir:
		mov	ax, DIR_REC_SIZE
		push	ax
		call	AllocMem
		mov	di, ax		; Allocate memory for this DTA

		push	di
		xor	ax, ax
		push	ax
		mov	ax, [bp+12h]
		push	ax
		mov	ax, [bp+10h]
		push	ax
		mov	ax, DIR_REC_SIZE
		push	ax
		call	CopyString	; Save the current DTA

		push	ds
		mov	ax, [bp+14h]
		push	ax
		mov	ax, [bp+12h]
		push	ax
		mov	ax, [bp+10h]
		add	ax, DTA_DATA_OFS
		push	ax
		mov	ax, DTA_DATA_LEN
		push	ax
		call	CopyString	; Copy the current DTA value
					;    into file_entry_buffer

		mov	ax, [bp+2ah]
		push	ax
		mov	ax, [bp+28h]
		push	ax
		mov	ax, [bp+26h]
		push	ax
		mov	ax, [bp+24h]
		push	ax
		mov	ax, [bp+22h]
		push	ax
		mov	ax, [bp+20h]
		push	ax

		mov	ax, [bp+1eh]
		push	ax
		mov	ax, [bp+1ch]
		push	ax
		mov	ax, [bp+1ah]
		push	ax
		mov	ax, [bp+18h]
		push	ax
		mov	ax, [bp+16h]
		push	ax
		mov	ax, [bp+14h]
		push	ax
		mov	ax, [bp+12h]
		push	ax
		mov	ax, [bp+10h]
		push	ax
		call	CopyDir		; Recur to copy the next directory

		mov	ax, [bp+12h]
		push	ax
		mov	ax, [bp+10h]
		push	ax
		push	di
		xor	ax, ax
		push	ax
		mov	ax, DIR_REC_SIZE
		push	ax
		call	CopyString	; Restore the current DTA

		push	di
		call	FreeMem		; Deallocate DTA's 'save' memory

		jmp	CopyDirNextEnt

CopyDirFile:
;push es
;mov ax, 40
;push ax
;mov ax, 24
;push ax
;call setlocation
;
;mov ax, bx
;add ax, dta_data_ofs + filename_offs
;push ax
;mov ax, reverse_video
;push ax
;mov ax, videosegment
;push ax
;call writestring
;pop es

		mov	ax, PATH_SIZE
		push	ax
		call	AllocMem		; Get mem for scr_buf save
		mov	si, ax

		push	si
		xor	ax, ax
		push	ax
		push	ds
		mov	ax, [bp+20h]
		push	ax
		mov	ax, PATH_SIZE
		push	ax
		call	CopyString		; Save the current value
						;    in scratch_buffer

		mov	ax, DIR_REC_SIZE
		push	ax
		call	AllocMem
		mov	di, ax		; Allocate memory for this DTA

		push	di
		xor	ax, ax
		push	ax
		mov	ax, [bp+12h]
		push	ax
		mov	ax, [bp+10h]
		push	ax
		mov	ax, DIR_REC_SIZE
		push	ax
		call	CopyString	; Save the current DTA

		push	bx
		mov	bx, [bp+20h]
		mov	byte ptr [bx], SPACE
		pop	bx		; Init'se first char of scratch_buf
					;    as CopyFile expects it

		push	ds
		mov	ax, [bp+20h]
		inc	ax
		push	ax
		mov	ax, [bp+12h]
		push	ax
		mov	ax, [bp+10h]
		add	ax, DTA_DATA_LEN + FILENAME_OFFS
		dec	ax
		push	ax
		xor	ax, ax
		push	ax
		call	CopyString	; Copy the source file_name
					;    into scratch_buffer

		mov	ax, [bp+28h]
		push	ax
		mov	ax, [bp+1eh]
		push	ax
		mov	ax, [bp+26h]
		push	ax
		mov	ax, [bp+16h]
		push	ax
		mov	ax, [bp+2ah]
		push	ax
		mov	ax, [bp+18h]
		push	ax
		mov	ax, [bp+24h]
		push	ax
		mov	ax, [bp+22h]
		push	ax
		mov	ax, [bp+20h]
		push	ax
		mov	ax, [bp+12h]
		push	ax
		mov	ax, [bp+10h]
		push	ax
		call	CopyFile		; Copy the source file

		mov	ax, [bp+12h]
		push	ax
		mov	ax, [bp+10h]
		push	ax
		push	di
		xor	ax, ax
		push	ax
		mov	ax, DIR_REC_SIZE
		push	ax
		call	CopyString	; Restore the current DTA

		push	di
		call	FreeMem		; Deallocate DTA's 'save' memory

		push	ds
		mov	ax, [bp+20h]
		push	ax
		push	si
		xor	ax, ax
		push	ax
		mov	ax, PATH_SIZE
		push	ax
		call	CopyString	; Restore value in scratch_buffer

		push	si
		call	FreeMem		; Dealloc scratch_buf save memory

CopyDirNextEnt:
		call	GetNextFile	; Get the next directory entry from DOS

		jmp	Reset_Copy	; Loop back for more

DoneCopyDir:
		pop	es

		push	ds
		mov	ax, [bp+22h]
		push	ax
		push	es
		xor	ax, ax
		push	ax
		mov	ax, PATH_SIZE
		push	ax
		call	CopyString	; Restore the value of non_active_dir

		push	es
		call	FreeMem		; Deallocate non_active_dir save buffer

		mov	ax, [bp+18h]
		push	ax
		mov	ax, [bp+16h]
		push	ax
		mov	ax, [bp+1ch]
		push	ax
		call	ChangeDirectory		; Return to parent directory

		push	es
		call	FreeMem		; Release scratch_buffer's memory

		mov	bx, [bp+16h]
		mov	si, [bp+18h]
		and	byte ptr [bx][si], NUPD_NEWPATH
		and	byte ptr [bx][si], NUPD_RELOAD
		not	si
		and	si, 01h
		or	byte ptr [bx][si], UPD_RESET
		and	byte ptr [bx][si], NUPD_NEWPATH
		or	byte ptr [bx][si], UPD_RELOAD	; Set update flags

		pop	bp		; Restore registers
		pop	es
		pop	di
		pop	si
		pop	cx
		pop	bx
		pop	ax
		ret	28
CopyDir		endp


; COPIES A FILE FROM THE LOGGED PATH TO THE NON-ACTIVE PATH
;	- RETURNS : Nothing
;	- STACK : (ref) scratch_word,
;                 (ref) error_code, (val) block_size_in_bytes,
;                 (ref) update_flags, (ref) memory_avail,
;                 (val) active_window, (ref) non_active_drive,
;                 (ref) non_active_directory, (ref) logged_file_(scratch_buf),
;                 (val) DTA_segment, (ref) DTA_offset, R_IP
;                 AX, BX, CX, DX, SI, DI, ES, BP

CopyFile	proc	near
		push	ax
		push	bx
		push	cx
		push	dx
		push	si
		push	di
		push	es
		push	bp

		mov	bp, sp

		xor	ax, ax
		push	ax
		mov	ax, [bp+26h]
		push	ax
		mov	ax, [bp+16h]
		inc	ax
		push	ax
		mov	ax, READ_ONLY
		push	ax
		call	OpenFile

;mov ax, 0
;push ax
;mov ax, 24
;push ax
;call setlocation
;
;mov ax, [bp+16h]
;push ax
;mov ax, reverse_video
;push ax
;mov ax, videosegment
;push ax
;call writestring

		mov	bx, [bp+26h]
		mov	ax, [bx]
		cmp	ax, FILE_OPEN_FAILED
		jne	short CheckTargetCopy
		jmp	DoneCopyFil

CheckTargetCopy:
		mov	cx, ax

		mov	ax, [bp+24h]
		push	ax
		mov	ax, [bp+26h]
		push	ax
		push	ds
		mov	ax, [bp+16h]
		inc	ax
		push	ax
		call	GetAttr

		mov	bx, [bp+24h]
		mov	ax, [bx]
		cmp	ax, NULL
		je	short CopAttrOK

		mov	ax, ERROR_FIL_LOST_ATTR
		push	ax
		call	ErrorProc
		mov	bx, [bp+26h]
		mov	word ptr [bx], NULL

CopAttrOK:
		mov	bx, [bp+26h]
		mov	di, [bx]

		mov	ax, [bp+16h]
		push	ax
		mov	ax, [bp+1ah]
		push	ax
		mov	ax, [bp+18h]
		push	ax
		mov	ax, [bp+12h]
		add	ax, DTA_DATA_OFS + FILENAME_OFFS
		push	ax
		call	BuildPathName

		push	di
		mov	ax, [bp+26h]
		push	ax
		mov	ax, [bp+16h]
		inc	ax
		push	ax
		mov	ax, WRITE_ONLY
		push	ax
		call	OpenFile

;mov ax, 0
;push ax
;mov ax, 24
;push ax
;call setlocation
;
;mov ax, [bp+16h]
;push ax
;mov ax, reverse_video
;push ax
;mov ax, videosegment
;push ax
;call writestring

		mov	bx, [bp+26h]
		mov	ax, [bx]
		cmp	ax, FILE_OPEN_FAILED
		jne	short DoCopyFil
		jmp	CrashOnTwo

DoCopyFil:
		mov	dx, ax

		mov	ax, [bp+22h]
		push	ax
		call	AllocMem
		mov	si, ax

CopyFilRest:
		mov	bx, [bp+24h]
		mov	word ptr [bx], NULL

		mov	ax, [bp+24h]
		push	ax
		push	cx
		mov	ax, [bp+22h]
		push	ax
		mov	ax, [bp+1eh]
		push	ax
		push	si
		call	ReadRecur

		mov	bx, [bp+24h]
		mov	ax, [bx]
		mov	bx, [bp+26h]
		mov	[bx], ax

		mov	ax, [bp+24h]
		push	ax
		push	dx
		mov	ax, [bp+22h]
		push	ax
		mov	ax, [bp+1eh]
		push	ax
		push	si
		call	WriteRecur

		mov	bx, [bp+26h]
		mov	ax, [bx]
		cmp	ax, NOT_TRANSFERRED
		je	short CopyFilRest

		push	si
		call	FreeMem

		mov	ax, [bp+24h]
		push	ax
		mov	bx, [bp+16h]
		push	bx
		add	bx, 02h
		push	bx
		push	cx
		call	GetDateTime

		mov	ax, [bp+24h]
		push	ax
		mov	bx, [bp+16h]
		mov	ax, [bx]
		push	ax
		add	bx, 02h
		mov	ax, [bx]
		push	ax
		push	dx
		call	SetDateTime

		push	dx
		call	CloseFile

		mov	bx, [bp+20h]
		mov	si, [bp+1ch]
		not	si
		and	si, 01h
		or	byte ptr [bx][si], UPD_RELOAD
		or	byte ptr [bx][si], UPD_RESET

CrashOnTwo:
		push	cx
		call	CloseFile

DoneCopyFil:
		pop	bp
		pop	es
		pop	di
		pop	si
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	22
CopyFile	endp


; MOVES OR RENAMES THE SPECIFIED DIRECTORY
;	- RETURNS : Nothing
;	- STACK : (val) action_code,
;                 (ref) memory_available, (ref) scratch_word,
;                 (val) block_size_in_bytes, (ref) non_active_drive,
;                 (ref) non_active_directory, (ref) scratch_buffer,
;
;                 (ref) error_code, (ref) parent_directory_tag,
;                 (ref) mask_star_dot_star, (val) active_window,
;                 (ref) update_flags, (ref) base_source_path_(file_entry_buf),
;                 (val) current_DTA_segment, (ref) current_DTA_offset, R_IP
;                 AX, BX, CX, DX, SI, DI, ES, BP

MoveDir		proc	near
		push	ax
		push	bx
		push	cx
		push	dx
		push	si
		push	di
		push	es
		push	bp

		mov	bp, sp

		mov	ax, [bp+2eh]
		cmp	ax, FUNC_MOVE_DIR
		jne	short RenameThisDir
		jmp	MoveThisDir

RenameThisDir:
mov ax, 0
push ax
mov ax, 0
push ax
call setlocation

mov ax, [bp+22h]
push ax
mov ax, reverse_video
push ax
mov ax, videosegment
push ax
call writestring

		mov	ax, PATH_SIZE
		push	ax
		call	AllocMem
		mov	es, ax		; Get memory to save source path

		push	es
		xor	ax, ax
		push	ax
		push	ds
		mov	ax, [bp+22h]
		inc	ax
		push	ax
		mov	ax, PATH_SIZE - 01h
		push	ax
		call	CopyString		; Save source path in ES:0

		mov	ax, [bp+22h]
		push	ax
		mov	ax, [bp+26h]
		push	ax
		mov	ax, [bp+24h]
		push	ax
		mov	ax, [bp+16h]
		add	ax, FILENAME_OFFS
		push	ax
		call	BuildPathName		; Build up the target pathname

mov ax, 40
push ax
mov ax, 0
push ax
call setlocation

mov ax, [bp+22h]
push ax
mov ax, reverse_video
push ax
mov ax, videosegment
push ax
call writestring

		mov	ax, [bp+20h]
		push	ax
		mov	ax, [bp+18h]
		push	ax
		mov	ax, [bp+1ah]
		push	ax
		push	es
		mov	ax, [bp+22h]
		inc	ax
		push	ax
		call	RenameFile	; Change the name of the source dir

		mov	bx, [bp+22h]
		mov	byte ptr [bx], SPACE

		push	ds
		mov	ax, [bp+22h]
		inc	ax
		push	ax
		push	es
		xor	ax, ax
		push	ax
		mov	ax, PATH_SIZE - 01h
		push	ax
		call	CopyString		; Restore original source path

		push	es
		call	FreeMem			; Release save-buffer memory

mov ax, 40
push ax
mov ax, 24
push ax
call setlocation

mov ax, [bp+22h]
push ax
mov ax, reverse_video
push ax
mov ax, videosegment
push ax
call writestring

		mov	bx, [bp+20h]
		mov	ax, [bx]
		cmp	ax, NULL		; Did an error occur ?
		je	short RenamedDirOK	; No -- proceed

		push	ax
		call	ErrorProc		; Yes -- display error message

RenamedDirOK:
		jmp	DoneMoveDir		; Finished the RENAME

MoveThisDir:
		mov	ax, PATH_SIZE
		push	ax
		call	AllocMem
		mov	si, ax		; Get mem to save non-active dir

		push	si
		xor	ax, ax
		push	ax
		push	ds
		mov	ax, [bp+24h]
		push	ax
		mov	ax, PATH_SIZE
		push	ax
		call	CopyString	; Save the non-active directory

		mov	ax, PATH_SIZE
		push	ax
		call	AllocMem
		mov	dx, ax		; Get memory to save scratch_buffer

		push	dx
		xor	ax, ax
		push	ax
		push	ds
		mov	ax, [bp+22h]
		push	ax
		mov	ax, PATH_SIZE
		push	ax
		call	CopyString	; Save scratch_buffer

		mov	ax, DIR_REC_SIZE
		push	ax
		call	AllocMem
		mov	cx, ax		; Get memory to save DTA

		push	cx
		xor	ax, ax
		push	ax
		mov	ax, [bp+14h]
		push	ax
		mov	ax, [bp+12h]
		push	ax
		mov	ax, DIR_REC_SIZE
		push	ax
		call	CopyString	; Save the current DTA

		mov	ax, DTA_DATA_LEN
		push	ax
		call	AllocMem
		mov	di, ax		; Get memory to save file_entry_buffer

		push	di
		xor	ax, ax
		push	ax
		push	ds
		mov	ax, [bp+16h]
		push	ax
		mov	ax, DTA_DATA_LEN
		push	ax
		call	CopyString	; Save the original file_entry_buffer

		push	si		; Save segment addresses of data
		push	dx		;    blocks
		push	cx		;    (just in case)
		push	di		;

		mov	ax, [bp+2ch]
		push	ax
		mov	ax, [bp+2ah]
		push	ax
		mov	ax, [bp+28h]
		push	ax
		mov	ax, [bp+26h]
		push	ax
		mov	ax, [bp+24h]
		push	ax
		mov	ax, [bp+22h]
		push	ax
		mov	ax, [bp+20h]
		push	ax
		mov	ax, [bp+1eh]
		push	ax
		mov	ax, [bp+1ch]
		push	ax
		mov	ax, [bp+1ah]
		push	ax
		mov	ax, [bp+18h]
		push	ax
		mov	ax, [bp+16h]
		push	ax
		mov	ax, [bp+14h]
		push	ax
		mov	ax, [bp+12h]
		push	ax
		call	CopyDir		; Copy the specified directory
					;    (name is in scratch_buffer)

		pop	di		; Restore segment addresses of saved
		pop	cx		;    data blocks
		pop	dx		;
		pop	si		;

		push	ds
		mov	ax, [bp+16h]
		push	ax
		push	di
		xor	ax, ax
		push	ax
		mov	ax, DTA_DATA_LEN
		push	ax
		call	CopyString	; Restore the original file_entry_buf

		push	di
		call	FreeMem		; Release save-buffer memory

		mov	ax, [bp+14h]
		push	ax
		mov	ax, [bp+12h]
		push	ax
		push	cx
		xor	ax, ax
		push	ax
		mov	ax, DIR_REC_SIZE
		push	ax
		call	CopyString	; Restore the current DTA

		push	cx
		call	FreeMem		; Release save-buffer memory

		push	ds
		mov	ax, [bp+22h]
		push	ax
		push	dx
		xor	ax, ax
		push	ax
		mov	ax, PATH_SIZE
		push	ax
		call	CopyString	; Restore original scratch_buffer

		push	dx
		call	FreeMem		; Release save-buffer memory

;push es
;push di
;push si
;
;mov ax, 0
;push ax
;mov ax, 24
;push ax
;call setlocation
;
;mov ax, [bp+16h]
;add ax, filename_offs
;push ax
;mov ax, reverse_video
;push ax
;mov ax, videosegment
;push ax
;call writestring
;
;pop si
;pop di
;pop es

		push	si

		mov	ax, [bp+20h]
		push	ax
		mov	ax, [bp+1eh]
		push	ax
		mov	ax, [bp+1ch]
		push	ax
		mov	ax, [bp+1ah]
		push	ax
		mov	ax, [bp+18h]
		push	ax
		mov	ax, [bp+16h]
		add	ax, FILENAME_OFFS
		push	ax
		mov	ax, [bp+14h]
		push	ax
		mov	ax, [bp+12h]
		push	ax
		call	DelDir		; Delete file (with name in
					;    scratch_buffer)

		pop	si

		push	ds
		mov	ax, [bp+24h]
		push	ax
		push	si
		xor	ax, ax
		push	ax
		mov	ax, PATH_SIZE
		push	ax
		call	CopyString	; Restore the non-active directory

		push	si
		call	FreeMem		; Release save-buf memory

DoneMoveDir:
		mov	bx, [bp+18h]
		mov	si, [bp+1ah]
		or	byte ptr [bx][si], UPD_RESET
		or	byte ptr [bx][si], UPD_RELOAD
		and	byte ptr [bx][si], NUPD_NEWPATH
		not	si
		and	si, 01h
		or	byte ptr [bx][si], UPD_RELOAD
		and	byte ptr [bx][si], NUPD_NEWPATH

		pop	bp
		pop	es
		pop	di
		pop	si
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	30
MoveDir		endp


; MOVES OR RENAMES THE SPECIFIED FILESPEC (FILE)
;	- RETURNS : Nothing
;	- STACK : (ref) scratch_offset,
;                 (ref) error_code, (val) block_size_in_bytes,
;                 (ref) update_flags, (ref) memory_avail,
;                 (val) active_window, (ref) non_active_drive,
;                 (ref) non_active_dir, (ref) logged_file_(file_entry_buffer),
;                 (val) DTA_segment, (ref) DTA_offset, R_IP
;                 AX, BX, CX, DX, SI, DI, ES, BP

MoveFile	proc	near
		push	ax
		push	bx
		push	cx
		push	dx
		push	si
		push	di
		push	es
		push	bp

		mov	bp, sp

		mov	bx, [bp+16h]
		inc	bx
		mov	dl, [bx]
		mov	bx, [bp+1ah]
		mov	ax, [bx]
		add	al, 64
		cmp	al, dl
		je	short RenameThisFile
		jmp	MoveThisFile

RenameThisFile:
		mov	ax, PATH_SIZE
		push	ax
		call	AllocMem
		mov	es, ax

		push	es
		xor	ax, ax
		push	ax
		push	ds
		mov	ax, [bp+16h]
		inc	ax
		push	ax
		mov	ax, PATH_SIZE
		push	ax
		call	CopyString

		mov	ax, [bp+16h]
		push	ax
		mov	ax, [bp+1ah]
		push	ax
		mov	ax, [bp+18h]
		push	ax
		mov	ax, [bp+12h]
		add	ax, DTA_DATA_OFS + FILENAME_OFFS
		push	ax
		call	BuildPathName

		mov	ax, [bp+24h]
		push	ax
		mov	ax, [bp+20h]
		push	ax
		mov	ax, [bp+1ch]
		push	ax
		push	es
		mov	ax, [bp+16h]
		inc	ax
		push	ax
		call	RenameFile

		push	es
		call	FreeMem

		mov	bx, [bp+24h]
		mov	ax, [bx]
		cmp	ax, NULL
		je	short RenamedOK

		push	ax
		call	ErrorProc

RenamedOK:
		jmp	DoneMoveFile

MoveThisFile:
		mov	ax, PATH_SIZE
		push	ax
		call	AllocMem
		mov	si, ax		; Get memory to save file_entry_buf

		push	si
		xor	ax, ax
		push	ax
		push	ds
		mov	ax, [bp+16h]
		push	ax
		mov	ax, PATH_SIZE
		push	ax
		call	CopyString	; Save value in file_entry_buffer

		mov	ax, [bp+26h]
		push	ax
		mov	ax, [bp+24h]
		push	ax
		mov	ax, [bp+22h]
		push	ax
		mov	ax, [bp+20h]
		push	ax
		mov	ax, [bp+1eh]
		push	ax
		mov	ax, [bp+1ch]
		push	ax
		mov	ax, [bp+1ah]
		push	ax
		mov	ax, [bp+18h]
		push	ax
		mov	ax, [bp+16h]
		push	ax
		mov	ax, [bp+14h]
		push	ax
		mov	ax, [bp+12h]
		push	ax
		call	CopyFile	; Duplicate the file (with name in
					;    file_entry_buffer)

		push	ds
		mov	ax, [bp+16h]
		push	ax
		push	si
		xor	ax, ax
		push	ax
		mov	ax, PATH_SIZE
		push	ax
		call	CopyString	; Restore file_entry_buffer

		push	si
		call	FreeMem		; Release save-buf memory

;mov ax, 0
;push ax
;mov ax, 0
;push ax
;call setlocation
;
;mov ax, [bp+16h]
;inc ax
;push ax
;mov ax, reverse_video
;push ax
;mov ax, videosegment
;push ax
;call writestring

		mov	ax, [bp+24h]
		push	ax
		mov	ax, [bp+1ch]
		push	ax
		mov	ax, [bp+20h]
		push	ax
		push	ds
		mov	ax, [bp+16h]
		inc	ax
		push	ax
		call	DelFile

DoneMoveFile:
		pop	bp
		pop	es
		pop	di
		pop	si
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	22
MoveFile	endp


; DELETES THE "SELECTED" FILE ENTRIES FROM THE LOGGED PATH
;	- RETURNS : Nothing
;	- STACK : (ref) error_code, (ref) parent_dir_tag,
;                 (ref) mask_star_dot_star, (val) DTA_segment,
;                 (ref) DTA_offset, (val) active_window,
;                 (ref) update_flags, (val) file_count,
;
;                 (ref) scratch_offset, (ref) drive_number,
;                 (ref) directory, (val) list_block_segment,
;                 (ref) file_entry_buffer, (ref) scratch_buffer, R_IP
;                 AX, BX, CX, DX, SI, DI, ES, BP

Delete		proc	near
		push	ax		; Save registers
		push	bx
		push	cx
		push	dx
		push	si
		push	di
		push	es
		push	bp

		mov	bp, sp
		xor	dx, dx		; Clear flag to tell if any entries
					;    were selected

DeleteLoop:
		mov	ax, SEARCH_SELECTED	; Search for files marked as

DelDefault:
		push	ax			;    selected (not current)
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
		jne	short ProcessDel	; Yes -- delete it
		jmp	DelCurrent		; No -- delete current entry

ProcessDel:
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

;xor ax, ax
;push ax
;push ax
;call setlocation
;
;mov ax, [bp+12h]
;push ax
;mov ax, reverse_video
;push ax
;mov ax, videosegment
;push ax
;call writestring

		inc	dx

		push	es		; Save whatever's there

		mov	ax, [bp+16h]	; Load the segment value
		mov	es, ax		;    of the current list's block
		mov	bx, [bp+1ch]	; Addr of offset of entry in list block
		mov	ax, [bx]	; Offset of entry in list block
		mov	bx, ax		; Point to file entry being processed
		mov	al, byte ptr es:[bx]
		and	al, ATTR_DIR
		cmp	al, ATTR_DIR	; Is this entry for a directory ?

		je	short DelDirEntry	; Yes -- branch REMOVE DIR sect

		mov	ax, [bp+2ch]
		push	ax
		mov	ax, [bp+22h]
		push	ax
		mov	ax, [bp+20h]
		push	ax
		push	ds
		mov	ax, [bp+12h]		; Offset of scratch_buffer
		inc	ax			; Skip over leading SPACE
		push	ax
		call	DelFile			; Wipe the file

		jmp	DelFilEntry

DelDirEntry:
		mov	al, es:[bx+FILENAME_OFFS]	; Get filename
		cmp	al, '.'			; Is file '.' or '..' ?
		je	short DelFilEntry	; Yes -- skip it

		mov	ax, DIR_REC_SIZE
		push	ax
		call	AllocMem
		mov	si, ax			; Get memory to save
						;    file_entry_buffer

		push	si
		xor	ax, ax
		push	ax
		push	ds
		mov	ax, [bp+14h]
		push	ax
		mov	ax, DIR_REC_SIZE
		push	ax
		call	CopyString		; Save file_entry_buffer

		mov	ax, [bp+2ch]
		push	ax
		mov	ax, [bp+2ah]
		push	ax
		mov	ax, [bp+28h]
		push	ax
		mov	ax, [bp+22h]
		push	ax
		mov	ax, [bp+20h]
		push	ax
		mov	ax, [bp+12h]		; Offset of scratch_buffer
		inc	ax			; Skip over leading SPACE
		push	ax
		mov	ax, [bp+26h]
		push	ax
		mov	ax, [bp+24h]
		push	ax
		call	DelDir			; Recur to next level below

		push	ds
		mov	ax, [bp+14h]
		push	ax
		push	si
		xor	ax, ax
		push	ax
		mov	ax, DIR_REC_SIZE
		push	ax
		call	CopyString	; Restore file_entry_buffer

		push	si
		call	FreeMem		; Deallocate save-buffer memory

DelFilEntry:
		mov	byte ptr es:[bx+ENTRY_LENGTH-03h], NULL
					; Reset the SELECTION byte
					;    (So that this file is not "hit"
					;        by the search routine again)

		pop	es		; Get back original value

DelResume:
		jmp	short DeleteLoop	; Check for more selections

DelCurrent:
		cmp	dx, NULL		; If 0, no entries were sel'd
		jne	short DoneDelete

		mov	ax, SEARCH_CURRENT	; Pick the curr entry for del
		jmp	short DelDefault

DoneDelete:
		pop	bp		; Restore registers
		pop	es
		pop	di
		pop	si
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	28
Delete		endp


; REMOVES A DIRECTORY TREE FROM THE DISK STRUCTURE
;	- RETURNS : Nothing
;	- STACK : (ref) error_code, (ref) parent_directory_tag,
;                 (ref) mask_star_dot_star, (val) active_window,
;                 (ref) update_flags, (ref) directory_to_remove,
;                 (val) current_DTA_segment, (ref) current_DTA_offset, R_IP
;                 AX, BX, CX, SI, DI, ES, BP

DelDir		proc	near
		push	ax
		push	bx
		push	cx
		push	si
		push	di
		push	es
		push	bp

		mov	bp, sp

		mov	ax, [bp+18h]
		push	ax
		mov	ax, [bp+16h]
		push	ax
		mov	ax, [bp+14h]
		push	ax
		call	ChangeDirectory		; Change to the dir to delete

;push ax
;mov ax, 0
;push ax
;mov ax, 0
;push ax
;call setlocation
;
;mov ax, [bp+14h]
;push ax
;mov ax, reverse_video
;push ax
;mov ax, videosegment
;push ax
;call writestring
;pop ax

		mov	ax, [bp+12h]		; Load segment of DTA
		mov	es, ax
		mov	bx, [bp+10h]		; Load offset of DTA

		mov	ax, [bp+1ah]		; Search for files matching
		push	ax			;    '*.*' filemask
		call	GetFirstFile		; Get the first one

Reset_Wipe:
		cmp	ax, ERROR_FIL_NMORE	; Any more files ?
		jne	short DoDelDirFil	; Yes -- waste them !
		jmp	DoneDelDir		; No -- end this routine

DoDelDirFil:
		mov	al, es:[bx+DTA_DATA_OFS]	; Get file's attr
		and	al, ATTR_DIR
		cmp	al, ATTR_DIR		; Is entry for a dir ?
		je	short DelDirNext	; Yes -- prep for recursion
		jmp	DelFilNext		; No -- delete the file

DelDirNext:
		mov	al, es:[bx+DTA_DATA_OFS+FILENAME_OFFS]
		cmp	al, '.'			; Is it '.' or '..' ?
		jne	short DelDirValidDir	; No -- we've got a valid dir !
		jmp	DelDirNextEntry		; Don't even try to remove it

DelDirValidDir:
;;; ************* -------->> if you want to display the names of the
;;; ************* -------->> directories that are being deleted,
;;; ************* -------->> get the filename from the DTA at this point

;push ax
;mov ax, 40
;push ax
;mov ax, 0
;push ax
;call setlocation
;
;push bx
;mov ax, [bp+14h]
;push ax
;mov ax, reverse_video
;push ax
;mov ax, videosegment
;push ax
;call writestring
;pop bx
;pop ax

		mov	ax, PATH_SIZE
		push	ax
		call	AllocMem
		mov	si, ax			; Get memory to save
						;    scratch_buffer

		push	si
		xor	ax, ax
		push	ax
		push	ds
		mov	ax, [bp+14h]
		push	ax
		mov	ax, PATH_SIZE
		push	ax
		call	CopyString		; Save dir_to_remove
						;    (scratch_buffer)

		mov	ax, DIR_REC_SIZE
		push	ax
		call	AllocMem
		mov	di, ax			; Get mem to save old DTA data

		push	di
		xor	ax, ax
		push	ax
		mov	ax, [bp+12h]
		push	ax
		mov	ax, [bp+10h]
		push	ax
		mov	ax, DIR_REC_SIZE
		push	ax
		call	CopyString		; Save the DTA data

		push	ds
		mov	ax, [bp+14h]
		push	ax
		push	es
		mov	ax, bx
		add	ax, DTA_DATA_OFS + FILENAME_OFFS
		push	ax
		mov	ax, FILENAME_LEN
		push	ax
		call	CopyString		; Copy next-level dir-entry
						;    from DTA into scratch_buf

		mov	ax, [bp+1eh]
		push	ax
		mov	ax, [bp+1ch]
		push	ax
		mov	ax, [bp+1ah]
		push	ax
		mov	ax, [bp+18h]
		push	ax
		mov	ax, [bp+16h]
		push	ax
		mov	ax, [bp+14h]
		push	ax
		mov	ax, [bp+12h]
		push	ax
		mov	ax, [bp+10h]
		push	ax
		call	DelDir			; Recur to delete lower dirs

		mov	ax, [bp+12h]
		push	ax
		mov	ax, [bp+10h]
		push	ax
		push	di
		xor	ax, ax
		push	ax
		mov	ax, DIR_REC_SIZE
		push	ax
		call	CopyString		; Restore old DTA data

		push	di
		call	FreeMem			; Release save-buffer memory

		push	ds
		mov	ax, [bp+14h]
		push	ax
		push	si
		xor	ax, ax
		push	ax
		mov	ax, PATH_SIZE
		push	ax
		call	CopyString		; Restore dir_to_remove
						;    (scratch_buffer)

		push	si
		call	FreeMem			; Deallocate save-buffer mem

		jmp	short DelDirNextEntry

DelFilNext:
;;; ************* -------->> if you want to display the names of the files
;;; ************* -------->> in the directories that are being deleted,
;;; ************* -------->> get the filename from the DTA at this point

;mov ax, 0
;push ax
;mov ax, 24
;push ax
;call setlocation
;
;push bx
;mov ax, bx
;add ax, dta_data_ofs + filename_offs
;push ax
;mov ax, reverse_video
;push ax
;mov ax, videosegment
;push ax
;call writestring
;pop bx

		mov	ax, [bp+1eh]
		push	ax
		mov	ax, [bp+18h]
		push	ax
		mov	ax, [bp+16h]
		push	ax
		push	es
		mov	ax, bx
		add	ax, DTA_DATA_OFS + FILENAME_OFFS
		push	ax
		call	DelFile			; Wipe the file

DelDirNextEntry:
		call	GetNextFile

		jmp	Reset_Wipe

DoneDelDir:
		mov	ax, [bp+18h]
		push	ax
		mov	ax, [bp+16h]
		push	ax
		mov	ax, [bp+1ch]		; Load filemask '..'
		push	ax
		call	ChangeDirectory		; Change back to parent dir

		mov	ax, [bp+1eh]
		push	ax
		push	ds
		mov	ax, [bp+14h]
		push	ax
		call	RemoveDirectory		; Waste the directory spec'd

		pop	bp			; Restore registers
		pop	es
		pop	di
		pop	si
		pop	cx
		pop	bx
		pop	ax
		ret	16
DelDir		endp


; READS AS MUCH OF A FILE INTO MEMORY AS POSSIBLE
;	- RETURNS : Nothing
;	- STACK : (ref) error_code,
;                 (val) file_handle, (val) block_size_in_bytes,
;                 (ref) memory_avail, (val) prev_node_of_linked_list, R_IP
;                 AX, BX, SI, DI, ES, BP
; NB ! prev_node_of_linked_list must be a previously-allocated memory block

ReadRecur	proc	near
		push	ax
		push	bx
		push	si
		push	di
		push	es
		push	bp

		mov	bp, sp

		mov	ax, [bp+0eh]
		mov	es, ax

		mov	ax, [bp+16h]
		push	ax
		push	es
		mov	ax, BUF_HEADER_LEN
		push	ax
		mov	ax, [bp+12h]
		sub	ax, BUF_HEADER_LEN
		push	ax
		mov	ax, [bp+14h]
		push	ax
		call	ReadFile

		mov	bx, [bp+16h]
		mov	ax, [bx]
		mov	bx, BUF_HEADER_LEN-02h
		mov	es:[bx], ax
		mov	bx, BUF_HEADER_LEN-04h
		mov	word ptr es:[bx], END_OF_LIST

		mov	di, [bp+12h]
		sub	di, BUF_HEADER_LEN
		cmp	ax, di
		jb	short DoneRRecur

		mov	ax, [bp+12h]
		push	ax
		call	AllocMem
		mov	si, ax
		cmp	si, NOT_ALLOC_FLAG
		jne	short CopyRecOKAlloc

		mov	bx, [bp+16h]
		mov	word ptr [bx], NOT_TRANSFERRED
		jmp	short DoneRRecur

CopyRecOKAlloc:
		mov	bx, BUF_HEADER_LEN-04h
		mov	es:[bx], si
		push	si
		pop	es
		mov	word ptr es:[bx], END_OF_LIST

		mov	ax, [bp+16h]
		push	ax
		mov	ax, [bp+14h]
		push	ax
		mov	ax, [bp+12h]
		push	ax
		mov	ax, [bp+10h]
		push	ax
		push	si
		call	ReadRecur

DoneRRecur:
		pop	bp
		pop	es
		pop	di
		pop	si
		pop	bx
		pop	ax
		ret	10
ReadRecur	endp


; WRITES ALL FILE DATA IN MEMORY TO DISK FILES
;	- RETURNS : Nothing
;	- STACK : (ref) error_code,
;                 (val) file_handle, (val) block_size_in_bytes,
;                 (ref) memory_avail, (val) first_node_of_linked_list, R_IP
;                 AX, BX, SI, DI, ES, BP
; NB ! first_node_of_linked_list must be a previously-allocated memory block

WriteRecur	proc	near
		push	ax
		push	bx
		push	si
		push	di
		push	es
		push	bp

		mov	bp, sp

		mov	ax, [bp+0eh]
		mov	es, ax

		mov	bx, BUF_HEADER_LEN-02h
		mov	di, es:[bx]

		mov	ax, [bp+16h]
		push	ax
		push	es
		mov	ax, BUF_HEADER_LEN
		push	ax
		push	di
		mov	ax, [bp+14h]
		push	ax
		call	WriteFile

		mov	di, [bp+16h]
		mov	ax, [di]
		mov	bx, BUF_HEADER_LEN-02h
		mov	di, es:[bx]
		cmp	ax, di
		je	short WroteAllData
		jmp	short DoneWRecur

WroteAllData:
		mov	bx, BUF_HEADER_LEN-04h
		mov	si, es:[bx]
		cmp	si, END_OF_LIST
		je	short DoneWRecur

		mov	ax, [bp+16h]
		push	ax
		mov	ax, [bp+14h]
		push	ax
		mov	ax, [bp+12h]
		push	ax
		mov	ax, [bp+10h]
		push	ax
		push	si
		call	WriteRecur

		push	si
		call	FreeMem

DoneWRecur:
		pop	bp
		pop	es
		pop	di
		pop	si
		pop	bx
		pop	ax
		ret	10
WriteRecur	endp
