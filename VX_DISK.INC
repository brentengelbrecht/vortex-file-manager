; RENAMES A FILE ON THE SAME DISK
;	- RETURNS : Nothing
;	- STACK : (ref) error_code,
;                 (ref) update_flags, (val) active_window,
;                 (val) old_file_spec_seg, (ref) new_file_spec_ofs, R_IP
;                 AX, BX, DX, DI, DS, ES, BP

RenameFile	proc	near
		push	ax
		push	bx
		push	dx
		push	di
		push	ds
		push	es
		push	bp

		mov	bp, sp

		push	ds
		pop	es

		mov	di, [bp+10h]
		mov	dx, [bp+12h]
		mov	ds, dx
		xor	dx, dx
		mov	ah, 56h
		int	21h

		push	es
		pop	ds

		jnc	short RenameFilOK

		mov	bx, [bp+18h]
		mov	word ptr [bx], ax

RenameFilOK:
		mov	bx, [bp+18h]
		mov	word ptr [bx], NULL

		mov	bx, [bp+16h]
		mov	di, [bp+14h]
		or	byte ptr [bx][di], UPD_RELOAD
		or	byte ptr [bx][di], UPD_RESET
		not	di
		and	di, 01h
		or	byte ptr [bx][di], UPD_RELOAD
		or	byte ptr [bx][di], UPD_RESET

DoneRenameFile:
		pop	bp
		pop	es
		pop	ds
		pop	di
		pop	dx
		pop	bx
		pop	ax
		ret	10
RenameFile	endp


; WRITES A SPECIFIED NUMBER OF BYTES FROM A BUFFER TO A FILE
;	- RETURNS : Nothing
;	- STACK : (ref) error_code,
;                 (val) buffer_segment, (ref) buffer_offset,
;                 (val) count_of_bytes, (val) file_handle, R_IP
;                 AX, BX, CX, DX, DS, BP

WriteFile	proc	near
		push	ax
		push	bx
		push	cx
		push	dx
		push	ds
		push	bp

		mov	bp, sp

		mov	bx, [bp+0eh]
		mov	cx, [bp+10h]
		mov	dx, [bp+12h]
		mov	ax, [bp+14h]
		mov	ds, ax
		mov	ah, 40h
		int	21h

		jnc	short DoneWriteFil

		mov	ax, FILE_WRITE_FAILED

DoneWriteFil:
		mov	bx, [bp+16h]
		mov	cs:[bx], ax

		pop	bp
		pop	ds
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	10
WriteFile	endp


; OPENS A FILE WITH READ-ONLY / READ-WRITE / WRITE-ONLY ACCESS
;	- RETURNS : handle = file handle of opened file
;	- STACK : (val) file_attributes, (ref) handle,
;                 (ref) path_to_file, (val) access_mode, R_IP
;                 AX, BX, CX, DX, DS, BP
; NB ! For read-only / read-write access, attributes are ignored

OpenFile	proc	near
		push	ax
		push	bx
		push	cx
		push	dx
		push	ds
		push	bp

		mov	bp, sp

		mov	ax, [bp+0eh]
		cmp	ax, WRITE_ONLY
		je	short CreateFile

		mov	dx, [bp+10h]
		mov	ax, [bp+0eh]
		mov	ah, 3dh
		int	21h

		jmp	short CheckOpenErrors

CreateFile:
		mov	dx, [bp+10h]
		mov	cx, [bp+14h]
		mov	ah, 3ch
		int	21h

CheckOpenErrors:
		jnc	short DoneOpen

		cmp	ax, 01h
		jne	short NotOpen1

		mov	ax, ERROR_FIL_FUNC
		jmp	short NotOpen12

NotOpen1:
		cmp	ax, 02h
		jne	short NotOpen2

		mov	ax, ERROR_FIL_INVAL
		jmp	short NotOpen12

NotOpen2:
		cmp	ax, 03h
		jne	short NotOpen3

		mov	ax, ERROR_FIL_PATH
		jmp	short NotOpen12

NotOpen3:
		cmp	ax, 04h
		jne	short NotOpen4

		mov	ax, ERROR_FIL_NHAND
		jmp	short NotOpen12

