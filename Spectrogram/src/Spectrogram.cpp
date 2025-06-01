#include "Spectrogram.hpp"
#include "Util.hpp"

#include <cmath>
#include <assert.h>
#include <iostream>

namespace ks {
	static constexpr int CHUNK_SIZE = 128;

	static auto calculate_color(float t) -> sf::Color {
		t = std::clamp<float>(t, 0.0f, 1.0f);
		sf::Color c;

		if (t < 0.125f) {
			return interpolate_color(t * 8.0f, { 0, 0, 0 }, { 29, 17, 70 });
		}
		else if (t < 0.25f) {
			return interpolate_color((t - 0.125f) * 8.0f, { 29, 17, 70 }, { 80, 18, 123 });
		}
		else if (t < 0.375f) {
			return interpolate_color((t - 0.25f) * 8.0f, { 80, 18, 123 }, { 129, 37, 130 });
		}
		else if (t < 0.5f) {
			return interpolate_color((t - 0.375f) * 8.0f, { 129, 37, 130 }, { 181, 55, 122 });
		}
		else if (t < 0.625f) {
			return interpolate_color((t - 0.5f) * 8.0f, { 181, 55, 122 }, { 229, 80, 100 });
		}
		else if (t < 0.75f) {
			return interpolate_color((t - 0.625f) * 8.0f, { 229, 80, 100 }, { 251, 135, 97 });
		}
		else if (t < 0.875f) {
			return interpolate_color((t - 0.75f) * 8.0f, { 251, 135, 97 }, { 254, 195, 135 });
		}
		else {
			return interpolate_color((t - 0.875f) * 8.0f, { 254, 195, 135 }, { 252, 252, 190 });
		}

		assert(0);
		return {};
	};

	Spectrogram::Spectrogram(sf::SoundBuffer& soundBuffer)
		:	m_soundBuffer(soundBuffer) {

	}

	auto Spectrogram::update(const float deltaTime) -> void {
		int updates{};
		const int startIndex = std::max(0, static_cast<int>(m_offset / CHUNK_SIZE - std::ceilf(static_cast<float>(m_viewportSize.x) / CHUNK_SIZE)));
		const int endIndex = std::min(static_cast<int>(m_chunks.size()), static_cast<int>(m_offset / CHUNK_SIZE + std::ceilf(static_cast<float>(m_viewportSize.x) / CHUNK_SIZE)));
		for (int i = startIndex; i < endIndex; i++) {
			if (m_chunks[i].generated) continue;
			generate_chunk(i);
			updates++;
		}
	}

	auto Spectrogram::draw(sf::RenderTarget& window) -> void {
		const int startIndex = std::max(0, static_cast<int>(m_offset / CHUNK_SIZE - std::ceilf(static_cast<float>(m_viewportSize.x) / CHUNK_SIZE)));
		const int endIndex = std::min(static_cast<int>(m_chunks.size()), static_cast<int>(m_offset / CHUNK_SIZE + std::ceilf(static_cast<float>(m_viewportSize.x) / CHUNK_SIZE)));
		for (int i = startIndex; i < endIndex; i++) {
			if (!m_chunks[i].generated) continue;

			if (i * CHUNK_SIZE - m_offset > m_viewportSize.x) {
				break;
			}
			sf::RectangleShape shape;
			shape.setTexture(&m_chunks[i].texture);
			shape.setSize(static_cast<sf::Vector2f>(m_chunks[i].texture.getSize()));
			shape.setPosition({ i * CHUNK_SIZE - m_offset + m_viewportSize.x / 2.0f, 0.0f });
			window.draw(shape);
		}
	}
	auto Spectrogram::set_collection(const std::vector<std::vector<float>>& collection) -> void {
		m_magnitudeCollection = collection;
		m_mindB = m_magnitudeCollection[0][0];
		m_maxdB = m_magnitudeCollection[0][0];
		for (int i = 0; i < m_magnitudeCollection.size(); i++) {
			for (int j = 0; j < m_magnitudeCollection[i].size(); j++) {
				if (m_magnitudeCollection[i][j] >= m_maxdB) {
					m_maxdB = m_magnitudeCollection[i][j];
				}
				else if (m_magnitudeCollection[i][j] < m_mindB) {
					m_mindB = m_magnitudeCollection[i][j];
				}
			}
		}

		resize_chunks();
	}

	auto Spectrogram::generate_chunk(const int index) -> void {
		sf::Image image;
		image.create(CHUNK_SIZE, m_viewportSize.y);

		for (int x = 0; x < CHUNK_SIZE; x++) {
			const int magIndex = static_cast<int>((x + index * CHUNK_SIZE) / m_timeScale);
			if (magIndex >= m_magnitudeCollection.size()) break;

			auto& magnitudes = m_magnitudeCollection[magIndex];
			for (int j = 0; j < magnitudes.size(); j++) {
				const float mag = std::clamp<float>(magnitudes[j] * 1.0f, 0.0f, 255.0f);

				const float normalized = (mag - m_mindB) / (m_maxdB - m_mindB);
				const sf::Color color = calculate_color(normalized * m_saturation);

				if (m_scaleOption == ks::ScaleOption::LINEAR) {
					const int lastY = get_linear_y(std::max(0, j - 1), m_viewportSize.y, m_maxFrequency, m_soundBuffer.getSampleRate());
					const int y = get_linear_y(j, m_viewportSize.y, m_maxFrequency, m_soundBuffer.getSampleRate());
					if (lastY < 0 && y < 0) {
						break;
					}
					draw_horizontal_line(image, color, x, lastY, y);
				}
				else {
					const int logLastY = get_logarithmic_y(std::max(0, j - 1), m_viewportSize.y, m_maxFrequency, m_soundBuffer.getSampleRate());
					const int logY = get_logarithmic_y(j, m_viewportSize.y, m_maxFrequency, m_soundBuffer.getSampleRate());
					if (logLastY < 0 && logY < 0) {
						break;
					}
					draw_horizontal_line(image, color, x, logLastY, logY);
				}
			}
		}

		m_chunks[index].texture.loadFromImage(image);
		m_chunks[index].generated = 1;
	}
	auto Spectrogram::resize_chunks() -> void {
		const int chunkNum = static_cast<int>(std::ceilf(static_cast<float>(m_magnitudeCollection.size()) / CHUNK_SIZE * m_timeScale));
		m_chunks.clear();
		m_chunks.resize(chunkNum);
	}
}
