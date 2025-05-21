#pragma once
#include "duckBase.h"

namespace mini
{
	namespace gk2
	{
		class Duck : public DuckBase
		{
		public:
			explicit Duck(HINSTANCE hInst);

		protected:
			void update(utils::clock const& clock) override;
			void updateWater();

			void calculateDamping();
			void updateBumps();

		private:
			static constexpr UINT N = 256;
			static constexpr float h = 2.f / (N - 1);
			static constexpr float c = 1.f;
			static constexpr float dt = 1.f / N;
			static constexpr float A = c * c * dt * dt / (h * h);
			static constexpr float B = 2.f - 4.f * A;

			std::array<std::array<float, N>, N> d;
			std::array<std::array<float, N + 2>, N + 2> z1, z2;
		};
	}
}
