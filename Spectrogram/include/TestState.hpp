#pragma once

namespace ks {
	enum class TestState: char {
		NOT_TESTING=0,
		BEGIN_TEST,
		TESTING,
		END_TEST,
		FAILED_TO_BEGIN,
	};
}