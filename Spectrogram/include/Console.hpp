#pragma once

#include <vector>
#include <string>

namespace ks {
	class Console {
	public:
		Console() = default;
		~Console() = default;

		auto log(const std::string& message) -> void;
		auto draw_logs() -> void;

	private:
		struct Message {
			std::string content;
			int ms{};
			std::tm tm{};
			auto make_time_string() const -> std::string;
		};

	private:

		std::vector<Message> m_messages;
	};
}