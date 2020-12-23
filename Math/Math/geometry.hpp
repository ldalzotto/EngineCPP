#pragma once

#include "math.hpp"
#include "geometry_def.hpp"
#include "transform_def.hpp"
#include <float.h>
#include "Common/Memory/memory_slice.hpp"

using namespace Math;

struct Geometry
{
	template<class TYPE>
	inline static bool overlap(const AABB<TYPE>& p_left, const AABB<TYPE>& p_right)
	{
		if (fabsf(p_left.center.x - p_right.center.x) > (p_left.radiuses.x + p_right.radiuses.x)) return false;
		if (fabsf(p_left.center.y - p_right.center.y) > (p_left.radiuses.y + p_right.radiuses.y)) return false;
		if (fabsf(p_left.center.z - p_right.center.z) > (p_left.radiuses.z + p_right.radiuses.z)) return false;

		return true;
	};

	template<class TYPE>
	inline static AABB<TYPE> project(const AABB<TYPE>& p_left, const Transform& p_transform)
	{
		return AABB<float>(
			Math::add(p_left.center, Math::add(p_transform.position, Math::mul(p_left.center, p_transform.scale))),
			Math::mul(p_left.radiuses, p_transform.scale)
			);
	};

	template<class TYPE>
	inline static OBB<TYPE> to_obb(const AABB<TYPE>& p_aabb, const Transform& p_transform, const Matrix<3, TYPE>& p_rotation_axis)
	{
		return OBB<TYPE>(add(p_transform.position, p_aabb.center), p_rotation_axis, p_aabb.radiuses);
	};

	template<class TYPE>
	inline static void extract_vertices(const OBB<TYPE>& p_obb, Vector<3, TYPE>* out_vertices, short int* in_out_index)
	{
		Vector<3, TYPE> l_rad_delta[3];
		l_rad_delta[0] = mul(p_obb.rotation.Points2D[0], p_obb.radiuses.Points[0]);
		l_rad_delta[1] = mul(p_obb.rotation.Points2D[1], p_obb.radiuses.Points[1]);
		l_rad_delta[2] = mul(p_obb.rotation.Points2D[2], p_obb.radiuses.Points[2]);

		out_vertices[(*in_out_index)] = add(p_obb.center, add(add(l_rad_delta[0], l_rad_delta[1]), l_rad_delta[2]));
		out_vertices[(*in_out_index) + 1] = add(p_obb.center, add(min(l_rad_delta[0], l_rad_delta[1]), l_rad_delta[2]));
		out_vertices[(*in_out_index) + 2] = add(p_obb.center, min(add(l_rad_delta[0], l_rad_delta[1]), l_rad_delta[2]));
		out_vertices[(*in_out_index) + 3] = add(p_obb.center, min(min(l_rad_delta[0], l_rad_delta[1]), l_rad_delta[2]));


		out_vertices[(*in_out_index) + 4] = add(p_obb.center, add(add(mul(l_rad_delta[0], -1.0f), l_rad_delta[1]), l_rad_delta[2]));
		out_vertices[(*in_out_index) + 5] = add(p_obb.center, add(min(mul(l_rad_delta[0], -1.0f), l_rad_delta[1]), l_rad_delta[2]));
		out_vertices[(*in_out_index) + 6] = add(p_obb.center, min(add(mul(l_rad_delta[0], -1.0f), l_rad_delta[1]), l_rad_delta[2]));
		out_vertices[(*in_out_index) + 7] = add(p_obb.center, min(min(mul(l_rad_delta[0], -1.0f), l_rad_delta[1]), l_rad_delta[2]));

		(*in_out_index) += 8;
	};


