#pragma once

namespace ks {
	enum class SeekState : char {
		DEFAULT=0,
		SEEK_STARTING,
		SEEK,
		SEEK_ENDING,
	};
}