#include "Console.hpp"
#include "Util.hpp"

#include <imgui.h>
#include <chrono>

#ifndef PRODUCTION_BUILD
#include <iostream>
#endif


namespace ks {
	auto Console::log(const std::string& message) -> void {
		const auto now = std::chrono::system_clock::now();
		auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
		auto fraction = now - seconds;
		auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(fraction);

		const std::time_t time = std::chrono::system_clock::to_time_t(now);
		std::tm tm{};
		localtime_s(&tm, &time);

		Message msg;
		msg.content = message + ".";
		msg.tm = tm;
		msg.ms = static_cast<int>(milliseconds.count());

		m_messages.emplace_back(msg);
		if (m_messages.size() > 100) {
			m_messages.erase(m_messages.begin());
		}

#ifndef PRODUCTION_BUILD
		std::cout << msg.make_time_string() << "[LOG] " << message << "." << std::endl;
#endif
	}
	auto Console::draw_logs() -> void {
		for (const auto& message : m_messages) {
			const std::string info = message.make_time_string() + "[LOG]";

			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), info.c_str());
			ImGui::SameLine();
			ImGui::TextWrapped(message.content.c_str());
		}

		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
			ImGui::SetScrollHereY(1.0f);
		}
	}
	auto Console::Message::make_time_string() const -> std::string {
		const std::string timeString =
			"[" + pad_integer_to_string(tm.tm_hour, 2)
			+ ":" + pad_integer_to_string(tm.tm_min, 2)
			+ ":" + pad_integer_to_string(tm.tm_sec, 2)
			+ "." + pad_integer_to_string(ms, 3)
			+ "]";

		return timeString;
	}
}