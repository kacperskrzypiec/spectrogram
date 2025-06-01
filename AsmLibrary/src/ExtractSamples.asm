.data
divisorp REAL4 32767.0, 32767.0, 32767.0, 32767.0, 32767.0, 32767.0, 32767.0, 32767.0
divisorn REAL4 32768.0, 32768.0, 32768.0, 32768.0, 32768.0, 32768.0, 32768.0, 32768.0
zero REAL4 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
.code

extract_stereo_samples PROC
; RCX - pointer to array of raw samples (const short*)
; RDX - pointer to array of samples (float*)
; R8 - holds value of raw samples size(std::size_t)

	; initialize some registers for later
	shl r8, 2
	xor r9, r9 ; loop counter
	
	vmovups ymm1, zero
	vmovups ymm2, divisorp
	vmovups ymm3, divisorn

extractionLoopStereo:
	; load samples
	vmovups ymm0, ymmword ptr [rcx + r9 * 8]
	
	; divide by two and add together
	vpsraw ymm0, ymm0, 1
	vphaddw ymm0, ymm0, ymm1
	; swap places so the sums are consecutive
	vpermq ymm0, ymm0, 11011000b
	
	; convert 8 shorts to 8 floats
	vpmovsxwd ymm0, xmm0
	vcvtdq2ps ymm0, ymm0

	; choose divisor to use
	vcmpps ymm4, ymm0, ymm1, 30
	vblendvps ymm4, ymm3, ymm2, ymm4

	; normalize samples
	vdivps ymm0, ymm0, ymm4

	; store the converted samples
	vmovups ymmword ptr[rdx + r9 * 8], ymm0

	add r9, 4
	cmp r9, r8
	jl extractionLoopStereo

	ret
extract_stereo_samples ENDP

; ==========================================================
; **********************************************************
; ==========================================================

extract_mono_samples PROC
; RCX - pointer to array of raw samples (const short*)
; RDX - pointer to array of samples (float*)
; R8 - holds value of raw samples size(std::size_t)

	; initialize some registers for later
	shl r8, 2
	xor r9, r9 ; loop counter
	
	vmovups ymm1, zero
	vmovups ymm2, divisorp
	vmovups ymm3, divisorn

extractionLoopMono:
	; load samples
	vmovups xmm0, xmmword ptr [rcx + r9 * 4]
	
	; convert 8 shorts to 8 floats
	vpmovsxwd ymm0, xmm0
	vcvtdq2ps ymm0, ymm0

	; choose divisor to use
	vcmpps ymm4, ymm0, ymm1, 30
	vblendvps ymm4, ymm3, ymm2, ymm4

	; normalize samples
	vdivps ymm0, ymm0, ymm4

	; store the converted samples
	vmovups ymmword ptr[rdx + r9 * 8], ymm0

	add r9, 4
	cmp r9, r8
	jl extractionLoopMono

	ret
extract_mono_samples ENDP


END