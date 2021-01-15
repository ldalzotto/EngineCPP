#pragma once

#include "math.hpp"
#include "geometry_def.hpp"
#include "transform_def.hpp"
#include <float.h>
#include "Common/Memory/memory_slice.hpp"

struct SATKernel
{
	//TODO -> We can have a more performant version by transforming right in left reference
	template<class TYPE>
	inline static bool separation_test_symetrical_LRoriented_shape(const Math::Vector<3, TYPE>& p_left_center, const Math::Vector<3, TYPE>& p_left_radii, const Math::Matrix<3, TYPE>& p_left_rotation,
			const Math::Vector<3, TYPE>& p_right_center, const Math::Vector<3, TYPE>& p_right_radii, const Math::Matrix<3, TYPE>& p_right_rotation, const Math::Vector<3, TYPE>& p_tested_axis)
	{
		float l_left_radii_projected = 0;
		float l_right_radii_projected = 0;
		for (short int j = 0; j < 3; j++)
		{
			l_left_radii_projected += fabsf(Math::dot(Math::mul(p_left_rotation.Points2D[j], p_left_radii.Points[j]), p_tested_axis));
			l_right_radii_projected += fabsf(Math::dot(Math::mul(p_right_rotation.Points2D[j], p_right_radii.Points[j]), p_tested_axis));
		}

		float l_left_center_projected = Math::dot(p_left_center, p_tested_axis);
		float l_right_center_projected = Math::dot(p_right_center, p_tested_axis);

		float l_tl = fabsf(l_right_center_projected - l_left_center_projected);

		return (l_tl - (l_left_radii_projected + l_right_radii_projected) >= Tolerance<float>::tol);
	};

	template<class TYPE>
	inline static bool separation_test_LRoriented_shape(const com::MemorySlice<Math::Vector<3, TYPE>>& p_left_vertices, const Math::Matrix<3, TYPE>& p_left_rotation,
		const com::MemorySlice<Math::Vector<3, TYPE>>& p_right_vertices, const Math::Matrix<3, TYPE>& p_right_rotation, const Math::Vector<3, TYPE>& p_tested_axis)
	{
		float l_left_min, l_left_max, l_right_min, l_right_max;
		collapse_points_to_axis(p_tested_axis, p_left_vertices, &l_left_min, &l_left_max);
		collapse_points_to_axis(p_tested_axis, p_right_vertices, &l_right_min, &l_right_max);

		return (l_left_min < l_right_min || l_left_min > l_right_max) && (l_right_min < l_left_min || l_right_min > l_left_max);
	};

	template<class TYPE>
	inline static void collapse_points_to_axis(const Math::Vector<3, TYPE>& p_axis, com::MemorySlice<Math::Vector<3, TYPE>> p_points, float* p_min, float* p_max)
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
};

struct Geometry
{
	template<class TYPE>
	inline static bool overlap(const Math::AABB<TYPE>& p_left, const Math::AABB<TYPE>& p_right)
	{
		if (fabsf(p_left.center.x - p_right.center.x) > (p_left.radiuses.x + p_right.radiuses.x)) return false;
		if (fabsf(p_left.center.y - p_right.center.y) > (p_left.radiuses.y + p_right.radiuses.y)) return false;
		if (fabsf(p_left.center.z - p_right.center.z) > (p_left.radiuses.z + p_right.radiuses.z)) return false;

		return true;
	};

	template<class TYPE>
	inline static Math::AABB<TYPE> project(const Math::AABB<TYPE>& p_left, const Math::Transform& p_transform)
	{
		return AABB<float>(
			Math::add(p_left.center, Math::add(p_transform.position, Math::mul(p_left.center, p_transform.scale))),
			Math::mul(p_left.radiuses, p_transform.scale)
		);
	};