NotOpen4:
		cmp	ax, 05h
		jne	short NotOpen5

		mov	ax, ERROR_FIL_ACCDEN
		jmp	short NotOpen12

NotOpen5:
		cmp	ax, 0ch
		jne	short NotOpen12

		mov	ax, ERROR_FIL_MODE

NotOpen12:
;		push	ax
;		call	ErrorProc
		mov	ax, FILE_OPEN_FAILED

DoneOpen:
		mov	bx, [bp+12h]
		mov	[bx], ax

		pop	bp
		pop	ds
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	8
OpenFile	endp


; CLOSES A FILE
;	- RETURNS : Nothing
;	- STACK : (val) file_handle, R_IP
;                 AX, BX, BP

CloseFile	proc	near
		push	ax
		push	bx
		push	bp

		mov	bp, sp

		mov	bx, [bp+08h]
		mov	ah, 3eh
		int	21h

		jnc	short DoneClose

		mov	ax, ERROR_FIL_BHAND
		push	ax
		call	ErrorProc

DoneClose:
		pop	bp
		pop	bx
		pop	ax
		ret	2
CloseFile	endp


; CREATES THE SPECIFIED DIRECTORY-PATH
;	- RETURNS : Nothing
;	- STACK : (ref) error_code, (val) attribute,
;                 (val) segment_of_pathname, (ref) offset_of_pathname, R_IP
;                 AX, BX, DX, DS, BP

MakeDirectory	proc	near
		push	ax
		push	bx
		push	dx
		push	ds
		push	bp

		mov	bp, sp

		mov	dx, [bp+0ch]
		mov	ax, [bp+0eh]
		mov	ds, ax
		mov	ah, 39h
		int	21h

		push	cs
		pop	ds

		jnc	short SetDirAttr

		mov	bx, [bp+12h]
		mov	[bx], ax
		jmp	short DoneMakeDir

SetDirAttr:
		mov	bx, [bp+12h]
		mov	word ptr [bx], NULL

		mov	ax, [bp+12h]
		push	ax
		mov	ax, [bp+10h]
		push	ax
		mov	ax, [bp+0eh]
		push	ax
		mov	ax, [bp+0ch]
		push	ax
		call	SetAttr

DoneMakeDir:
		pop	bp
		pop	ds
		pop	dx
		pop	bx
		pop	ax
		ret	8
MakeDirectory	endp


; GETS A FILE'S ATTRIBUTE
;	- RETURNS : Nothing
;	- STACK : (ref) error_code, (ref) old_attr,
;                 (val) segment_of_filespec, (ref) offset_of_filespec, R_IP
;                 AX, CX, DX, DS, BP

GetAttr		proc	near
		push	ax
		push	cx
		push	dx
		push	ds
		push	bp

		mov	bp, sp

		mov	dx, [bp+0ch]
		mov	ax, [bp+0eh]
		mov	ds, ax
		xor	al, al
		mov	ah, 43h
		int	21h

		jnc	short GotAttrOK

		cmp	ax, 01h
		jne	short NotGetAttr1

		mov	ax, ERROR_FIL_FUNC
		jmp	short NotGetAttr5

NotGetAttr1:
		cmp	ax, 02h
		jne	short NotGetAttr2

		mov	ax, ERROR_FIL_INVAL
		jmp	short NotGetAttr5

NotGetAttr2:
		cmp	ax, 03h
		jne	short NotGetAttr3

		mov	ax, ERROR_FIL_PATH
		jmp	short NotGetAttr5

NotGetAttr3:
		cmp	ax, 05h
		jne	short NotGetAttr5

		mov	ax, ERROR_FIL_ACCDEN

NotGetAttr5:
		mov	bx, [bp+12h]
		mov	cs:[bx], ax
		jmp	short DoneGetAttr

GotAttrOK:
		mov	bx, [bp+12h]
		mov	word ptr cs:[bx], NULL
		mov	bx, [bp+10h]
		mov	cs:[bx], cx

DoneGetAttr:
		pop	bp
		pop	ds
		pop	dx
		pop	cx
		pop	ax
		ret	8
GetAttr		endp


; SETS A FILE'S ATTRIBUTE
;	- RETURNS : Nothing
;	- STACK : (ref) error_code, (val) new_attr,
;                 (val) segment_of_filespec, (ref) offset_of_filespec, R_IP
;                 AX, CX, DX, DS, BP

