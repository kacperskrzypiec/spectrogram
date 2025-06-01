#include "ProcessAudio.hpp"
#include "Functions.hpp"

#include <numbers>
#include <assert.h>
#include <thread>
#include <cstring>
#include <iostream>
#include <immintrin.h>

namespace ks {
	auto extract_samples(const sf::SoundBuffer& soundBuffer) -> std::vector<float>{
		std::vector<float> samples(
			soundBuffer.getChannelCount() == 2 
			? soundBuffer.getSampleCount() / 2 
			: soundBuffer.getSampleCount());

		auto put_raw_sample = [&](const uint64_t index, const float sample) -> void {
			samples[index] = sample > 0 ? sample / 32767.0f : sample / 32768.0f;
		};

		if (soundBuffer.getChannelCount() == 2) {
			ks::Library::extract_stereo_samples(soundBuffer.getSamples(), samples.data(), soundBuffer.getSampleCount() >> 4);

			for (uint64_t i = (soundBuffer.getSampleCount() >> 4) << 4; i < soundBuffer.getSampleCount(); i += 2) {
				const int16_t rawSample = soundBuffer.getSamples()[i] / 2 + soundBuffer.getSamples()[i + 1] / 2;
				put_raw_sample(i / 2, rawSample);
			}
		}
		else {
			ks::Library::extract_mono_samples(soundBuffer.getSamples(), samples.data(), soundBuffer.getSampleCount() >> 3);

			for (uint64_t i = (soundBuffer.getSampleCount() >> 3) << 3; i < soundBuffer.getSampleCount(); i++) {
				const int16_t rawSample = soundBuffer.getSamples()[i];
				put_raw_sample(i, rawSample);
			}
		}

		while (samples.size() % FFT_SIZE != 0) {
			samples.emplace_back(0.0f);
		}

		return samples;
	}

	auto process_samples(const std::vector<float>& samples, const Options& options) -> std::vector<std::vector<float>> {
		assert(samples.size() % FFT_SIZE == 0);

		auto process_chunk = [](std::vector<float>& block) -> std::vector<float> {
			ks::Library::hann_window(block.data(), block.size());

			auto frequencies = ks::fft(block);
			std::vector<float> magnitudes(block.size() / 2);
			ks::Library::compute_magnitudes(frequencies.data(), magnitudes.data(), frequencies.size() / 2);

			return magnitudes;
		};

		const std::size_t resultSize = (samples.size() - FFT_SIZE) / HOP_SIZE;
		std::vector<std::vector<float>> result(resultSize);

		auto audio_thread = [&](const int offset) -> void {
			std::vector<float> block(FFT_SIZE);
			for (int i = offset; i < resultSize; i += options.threads) {
				memcpy(block.data(), samples.data() + i * HOP_SIZE, FFT_SIZE * sizeof(float));
				result[i] = process_chunk(block);
			}
		};

		// Yes, the performance could be improved with a thread pool. 
		std::vector<std::jthread> threads;
		for (int i = 0; i < options.threads; i++) {
			threads.emplace_back(std::jthread(audio_thread, i));
		}
		
		for (auto& th : threads) {
			if (th.joinable())
				th.join();
		}

		return result;
	}
	
	auto fft(const std::vector<float>& in) -> std::vector<Complex8f> {
		std::vector<Complex8f> out(FFT_SIZE / 8);

		auto mirror_bits = [](uint32_t n, int size) -> int {
			n = ((n & 0x55555555) << 1) | ((n & 0xAAAAAAAA) >> 1); // swap bits
			n = ((n & 0x33333333) << 2) | ((n & 0xCCCCCCCC) >> 2); // swap pairs
			n = ((n & 0x0F0F0F0F) << 4) | ((n & 0xF0F0F0F0) >> 4); // swap nibbles
			n = ((n & 0x00FF00FF) << 8) | ((n & 0xFF00FF00) >> 8); // swap bytes
			n = ((n & 0x0000FFFF) << 16) | ((n & 0xFFFF0000) >> 16); // swap words

			return n >> (32 - size);
		};

		const int bits = static_cast<int>(std::log2f(static_cast<float>(in.size())));

		for (int i = 0; i < in.size(); i++) {
			const int index = mirror_bits(i, bits);
			out[index / 8].r[index % 8] = in[i];
		}

		for (int s = 1; s <= 3; s++) {
			const int m = 1 << s;
			const int m2 = m >> 1;
			const std::complex<float> w_m = std::exp(2.0f * std::complex<float>(0.0f, 1.0f) * std::numbers::pi_v<float> / static_cast<float>(m));

			std::complex<float> w = std::complex<float>(1.0f, 0.0f);
			for (int j = 0; j < m2; j++) {
				for (int k = 0; k < in.size(); k += m) {
					const int index1 = k + j;
					const int index2 = k + j + m2;

					const std::complex<float> u(out[index1 / 8].r[index1 % 8], out[index1 / 8].i[index1 % 8]);
					std::complex<float> t(out[index2 / 8].r[index2 % 8], out[index2 / 8].i[index2 % 8]);
					t *= w;

					std::complex<float> r1 = u + t;
					std::complex<float> r2 = u - t;

					out[index1 / 8].r[index1 % 8] = r1.real();
					out[index1 / 8].i[index1 % 8] = r1.imag();

					out[index2 / 8].r[index2 % 8] = r2.real();
					out[index2 / 8].i[index2 % 8] = r2.imag();
				}
				w *= w_m;
			}
		}

		for (int s = 4; s <= bits; s++) {
			const int m = 1 << s;
			const std::complex<float> w_m = std::exp(2.0f * std::complex<float>(0.0f, 1.0f) * std::numbers::pi_v<float> / static_cast<float>(m));

			ks::Complex8f w8;
			w8.r[0] = 1.0f;
			for (int dx = 1; dx < 8; dx++) {
				w8.r[dx] = w8.r[dx - 1] * w_m.real() - w8.i[dx - 1] * w_m.imag();
				w8.i[dx] = w8.r[dx - 1] * w_m.imag() + w8.i[dx - 1] * w_m.real();
			}

			ks::Library::compute_part_of_fft(out.data(), w8, in.size(), m);
		}

		return out;
	}

#ifdef ENABLE_TEST
	namespace Test {
		auto extract_samples(const sf::SoundBuffer& soundBuffer) -> std::vector<float> {
			std::vector<float> samples(
				soundBuffer.getChannelCount() == 2
				? soundBuffer.getSampleCount() / 2
				: soundBuffer.getSampleCount());

			auto put_raw_sample = [&](const uint64_t index, const float sample) -> void {
				samples[index] = sample > 0 ? sample / 32767.0f : sample / 32768.0f;
				};

			if (soundBuffer.getChannelCount() == 2) {
				for (uint64_t i = 0; i < soundBuffer.getSampleCount(); i += 2) {
					const int16_t rawSample = (soundBuffer.getSamples()[i] >> 1) + (soundBuffer.getSamples()[i + 1] >> 1);
					put_raw_sample(i / 2, rawSample);
				}
			}
			else {
				for (uint64_t i = 0; i < soundBuffer.getSampleCount(); i++) {
					const int16_t rawSample = soundBuffer.getSamples()[i];
					put_raw_sample(i, rawSample);
				}
			}

			while (samples.size() % FFT_SIZE != 0) {
				samples.emplace_back(0.0f);
			}

			return samples;
		}

