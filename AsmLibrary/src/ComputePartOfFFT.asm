.data
one REAL4 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
zero REAL4 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
.code

compute_part_of_fft PROC
; RCX - pointer to out array (Complex8f*)
; RDX - pointer to w (Complex8f*)
; R8 - size of frequencies array (std::size_t)
; R9 - value of m (int)

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

	push r12	
	push r13

	; SIMD + ASSEMBLY + COMPLEX NUMBERS = FRIED BRAIN
	; I am sure that I will forget what is happening in here
	; on the day of presentation.

	; counter
	xor r11, r11

	; m2
	mov R10, R9
	shr R10, 1

	; w
	vmovups ymm13, ymmword ptr [rdx] ; w_real
	vmovups ymm14, ymmword ptr [rdx + 32] ; w_imag

	; wm
	movss xmm9, dword ptr [rdx + 4] ; wm_real
	vbroadcastss ymm9, xmm9
	movss xmm10, dword ptr [rdx + 36] ; wm_imag
	vbroadcastss ymm10, xmm10

	; cube w_m8v
	vmulps ymm6, ymm10, ymm9
	vmulps ymm9, ymm9, ymm9

	vfnmadd231ps ymm9, ymm10, ymm10
	vaddps ymm10, ymm6, ymm6

	;;;
	vmulps ymm6, ymm10, ymm9
	vmulps ymm9, ymm9, ymm9

	vfnmadd231ps ymm9, ymm10, ymm10
	vaddps ymm10, ymm6, ymm6
	
	;;;
	vmulps ymm6, ymm10, ymm9
	vmulps ymm9, ymm9, ymm9

	vfnmadd231ps ymm9, ymm10, ymm10
	vaddps ymm10, ymm6, ymm6

outerLoop:
	cmp r11, r10
	jge outerLoopDone
	xor rax, rax
innerLoop:	
	cmp rax, r8
	jge innerLoopDone
	; fetch t
	mov r13, rax ; k
	add r13, r11 ; +j

	mov r12, r13 ; k + j
	add r12, r10 ; +m2
	vmovups ymm2, ymmword ptr [rcx + r12 * 8] ; t_real
	vmovups ymm3, ymmword ptr [rcx + r12 * 8 + 32] ; t_imag
	
	; multiply t with w
	vmulps ymm4, ymm2, ymm13
	vmulps ymm5, ymm3, ymm13

	vfnmadd231ps ymm4, ymm3, ymm14
	vfmadd231ps ymm5, ymm2, ymm14

	; fetch u
	vmovups ymm0, ymmword ptr [rcx + r13 * 8] ; u_real
	vmovups ymm1, ymmword ptr [rcx + r13 * 8 + 32] ; u_imag

	; store
	vaddps ymm6, ymm0, ymm4
	vaddps ymm7, ymm1, ymm5
	vmovups ymmword ptr [rcx + r13 * 8], ymm6 ; real
	vmovups ymmword ptr [rcx + r13 * 8 + 32], ymm7 ; imag

	vsubps ymm6, ymm0, ymm4
	vsubps ymm7, ymm1, ymm5
	vmovups ymmword ptr [rcx + r12 * 8], ymm6 ; real
	vmovups ymmword ptr [rcx + r12 * 8 + 32], ymm7 ; imag

	; innerLoop end
	add rax, r9 ; k
	jmp innerLoop
	;;;;
innerLoopDone:
	; multiply w with w_m8v
	vmulps ymm4, ymm13, ymm9
	vmulps ymm5, ymm14, ymm9

	vfnmadd231ps ymm4, ymm14, ymm10
	vfmadd231ps ymm5, ymm13, ymm10

	vmovups ymm13, ymm4
	vmovups ymm14, ymm5

	; outerLoop end
	add r11, 8 ; j
	jmp outerLoop
	;;;;
outerLoopDone:
	; restore non volatile registers
	pop r13
	pop r12

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
compute_part_of_fft ENDP

END