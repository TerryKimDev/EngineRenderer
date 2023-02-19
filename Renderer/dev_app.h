#pragma once
#include <cstdint>
#include <chrono>
#include <bitset>
#include <windowsx.h>
#include "math_types.h"
#include <DirectXMath.h>
#include "frustum_culling.h"
namespace end
{
	// Simple app class for development and testing purposes
	struct dev_app_t
	{

		float elapse = 0.f;
		std::bitset<15> mouse;
		std::bitset<256 > keypress;
		int mx, my, mx_prev, my_prev;
		void update(float4x4_a& camera);
		void keypressing();
		dev_app_t();
		void mouselook(float4x4_a& cam);
		void worldmove(float4x4_a& cam);
		double get_delta_time()const;

	};
	
	
}