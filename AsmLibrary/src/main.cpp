#include <Windows.h>
#include <immintrin.h>

extern "C" __declspec(dllexport)
__m256 __vectorcall compute_cos(__m256 v) {
	return _mm256_cos_ps(v);
}

extern "C" __declspec(dllexport)
__m256 __vectorcall compute_log(__m256 v) {
	return _mm256_log10_ps(v);
}

BOOL WINAPI main(HINSTANCE h, DWORD reason, LPVOID reserved) {
	switch (reason) {
	default:
		break;
	}
	return TRUE;
}