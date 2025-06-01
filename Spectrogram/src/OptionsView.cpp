#include "OptionsView.hpp"

#include <imgui.h>
#include <imgui-SFML.h>

#include <Windows.h>
#include <iostream>
#include <filesystem>
#include <tchar.h>

static std::string WStringToUTF8(const std::wstring& wstr) {
	if (wstr.empty()) {
		return std::string();
	}
	int utf8Size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if (utf8Size <= 0) {
		return std::string();
	}
	std::vector<char> utf8Buffer(utf8Size);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, utf8Buffer.data(), utf8Size, nullptr, nullptr);
	return std::string(utf8Buffer.data());
}

static std::wstring OpenFileDialog() {
	WCHAR fileName[MAX_PATH] = { 0 };
	OPENFILENAMEW ofn{};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr; 
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = L"All Files\0*.*\0MP3\0*.mp3\0OGG\0*.ogg\0WAV\0*.wav\0FLAC\0*.flac\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_DONTADDTORECENT | OFN_NOCHANGEDIR;

	if (GetOpenFileNameW(&ofn)) {
		return std::wstring(fileName);
	}

	// Return empty string if dialog is canceled
	return std::wstring();
}

namespace ks {
	OptionsView::OptionsView(ks::Options& options)
		: m_appOptions(options) {
		
	}
	auto OptionsView::update() -> void {
		switch (m_state) {
		case ks::OptionsState::WAIT:
			if (m_clock.getElapsedTime().asMicroseconds() > 10'000) {
				m_state = ks::OptionsState::COMPARE;
			}
			break;
		case ks::OptionsState::COMPARE:
			m_state = ks::OptionsState::DEFAULT;

			if (m_internalOptions.highFrequency != m_appOptions.highFrequency) {
				m_changes.push(ks::OptionChangeEvent::HIGH_FREQUENCY);
			}

			if (m_internalOptions.threads != m_appOptions.threads) {
				m_changes.push(ks::OptionChangeEvent::THREADS);
			}

			if (m_internalOptions.library != m_appOptions.library) {
				m_changes.push(ks::OptionChangeEvent::LIBRARY);
			}

			if (m_internalOptions.paused != m_appOptions.paused) {
				m_changes.push(ks::OptionChangeEvent::PAUSE);
			}

			if (m_internalOptions.showGrid != m_appOptions.showGrid) {
				m_changes.push(ks::OptionChangeEvent::GRID);
			}
			if (m_internalOptions.showCross != m_appOptions.showCross) {
				m_changes.push(ks::OptionChangeEvent::CROSS);
			}

			if (m_internalOptions.scale != m_appOptions.scale) {
				m_changes.push(ks::OptionChangeEvent::SCALE);
			}

			if (m_internalOptions.filePath != m_appOptions.filePath) {
				m_changes.push(ks::OptionChangeEvent::FILE);
			}

			if (m_internalOptions.saturation != m_appOptions.saturation) {
				m_changes.push(ks::OptionChangeEvent::STRENGTH);
			}

			if (m_internalOptions.timeScale != m_appOptions.timeScale) {
				m_changes.push(ks::OptionChangeEvent::TIME_SCALE);
			}

			if (m_internalOptions.volume != m_appOptions.volume) {
				m_changes.push(ks::OptionChangeEvent::VOLUME);
			}

			m_appOptions = m_internalOptions;

			break;
		default: break;
		}
	}

	auto OptionsView::draw_gui() -> void {
		ImGui::Text("Library:");
		if (ImGui::RadioButton("C++", m_internalOptions.library == ks::LibraryOption::CPP)) {
			m_internalOptions.library = ks::LibraryOption::CPP;
			restart_state();
		}
		if (ImGui::RadioButton("asm", m_internalOptions.library == ks::LibraryOption::ASSEMBLY)) {
			m_internalOptions.library = ks::LibraryOption::ASSEMBLY;
			restart_state();
		}

		ImGui::Separator();

		if (ImGui::SliderInt("threads", &m_internalOptions.threads, 1, 64)) {
			restart_state();
		}

		if (ImGui::DragInt("high f", &m_internalOptions.highFrequency, 1.0f, 100, 20000, "%dHz")) {
			m_internalOptions.highFrequency = std::clamp(m_internalOptions.highFrequency, 100, 20000);
			restart_state();
		}

		if (ImGui::DragFloat("saturation", &m_internalOptions.saturation, 0.1f, 0.1f, 10.0f, "%.2f")) {
			m_internalOptions.saturation = std::clamp(m_internalOptions.saturation, 0.1f, 10.0f);
			restart_state();
		}
		if (ImGui::DragFloat("time scale", &m_internalOptions.timeScale, 0.1f, 1.0f, 100.0f, "%.2fs")) {
			m_internalOptions.timeScale = std::clamp(m_internalOptions.timeScale, 1.0f, 100.0f);
			restart_state();
		}
		if (ImGui::DragFloat("volume", &m_internalOptions.volume, 1.0f, 0.0f, 100.0f, "%.0f%%")) {
			m_internalOptions.volume = std::clamp(m_internalOptions.volume, 0.0f, 100.0f);
			restart_state();
		}

		ImGui::Separator();

		ImGui::Text("Scale:");
		if (ImGui::RadioButton("Linear", m_internalOptions.scale == ks::ScaleOption::LINEAR)) {
			m_internalOptions.scale = ks::ScaleOption::LINEAR;
			restart_state();
		}
		if (ImGui::RadioButton("Logarithmic", m_internalOptions.scale == ks::ScaleOption::LOGARITHMIC)) {
			m_internalOptions.scale = ks::ScaleOption::LOGARITHMIC;
			restart_state();
		}

		ImGui::Separator();

		if (ImGui::Checkbox("Show grid", &m_internalOptions.showGrid)) {
			restart_state();
		}
		if (ImGui::Checkbox("Show cross", &m_internalOptions.showCross)) {
			restart_state();
		}

		ImGui::Separator();

		if (ImGui::Button("Reload file")) {
			m_changes.push(ks::OptionChangeEvent::FILE);
		}

		static std::wstring path;
		if (ImGui::Button("Choose file")) {
			path = OpenFileDialog();
			std::string str = WStringToUTF8(path);
			if (str != "") {
				std::cout << str << '\n';
				m_internalOptions.filePath = path;
				restart_state();
			}
		}
		if (ImGui::Button("Open tests directory")) {
			m_changes.push(ks::OptionChangeEvent::OPEN_TESTS_DIRECTORY);
		}

		if (ImGui::Button("Test")) {
			m_changes.push(ks::OptionChangeEvent::BEGIN_TEST);
		}

		ImGui::Separator();


		if (m_internalOptions.paused == 0) {
			if (ImGui::Button("Stop")) {
				m_internalOptions.paused = 1;
				restart_state();
			}
		}
		else {
			if (ImGui::Button("Resume")) {
				m_internalOptions.paused = 0;
				restart_state();
			}
		}
	}

	auto OptionsView::poll_changes(ks::OptionChangeEvent& event) -> bool {
		if (m_changes.empty()) return 0;

		event = m_changes.top();
		m_changes.pop();

		return 1;
	}

	auto OptionsView::restart_state() -> void {
		m_state = ks::OptionsState::WAIT;
		m_clock.restart();
	}
}
