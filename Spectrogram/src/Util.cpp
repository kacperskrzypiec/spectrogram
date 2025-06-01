#include "Util.hpp"

#include <cmath>
#include <sstream>
#include "ProcessAudio.hpp"

namespace ks {
	auto pad_integer_to_string(const int value, const int padding) -> std::string {
		if (value < 0) return std::string(padding, '0');
		const int digits = value == 0 ? 1 : static_cast<int>(std::log10f(static_cast<float>(value))) + 1;

		std::stringstream ss;
		if (padding >= digits) {
			for (int i = 0; i < padding - digits; i++) {
				ss << '0';
			}
		}
		ss << value;

		return ss.str();
	}
	auto make_time_string(const float x) -> std::string {
		const int hours = static_cast<int>(x / 3600.0f);
		const int minutes = static_cast<int>(x / 60.0f) % 60;
		const int seconds = static_cast<int>(x) % 60;

		std::stringstream ss;

		if (hours > 0) {
			ss << hours << ":" << pad_integer_to_string(minutes, 2) << pad_integer_to_string(seconds, 2);
		}
		else {
			ss << std::to_string(minutes) << ":" << pad_integer_to_string(seconds, 2);
		}

		return ss.str();
	}
	auto interpolate_color(const float t, const sf::Color c1, const sf::Color c2) -> sf::Color {
		sf::Color res;
		res.a = 255;

		res.r = static_cast<sf::Uint8>(c1.r * (1.0f - t) + c2.r * t);
		res.g = static_cast<sf::Uint8>(c1.g * (1.0f - t) + c2.g * t);
		res.b = static_cast<sf::Uint8>(c1.b * (1.0f - t) + c2.b * t);

		return res;
	}
	auto get_linear_y(const float k, const int height, const int maxFrequency, const int sampleRate) -> int {
		const float binSize = static_cast<float>(sampleRate) / FFT_SIZE;
		const float resolution = height / (maxFrequency / binSize);

		return static_cast<int>(
			std::floorf(
				static_cast<float>(height - 1) - (float(k) * resolution)));
	}
	auto get_logarithmic_y(const float k, const int height, const int maxFrequency, const int sampleRate) -> int {
		const float binSize = static_cast<float>(sampleRate) / FFT_SIZE;
		const float factor = std::log10f(static_cast<float>(FFT_SIZE) / 2.0f) / std::log10f(maxFrequency / binSize + 1.0f);

		return static_cast<int>(
			std::floorf(
				static_cast<float>(height - 1) - (static_cast<float>(std::log10f(static_cast<float>(k + 1))) / std::log10f(static_cast<float>(FFT_SIZE) / 2.0f) * height * factor)));

	}
	auto draw_horizontal_line(sf::Image& image, sf::Color color, const int x, int fromY, int toY) -> void {
		if (fromY > toY) {
			std::swap(fromY, toY);
		}
		for (int k = fromY; k <= toY; k++) {
			if (k >= 0 && k < static_cast<int>(image.getSize().y)) {
				image.setPixel(x, k, color);
			}
		}
	}
}