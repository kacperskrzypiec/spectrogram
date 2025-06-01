extern compute_cos@@32:PROC

.data
vcount REAL4 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0
vhalfn REAL4 -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5
vpi2 REAL4 6.28318530718, 6.28318530718, 6.28318530718, 6.28318530718, 6.28318530718, 6.28318530718, 6.28318530718, 6.28318530718
v8 REAL4 8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0
.code

hann_window PROC
; RCX - pointer to samples array (float*)
; RDX - size of array (std::size_t)

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
	push r14

	; set up counter
	dec rdx
	mov r14, rcx
	xor r13, r13 ; counter
	mov r12, rdx

	; convert size to float
	vcvtsi2ss xmm3, xmm3, r12
	vbroadcastss ymm13, xmm3

	; initialize some registers
	vmovups ymm14, vpi2
	vdivps ymm14, ymm14, ymm13 ; ymm14 = pi2 / (size - 1)
	vmulps ymm12, ymm14, vcount
	vmulps ymm14, ymm14, v8

loopHann:
	; calculate cosines
	vmovups ymm0, ymm12
	vaddps ymm12, ymm12, ymm14

	push r12
	push r13
	push r14

	sub rsp, 40
	call compute_cos@@32
	add rsp, 40

	pop r14
	pop r13
	pop r12

	; calculate hann window
	vmovups ymm10, vhalfn
	vfmsub132ps ymm0, ymm10, ymm10 ; ymm0 = (-0.5f) * cosf(ymm0) - (-0.5f)

	; apply hann window
	vmovups ymm1, ymmword ptr [r14 + r13 * 4]
	vmulps ymm1, ymm0, ymm1
	vmovups ymmword ptr[r14 + r13 * 4], ymm1

	add r13, 8
	cmp r13, r12
	jle loopHann
	
	; restore non volatile registers
	pop r14
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
hann_window ENDP

END