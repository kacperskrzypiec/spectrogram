#pragma once

#include <imgui.h>
#include <SFML/System/Clock.hpp>

namespace ks {
	class Layout {
	public:
		Layout() = default;
		~Layout() = default;

		auto dock_space() -> void;
		auto viewport_begin() -> void;
		auto viewport_end() -> void;
		auto side_panel_begin() -> void;
		auto side_panel_end() -> void;
		auto console_begin() -> void;
		auto console_end() -> void;

		auto has_viewport_size_changed() -> bool;
		auto get_viewport_size() const -> ImVec2 {
			return m_viewportSize;
		}

	private:
		ImGuiID m_dockspaceID{};
		ImGuiID m_sidePanelID{};
		ImGuiID m_consolePanelID{};

		ImVec2 m_lastViewportSize;
		ImVec2 m_viewportSize;
		sf::Clock m_lastViewportSizeChangedDelay;

		bool m_viewportSizeChanged{};
		bool m_firstTime{ 1 };
	};
}