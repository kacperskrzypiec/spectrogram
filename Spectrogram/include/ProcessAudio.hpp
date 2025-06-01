#pragma once

#include "Options.hpp"
#include "Complex8.hpp"

#include <SFML/Audio/SoundBuffer.hpp>

#include <vector>
#include <array>
#include <complex>

namespace ks {
	constexpr int FFT_SIZE = 4096;
	constexpr int HOP_SIZE = 512;

	extern auto extract_samples(const sf::SoundBuffer& soundBuffer) -> std::vector<float>;
	extern auto process_samples(const std::vector<float>& samples, const Options& options) -> std::vector<std::vector<float>>;
	extern auto fft(const std::vector<float>& in) -> std::vector<Complex8f>;

#ifdef ENABLE_TEST
	namespace Test {
		extern auto extract_samples(const sf::SoundBuffer& soundBuffer) -> std::vector<float>;
		extern auto process_samples(const std::vector<float>& samples, const Options& options) -> std::vector<std::vector<float>>;
		extern auto fft(const std::vector<float>& in) -> std::vector<std::complex<float>>;
	}
#endif
}