#include "Functions.hpp"
#include <Windows.h>

namespace ks::Library {
	GetLibraryID get_library_id{};
	HannWindow hann_window{};
	ComputeMagnitudes compute_magnitudes{};
	ComputePartOfFFT compute_part_of_fft{};
	ExtractMonoSamples extract_mono_samples{};
	ExtractStereoSamples extract_stereo_samples{};

	static HMODULE CppDLL{};
	static HMODULE AsmDLL{};
	
	static auto free_libraries() -> void {
		if (CppDLL != 0) {
			FreeLibrary(CppDLL);
		}
		if (AsmDLL != 0) {
			FreeLibrary(AsmDLL);
		}
	}
	static auto load_functions(HMODULE module) -> bool {
		if (module == 0) return 0;

		get_library_id = (GetLibraryID)GetProcAddress(module, "get_library_id");
		hann_window = (HannWindow)GetProcAddress(module, "hann_window");
		compute_magnitudes = (ComputeMagnitudes)GetProcAddress(module, "compute_magnitudes");
		compute_part_of_fft = (ComputePartOfFFT)GetProcAddress(module, "compute_part_of_fft");
		extract_mono_samples = (ExtractMonoSamples)GetProcAddress(module, "extract_mono_samples");
		extract_stereo_samples = (ExtractStereoSamples)GetProcAddress(module, "extract_stereo_samples");

		return 1;
	}

	auto load_cpp() -> bool {
		free_libraries();

	#if PRODUCTION_BUILD
		CppDLL = LoadLibraryA("CppLibrary.dll");
	#elif NDEBUG
		CppDLL = LoadLibraryA(CPP_DLL_DIR "CppLibrary.dll");
	#else
		CppDLL = LoadLibraryA(CPP_DLL_DIR "CppLibrary-d.dll");
	#endif

		return load_functions(CppDLL);
	}
	auto load_asm() -> bool {
		free_libraries();

	#if PRODUCTION_BUILD
		AsmDLL = LoadLibraryA("AsmLibrary.dll");
	#elif NDEBUG
		AsmDLL = LoadLibraryA(ASM_DLL_DIR "AsmLibrary.dll");
	#else
		AsmDLL = LoadLibraryA(ASM_DLL_DIR "AsmLibrary-d.dll");
	#endif

		return load_functions(AsmDLL);
	}
}