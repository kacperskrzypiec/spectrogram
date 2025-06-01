#include "Tester.hpp"
#include "Console.hpp"
#include "ProcessAudio.hpp"
#include "Functions.hpp"
#include "Util.hpp"
#include <SFML/System/Clock.hpp>
#include <sstream>
#include <fstream>
#include <chrono>
#include <filesystem>

namespace ks {
	Tester::Tester(sf::SoundBuffer& buffer) 
		: m_soundBuffer(buffer){
	
	}

	auto Tester::begin_test() -> void {
		if (m_soundBuffer.getDuration().asMilliseconds() > 0) {
			m_state = ks::TestState::BEGIN_TEST;
		}
		else {
			m_state = ks::TestState::FAILED_TO_BEGIN;
		}
	}

	auto Tester::update(ks::Console& console) -> void {
		switch (m_state) {
		case ks::TestState::FAILED_TO_BEGIN:
			m_state = ks::TestState::NOT_TESTING;
			console.log("Failed to begin the test");
			break;
		case ks::TestState::BEGIN_TEST:
			m_state = ks::TestState::TESTING;
			console.log("The test has started");
			m_threads = 1;
			m_library = ks::LibraryOption::CPP;
			m_lastLibrary = ks::Library::get_library_id();
			ks::Library::load_cpp();
			m_index = 0;
			break;
		case ks::TestState::TESTING: {
			float sum{};
			for (int i = 0; i < NUM_TESTS + 1; i++) {
				sf::Clock c;
				auto samples = ks::extract_samples(m_soundBuffer);

				ks::Options options;
				options.threads = m_threads;
				options.library = m_library;

				auto collection = ks::process_samples(samples, options);

				if (i != 0) {
					sum += c.getElapsedTime().asMicroseconds() / 1000.0f;
				}
			}

			const float time = sum / NUM_TESTS;

			std::stringstream ss;
			ss << (m_library == ks::LibraryOption::CPP ? "C++" : "asm");
			ss << ", " << m_threads << " threads: " << time << "ms";
			console.log(ss.str());

			if (m_library == ks::LibraryOption::ASSEMBLY) {
				m_asmResults[m_index] = time;
			}
			else {
				m_cppResults[m_index] = time;
			}

			m_index++;

			m_threads <<= 1;

			if (m_threads == 128) {
				if (m_library == ks::LibraryOption::ASSEMBLY) {
					m_state = ks::TestState::END_TEST;
				}
				else {
					m_index = 0;
					m_threads = 1;
					m_library = ks::LibraryOption::ASSEMBLY;
					ks::Library::load_asm();
				}
			}

		}	break;
		case ks::TestState::END_TEST: {
			m_state = ks::TestState::NOT_TESTING;
			if (m_lastLibrary == 67) {
				ks::Library::load_cpp();
			}

			const auto now = std::chrono::system_clock::now();
			const std::time_t time = std::chrono::system_clock::to_time_t(now);
			std::tm tm{};
			localtime_s(&tm, &time);

			std::stringstream ss;
			ss << "test_" << ks::pad_integer_to_string(tm.tm_year + 1900, 2) << ks::pad_integer_to_string(tm.tm_mon + 1, 2) << ks::pad_integer_to_string(tm.tm_mday, 2) << "_"
				<< ks::pad_integer_to_string(tm.tm_hour, 2) << ks::pad_integer_to_string(tm.tm_min, 2) << ks::pad_integer_to_string(tm.tm_sec, 2)
				<< ".csv";

			const std::string filePath = "tests/" + ss.str();

			if (!std::filesystem::exists("tests")) {
				if (!std::filesystem::is_directory("tests")) {
					if (std::filesystem::create_directory("tests")) {
						console.log("Created a directory \"tests\"");
					}
					else {
						console.log("Could not create a directory \"tests\"");
					}
				}
			}

			std::string csv = "Threads,C++ [ms],asm [ms],Boost";

			for (int i = 0; i < NUM_RESULTS; i++) {
				std::stringstream ss;
				ss << '\n' << (1 << i) << ',' << m_cppResults[i] << ',' << m_asmResults[i] << ',' << m_cppResults[i] / m_asmResults[i];
				csv += ss.str();
			}

			std::ofstream file(filePath);
			if (file) {
				file << csv;
				file.close();
				console.log("The test has finished. The results have been saved to \"" + filePath + "\" file");
			}
			else {
				console.log("The test has finished. Could not create a test file");
			}
		}	break;
		default:
			break;
		}
	}
}