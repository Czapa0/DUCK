#pragma once
#include "duckBase.h"
#include <random>

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
			void updateDuck();

			void calculateDamping();
			void updateBumps();
			void spawnDrop();

			void addNewControlPoint();
			DirectX::XMVECTOR evaluateCubicBSpline(float t);
			DirectX::XMVECTOR evaluateCubicBSplineTangent(float t);

		private:
			static constexpr UINT N = 256;
			static constexpr float h = 2.f / (N - 1);
			static constexpr float c = 1.f;
			static constexpr float dt = 1.f / N;
			static constexpr float A = c * c * dt * dt / (h * h);
			static constexpr float B = 2.f - 4.f * A;
			static constexpr std::pair<float, float> BUMP_RANGE = { 0.05f, 0.1f };
			static constexpr float BSPLINE_SEGMENT_TIME = 1.f;

			size_t m_duck;
			float m_time = 0.f;
			std::deque<DirectX::XMVECTOR> m_controlPoints;

			std::array<std::array<float, N>, N> m_d;
			std::array<std::array<float, N + 2>, N + 2> m_z1, m_z2;

			std::mt19937 m_randomGen;
			std::uniform_real_distribution<float> m_spawnHeight{ BUMP_RANGE.first, BUMP_RANGE.second };
			std::uniform_real_distribution<float> m_spawnProbability{ 0.f, 1.f };
			std::uniform_int_distribution<UINT> m_spawnPosition{ 1U, N };
			std::uniform_real_distribution<float> m_controlPointPosition{ -0.8f, 0.8f };
		};
	}
}