	// This can be used as a generic shape to shape separate axis
	/*

	template<class TYPE>
	inline static void collapse_points_to_axis(const Vector<3, TYPE>& p_axis, com::MemorySlice<Vector<3, TYPE>> p_points, float* p_min, float* p_max)
	{
		*p_min = FLT_MAX;
		*p_max = -FLT_MAX;

		for (short int i = 0; i < p_points.count(); i++)
		{
			TYPE l_proj = dot(p_points[i], p_axis);
			if (l_proj < *p_min) { *p_min = l_proj; }
			if (l_proj > *p_max) { *p_max = l_proj; }
		}
	};

	template<class TYPE>
	inline static bool overlap(const OBB<TYPE>& p_left, const OBB<TYPE>& p_right)
	{
		Vector<3, TYPE> p_left_points_arr[8];
		Vector<3, TYPE> p_right_points_arr[8];
		com::MemorySlice<Vector<3, TYPE>> p_left_points = com::MemorySlice<Vector<3, TYPE>>(p_left_points_arr, 8);
		com::MemorySlice<Vector<3, TYPE>> p_right_points = com::MemorySlice<Vector<3, TYPE>>(p_right_points_arr, 8);

		short int l_index = 0;
		extract_vertices(p_left, p_left_points.Memory, &l_index); l_index = 0;
		extract_vertices(p_right, p_right_points.Memory, &l_index);

		Vector<3, TYPE> p_axes_arr[15];
		com::MemorySlice<Vector<3, TYPE>> p_axes = com::MemorySlice<Vector<3, TYPE>>(p_axes_arr, 15);
		p_axes[0] = p_left.rotation.Points2D[0];
		p_axes[1] = p_left.rotation.Points2D[1];
		p_axes[2] = p_left.rotation.Points2D[2];

		p_axes[3] = p_right.rotation.Points2D[0];
		p_axes[4] = p_right.rotation.Points2D[1];
		p_axes[5] = p_right.rotation.Points2D[2];

		p_axes[6] = cross(p_left.rotation.Points2D[0], p_right.rotation.Points2D[0]);
		p_axes[7] = cross(p_left.rotation.Points2D[0], p_right.rotation.Points2D[1]);
		p_axes[8] = cross(p_left.rotation.Points2D[0], p_right.rotation.Points2D[2]);
		p_axes[9] = cross(p_left.rotation.Points2D[1], p_right.rotation.Points2D[0]);
		p_axes[10] = cross(p_left.rotation.Points2D[1], p_right.rotation.Points2D[1]);
		p_axes[11] = cross(p_left.rotation.Points2D[1], p_right.rotation.Points2D[2]);
		p_axes[12] = cross(p_left.rotation.Points2D[2], p_right.rotation.Points2D[0]);
		p_axes[13] = cross(p_left.rotation.Points2D[2], p_right.rotation.Points2D[1]);
		p_axes[14] = cross(p_left.rotation.Points2D[2], p_right.rotation.Points2D[2]);

		float l_left_min, l_left_max, l_right_min, l_right_max;
		for (short int i = 0; i < p_axes.count(); i++)
		{
			collapse_points_to_axis(p_axes[i], p_left_points, &l_left_min, &l_left_max);
			collapse_points_to_axis(p_axes[i], p_right_points, &l_right_min, &l_right_max);

			if (!((l_left_min >= l_right_min && l_left_min <= l_right_max) || (l_right_min >= l_left_min && l_right_min <= l_left_max)))
			{
				return false;
			}
		}

		return true;
	}
	*/

	template<class TYPE>
	inline static bool overlap2(const OBB<TYPE>& p_left, const OBB<TYPE>& p_right)
	{
		static_assert(false, "DEPRECATED");

		Vector<3, TYPE> p_axes_arr[15];
		com::MemorySlice<Vector<3, TYPE>> p_axes = com::MemorySlice<Vector<3, TYPE>>(p_axes_arr, 15);
		p_axes[0] = p_left.rotation.Points2D[0];
		p_axes[1] = p_left.rotation.Points2D[1];
		p_axes[2] = p_left.rotation.Points2D[2];

		p_axes[3] = p_right.rotation.Points2D[0];
		p_axes[4] = p_right.rotation.Points2D[1];
		p_axes[5] = p_right.rotation.Points2D[2];

		p_axes[6] = cross(p_left.rotation.Points2D[0], p_right.rotation.Points2D[0]);
		p_axes[7] = cross(p_left.rotation.Points2D[0], p_right.rotation.Points2D[1]);
		p_axes[8] = cross(p_left.rotation.Points2D[0], p_right.rotation.Points2D[2]);
		p_axes[9] = cross(p_left.rotation.Points2D[1], p_right.rotation.Points2D[0]);
		p_axes[10] = cross(p_left.rotation.Points2D[1], p_right.rotation.Points2D[1]);
		p_axes[11] = cross(p_left.rotation.Points2D[1], p_right.rotation.Points2D[2]);
		p_axes[12] = cross(p_left.rotation.Points2D[2], p_right.rotation.Points2D[0]);
		p_axes[13] = cross(p_left.rotation.Points2D[2], p_right.rotation.Points2D[1]);
		p_axes[14] = cross(p_left.rotation.Points2D[2], p_right.rotation.Points2D[2]);

		for (short int i = 0; i < p_axes.count(); i++)
		{
			float l_left_radii_projected = 0;
			float l_right_radii_projected = 0;
			for (short int j = 0; j < 3; j++)
			{
				l_left_radii_projected += fabsf(dot(mul(p_left.rotation.Points2D[j], p_left.radiuses.Points[j]), p_axes[i]));
				l_right_radii_projected += fabsf(dot(mul(p_right.rotation.Points2D[j], p_right.radiuses.Points[j]), p_axes[i]));
			}

			float l_left_center_projected = dot(p_left.center, p_axes[i]);
			float l_right_center_projected = dot(p_right.center, p_axes[i]);

			float l_tl = fabsf(l_right_center_projected - l_left_center_projected);

			if ((l_tl - (l_left_radii_projected + l_right_radii_projected) >= Tolerance<float>::tol))
			{
				return false;
			}
		}

		return true;
	}

