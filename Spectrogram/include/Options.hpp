#pragma once

#include <filesystem>

namespace ks {
	enum class LibraryOption : char {
		CPP = 0,
		ASSEMBLY,
	};
	enum class ScaleOption : char {
		LINEAR=0,
		LOGARITHMIC,
	};

	struct Options {
		int threads{ 1 };
		int highFrequency{ 20000 };
		float saturation{ 1.0f };
		LibraryOption library{ LibraryOption::CPP };
		ScaleOption scale{ ScaleOption::LOGARITHMIC };
		bool paused{};
		bool showGrid{};
		bool showCross{};
		std::filesystem::path filePath{ "" };
		float timeScale{ 10.0f };
		float volume{ 100.0f };
	};
};