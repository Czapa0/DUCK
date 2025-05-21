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

		private:
			static constexpr UINT N = 256;
		};
	}
}