	template<class TYPE>
	inline static Math::OBB<TYPE> to_obb(const Math::AABB<TYPE>& p_aabb, const Math::Transform& p_transform, const Math::Matrix<3, TYPE>& p_rotation_axis)
	{
		return Math::OBB<TYPE>(add(p_transform.position, p_aabb.center), p_rotation_axis, p_aabb.radiuses);
	};

	template<class TYPE>
	inline static void extract_vertices(const Math::OBB<TYPE>& p_obb, Math::Vector<3, TYPE>* out_vertices, short int* in_out_index)
	{
		Math::Vector<3, TYPE> l_rad_delta[3];
		l_rad_delta[0] = Math::mul(p_obb.rotation.Points2D[0], p_obb.radiuses.Points[0]);
		l_rad_delta[1] = Math::mul(p_obb.rotation.Points2D[1], p_obb.radiuses.Points[1]);
		l_rad_delta[2] = Math::mul(p_obb.rotation.Points2D[2], p_obb.radiuses.Points[2]);

		out_vertices[(*in_out_index)] = Math::add(p_obb.center, Math::add(Math::add(l_rad_delta[0], l_rad_delta[1]), l_rad_delta[2]));
		out_vertices[(*in_out_index) + 1] = Math::add(p_obb.center, Math::add(Math::min(l_rad_delta[0], l_rad_delta[1]), l_rad_delta[2]));
		out_vertices[(*in_out_index) + 2] = Math::add(p_obb.center, Math::min(Math::add(l_rad_delta[0], l_rad_delta[1]), l_rad_delta[2]));
		out_vertices[(*in_out_index) + 3] = Math::add(p_obb.center, Math::min(Math::min(l_rad_delta[0], l_rad_delta[1]), l_rad_delta[2]));


		out_vertices[(*in_out_index) + 4] = Math::add(p_obb.center, Math::add(Math::add(Math::mul(l_rad_delta[0], -1.0f), l_rad_delta[1]), l_rad_delta[2]));
		out_vertices[(*in_out_index) + 5] = Math::add(p_obb.center, Math::add(Math::min(Math::mul(l_rad_delta[0], -1.0f), l_rad_delta[1]), l_rad_delta[2]));
		out_vertices[(*in_out_index) + 6] = Math::add(p_obb.center, Math::min(Math::add(Math::mul(l_rad_delta[0], -1.0f), l_rad_delta[1]), l_rad_delta[2]));
		out_vertices[(*in_out_index) + 7] = Math::add(p_obb.center, Math::min(Math::min(Math::mul(l_rad_delta[0], -1.0f), l_rad_delta[1]), l_rad_delta[2]));

		(*in_out_index) += 8;
	};


	// This can be used as a generic shape to shape separate axis