SetAttr		proc	near
		push	ax
		push	cx
		push	dx
		push	ds
		push	bp

		mov	bp, sp

		mov	dx, [bp+0ch]
		mov	cx, [bp+10h]
		mov	ax, [bp+0eh]
		mov	ds, ax
		mov	al, 01h
		mov	ah, 43h
		int	21h

		jnc	short SetAttrOK

		cmp	ax, 01h
		jne	short NotSetAttr1

		mov	ax, ERROR_FIL_FUNC
		jmp	short NotSetAttr5

NotSetAttr1:
		cmp	ax, 02h
		jne	short NotSetAttr2

		mov	ax, ERROR_FIL_INVAL
		jmp	short NotSetAttr5

NotSetAttr2:
		cmp	ax, 03h
		jne	short NotSetAttr3

		mov	ax, ERROR_FIL_PATH
		jmp	short NotSetAttr5

NotSetAttr3:
		cmp	ax, 05h
		jne	short NotSetAttr5

		mov	ax, ERROR_FIL_ACCDEN

NotSetAttr5:
		mov	bx, [bp+12h]
		mov	cs:[bx], ax
		jmp	short DoneSetAttr

SetAttrOK:
		mov	bx, [bp+12h]
		mov	word ptr cs:[bx], NULL

DoneSetAttr:
		pop	bp
		pop	ds
		pop	dx
		pop	cx
		pop	ax
		ret	8
SetAttr		endp


; READS A SPECIFIED NUMBER OF BYTES FROM A FILE INTO A BUFFER
;	- RETURNS : Nothing
;	- STACK : (ref) error_code,
;                 (val) buffer_segment, (ref) buffer_offset,
;                 (val) count_of_bytes, (val) file_handle, R_IP
;                 AX, BX, CX, DX, DS, BP

ReadFile	proc	near
		push	ax
		push	bx
		push	cx
		push	dx
		push	ds
		push	bp

		mov	bp, sp

		mov	bx, [bp+0eh]
		mov	cx, [bp+10h]
		mov	dx, [bp+12h]
		mov	ax, [bp+14h]
		mov	ds, ax
		mov	ah, 3fh
		int	21h

		jnc	short DoneReadFil

		mov	ax, FILE_READ_FAILED

DoneReadFil:
		mov	bx, [bp+16h]
		mov	cs:[bx], ax

		pop	bp
		pop	ds
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	10
ReadFile	endp


; SETS A FILE HANDLE'S DATE AND TIME STAMP TO THOSE SPECIFIED
;	- RETURNS : Nothing
;	- STACK : (ref) error_code, (val) file_date,
;                 (val) file_time, (val) file_handle, R_IP
;                 AX, BX, CX, DX, BP

SetDateTime	proc	near
		push	ax
		push	bx
		push	cx
		push	dx
		push	bp

		mov	bp, sp

		mov	cx, [bp+0eh]
		mov	dx, [bp+10h]
		mov	bx, [bp+0ch]
		mov	al, 01h
		mov	ah, 57h
		int	21h

		jnc	short SetDateOK

		mov	bx, [bp+12h]
		mov	[bx], ax
		jmp	short DoneSetDate

SetDateOK:
		mov	bx, [bp+12h]
		mov	word ptr [bx], NULL

DoneSetDate:
		pop	bp
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	8
SetDateTime	endp


; RETURNS A FILE HANDLE'S DATE AND TIME STAMP
;	- RETURNS : Nothing
;	- STACK : (ref) error_code, (ref) file_date,
;                 (ref) file_time, (val) file_handle, R_IP
;                 AX, BX, CX, DX, BP

GetDateTime	proc	near
		push	ax
		push	bx
		push	cx
		push	dx
		push	bp

		mov	bp, sp

		mov	bx, [bp+0ch]
		xor	al, al
		mov	ah, 57h
		int	21h

		jnc	short GotDateOK

		mov	bx, [bp+12h]
		mov	[bx], ax
		jmp	short DoneGetDate

GotDateOK:
		mov	bx, [bp+12h]
		mov	word ptr [bx], NULL
		mov	bx, [bp+10h]
		mov	[bx], dx
		mov	bx, [bp+0eh]
		mov	[bx], cx

