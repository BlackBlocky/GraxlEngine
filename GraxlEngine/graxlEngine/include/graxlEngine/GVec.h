#pragma once

// GVec<T,N> by blackblocky
// requires c++20 or newer

#include <concepts>
#include <array>
#include <cassert>
#include <exception>
#include <string>
#include <sstream>
#include <type_traits>

namespace graxl {

	template<typename T>
	concept AllowedVecTypes =
		std::same_as<T, int> ||
		std::same_as<T, long> ||
		std::same_as<T, short> ||
		std::same_as<T, char> ||

		std::same_as<T, unsigned int> ||
		std::same_as<T, unsigned long> ||
		std::same_as<T, unsigned short> ||

		std::same_as<T, unsigned char> ||
		std::same_as<T, signed char> ||

		std::same_as<T, float> ||
		std::same_as<T, double> ||
		std::same_as<T, long double>;

	template<AllowedVecTypes T, size_t N>
	class GVec {
		static_assert(N > 0, "GVec<N> N needs to be larger than 0!");

	public:
		// Constructur

		GVec<T, N>() : _values{} {};

		template<typename U>
			requires std::is_convertible_v<U, T>
		GVec<T, N>(U u) : _values{}
		{
			_values.fill(static_cast<T>(u));
		};

		template<typename... Args>
			requires (sizeof...(Args) == N) && (std::conjunction_v<std::is_convertible<Args, T>...>)
		GVec<T, N>(Args... args) : _values{}
		{
			_values = { static_cast<T>(args)... };
		};


		// Reference Getter for _values N

		T& x() noexcept requires (N >= 1) { return _values[0]; };
		T& y() noexcept requires (N >= 2) { return _values[1]; };
		T& z() noexcept requires (N >= 3) { return _values[2]; };
		T& a() noexcept requires (N >= 4) { return _values[3]; };
		T& b() noexcept requires (N >= 5) { return _values[4]; };
		T& c() noexcept requires (N >= 6) { return _values[5]; };

		const T& x() const noexcept requires (N >= 1) { return _values[0]; };
		const T& y() const noexcept requires (N >= 2) { return _values[1]; };
		const T& z() const noexcept requires (N >= 3) { return _values[2]; };
		const T& a() const noexcept requires (N >= 4) { return _values[3]; };
		const T& b() const noexcept requires (N >= 5) { return _values[4]; };
		const T& c() const noexcept requires (N >= 6) { return _values[5]; };

		T& operator[](size_t index) {
			assert(index < N);
			return _values[index];
		};
		const T& operator[](size_t index) const {
			assert(index < N);
			return _values[index];
		};


		// Operators - Math

		// compound assignment operators

		template<typename U>
			requires std::is_convertible_v<U, T>
		GVec<T, N>& operator+=(const GVec<U, N>& other) noexcept {
			for (size_t i = 0; i < N; i++)
				_values[i] += static_cast<T>(other[i]);
			return *this;
		};

		template<typename U>
			requires std::is_convertible_v<U, T>
		GVec<T, N>& operator-=(const GVec<U, N>& other) noexcept {
			for (size_t i = 0; i < N; i++)
				_values[i] -= static_cast<T>(other[i]);
			return *this;
		};


		template<typename U>
			requires std::is_convertible_v<U, T>
		GVec<T, N>& operator*=(const GVec<U, N>& other) noexcept {
			for (size_t i = 0; i < N; i++)
				_values[i] *= static_cast<T>(other[i]);
			return *this;
		};

		template<typename U>
			requires std::is_convertible_v<U, T>
		GVec<T, N>& operator*=(const U& factor) noexcept {
			for (size_t i = 0; i < N; i++)
				_values[i] *= static_cast<T>(factor);
			return *this;
		};


		// For a divison with zero will the result be 0
		template<typename U>
			requires std::is_convertible_v<U, T>
		GVec<T, N>& operator/=(const GVec<U, N>& other) {
			for (size_t i = 0; i < N; i++) {
				if (other[i] == T{}) _values[i] = T{}; // Divison by zero catch
				else _values[i] /= static_cast<T>(other[i]);
			}
			return *this;
		};