	template<class TYPE>
	inline static bool overlap(const Math::OBB<TYPE>& p_left, const Math::OBB<TYPE>& p_right)
	{
		Math::Vector<3, TYPE> p_left_points_arr[8];
		Math::Vector<3, TYPE> p_right_points_arr[8];
		com::MemorySlice<Math::Vector<3, TYPE>> p_left_points = com::MemorySlice<Math::Vector<3, TYPE>>(p_left_points_arr, 8);
		com::MemorySlice<Math::Vector<3, TYPE>> p_right_points = com::MemorySlice<Math::Vector<3, TYPE>>(p_right_points_arr, 8);

		short int l_index = 0;
		extract_vertices(p_left, p_left_points.Memory, &l_index); l_index = 0;
		extract_vertices(p_right, p_right_points.Memory, &l_index);

		if (   SATKernel::separation_test_LRoriented_shape(p_left_points, p_left.rotation, p_right_points, p_right.rotation, p_left.rotation.Points2D[0])
			|| SATKernel::separation_test_LRoriented_shape(p_left_points, p_left.rotation, p_right_points, p_right.rotation, p_left.rotation.Points2D[1])
			|| SATKernel::separation_test_LRoriented_shape(p_left_points, p_left.rotation, p_right_points, p_right.rotation, p_left.rotation.Points2D[2])
			|| SATKernel::separation_test_LRoriented_shape(p_left_points, p_left.rotation, p_right_points, p_right.rotation, p_right.rotation.Points2D[0])
			|| SATKernel::separation_test_LRoriented_shape(p_left_points, p_left.rotation, p_right_points, p_right.rotation, p_right.rotation.Points2D[1])
			|| SATKernel::separation_test_LRoriented_shape(p_left_points, p_left.rotation, p_right_points, p_right.rotation, p_right.rotation.Points2D[2])
			|| SATKernel::separation_test_LRoriented_shape(p_left_points, p_left.rotation, p_right_points, p_right.rotation, Math::cross(p_left.rotation.Points2D[0], p_right.rotation.Points2D[0]))
			|| SATKernel::separation_test_LRoriented_shape(p_left_points, p_left.rotation, p_right_points, p_right.rotation, Math::cross(p_left.rotation.Points2D[0], p_right.rotation.Points2D[1]))
			|| SATKernel::separation_test_LRoriented_shape(p_left_points, p_left.rotation, p_right_points, p_right.rotation, Math::cross(p_left.rotation.Points2D[0], p_right.rotation.Points2D[2]))
			|| SATKernel::separation_test_LRoriented_shape(p_left_points, p_left.rotation, p_right_points, p_right.rotation, Math::cross(p_left.rotation.Points2D[1], p_right.rotation.Points2D[0]))
			|| SATKernel::separation_test_LRoriented_shape(p_left_points, p_left.rotation, p_right_points, p_right.rotation, Math::cross(p_left.rotation.Points2D[1], p_right.rotation.Points2D[1]))
			|| SATKernel::separation_test_LRoriented_shape(p_left_points, p_left.rotation, p_right_points, p_right.rotation, Math::cross(p_left.rotation.Points2D[1], p_right.rotation.Points2D[2]))
			|| SATKernel::separation_test_LRoriented_shape(p_left_points, p_left.rotation, p_right_points, p_right.rotation, Math::cross(p_left.rotation.Points2D[2], p_right.rotation.Points2D[0]))
			|| SATKernel::separation_test_LRoriented_shape(p_left_points, p_left.rotation, p_right_points, p_right.rotation, Math::cross(p_left.rotation.Points2D[2], p_right.rotation.Points2D[1]))
			|| SATKernel::separation_test_LRoriented_shape(p_left_points, p_left.rotation, p_right_points, p_right.rotation, Math::cross(p_left.rotation.Points2D[2], p_right.rotation.Points2D[2]))
			)
		{
			return false;
		}

		return true;
	}

