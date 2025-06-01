#pragma once

#include <cstddef>
#include <complex>

namespace ks {
	struct Complex8f;
}

namespace ks::Library {
	using GetLibraryID = int(*)();
	extern GetLibraryID get_library_id;
	using HannWindow = void(*)(float* samples, const std::size_t size);
	extern HannWindow hann_window;
	using ComputeMagnitudes = void(*)(const ks::Complex8f* frequencies, float* magnitudes, const std::size_t size);
	extern ComputeMagnitudes compute_magnitudes;
	using ComputePartOfFFT = void(*)(ks::Complex8f* out, const ks::Complex8f& w, const std::size_t inSize, const int m);
	extern ComputePartOfFFT compute_part_of_fft;
	using ExtractMonoSamples = void(*)(const short* rawSamples, float* samples, const std::size_t size);
	extern ExtractMonoSamples extract_mono_samples;
	using ExtractStereoSamples = void(*)(const short* rawSamples, float* samples, const std::size_t size);
	extern ExtractStereoSamples extract_stereo_samples;

	extern auto load_cpp() -> bool;
	extern auto load_asm() -> bool;
}