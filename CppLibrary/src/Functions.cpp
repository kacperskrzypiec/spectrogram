#include <numbers>
#include <cmath>
#include <vector>
#include <complex>

namespace ks {
	struct Complex8f {
		float r[8]{};
		float i[8]{};
	};
}

namespace ks::Cpp {
    extern "C" __declspec(dllexport) 
    auto get_library_id() -> int {
        return 67;
    }

    extern "C" __declspec(dllexport)
    auto hann_window(float* samples, const std::size_t size) -> void{
        for (int i = 0; i < size; i++) {
			samples[i] *= 0.5f * (1.0f - std::cosf((2.0f * std::numbers::pi_v<float> * i) / (size - 1)));
        }
    }

	extern "C" __declspec(dllexport)
	auto compute_magnitudes(const ks::Complex8f* frequencies, float* magnitudes, const std::size_t size) -> void {
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < 8; j++) {
				const float real = frequencies[i].r[j];
				const float imag = frequencies[i].i[j];

				float mag = 2.0f * std::sqrtf(real * real + imag * imag) + 0.0001f;
				mag = 20.0f * std::log10f(mag);
				mag = std::max(mag, 0.0f);

				magnitudes[i * 8 + j] = mag;
			}
		}
	}

	extern "C" __declspec(dllexport)
	auto compute_part_of_fft(ks::Complex8f* out, const Complex8f& win, const std::size_t inSize, const int m) -> void {
		const std::complex<float> wm(win.r[1], win.i[1]);
		const std::complex<float> w_m8 = ((wm * wm) * (wm * wm)) * ((wm * wm) * (wm * wm));

		Complex8f w = win;

		Complex8f w_m8v;
		for (int i = 0; i < 8; i++) {
			w_m8v.r[i] = w_m8.real();
			w_m8v.i[i] = w_m8.imag();
		}

		const int m2 = m >> 1;

		for (int j = 0; j < m2; j += 8) {
			for (int k = 0; k < inSize; k += m) {
				const int index1 = (k + j) / 8;
				const int index2 = (k + j + m2) / 8;

				Complex8f u = out[index1];
				Complex8f t = out[index2];

				for (int l = 0; l < 8; l++) {
					const float realPart = t.r[l] * w.r[l] - t.i[l] * w.i[l];
					t.i[l] = t.r[l] * w.i[l] + t.i[l] * w.r[l];
					t.r[l] = realPart;

					out[index1].r[l] = u.r[l] + t.r[l];
					out[index1].i[l] = u.i[l] + t.i[l];

					out[index2].r[l] = u.r[l] - t.r[l];
					out[index2].i[l] = u.i[l] - t.i[l];
				}
			}
			for (int l = 0; l < 8; l++) {
				const float realPart = w.r[l] * w_m8v.r[l] - w.i[l] * w_m8v.i[l];
				w.i[l] = w.r[l] * w_m8v.i[l] + w.i[l] * w_m8v.r[l];
				w.r[l] = realPart;
			}
		}
	}

	extern "C" __declspec(dllexport)
	auto extract_mono_samples(const short* rawSamples, float* samples, const std::size_t size) -> void {
		for (uint64_t i = 0; i < (size << 3); i++) {
			const short sample = rawSamples[i];
			samples[i] = sample > 0 ? sample / 32767.0f : sample / 32768.0f;
		}
	}

	extern "C" __declspec(dllexport)
	auto extract_stereo_samples(const short* rawSamples, float* samples, const std::size_t size) -> void {
		for (uint64_t i = 0; i < (size << 4); i += 2) {
			const short sample = (rawSamples[i] >> 1) + (rawSamples[i + 1] >> 1);
			samples[i / 2] = sample > 0 ? sample / 32767.0f : sample / 32768.0f;
		}
	}
}
