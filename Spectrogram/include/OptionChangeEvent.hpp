#pragma once

namespace ks {
	enum class OptionChangeEvent : char {
		HIGH_FREQUENCY,
		THREADS,
		LIBRARY,
		PAUSE,
		GRID,
		CROSS,
		SCALE,
		FILE,
		STRENGTH,
		TIME_SCALE,
		VOLUME,
		BEGIN_TEST,
		OPEN_TESTS_DIRECTORY,
	};
}