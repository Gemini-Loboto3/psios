    opt     c+

	section	.rdata
	section	.ctors
	section	.dtors
	section	.sdata
	section	.sbss
	section	.bss

 	section	.data

hirom group
	xdef HiRom		; secondary flash rom

; externs for SN entry redefinition
	xdef	__SN_ENTRY_POINT,__main
	xdef	__heapbase,__heapsize
	xdef	__text,__textlen
	xdef	__data,__datalen
	xdef	__bss,__bsslen
	xref	main,InitHeap
	
	global _stacksize
	global _ramsize

	section	.rdata		; define as read-only data.
HiRom	dw	group(hirom)

;====================================================
;= Entry point function with less fuss				=
;====================================================
	section	.text

;----------------------------------------------------------------------------
; ROM entry point
;----------------------------------------------------------------------------
;	dw entry_point_2
;	db "Licensed by Sony Computer Entertainment Inc."
;	dcb $50, $20

;	dw entry_point_1
;	db "Licensed by Sony Computer Entertainment Inc."
;	dcb $50, $20

entry_point_1:	; Call our init routine
;	j __SN_ENTRY_POINT
;	nop
entry_point_2:	; Return to BIOS
;	jr ra
;	nop

__SN_ENTRY_POINT:
stup1:
stup2:
	la	t0, sect(.sbss)
	la	t1, sectend(.bss)
@clrit:						; clear bss
	opt	at-
	sw r0, 0(t0)			; write to bottom of area to clear
	addiu t0, t0, 4			; increment bottom
	sltu at, t0, t1			; at = bottom - top
	bnez at, @clrit			; if at is nonzero, loop
	nop
	opt	at+

	la	gp, sect(.sdata)
stup0:
	j	main
	nop

__main:
	jr	ra
	nop

;====================================================
;= Redefine some SN variables for entry point		=
;====================================================
	section	.data

	cnop	0,4				; longword align

_stacksize	dw	$00008000	; default stack is 32k
_ramsize	dw	$00200000	; 2 Megabytes

__heapbase	dw	0
__heapsize	dw	0
__text	dw	sect(.text)
__textlen	dw	sectend(.text)-sect(.text)
__data	dw	sect(.data)
__datalen	dw	sectend(.data)-sect(.data)
__bss	dw	sect(.bss)
__bsslen	dw	sectend(.bss)-sect(.bss)

	section	.sbss

__ra_temp	dsw	1

	end
