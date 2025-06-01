#pragma once

#include "Options.hpp"
#include "OptionsState.hpp"
#include "OptionChangeEvent.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <stack>

namespace ks {
	class OptionsView {
	public:
		OptionsView(ks::Options& options);
		~OptionsView() = default;

		auto update() -> void;
		auto draw_gui() -> void;

		auto poll_changes(ks::OptionChangeEvent& event) -> bool;

		auto refresh_internal_state() -> void {
			m_internalOptions = m_appOptions;
		}

	private:
		auto restart_state() -> void;

	private:
		ks::Options& m_appOptions;
		ks::Options m_internalOptions;
		ks::OptionsState m_state{ ks::OptionsState::DEFAULT };
		sf::Clock m_clock;
		std::stack<ks::OptionChangeEvent> m_changes;
	};
}