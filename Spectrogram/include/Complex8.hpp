#pragma once

namespace ks{
	struct alignas(32) Complex8f {
		float r[8]{};
		float i[8]{};
	};
}