		auto process_samples(const std::vector<float>& samples, const Options& options) -> std::vector<std::vector<float>> {
			assert(samples.size() % FFT_SIZE == 0);

			auto hann_window = [](std::vector<float>& samples) {
				for (int i = 0; i < samples.size(); i++) {
					samples[i] *= 0.5f * (1.0f - std::cosf(2.0f * std::numbers::pi_v<float> *i / (samples.size() - 1)));
				}
				};

			auto get_magnitudes = [](const std::vector<std::complex<float>>& frequencies)->std::vector<float> {
				std::vector<float> out(FFT_SIZE / 2);
				for (int j = 0; j < out.size(); j++) {
					const std::complex<float> c = frequencies[j];
					out[j] = 2.0f * std::sqrtf(c.real() * c.real() + c.imag() * c.imag());
				}

				for (int i = 0; i < out.size(); i++) {
					out[i] = 20.0f * std::log10f(out[i] + 0.00001f);
					if (out[i] < 0.0f) {
						out[i] = 0.0f;
					}
				}

				return out;
			};

			auto process_chunk = [&](std::vector<float>& block)->std::vector<float> {
				hann_window(block);

				auto frequencies = ks::Test::fft(block);
				auto magnitudes = get_magnitudes(frequencies);

				return magnitudes;
			};

			const std::size_t resultSize = (samples.size() - FFT_SIZE) / HOP_SIZE;
			std::vector<std::vector<float>> result(resultSize);

			auto audio_thread = [&](const int offset) -> void {
				std::vector<float> block(FFT_SIZE);
				for (int i = offset; i < resultSize; i += options.threads) {
					memcpy(block.data(), samples.data() + i * HOP_SIZE, FFT_SIZE * sizeof(float));
					auto mags = process_chunk(block);
					result[i] = mags;
				}
				};

			std::vector<std::jthread> threads;
			for (int i = 0; i < options.threads; i++) {
				threads.emplace_back(std::jthread(audio_thread, i));
			}

			for (auto& th : threads) {
				if (th.joinable())
					th.join();
			}

			return result;
		}

		auto fft(const std::vector<float>& in) -> std::vector<std::complex<float>> {
			std::vector<std::complex<float>> out(FFT_SIZE);

			auto mirror_bits = [](uint32_t n, int size) -> int {
				n = ((n & 0x55555555) << 1) | ((n & 0xAAAAAAAA) >> 1);
				n = ((n & 0x33333333) << 2) | ((n & 0xCCCCCCCC) >> 2);
				n = ((n & 0x0F0F0F0F) << 4) | ((n & 0xF0F0F0F0) >> 4);
				n = ((n & 0x00FF00FF) << 8) | ((n & 0xFF00FF00) >> 8);
				n = ((n & 0x0000FFFF) << 16) | ((n & 0xFFFF0000) >> 16);

				return n >> (32 - size);
				};

			const int bits = static_cast<int>(std::log2f(static_cast<float>(in.size())));

			for (int i = 0; i < in.size(); i++) {
				out[mirror_bits(i, bits)] = in[i];
			}

			for (int s = 1; s <= bits; s++) {
				const int m = 1 << s;
				const int m2 = m >> 1;
				const std::complex<float> w_m = std::exp(2.0f * std::complex<float>(0.0f, 1.0f) * std::numbers::pi_v<float> / static_cast<float>(m));

				for (int k = 0; k < in.size(); k += m) {
					std::complex<float> w = std::complex<float>(1.0f, 0.0f);
					for (int j = 0; j < m2; j++) {
						const std::complex<float> t = w * out[static_cast<std::size_t>(k) + j + m2];
						const std::complex<float> u = out[static_cast<std::size_t>(k) + j];

						out[static_cast<std::size_t>(k) + j] = u + t;
						out[static_cast<std::size_t>(k) + j + m2] = u - t;
						w *= w_m;
					}
				}
			}

			return out;
		}
	}
#endif
}