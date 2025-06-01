#pragma once

#include "OptionsView.hpp"
#include "Options.hpp"
#include "Spectrogram.hpp"
#include "SeekState.hpp"
#include "Layout.hpp"
#include "Console.hpp"
#include "SmoothReal.hpp"
#include "Tester.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <filesystem>

namespace ks {
	class App {
	public:
		App();
		~App();

		auto run() -> void;

	private:
		auto handle_events() -> void;
		auto handle_input() -> void;
		auto update(const float deltaTime) -> void;
		auto draw_gui() -> void;
		auto draw() -> void;
		auto reload_audio() -> void;
		auto log_library(const std::string& name) -> void;

	private:
		sf::RenderWindow m_window;
		sf::SoundBuffer m_soundBuffer;
		sf::Sound m_sound;

		ks::OptionsView m_optionsView;
		ks::Options m_options;
		ks::Spectrogram m_spectrogram;
		ks::SeekState m_seekState{ ks::SeekState::DEFAULT };
		ks::Layout m_layout;
		ks::Console m_console;
		ks::Tester m_tester;

		sf::RenderTexture m_viewportTexture;
		sf::Vector2f m_lastViewportSize;
		sf::Font m_font;

		float m_offset{};
		bool m_wasPaused{};
		bool m_stopOnce{};
		bool m_mouseTeleported{};
		bool m_fillScreen{};
		bool m_pressedFonce{};
		bool m_pressedGUI{};

		ks::SmoothFloat m_smoothOffset;
		ks::SmoothFloat m_smoothDecay;
	};
}