#include "App.hpp"
#include "ProcessAudio.hpp"
#include "Util.hpp"
#include "Functions.hpp"

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-SFML.h>

#include <iostream>
#include <thread>
#include <windows.h>

namespace ks {
	App::App() 
		:	m_optionsView(m_options), m_spectrogram(m_soundBuffer), m_tester(m_soundBuffer) {
		m_window.create(sf::VideoMode(1280, 720), "Spectrogram");
		m_window.setFramerateLimit(60);

		HICON hIcon = (HICON)LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
		SendMessage(m_window.getSystemHandle(), WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon));
		DestroyIcon(hIcon);

		ImGui::SFML::Init(m_window);
		ImGuiIO& io = ImGui::GetIO();
		io.IniFilename = nullptr;
		io.LogFilename = nullptr;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		sf::Event event;
		event.type = sf::Event::GainedFocus;
		ImGui::SFML::ProcessEvent(m_window, event);

		ks::Library::load_asm();
		log_library("asm");
		m_options.library = ks::LibraryOption::ASSEMBLY;

		m_options.threads = std::thread::hardware_concurrency();
		m_options.paused = 1;
		m_optionsView.refresh_internal_state();

		m_font.loadFromFile(DATA_PATH "arial.ttf");

#if !PRODUCTION_BUILD
		m_options.filePath = DATA_PATH "audio/cave14.mp3";
		reload_audio();
#endif
	}

	App::~App() {

	}

	auto App::run() -> void {
		sf::Clock deltaTimeClock;
		while (m_window.isOpen()) {
			handle_events();
			handle_input();
			update(deltaTimeClock.restart().asSeconds());
			draw_gui();
			draw();
		}
	}

	auto App::handle_events() -> void {
		for (sf::Event event; m_window.pollEvent(event);) {
			ImGui::SFML::ProcessEvent(m_window, event);
			switch (event.type) {
			case sf::Event::Closed:
				m_window.close();
				break;
			case sf::Event::Resized: {
				sf::FloatRect view(0, 0, static_cast<float>(event.size.width), static_cast<float>(event.size.height));
				m_window.setView(sf::View(view));
			}	break;
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::F && m_pressedFonce == 0) {
					m_fillScreen = !m_fillScreen;
					m_pressedFonce = 1;
				}
				break;
			case sf::Event::KeyReleased:
				if (event.key.code == sf::Keyboard::F) {
					m_pressedFonce = 0;
				}
				break;
			}
		}
	}

	auto App::handle_input() -> void {

	}

	auto App::update(const float deltaTime) -> void {
		ImGui::SFML::Update(m_window, sf::Time(sf::seconds(deltaTime)));

		if (m_mouseTeleported) {
			m_mouseTeleported = 0;
			ImGui::ResetMouseDragDelta(0);
		}

		m_optionsView.update();

		m_tester.update(m_console);

		if (!m_tester.is_testing()) {
			for (ks::OptionChangeEvent event; m_optionsView.poll_changes(event);) {
				switch (event) {
				case ks::OptionChangeEvent::HIGH_FREQUENCY:
					m_spectrogram.set_max_frequency(m_options.highFrequency);
					break;
				case ks::OptionChangeEvent::THREADS:

					break;
				case ks::OptionChangeEvent::LIBRARY:
					if (m_options.library == ks::LibraryOption::ASSEMBLY) {
						ks::Library::load_asm();
						log_library("assembly");
					}
					else if (m_options.library == ks::LibraryOption::CPP) {
						ks::Library::load_cpp();
						log_library("C++");
					}
					break;
				case ks::OptionChangeEvent::PAUSE:
					if (m_options.paused) {
						m_sound.pause();
					}
					else {
						m_sound.setPlayingOffset(sf::seconds(m_offset));
						m_sound.play();
					}
					break;
				case ks::OptionChangeEvent::GRID:

					break;
				case ks::OptionChangeEvent::CROSS:

					break;
				case ks::OptionChangeEvent::SCALE:
					m_spectrogram.set_scale(m_options.scale);
					break;
				case ks::OptionChangeEvent::FILE:
					reload_audio();
					break;
				case ks::OptionChangeEvent::STRENGTH:
					m_spectrogram.set_saturation(m_options.saturation);
					break;
				case ks::OptionChangeEvent::TIME_SCALE:
					m_spectrogram.set_time_scale(m_options.timeScale);
					break;
				case ks::OptionChangeEvent::VOLUME:
					m_sound.setVolume(m_options.volume);
					break;
				case ks::OptionChangeEvent::BEGIN_TEST:
					m_tester.begin_test();
					break;
				case ks::OptionChangeEvent::OPEN_TESTS_DIRECTORY:
					if (std::filesystem::exists("tests")) {
						if (std::filesystem::is_directory("tests")) {
							const std::wstring pathToOpen = std::filesystem::current_path().wstring() + L"/tests";
							ShellExecuteW(nullptr, L"open", pathToOpen.c_str(), 0, 0, SW_SHOWNORMAL);
						}
						else {
							m_console.log("Directory tests does not exists");
						}
					}
					else {
						m_console.log("Directory tests does not exists");
					}
					
					break;
				}
			}
		}

		if (m_soundBuffer.getDuration().asMilliseconds() > 0) {

			if (ImGui::IsMouseClicked(0)) {
				const sf::Vector2i mousePos = sf::Mouse::getPosition(m_window);
				if (mousePos.x < m_layout.get_viewport_size().x - 5 && mousePos.y < m_layout.get_viewport_size().y - 5) {
					m_pressedGUI = 0;
				}
				else {
					m_pressedGUI = 1;
				}
			}

			switch (m_seekState) {
			case ks::SeekState::DEFAULT:
				if (ImGui::IsMouseDragging(0)) {
					if (ImGui::IsMouseDown(0) && m_pressedGUI == 0) {
						const sf::Vector2i mousePos = sf::Mouse::getPosition(m_window);
						if (mousePos.x < m_layout.get_viewport_size().x && mousePos.y < m_layout.get_viewport_size().y) {
							m_seekState = ks::SeekState::SEEK_STARTING;
							break;
						}
					}
				}

				if (m_sound.getStatus() != sf::Sound::Status::Stopped) {
					m_offset = m_sound.getPlayingOffset().asSeconds();
					m_stopOnce = 1;
				}
				else {
					if (m_stopOnce) {
						m_stopOnce = 0;
						m_options.paused = 1;
						m_optionsView.refresh_internal_state();
					}
				}
				break;
			case ks::SeekState::SEEK_STARTING:
				m_seekState = ks::SeekState::SEEK;
				m_wasPaused = m_sound.getStatus() == sf::Sound::Status::Paused;
				m_sound.pause();
				break;
			case ks::SeekState::SEEK:
				if (!ImGui::IsMouseDown(0)) {
					m_seekState = ks::SeekState::SEEK_ENDING;
					break;
				}

				if (ImGui::IsMouseDragging(0)) {
					ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

					if (sf::Mouse::getPosition(m_window).x <= 0) {
						sf::Mouse::setPosition(sf::Mouse::getPosition(m_window) + sf::Vector2i(m_viewportTexture.getSize().x - 5, 0), m_window);
						m_mouseTeleported = 1;
					}
					else if (sf::Mouse::getPosition(m_window).x + 1 >= static_cast<int>(m_viewportTexture.getSize().x)) {
						sf::Mouse::setPosition(sf::Mouse::getPosition(m_window) - sf::Vector2i(m_viewportTexture.getSize().x - 5, 0), m_window);
						m_mouseTeleported = 1;
					}

					const float newOffset = std::clamp<float>(
						m_offset - ImGui::GetMouseDragDelta().x * ks::HOP_SIZE * m_viewportTexture.getSize().x / m_soundBuffer.getSampleRate() * m_options.timeScale / 10000.0f,
						0.0f,
						m_soundBuffer.getDuration().asSeconds() - 0.01f);

					m_offset = newOffset;
					m_sound.setPlayingOffset(sf::seconds(m_offset));
					ImGui::ResetMouseDragDelta(0);
				}
				break;
			case ks::SeekState::SEEK_ENDING:
				m_seekState = ks::SeekState::DEFAULT;
				ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
				if (!m_wasPaused && m_sound.getStatus() != sf::Sound::Status::Stopped) {
					m_options.paused = 0;
					m_sound.play();
					m_sound.setPlayingOffset(sf::seconds(m_offset));
				}
				else {
					m_options.paused = 1;
				}
				m_optionsView.refresh_internal_state();
				break;
			}

			if (ImGui::IsKeyPressed(ImGuiKey_Space, 0)) {
				m_seekState = ks::SeekState::DEFAULT;

				if (m_sound.getStatus() != sf::Sound::Status::Playing) {
					m_options.paused = 0;
					m_sound.play();
					m_sound.setPlayingOffset(sf::seconds(m_offset));
				}
				else {
					m_sound.pause();
					m_options.paused = 1;
				}
				m_optionsView.refresh_internal_state();
			}
		}

		if (m_layout.has_viewport_size_changed()) {
			m_viewportTexture.create(
				static_cast<int>(m_layout.get_viewport_size().x),
				static_cast<int>(m_layout.get_viewport_size().y));
			m_spectrogram.on_viewport_size_change(static_cast<sf::Vector2i>(m_viewportTexture.getSize()));
		}

		m_smoothDecay.decay(10.0f);
		m_smoothDecay.update(deltaTime);
		if (m_sound.getStatus() != sf::Sound::Status::Playing) {
			m_smoothDecay = 20.0f;
		}
		else {
			m_smoothDecay = 100.0f;
		}
		m_smoothOffset.decay(m_smoothDecay);

		m_smoothOffset = m_offset;
		m_smoothOffset.update(deltaTime);

		m_spectrogram.set_offset(m_smoothOffset);

		m_spectrogram.update(deltaTime);
	}

	auto App::draw_gui() -> void {
		m_viewportTexture.clear();
		m_spectrogram.draw(m_viewportTexture);

		auto draw_line = [&](const float x1, const float x2, const float y1, const float y2, const sf::Color color) -> void {
			sf::VertexArray line(sf::Lines, 2);
			line[0] = sf::Vertex(sf::Vector2f(x1, y1), color);
			line[1] = sf::Vertex(sf::Vector2f(x2, y2), color);
			m_viewportTexture.draw(line);
		};
		
		if (m_soundBuffer.getDuration().asMilliseconds() > 0) {
			{ // DRAW CROSS
				if (m_options.showCross) {
					const sf::Vector2i mousePos = sf::Mouse::getPosition(m_window);
					if (mousePos.x >= 0 && mousePos.y >= 0 && mousePos.x < static_cast<int>(m_viewportTexture.getSize().x) && mousePos.y < static_cast<int>(m_viewportTexture.getSize().y)) {
						draw_line(static_cast<float>(mousePos.x), static_cast<float>(mousePos.x), 0.0f, static_cast<float>(m_viewportTexture.getSize().y), sf::Color(150, 150, 150));
						draw_line(0.0f, static_cast<float>(m_viewportTexture.getSize().x), static_cast<float>(mousePos.y), static_cast<float>(mousePos.y), sf::Color(150, 150, 150));

						const float timeScale = (static_cast<float>(HOP_SIZE * m_viewportTexture.getSize().x) / m_soundBuffer.getSampleRate()) / m_options.timeScale;
						const float secondsInOnePixel = HOP_SIZE / (timeScale * m_soundBuffer.getSampleRate());
						const float seconds = m_smoothOffset + secondsInOnePixel * (mousePos.x - m_viewportTexture.getSize().x * 0.5f);

						float k{};
						const float height = m_viewportTexture.getSize().y;
						const float binSize = static_cast<float>(m_soundBuffer.getSampleRate()) / FFT_SIZE;
						if (m_options.scale == ks::ScaleOption::LOGARITHMIC) {
							const float factor = std::log10f(FFT_SIZE / 2.0f) / std::log10f(m_options.highFrequency / binSize + 1.0f);

							const float a = ((height - 1 - (float)mousePos.y) / (height * factor)) * std::log10f(FFT_SIZE / 2.0f);
							k = (std::powf(10.0f, a) - 1.0f);
						}
						else {
							const float resolution = height / (m_options.highFrequency / binSize);
							k = (height - 1 - mousePos.y) / resolution;
						}


						sf::Text text;
						text.setFont(m_font);
						text.setCharacterSize(14);
						text.setOutlineThickness(1.0f);
						text.setOutlineColor(sf::Color::Black);

						std::stringstream ss;
						ss << std::roundf(seconds * 100.0f) / 100.0f << "s, " << std::roundf(k * binSize) << "Hz";

						text.setString(ss.str());
						text.setPosition({ 1.0f * mousePos.x + 2.0f , 1.0f * mousePos.y - 2.0f });
						text.setOrigin({ 0.0f, text.getGlobalBounds().getSize().y });
						m_viewportTexture.draw(text);
					}
				}
			}
			{ // DRAW LABELS
				sf::Text text;
				text.setFont(m_font);
				text.setCharacterSize(14);
				text.setOutlineThickness(1.0f);
				text.setOutlineColor(sf::Color::Black);

				auto draw_label = [&](sf::RenderTarget& target, const sf::Vector2f position, const float freq) -> void {
					if (position.y + 5.0f > m_viewportTexture.getSize().y) return;

					text.setString(std::to_string(static_cast<int>(std::ceilf(freq))) + "Hz");
					text.setPosition({ position.x - 20.0f, position.y });
					if (m_options.showGrid) {
						text.setOrigin({ std::floorf(text.getGlobalBounds().getSize().x * 0.7f), std::floorf(text.getGlobalBounds().getSize().y * 0.6f) });
					}
					else {
						text.setOrigin({ std::floorf(text.getGlobalBounds().getSize().x), std::floorf(text.getGlobalBounds().getSize().y * 0.7f) });
					}

					sf::RectangleShape shape;
					if (m_options.showGrid) {
						shape.setPosition({ -1.0f, position.y });
						shape.setSize({ position.x - text.getGlobalBounds().getSize().x * 0.7f - 25.0f, 1.0f });
						shape.setOrigin({ 0.0f, 0.0f });
					}
					else {
						shape.setPosition({ position.x - 10.0f, position.y });
						shape.setSize({ 10.0f, 1.0f });
						shape.setOrigin({ 5.0f, 0.0f });
						shape.setOutlineThickness(1.0f);
						shape.setOutlineColor(sf::Color::Black);
					}
					target.draw(shape);

					target.draw(text);
					};

				if (m_options.scale == ks::ScaleOption::LOGARITHMIC) {
					const float f[7]{ 100, 500, 1000, 2000, 5000, 10000, 15000 };
					for (int i = 0; i < 7; i++) {
						const float freq = f[i] * m_options.highFrequency / 20000.0f;
						const float y =(freq * FFT_SIZE / m_soundBuffer.getSampleRate());

						const sf::Vector2f position = {
							std::floorf(static_cast<float>(m_viewportTexture.getSize().x)),
							std::floorf(static_cast<float>(get_logarithmic_y(y, m_viewportTexture.getSize().y, m_options.highFrequency, m_soundBuffer.getSampleRate()))) };

						draw_label(m_viewportTexture, position, freq);
					}
				}
				else {
					for (int i = 0; i < 7; i++) {
						const float freq = (1000.0f + i * (20000.0f - 2000.0f) / 6.0f) * m_options.highFrequency / 20000.0f;
						const float y = (freq * FFT_SIZE / m_soundBuffer.getSampleRate());

						const sf::Vector2f position = {
							static_cast<float>(m_viewportTexture.getSize().x),
							static_cast<float>(get_linear_y(y, m_viewportTexture.getSize().y, m_options.highFrequency, m_soundBuffer.getSampleRate())) };

						draw_label(m_viewportTexture, position, freq);
					}
				}
			}
			{ // DRAW VERTICAL LINES
				sf::Text text;
				text.setFont(m_font);
				text.setCharacterSize(14);
				text.setOutlineThickness(1.0f);
				text.setOutlineColor(sf::Color::Black);

				const float timeScale = (static_cast<float>(HOP_SIZE * m_viewportTexture.getSize().x) / m_soundBuffer.getSampleRate()) / m_options.timeScale;
				const float secondsInOnePixel = HOP_SIZE / (timeScale * m_soundBuffer.getSampleRate());
				const float pixelsInOneSecond = 1.0f / secondsInOnePixel;
				const int iterations = m_options.showGrid ? static_cast<int>((m_viewportTexture.getSize().x - 150.0f) / 200.0f) : 1;
				for (int i = 0; i < iterations; i++) {
					const float x = m_viewportTexture.getSize().x * 0.5f + i * 100;
					const float y = std::roundf(m_viewportTexture.getSize().y - 10.0f);
					const float mirroredX = m_viewportTexture.getSize().x - x;
					const float seconds = m_smoothOffset + secondsInOnePixel * (i * 100);
					draw_line(x, x, 0, y - 15.0f, i == 0 ? sf::Color::Red : sf::Color::White);
					if (i != 0) {
						draw_line(mirroredX, mirroredX, 0, y - 15.0f, sf::Color::White);
					}

					std::stringstream ss;
					ss << std::roundf(seconds * 100.0f) / 100.0f << "s";
					text.setString(ss.str());
					text.setPosition({ std::floorf(x), y });
					text.setOrigin({ std::floorf(text.getGlobalBounds().getSize().x * 0.5f), std::floorf(text.getGlobalBounds().getSize().y) });
					text.setFillColor(i == 0 ? sf::Color::Red : sf::Color::White);
					m_viewportTexture.draw(text);

					if (i != 0) {
						const float seconds2 = m_smoothOffset + secondsInOnePixel * (-i * 100);
						ss.str("");
						ss << std::roundf(seconds2 * 100.0f) / 100.0f << "s";
						text.setString(ss.str());
						text.setPosition({ std::floorf(mirroredX), y });
						text.setOrigin({ std::floorf(text.getGlobalBounds().getSize().x * 0.5f), std::floorf(text.getGlobalBounds().getSize().y) });
						m_viewportTexture.draw(text);
					}
				}
			}
		}

		m_viewportTexture.display();

		m_layout.dock_space();

		m_layout.viewport_begin();
			ImGui::Image(m_viewportTexture);
		m_layout.viewport_end();

		if (!m_fillScreen) {
			m_layout.side_panel_begin();
			m_optionsView.draw_gui();

			ImGui::Text("%s / %s", ks::make_time_string(m_offset).c_str(), ks::make_time_string(m_soundBuffer.getDuration().asSeconds()).c_str());
			m_layout.side_panel_end();

			m_layout.console_begin();
			m_console.draw_logs();
			m_layout.console_end();
		}
	}

	auto App::draw() -> void {
		m_window.clear();

		ImGui::SFML::Render(m_window);
		m_window.display();
	}
	auto App::reload_audio() -> void {
		if (m_options.filePath == "") return;
		m_offset = 0.0f;
		m_smoothOffset = 0.0f;
		m_smoothOffset.finish();

		m_options.paused = 1;
		m_optionsView.refresh_internal_state();

		m_spectrogram.set_max_frequency(m_options.highFrequency);
		m_spectrogram.set_scale(m_options.scale);

		if (m_sound.getStatus() == sf::Sound::Status::Playing) {
			m_sound.stop();
		}
		m_sound = sf::Sound();
		m_soundBuffer = sf::SoundBuffer();
		sf::Clock c;
		if (!m_soundBuffer.loadFromFile(m_options.filePath.string())) {
			m_soundBuffer = sf::SoundBuffer();
			m_console.log("Invalid file. Supported formats: .wav, .mp3, .ogg, .flac");
			return;
		}
		m_sound.setBuffer(m_soundBuffer);
		m_sound.setVolume(m_options.volume);
		const float loadingTime = c.restart().asMicroseconds() / 1000.0f;

		c.restart();
		auto samples = ks::extract_samples(m_soundBuffer);
		//const float extractionTime = c.restart().asMicroseconds() / 1000.0f;

		auto collection = ks::process_samples(samples, m_options);
		m_console.log("Loaded \"" + m_options.filePath.filename().string() + "\" in " 
			+ std::to_string(loadingTime) + "ms. Processed in " 
			+ std::to_string(c.getElapsedTime().asMicroseconds() / 1000.0f) 
			+ "ms. (Library: " + (m_options.library == ks::LibraryOption::CPP ? "C++" : "asm") + "; threads: " + std::to_string(m_options.threads) + ")");
		m_spectrogram.set_collection(collection);

		// Used to verify the correctness of the assembly implementation.
#ifdef ENABLE_TEST
		auto referenceSamples = ks::Test::extract_samples(m_soundBuffer);
		auto referenceMags = ks::Test::process_samples(referenceSamples, m_options);
		auto normalMags = ks::process_samples(samples, m_options);

		bool ok = 1;
		for (int j = 0; j < normalMags.size() && ok; j++) {
			for (int i = 0; i < normalMags[j].size(); i++) {
				const float d = normalMags[j][i] - referenceMags[j][i];
				if (d > 0.01f || d < -0.01f) {
					std::cout << i << ") " << normalMags[j][i] << ", " << referenceMags[j][i] << '\n';
					m_console.log("Failed test");
					ok = 0;
					break;
				}
			}
		}
		if (ok) {
			m_console.log("Passed test");
		}
#endif
	}
	auto App::log_library(const std::string& name) -> void {
		m_console.log("Loaded " + name + " library (" + std::to_string(ks::Library::get_library_id()) + ")");
	}
}