	template<class TYPE>
	inline static bool overlap3(const OBB<TYPE>& p_left, const OBB<TYPE>& p_right)
	{
		float l_left_radii_projected, l_right_radii_projected;
		Matrix<3, TYPE> l_radii, l_radii_abs;
		Vector<3, TYPE> l_t;

		for (short int i = 0; i < 3; i++)
		{
			for (short int j = 0; j < 3; j++)
			{
				l_radii.Points2D[i].Points[j] = dot(p_left.rotation.Points2D[i], p_right.rotation.Points2D[j]);
				l_radii_abs.Points2D[i].Points[j] = fabsf(l_radii.Points2D[i].Points[j]) + Tolerance<TYPE>::tol;
			}
		}

		l_t = min(p_right.center, p_left.center);
		l_t = Vector<3, TYPE>(dot(l_t, p_left.rotation.Points2D[0]), dot(l_t, p_left.rotation.Points2D[1]), dot(l_t, p_left.rotation.Points2D[2]));

		for (short int i = 0; i < 3; i++)
		{
			l_left_radii_projected = p_left.radiuses.Points[i];
			l_right_radii_projected = (p_right.radiuses.Points[0] * l_radii_abs.Points2D[i].Points[0]) + (p_right.radiuses.Points[1] * l_radii_abs.Points2D[i].Points[1]) + (p_right.radiuses.Points[2] * l_radii_abs.Points2D[i].Points[2]);
			if (fabsf(l_t.Points[i]) > (l_left_radii_projected + l_right_radii_projected)) return false;
		}

		for (short int i = 0; i < 3; i++)
		{
			l_left_radii_projected = (p_left.radiuses.Points[0] * l_radii_abs.Points2D[0].Points[i]) + (p_left.radiuses.Points[1] * l_radii_abs.Points2D[1].Points[i]) + (p_left.radiuses.Points[2] * l_radii_abs.Points2D[2].Points[i]);
			l_right_radii_projected = p_right.radiuses.Points[i];
			if (fabsf((l_t.Points[0] * l_radii.Points2D[0].Points[i]) + (l_t.Points[1] * l_radii.Points2D[1].Points[i]) + (l_t.Points[2] * l_radii.Points2D[2].Points[i])) > (l_left_radii_projected + l_right_radii_projected)) return false;
		}


		l_left_radii_projected = (p_left.radiuses.Points[1] * l_radii_abs.Points2D[2].Points[0]) + (p_left.radiuses.Points[2] * l_radii_abs.Points2D[1].Points[0]);
		l_right_radii_projected = (p_right.radiuses.Points[1] * l_radii_abs.Points2D[0].Points[2]) + (p_right.radiuses.Points[2] * l_radii_abs.Points2D[0].Points[1]);
		if (fabsf((l_t.Points[2] * l_radii.Points2D[1].Points[0]) - (l_t.Points[1] * l_radii.Points2D[2].Points[0])) > (l_left_radii_projected + l_right_radii_projected)) return false;

		l_left_radii_projected = (p_left.radiuses.Points[1] * l_radii_abs.Points2D[2].Points[1]) + (p_left.radiuses.Points[2] * l_radii_abs.Points2D[1].Points[1]);
		l_right_radii_projected = (p_right.radiuses.Points[0] * l_radii_abs.Points2D[0].Points[2]) + (p_right.radiuses.Points[2] * l_radii_abs.Points2D[0].Points[0]);
		if (fabsf((l_t.Points[2] * l_radii.Points2D[1].Points[1]) - (l_t.Points[1] * l_radii.Points2D[2].Points[1])) > (l_left_radii_projected + l_right_radii_projected)) return false;

		l_left_radii_projected = (p_left.radiuses.Points[1] * l_radii_abs.Points2D[2].Points[2]) + (p_left.radiuses.Points[2] * l_radii_abs.Points2D[1].Points[2]);
		l_right_radii_projected = (p_right.radiuses.Points[0] * l_radii_abs.Points2D[0].Points[1]) + (p_right.radiuses.Points[1] * l_radii_abs.Points2D[0].Points[0]);
		if (fabsf((l_t.Points[2] * l_radii.Points2D[1].Points[2]) - (l_t.Points[1] * l_radii.Points2D[2].Points[2])) > (l_left_radii_projected + l_right_radii_projected)) return false;

		l_left_radii_projected = (p_left.radiuses.Points[0] * l_radii_abs.Points2D[2].Points[0]) + (p_left.radiuses.Points[2] * l_radii_abs.Points2D[0].Points[0]);
		l_right_radii_projected = (p_right.radiuses.Points[1] * l_radii_abs.Points2D[1].Points[2]) + (p_right.radiuses.Points[2] * l_radii_abs.Points2D[1].Points[1]);
		if (fabsf((l_t.Points[0] * l_radii.Points2D[2].Points[0]) - (l_t.Points[2] * l_radii.Points2D[0].Points[0])) > (l_left_radii_projected + l_right_radii_projected)) return false;
		
		l_left_radii_projected = (p_left.radiuses.Points[0] * l_radii_abs.Points2D[2].Points[1]) + (p_left.radiuses.Points[2] * l_radii_abs.Points2D[0].Points[1]);
		l_right_radii_projected = (p_right.radiuses.Points[0] * l_radii_abs.Points2D[1].Points[2]) + (p_right.radiuses.Points[2] * l_radii_abs.Points2D[1].Points[0]);
		if (fabsf((l_t.Points[0] * l_radii.Points2D[2].Points[1]) - (l_t.Points[2] * l_radii.Points2D[0].Points[1])) > (l_left_radii_projected + l_right_radii_projected)) return false;

		l_left_radii_projected = (p_left.radiuses.Points[0] * l_radii_abs.Points2D[2].Points[2]) + (p_left.radiuses.Points[2] * l_radii_abs.Points2D[0].Points[2]);
		l_right_radii_projected = (p_right.radiuses.Points[0] * l_radii_abs.Points2D[1].Points[1]) + (p_right.radiuses.Points[1] * l_radii_abs.Points2D[1].Points[0]);
		if (fabsf((l_t.Points[0] * l_radii.Points2D[2].Points[2]) - (l_t.Points[2] * l_radii.Points2D[0].Points[2])) > (l_left_radii_projected + l_right_radii_projected)) return false;

		l_left_radii_projected = (p_left.radiuses.Points[0] * l_radii_abs.Points2D[1].Points[0]) + (p_left.radiuses.Points[1] * l_radii_abs.Points2D[0].Points[0]);
		l_right_radii_projected = (p_right.radiuses.Points[1] * l_radii_abs.Points2D[2].Points[2]) + (p_right.radiuses.Points[2] * l_radii_abs.Points2D[2].Points[1]);
		if (fabsf((l_t.Points[1] * l_radii.Points2D[0].Points[0]) - (l_t.Points[0] * l_radii.Points2D[1].Points[0])) > (l_left_radii_projected + l_right_radii_projected)) return false;
		
		l_left_radii_projected = (p_left.radiuses.Points[0] * l_radii_abs.Points2D[1].Points[1]) + (p_left.radiuses.Points[1] * l_radii_abs.Points2D[0].Points[1]);
		l_right_radii_projected = (p_right.radiuses.Points[0] * l_radii_abs.Points2D[2].Points[2]) + (p_right.radiuses.Points[2] * l_radii_abs.Points2D[2].Points[0]);
		if (fabsf((l_t.Points[1] * l_radii.Points2D[0].Points[1]) - (l_t.Points[0] * l_radii.Points2D[1].Points[1])) > (l_left_radii_projected + l_right_radii_projected)) return false;

		l_left_radii_projected = (p_left.radiuses.Points[0] * l_radii_abs.Points2D[1].Points[2]) + (p_left.radiuses.Points[1] * l_radii_abs.Points2D[0].Points[2]);
		l_right_radii_projected = (p_right.radiuses.Points[0] * l_radii_abs.Points2D[2].Points[1]) + (p_right.radiuses.Points[1] * l_radii_abs.Points2D[2].Points[0]);
		if (fabsf((l_t.Points[1] * l_radii.Points2D[0].Points[2]) - (l_t.Points[0] * l_radii.Points2D[1].Points[2])) > (l_left_radii_projected + l_right_radii_projected)) return false;


		return true;
	}
};
