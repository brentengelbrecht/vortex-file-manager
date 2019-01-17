; RESTORES THE CONTENTS OF THE USER'S (DOS) SCREEN AND CURSOR INFO
;	- RETURNS : Nothing
;	- STACK : (ref) save_parameters_block, (val) video_seg, R_IP
;                 AX, BX, BP

RestUserScreen	proc	near
		push	ax
		push	bx
		push	bp

		mov	bp, sp

		mov	ax, 0000h
		push	ax
		mov	ax, 0000h
		push	ax
		mov	ax, 0050h
		push	ax
		mov	ax, 0019h
		push	ax
		mov	bx, [bp+0ah]
		mov	ax, [bx]
		push	ax
		mov	ax, [bp+08h]
		push	ax
		call	RestoreWindow

		mov	bx, [bp+0ah]
		mov	dx, [bx+02h]
		xor	ah, ah
		mov	al, dl
		push	ax
		mov	al, dh
		push	ax
		call	SetCursor

		push	bp

;		push	bx
;		push	dx
;		mov	dx, [bx+02h]
;		mov	bh, 00h
;		mov	ah, 02h
;		int	10h
;		pop	dx
;		pop	bx

		push	bx
		push	cx
		mov	cx, [bx+04h]
		mov	bh, 00h
		mov	ah, 01h
		int	10h

		pop	cx
		pop	bx
		pop	bp

		mov	bx, [bp+0ah]
		mov	ax, [bx]
		push	ax
		call	FreeMem

		cmp	ax, 0000h
		jne	short DoneUserRest

		mov	ax, ERROR_USE_DEALL
		push	ax
		call	ErrorProc

DoneUserRest:
		pop	bp
		pop	bx
		pop	ax
		ret	4
RestUserScreen	endp


; SAVES THE CONTENTS OF THE USER'S (DOS) SCREEN AND CURSOR INFO
;	- RETURNS : Nothing
;	- STACK : (ref) save_parameters_block, (val) video_seg, R_IP
;                 AX, BX, BP

SaveUserScreen	proc	near
		push	ax
		push	bx
		push	bp

		mov	bp, sp

		mov	ax, SCREEN_SIZE
		push	ax
		call	AllocMem
		mov	bx, [bp+0ah]
		mov	[bx], ax
		cmp	ax, 0000h
		jne	short SaveUserOK

		mov	ax, ERROR_USE_ALLOC
		push	ax
		call	ErrorProc
		jmp	short DoneUserSave

SaveUserOK:
		xor	ax, ax
		push	ax
		push	ax
		mov	ax, 0050h
		push	ax
		mov	ax, 0019h
		push	ax
		mov	bx, [bp+0ah]
		mov	ax, [bx]
		push	ax
		mov	ax, [bp+08h]
		push	ax
		call	SaveWindow

		push	sp
		push	bp
		push	si
		push	di
		push	bx
		mov	bh, 00h
		mov	ah, 03h
		int	10h
		pop	bx
		pop	di
		pop	si
		pop	bp
		pop	sp
		mov	[bx+02h], dx
		mov	[bx+04h], cx

DoneUserSave:
		pop	bp
		pop	bx
		pop	ax
		ret	4
SaveUserScreen	endp