DoneGetDate:
		pop	bp
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret	8
GetDateTime	endp


; REMOVES A DIRECTORY FROM A TREE
;	- RETURNS : error_code = ERROR_DIR_NEMPTY 3 or more files still exist
;	- STACK : (ref) error_code,
;                 (val) segment_of_dir, (ref) offset_of_dir, R_IP
;                 AX, BX, DX, DS, BP

RemoveDirectory	proc	near
		push	ax
		push	bx
		push	dx
		push	ds
		push	bp

		mov	bp, sp

		mov	ax, [bp+0eh]
		mov	ds, ax
		mov	dx, [bp+0ch]
		mov	ah, 3ah
		int	21h

		jc	short BadRemDir
		xor	ax, ax
		jmp	short SuccessRD

BadRemDir:
		cmp	ax, 05h
		jne	short RDError

		mov	ax, ERROR_DIR_NEMPTY
		jmp	short SuccessRD

RDError:
		push	ax
		call	ErrorProc

SuccessRD:
		push	cs
		pop	ds

		mov	bx, [bp+10h]
		mov	[bx], ax

		pop	bp
		pop	ds
		pop	dx
		pop	bx
		pop	ax
		ret	6
RemoveDirectory	endp


; DELETES A FILE FROM A DIRECTORY ON DISK
;	- RETURNS : Nothing
;	- STACK : (ref) error_code,
;                 (val) active_window, (ref) update_flags,
;                 (val) file_spec_segment, (ref) file_spec_offset, R_IP
;                 AX, BX, DX, SI, DS, BP

DelFile		proc	near
		push	ax
		push	bx
		push	dx
		push	si
		push	ds
		push	bp

		mov	bp, sp

		mov	ax, [bp+16h]
		push	ax
		xor	ax, ax
		push	ax
		mov	ax, [bp+10h]
		push	ax
		mov	ax, [bp+0eh]
		push	ax
		call	SetAttr

		mov	ax, [bp+10h]
		mov	ds, ax
		mov	dx, [bp+0eh]
		mov	ah, 41h
		int	21h

		jnc	short DoneDelFile

		cmp	ax, 02h
		jne	short DelFNotFil

		mov	ax, ERROR_FIL_INVAL
		jmp	short DelFNotAcc

DelFNotFil:
		cmp	ax, 03h
		jne	short DelFNotPath

		mov	ax, ERROR_FIL_PATH
		jmp	short DelFNotAcc

DelFNotPath:
		cmp	ax, 05h
		jne	short DelFNotAcc

		mov	ax, ERROR_FIL_ACCDEN
		jmp	short DelFNotAcc

DelFNotAcc:
		push	ax
		call	ErrorProc

DoneDelFile:
		mov	bx, [bp+12h]
		mov	si, [bp+14h]
		or	byte ptr [bx][si], UPD_RELOAD
		or	byte ptr [bx][si], UPD_RESET

		pop	bp
		pop	ds
		pop	si
		pop	dx
		pop	bx
		pop	ax
		ret	10
DelFile		endp


; DETERMINES THE CURRENTLY-LOGGED DIRECTORY FOR A SPECIFIC DRIVE
;	- RETURNS : Nothing
;	- STACK : (val) buffer_seg, (ref) buffer_offset, (val) drv, R_IP
;                 AX, DX, DS, SI, BP

GetDefaultDir	proc	near
		push	ax
		push	dx
		push	ds
		push	si
		push	bp

		mov	bp, sp

		xor	ax, ax
		push	ax
		mov	ax, [bp+10h]
		push	ax
		mov	ax, [bp+0eh]
		push	ax
		mov	ax, PATH_SIZE
		push	ax
		call	InitBuffer

		mov	dx, [bp+0ch]
		mov	si, [bp+0eh]
		mov	ax, [bp+10h]
		mov	ds, ax

		mov	ah, 47h
		int	21h

		pop	bp
		pop	si
		pop	ds
		pop	dx
		pop	ax
		ret	6
GetDefaultDir	endp


; DETERMINES THE CURRENTLY-LOGGED DRIVE IN THE SYSTEM
;	- RETURNS : drive = # of active drive (1=A:, 2=B:, 3=C:, etc)
;	- STACK : (ref) drive, R_IP
;                 AX, BX, BP

