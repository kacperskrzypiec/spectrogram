#pragma once

#include "Options.hpp"
#include "ProcessAudio.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics.hpp>

namespace ks {
	class Spectrogram {
	public:
		Spectrogram(sf::SoundBuffer& soundBuffer);
		~Spectrogram() = default;

		auto update(const float deltaTime) -> void;
		auto draw(sf::RenderTarget& window) -> void;

		auto set_collection(const std::vector<std::vector<float>>& collection) -> void;

		auto set_offset(const float x) -> void {
			m_offset = x * (static_cast<float>(m_soundBuffer.getSampleRate()) / ks::HOP_SIZE) * m_timeScale;
		}
		auto on_viewport_size_change(const sf::Vector2i v) -> void {
			m_viewportSize = v;
			invalidate_chunks();
		}
		auto set_max_frequency(const int x) -> void {
			m_maxFrequency = x;
			invalidate_chunks();
		}
		auto set_scale(const ks::ScaleOption option) -> void {
			m_scaleOption = option;
			invalidate_chunks();
		}
		auto set_saturation(const float x) -> void {
			m_saturation = x;
			invalidate_chunks();
		}
		auto set_time_scale(const float x) -> void {
			m_timeScale = (static_cast<float>(HOP_SIZE * m_viewportSize.x) / m_soundBuffer.getSampleRate()) / x;
			resize_chunks();
		}

	private:
		struct Chunk {
			sf::Texture texture;
			bool generated{};
		};

		auto generate_chunk(const int index) -> void;
		auto resize_chunks() -> void;
		auto invalidate_chunks() -> void {
			for (auto& chunk : m_chunks) {
				chunk.generated = 0;
			}
		}

	private:
		sf::SoundBuffer& m_soundBuffer;
		std::vector<std::vector<float>> m_magnitudeCollection;
		std::vector<Chunk> m_chunks;
		float m_offset{};
		sf::Vector2i m_viewportSize;
		int m_maxFrequency{ 5000 };
		float m_saturation{ 1.0f };
		float m_mindB{};
		float m_maxdB{};
		float m_timeScale{ 1.0f };
		ks::ScaleOption m_scaleOption{ ks::ScaleOption::LOGARITHMIC };
	};
}