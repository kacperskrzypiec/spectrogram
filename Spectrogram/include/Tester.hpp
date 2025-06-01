#pragma once

#include "TestState.hpp"
#include "Options.hpp"

#include <SFML/Audio/SoundBuffer.hpp>
#include <array>

namespace ks {
	constexpr int NUM_TESTS = 5;
	constexpr int NUM_RESULTS = 7;

	class Console;

	class Tester {
	public:
		Tester(sf::SoundBuffer& buffer);
		~Tester() = default;

		auto begin_test() -> void;
		auto update(ks::Console& console) -> void;

		auto is_testing() const -> bool {
			return m_state != ks::TestState::NOT_TESTING;
		}

	private:
		sf::SoundBuffer& m_soundBuffer;
		ks::TestState m_state{ ks::TestState::NOT_TESTING };
		int m_threads{ 1 };
		ks::LibraryOption m_library{ ks::LibraryOption::CPP };
		int m_lastLibrary{};

		std::array<float, NUM_RESULTS> m_cppResults{};
		std::array<float, NUM_RESULTS> m_asmResults{};
		int m_index{};
	};
}