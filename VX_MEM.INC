; ALLOCATES A BLOCK OF MEMORY FROM HEAP
;	- RETURNS : AX = segment address of memory block
;	- STACK : (val) number_of_bytes_to_allocate, R_IP
;                 BX, CX, BP

AllocMem	proc	near
		push	bx			; Saved registers we'll use
		push	cx
		push	bp

		mov	bp, sp			; Use BP to index stack

		mov	bx, [bp+08h]		; Load # of bytes to allocate
		mov	cl, 04h			; Prepare for DIV 16
		shr	bx, cl			; Convert to paragraphs
		inc	bx			; Overshoot by 16 bytes

		mov	ah, 48h			; Request allocate_memory
		int	21h			; Call DOS

		jnc	short AllocBlockOK	; If no error, continue
		mov	ax, NOT_ALLOC_FLAG	; Indicate error condition

AllocBlockOK:
		lea	bx, MaxAvail		; Load addr of 'mem-free' var
		push	bx			; Refernce pass
		call	GetMemFree		; New amount of free mem

		pop	bp			; Restore used registers
		pop	cx
		pop	bx
		ret	2
AllocMem	endp


; RELEASES A BLOCK OF ALLOCATED MEMORY
;	- RETURNS : Nothing
;	- STACK : (val) segment_to_release, R_IP
;                 BX, ES, BP

FreeMem		proc	near
		push	bx			; Save registers
		push	es
		push	bp

		mov	bp, sp			; Index stack with BP

		mov	ax, [bp+08h]		; Get segment to free
		mov	es, ax			;    and load in ES

		mov	ah, 49h			; Request free_memory
		int	21h			; Call DOS

		jnc	short FreeOK		; If no errors, then continue
		push	ax			; Send DOS' error code
		call	ErrorProc		; Display proper message
		mov	ax, NOT_ALLOC_FLAG	; Indicate error condition

FreeOK:
		lea	bx, MaxAvail		; Load addr of return variable
		push	bx			; Pass by reference
		call	GetMemFree		; Get new amount of free mem

		pop	bp			; Restore registers
		pop	es
		pop	bx
		ret	2
FreeMem		endp


; CHANGES THE SIZE OF AN ALLOCATED MEMORY BLOCK
;	- RETURNS : Nothing
;	- STACK : (ref) error_code,
;                 (val) segment_to_resize, (val) new_size, R_IP
;                 AX, BX, ES, BP
		
ResizeMem	proc	near
		push	ax
		push	bx
		push	es
		push	bp

		mov	bp, sp

		mov	bx, [bp+0ah]
		mov	ax, [bp+0ch]
		mov	es, ax
		mov	ah, 4ah
		int	21h

		jnc	short ResizedOK

		mov	bx, [bp+0eh]
		mov	[bx], ax
		jmp	short DoneResize

ResizedOK:
		mov	bx, [bp+0eh]
		mov	word ptr [bx], NULL

DoneResize:
		pop	bp
		pop	es
		pop	bx
		pop	ax
		ret	6
ResizeMem	endp


; RELEASES ALL MEMORY NOT USED BY PROGRAM AT STARTUP
;	- RETURNS : max_avail = size of largest available memory block (para's)
;	- STACK : (ref) end_of_code_marker, (val) program_segment, R_IP
;                 AX, BX, ES, BP

FreeAllMem	proc	near
		push	ax			; Save registers
		push	bx
		push	es
		push	bp

		mov	bp, sp			; Index stack with BP

		mov	ax, [bp+0ah]		; Get this program's code seg
		mov	es, ax			; Prepare for call

		mov	bx, [bp+0ch]		; Last opcode of this program
		add	bx, STACK_SIZE + 1	; Leave some stack space
		mov	cl, 04h			; Prepare for DIV 16
		shr	bx, cl			; Convert prog-size to para's
		inc	bx			; Leave 1 para extra - in case

		mov	ah, 4ah			; Request set_memory_block
		int	21h			; Call DOS

		jnc	short ResizeOK		; If no error occurred, go here
		push	ax			; Pass DOS' error code
		call	ErrorProc		; Display an error message

ResizeOK:
		pop	bp			; Restore used registers
		pop	es
		pop	bx
		pop	ax
		ret	4			; Take 2 parms off the stack
FreeAllMem	endp


; RETURNS AMOUNT OF AVAILABLE MEMORY IN PARAGRAPHS
;	- RETURNS : return_var = # of free paragraphs
;	- STACK : (ref) return_var, R_IP
;                 AX, BX, BP

GetMemFree	proc	near
		push	ax			; Save registers
		push	bx
		push	bp

		mov	bp, sp			; Index stack with BP

		mov	bx, 0ffffh		; Try to allocate 640k
		mov	ah, 48h			; Request allocate_memory
		int	21h			; Call DOS

		cmp	ax, ERROR_MEM_OUT
		je	short GetMemOK

		push	ax
		call	ErrorProc

GetMemOK:
		mov	ax, bx			; Get # of free paragraphs
		mov	bx, [bp+08h]		; Load addr of return var
		mov	[bx], ax		; Save # of free paragraphs

		pop	bp			; Restore registers
		pop	bx
		pop	ax
		ret	2			; Take 1 parm off the stack
GetMemFree	endp