		// Returns a zero-vector on division with 0
		template<typename U>
			requires std::is_convertible_v<U, T>
		GVec<T, N>& operator/=(const U& divisor) {
			if (divisor == T{}) { // Divison by zero catch
				for (size_t i = 0; i < N; i++)
					_values[i] = T{};
				return *this;
			}

			for (size_t i = 0; i < N; i++)
				_values[i] /= static_cast<T>(divisor);
			return *this;
		};


		// normal operators

		template<typename U>
			requires std::is_convertible_v<U, T>
		GVec<T, N> operator+(const GVec<U, N>& other) const noexcept {
			GVec<T, N> result = *this;
			result += other;
			return result;
		};

		template<typename U>
			requires std::is_convertible_v<U, T>
		GVec<T, N> operator-(const GVec<U, N>& other) const noexcept {
			GVec<T, N> result = *this;
			result -= other;
			return result;
		};


		template<typename U>
			requires std::is_convertible_v<U, T>
		GVec<T, N> operator*(const GVec<U, N>& other) const noexcept {
			GVec<T, N> result = *this;
			result *= other;
			return result;
		};

		template<typename U>
			requires std::is_convertible_v<U, T>
		GVec<T, N> operator*(const U& factor) const noexcept {
			GVec<T, N> result = *this;
			result *= factor;
			return result;
		};


		template<typename U>
			requires std::is_convertible_v<U, T>
		GVec<T, N> operator/(const GVec<U, N>& other) const noexcept {
			GVec<T, N> result = *this;
			result /= other;
			return result;
		};

		template<typename U>
			requires std::is_convertible_v<U, T>
		GVec<T, N> operator/(const U& divisor) const noexcept {
			GVec<T, N> result = *this;
			result /= divisor;
			return result;
		};


		// Operator - Logical

		template<typename U>
			requires std::is_convertible_v<U, T>
		bool operator==(const GVec<U, N>& other) const noexcept {
			for (size_t i = 0; i < N; i++) {
				if (_values[i] != static_cast<T>(other[i])) return false;
			}
			return true;
		};
		
		template<typename U>
			requires std::is_convertible_v<U, T>
		bool operator!=(const GVec<U, N>& other) const noexcept {
			return !(*this == other);
		};

		// Functions

		std::string string() const noexcept {
			std::stringstream ss;
			ss << "(";
			for (size_t i = 0; i < N; ++i) {
				ss << _values[i];
				if (i + 1 < N) ss << ","; // No comma after last iteration
			}
			ss << ")";
			return ss.str();
		};

		
		
		T magnitude() const noexcept
			requires std::is_same_v<T, float>
		{
			float sum = 0.0f;
			for (auto f : _values) sum += (f * f);
			return std::sqrtf(sum);
		};

		T magnitude() const noexcept
			requires std::is_same_v<T, double>
		{
			double sum = 0.0;
			for (auto d : _values) sum += (d * d);
			return std::sqrt(sum);
		};

		T magnitude() const noexcept
			requires std::is_same_v<T, long double>
		{
			long double sum = 0.0;
			for (auto d : _values) sum += (d * d);
			return std::sqrtl(sum);
		};

		T magnitude() const noexcept // (Everyting instead of float, double, long double)
			requires (!std::is_floating_point_v<T>)
		{
			T sum = T{};
			for (auto t : _values) sum += (t * t);
			return static_cast<T>(std::sqrt(sum));
		};


		// Returns a zero-vector of magnitude() is zero
		GVec<T, N> normalized() const noexcept {
			GVec<T, N> result = *this;
			result /= this->magnitude(); // (Division by zero is already handled by the /= operator)
			return result;
		};


	private:
		std::array<T, N> _values;

	};


	// Some little typing saving
	using GVec2f = GVec<float, 2>;
	using GVec3f = GVec<float, 3>;
	using GVec4f = GVec<float, 4>;

	using GVec2i = GVec<int, 2>;
	using GVec3i = GVec<int, 3>;
	using GVec4i = GVec<int, 4>;

	using GVec2d = GVec<double, 2>;
	using GVec3d = GVec<double, 3>;
	using GVec4d = GVec<double, 4>;

}