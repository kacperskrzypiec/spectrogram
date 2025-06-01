#pragma once

#include <cmath>
#include <concepts>

namespace ks {
	template<std::floating_point T>
	class SmoothReal {
	public:
		SmoothReal() = default;
		SmoothReal(const T target)
			: m_target(target), m_isChanging(1) {
		}
		SmoothReal(const T target, const T decay)
			: m_target(target), m_isChanging(1), m_decay(decay) {
		}
		SmoothReal(const T target, const T decay, const T value)
			: m_target(target), m_isChanging(1), m_decay(decay), m_value(value) {
		}
		~SmoothReal() = default;

		auto update(const T deltaTime) -> void {
			if (!m_isChanging) return;

			const T diff = m_value - m_target;
			if constexpr (std::same_as<T, float>) {
				m_value = m_target + diff * std::exp2f(-m_decay * deltaTime);
			}
			else if constexpr (std::same_as<T, double>) {
				m_value = m_target + diff * std::exp2(-m_decay * deltaTime);
			}

			if (diff < T{ 0.001 } && diff > T{ -0.001 }) {
				m_value = m_target;
				m_isChanging = 0;
			}
		}

		auto finish() -> void {
			m_isChanging = 0;
			m_value = m_target;
		}

		auto value() const -> T {
			return m_value;
		}
		auto value(const T x) -> void {
			m_isChanging = 1;
			m_value = x;
		}
		auto target() const -> T {
			return m_target;
		}
		auto target(const T x) -> void {
			m_isChanging = 1;
			m_target = x;
		}
		auto decay() const -> T {
			return m_decay;
		}
		auto decay(const T x) -> void {
			m_decay = x;
		}
		auto is_changing() const -> bool {
			return m_isChanging;
		}

		auto operator =(const T x) -> SmoothReal<T>& {
			target(x);
			return *this;
		}
		operator T() const {
			return value();
		}

	private:
		T m_value{};
		T m_target{};
		T m_decay{};

		bool m_isChanging{};
	};

	using SmoothFloat = SmoothReal<float>;
	using SmoothDouble = SmoothReal<double>;
}