	template<class TYPE>
	inline static bool overlap2(const Math::OBB<TYPE>& p_left, const Math::OBB<TYPE>& p_right)
	{
		// static_assert(false, "DEPRECATED");

		if    ((SATKernel::separation_test_symetrical_LRoriented_shape(p_left.center, p_left.radiuses, p_left.rotation, p_right.center, p_right.radiuses, p_right.rotation, p_left.rotation.Points2D[0]))
			|| (SATKernel::separation_test_symetrical_LRoriented_shape(p_left.center, p_left.radiuses, p_left.rotation, p_right.center, p_right.radiuses, p_right.rotation, p_left.rotation.Points2D[1]))
			|| (SATKernel::separation_test_symetrical_LRoriented_shape(p_left.center, p_left.radiuses, p_left.rotation, p_right.center, p_right.radiuses, p_right.rotation, p_left.rotation.Points2D[2]))
			|| (SATKernel::separation_test_symetrical_LRoriented_shape(p_left.center, p_left.radiuses, p_left.rotation, p_right.center, p_right.radiuses, p_right.rotation, p_right.rotation.Points2D[0]))
			|| (SATKernel::separation_test_symetrical_LRoriented_shape(p_left.center, p_left.radiuses, p_left.rotation, p_right.center, p_right.radiuses, p_right.rotation, p_right.rotation.Points2D[1]))
			|| (SATKernel::separation_test_symetrical_LRoriented_shape(p_left.center, p_left.radiuses, p_left.rotation, p_right.center, p_right.radiuses, p_right.rotation, p_right.rotation.Points2D[2]))
			|| (SATKernel::separation_test_symetrical_LRoriented_shape(p_left.center, p_left.radiuses, p_left.rotation, p_right.center, p_right.radiuses, p_right.rotation, Math::cross(p_left.rotation.Points2D[0], p_right.rotation.Points2D[0])))
			|| (SATKernel::separation_test_symetrical_LRoriented_shape(p_left.center, p_left.radiuses, p_left.rotation, p_right.center, p_right.radiuses, p_right.rotation, Math::cross(p_left.rotation.Points2D[0], p_right.rotation.Points2D[1])))
			|| (SATKernel::separation_test_symetrical_LRoriented_shape(p_left.center, p_left.radiuses, p_left.rotation, p_right.center, p_right.radiuses, p_right.rotation, Math::cross(p_left.rotation.Points2D[0], p_right.rotation.Points2D[2])))
			|| (SATKernel::separation_test_symetrical_LRoriented_shape(p_left.center, p_left.radiuses, p_left.rotation, p_right.center, p_right.radiuses, p_right.rotation, Math::cross(p_left.rotation.Points2D[1], p_right.rotation.Points2D[0])))
			|| (SATKernel::separation_test_symetrical_LRoriented_shape(p_left.center, p_left.radiuses, p_left.rotation, p_right.center, p_right.radiuses, p_right.rotation, Math::cross(p_left.rotation.Points2D[1], p_right.rotation.Points2D[1])))
			|| (SATKernel::separation_test_symetrical_LRoriented_shape(p_left.center, p_left.radiuses, p_left.rotation, p_right.center, p_right.radiuses, p_right.rotation, Math::cross(p_left.rotation.Points2D[1], p_right.rotation.Points2D[2])))
			|| (SATKernel::separation_test_symetrical_LRoriented_shape(p_left.center, p_left.radiuses, p_left.rotation, p_right.center, p_right.radiuses, p_right.rotation, Math::cross(p_left.rotation.Points2D[2], p_right.rotation.Points2D[0])))
			|| (SATKernel::separation_test_symetrical_LRoriented_shape(p_left.center, p_left.radiuses, p_left.rotation, p_right.center, p_right.radiuses, p_right.rotation, Math::cross(p_left.rotation.Points2D[2], p_right.rotation.Points2D[1])))
			|| (SATKernel::separation_test_symetrical_LRoriented_shape(p_left.center, p_left.radiuses, p_left.rotation, p_right.center, p_right.radiuses, p_right.rotation, Math::cross(p_left.rotation.Points2D[2], p_right.rotation.Points2D[2]))))
		{
			return false;
		}

		return true;
	}

	template<class TYPE>
	inline static bool overlap3(const Math::OBB<TYPE>& p_left, const Math::OBB<TYPE>& p_right)
	{
		float l_left_radii_projected, l_right_radii_projected;
		Math::Matrix<3, TYPE> l_radii, l_radii_abs;
		Math::Vector<3, TYPE> l_t;

		for (short int i = 0; i < 3; i++)
		{
			for (short int j = 0; j < 3; j++)
			{
				l_radii.Points2D[i].Points[j] = Math::dot(p_left.rotation.Points2D[i], p_right.rotation.Points2D[j]);
				l_radii_abs.Points2D[i].Points[j] = fabsf(l_radii.Points2D[i].Points[j]) + Tolerance<TYPE>::tol;
			}
		}

		l_t = Math::min(p_right.center, p_left.center);
		l_t = Math::Vector<3, TYPE>(Math::dot(l_t, p_left.rotation.Points2D[0]), Math::dot(l_t, p_left.rotation.Points2D[1]), Math::dot(l_t, p_left.rotation.Points2D[2]));

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
