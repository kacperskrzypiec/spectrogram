extern compute_log@@32:PROC

.data
two REAL4 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0
epsilon REAL4 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001
twenty REAL4 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0
zero REAL4 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
.code

compute_magnitudes PROC
; RCX - pointer to frequencies array (Complex8f*)
; RDX - pointer to magnitudes array (float*)		
; R8 - size of magnitudes (halved frequencies) (std::size_t)		
	
	; preserve non volatile registers
	sub rsp, 32
	vmovups [rsp], ymm6
	sub rsp, 32
	vmovups [rsp], ymm7
	sub rsp, 32
	vmovups [rsp], ymm8
	sub rsp, 32
	vmovups [rsp], ymm9
	sub rsp, 32
	vmovups [rsp], ymm10
	sub rsp, 32
	vmovups [rsp], ymm11
	sub rsp, 32
	vmovups [rsp], ymm12
	sub rsp, 32
	vmovups [rsp], ymm13
	sub rsp, 32
	vmovups [rsp], ymm14
	sub rsp, 32
	vmovups [rsp], ymm15

	push r12
	push r13

	xor r10, r10 ; offset
	mov r11, rcx
	mov r12, rdx
	mov r13, r8

	shl r13, 5 ; *32
	
loopMag:
	; compute modulus
	vmovups ymm0, ymmword ptr [r11 + r10 * 2]
	vmovups ymm1, ymmword ptr [r11 + r10 * 2 + 32]

	vmulps ymm0, ymm0, ymm0
	vfmadd132ps ymm1, ymm0, ymm1 ; ymm1 = ymm1 * ymm1 + ymm0
	vsqrtps ymm0, ymm1

	; take a logarithm
	vmovups ymm2, epsilon
	vfmadd132ps ymm0, ymm2, two

	; 

	push r10
	push r11
	push r12
	push r13

	sub rsp, 40
	call compute_log@@32
	add rsp, 40

	pop r13
	pop r12
	pop r11
	pop r10

	; adjust
	vmaxps ymm0, ymm0, zero
	vmulps ymm0, ymm0, twenty

	; store
	vmovups ymmword ptr [r12 + r10], ymm0
	
	add r10, 32
	cmp r10, r13
	jl loopMag

doneMag:
	; restore non volatile registers
	pop r13
	pop r12

	vmovups ymm15, [rsp]
	add rsp, 32
	vmovups ymm14, [rsp]
	add rsp, 32
	vmovups ymm13, [rsp]
	add rsp, 32
	vmovups ymm12, [rsp]
	add rsp, 32
	vmovups ymm11, [rsp]
	add rsp, 32
	vmovups ymm10, [rsp]
	add rsp, 32
	vmovups ymm9, [rsp]
	add rsp, 32
	vmovups ymm8, [rsp]
	add rsp, 32
	vmovups ymm7, [rsp]
	add rsp, 32
	vmovups ymm6, [rsp]
	add rsp, 32
	ret
compute_magnitudes ENDP

END