GetDefaultDrv	proc	near
		push	ax
		push	bx
		push	bp

		mov	bp, sp

		mov	ah, 19h
		int	21h

		mov	bx, [bp+08h]
		xor	ah, ah
		inc	al
		mov	[bx], ax

		pop	bp
		pop	bx
		pop	ax
		ret	2
GetDefaultDrv	endp


; LOGS TO THE SPECIFIED DRIVE
;	- RETURNS : Nothing
;	- STACK : (val) new_drive, R_IP
;                 AX, DX, BP

ChangeDrive	proc	near
		push	ax			; Save registers
		push	dx
		push	bp

		mov	bp, sp

		mov	dx, [bp+08h]		; Get new drive # (1-based)
		dec	dx			; Make it zero-based
		mov	ah, 0eh			; Request set_default_drive
		int	21h			; Call DOS

		pop	bp			; Restore registers
		pop	dx
		pop	ax
		ret	2
ChangeDrive	endp


; MAKES THE SPECIFIED DIRECTORY THE DEFAULT ONE
;	- RETURNS : Nothing
;	- STACK : (val) active_window,
;                 (ref) update_flags, (ref) new_path_string, R_IP
;                 AX, BX, DX, SI, BP

ChangeDirectory	proc	near
		push	ax			; Save registers
		push	bx
		push	dx
		push	si
		push	bp

		mov	bp, sp

		mov	dx, [bp+0ch]		; Offset of new dir string
		mov	ah, 3bh			; Request set_default_dir
		int	21h			; Call DOS

		jnc	short ChgDirUpdate	; If no errors -- jump

		mov	ax, ERROR_FIL_PATH
		push	ax
		call	ErrorProc

ChgDirUpdate:
		mov	bx, [bp+0eh]		; Load offset of update_flags
		mov	si, [bp+10h]		; Load value of active_window
		mov	al, byte ptr [bx][si]	; Get the flag-set for curr win
		or	al, UPD_NEWPATH		; Indicate new path
		mov	byte ptr [bx][si], al	; Save flag-set

DoneChange:
		pop	bp			; Restore registers
		pop	si
		pop	dx
		pop	bx
		pop	ax
		ret	6
ChangeDirectory	endp


; GETS THE FIRST FILE FOR A FILESPEC IF IT EXISTS
;	- RETURNS : AX = DOS' error code
;	- STACK : (ref) filespec, R_IP
;                 BX, CX, DX, BP

GetFirstFile	proc	near
		push	bx
		push	cx
		push	dx
		push	bp

		mov	bp, sp

		mov	dx, [bp+0ah]
		mov	cx, ANY_FILE - ATTR_VOL
		mov	ah, 4eh
		int	21h

		jnc	short FoundOK
		cmp	ax, 02h
		jne	short GetFilePath
		mov	ax, ERROR_FIL_INVAL
		push	ax
		jmp	short GetFileErr

GetFilePath:
		cmp	ax, 03h
		jne	short GetFileNoMore
		mov	ax, ERROR_FIL_PATH
		push	ax
		jmp	short GetFileErr

GetFileNoMore:
		mov	ax, ERROR_FIL_NMORE
		jmp	short FoundOK

GetFileErr:
		call	ErrorProc

FoundOK:
		pop	bp
		pop	dx
		pop	cx
		pop	bx
		ret	2
GetFirstFile	endp


; GETS THE NEXT FILE FOR A FILESPEC IF IT EXISTS
;	- RETURNS : AX = DOS' error code
;	- STACK : Nothing

GetNextFile	proc	near
		mov	ah, 4fh
		int	21h

		jnc	short FoundNextOK
		cmp	ax, 02h
		jne	short GetNextPath
		mov	ax, ERROR_FIL_INVAL
		push	ax
		jmp	short GetNextErr

GetNextPath:
		cmp	ax, 03h
		jne	short GetNextNoMore
		mov	ax, ERROR_FIL_PATH
		push	ax
		jmp	short GetNextErr

GetNextNoMore:
		mov	ax, ERROR_FIL_NMORE
		jmp	short FoundNextOK

GetNextErr:
		call	ErrorProc

FoundNextOK:
		ret
GetNextFile	endp
