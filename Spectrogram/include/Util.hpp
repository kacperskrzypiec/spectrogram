#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <string>

namespace ks {
	extern auto pad_integer_to_string(const int value, const int padding) -> std::string;
	extern auto make_time_string(const float x) -> std::string;
	extern auto interpolate_color(const float t, const sf::Color c1, const sf::Color c2) -> sf::Color;
	extern auto get_linear_y(const float k, const int height, const int maxFrequency, const int sampleRate) -> int;
	extern auto get_logarithmic_y(const float k, const int height, const int maxFrequency, const int sampleRate) -> int;
	extern auto draw_horizontal_line(sf::Image& image, sf::Color color, const int x, int fromY, int toY) -> void;
}