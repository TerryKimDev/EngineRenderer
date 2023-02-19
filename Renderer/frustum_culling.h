#pragma once

#include <array>
#include "view.h"
#include "debug_renderer.h"
#include <iostream>
// Note: You are free to make adjustments/additions to the declarations provided here.

namespace end
{
	struct sphere_t { float3 center; float radius; }; //Alterative: using sphere_t = float4;

	struct aabb_t { float3 center; float3 extents; }; //Alternative: aabb_t { float3 min; float3 max; };

	struct plane_t { float3 normal; float offset; };  //Alterative: using plane_t = float4;

	using frustum_t = std::array<plane_t, 6>;

	// Calculates the plane of a triangle from three points.
	inline plane_t calculate_plane(float3 a, float3 b, float3 c) {
		plane_t plane;
		float3 norm = norm.cross(b - a, c - b);
		plane.normal = norm.normalize(norm);

		plane.offset = norm.dot(a, plane.normal);
		return plane;
	};

	// Calculates a frustum (6 planes) from the input view parameter.
	//
	// Calculate the eight corner points of the frustum. 
	// Use your debug renderer to draw the edges.
	// 
	// Calculate the frustum planes.
	// Use your debug renderer to draw the plane normals as line segments.
	inline void calculate_frustum(frustum_t& frustum, const view_t& view) {
		float fOV = DirectX::XMConvertToRadians(55);
		float VRatio = 1280.f / 720;
		float nDist = 1;
		float fDist = 10;

		float3 FBR, FBL, FTL, FTR, NBR, NBL, NTR, NTL, FARCENTER,NEARCENTER;
		NEARCENTER = view.view_mat[3].xyz + view.view_mat[2].xyz * nDist;
		FARCENTER = view.view_mat[3].xyz + view.view_mat[2].xyz * fDist;

		float Hnear, Hfar, Wnear, Wfar;
		Hnear = 2 * tan(fOV / 2) * nDist;
		Hfar = 2 * tan(fOV / 2) * fDist;
		Wnear = Hnear * VRatio;
		Wfar = Hfar * VRatio;

		FTL = FARCENTER + view.view_mat[1].xyz * (Hfar * .5) - view.view_mat[0].xyz * (Wfar * .5);
		FTR = FARCENTER + view.view_mat[1].xyz * (Hfar * .5) + view.view_mat[0].xyz * (Wfar * .5);
		FBL = FARCENTER - view.view_mat[1].xyz * (Hfar * .5) - view.view_mat[0].xyz * (Wfar * .5);
		FBR = FARCENTER - view.view_mat[1].xyz * (Hfar * .5) + view.view_mat[0].xyz * (Wfar * .5);

		NTL = NEARCENTER + view.view_mat[1].xyz * (Wnear * .5) - view.view_mat[0].xyz * (Wnear * .5);
		NTR = NEARCENTER + view.view_mat[1].xyz * (Wnear * .5) + view.view_mat[0].xyz * (Wnear * .5);
		NBL = NEARCENTER - view.view_mat[1].xyz * (Wnear * .5) - view.view_mat[0].xyz * (Wnear * .5);
		NBR = NEARCENTER - view.view_mat[1].xyz * (Wnear * .5) + view.view_mat[0].xyz * (Wnear * .5);

		float4 white = { 1,1,1,1 };
		end::debug_renderer::add_line(FTL, FTR, white);	 end::debug_renderer::add_line(NTR, NTL, white); end::debug_renderer::add_line(NTR, FTR, white);
		end::debug_renderer::add_line(FBL, FBR, white);	 end::debug_renderer::add_line(NBR, NBL, white); end::debug_renderer::add_line(NTL, FTL, white);
		end::debug_renderer::add_line(FTL, FBL, white);	 end::debug_renderer::add_line(NTL, NBL, white); end::debug_renderer::add_line(NBR, FBR, white);
		end::debug_renderer::add_line(FTR, FBR, white);	 end::debug_renderer::add_line(NTR, NBR, white); end::debug_renderer::add_line(NBL, FBL, white);
		
		frustum[0] = calculate_plane(FTL, FBL, NBL); /*left*/
		frustum[1] = calculate_plane(FTL, NTL , FTR); /*top*/
		frustum[2] = calculate_plane(FBL, FBR, NBL); /*btm*/
		frustum[3] = calculate_plane(NBR, FBR, FTR); /*rite*/
		frustum[4] = calculate_plane(NBL, NBR, NTL); /*near*/
		frustum[5] = calculate_plane(FTL, FBR, FBL); /*far*/

		end::debug_renderer::add_line(FARCENTER, FARCENTER + frustum[5].normal, {1,0,1,1});
		end::debug_renderer::add_line(NEARCENTER, NEARCENTER + frustum[4].normal, { 1,0,1,1 });
		float3 RC, LC, TC, BC;
		RC = (NBR + FBR + FTR + NTR) / 4;
		LC = (NBL + FBL + FTL + NTL) / 4;
		TC = (NTL + FTR + FTL + NTR) / 4;
		BC = (NBL + FBR + FBL + NBR) / 4;

		end::debug_renderer::add_line(RC, RC + frustum[3].normal, { 1,0,1,1 });
		end::debug_renderer::add_line(LC, LC + frustum[0].normal, { 1,0,1,1 });

		end::debug_renderer::add_line(TC, TC + frustum[1].normal, { 1,0,1,1 });
		end::debug_renderer::add_line(BC, BC + frustum[2].normal, { 1,0,1,1 });

	};

	// Calculates which side of a plane the sphere is on.
	//
	// Returns -1 if the sphere is completely behind the plane.
	// Returns 1 if the sphere is completely in front of the plane.
	// Otherwise returns 0 (Sphere overlaps the plane)
	inline int classify_sphere_to_plane(const sphere_t& sphere, const plane_t& plane) {
		float3 sph;
		if (sph.dot(sphere.center, plane.normal) - plane.offset > sphere.radius)
		{
			return 1;
		}
		else if(sph.dot(sphere.center, plane.normal) - plane.offset < -sphere.radius)
		{
			return -1;
		}
		return 0;
	}

	// Calculates which side of a plane the aabb is on.
	//
	// Returns -1 if the aabb is completely behind the plane.
	// Returns 1 if the aabb is completely in front of the plane.
	// Otherwise returns 0 (aabb overlaps the plane)
	// MUST BE IMPLEMENTED UsING THE PROJECTED RADIUS TEST
	inline int classify_aabb_to_plane(const aabb_t& aabb, const plane_t& plane) {
		float3 ai;
		
		float r = ai.dot(aabb.extents, (float3&)DirectX::XMVectorAbs((DirectX::XMVECTOR&)plane.normal));
		if (ai.dot(aabb.center, plane.normal) - plane.offset > r)
		{
			return 1;
		}else if (ai.dot(aabb.center, plane.normal) - plane.offset < -r)
		{
			return -1;
		}
		
		return 0;
	};

	// Determines if the aabb is inside the frustum.
	//
	// Returns false if the aabb is completely behind any plane.
	// Otherwise returns true.
	inline bool aabb_to_frustum(const aabb_t& aabb, const frustum_t& frustum) {
		for (int i = 0; i < 6; i++)
		{
			if (classify_aabb_to_plane(aabb, frustum[i]) == -1)
			{
				return false;
			}
			std::cout << i<< ": " << classify_aabb_to_plane(aabb, frustum[i]) << std::endl;
		}
		return true;
	}
}