#pragma once

struct obb;
struct aabb;


struct aabb
{
	v3f center;
	v3f radiuses;

	inline char overlap(const aabb& p_other) const;
	inline aabb add_position(const v3f& p_position) const;
	inline obb add_position_rotation(const transform_pa& p_position_rotation) const;
};

struct obb
{
	aabb box;
	m33f axis;

	inline void extract_vertices(Slice<v3f>* in_out_vertices) const;

	inline char overlap(const obb& p_other) const;
	inline char overlap1(const obb& p_other) const;
	inline char overlap2(const obb& p_other) const;
};

struct SAT
{
	inline static char mainaxis_x(const v3f& p_left_center, const v3f& p_left_radii, const v3f& p_right_center, const v3f& p_right_radii);
	inline static char mainaxis_y(const v3f& p_left_center, const v3f& p_left_radii, const v3f& p_right_center, const v3f& p_right_radii);
	inline static char mainaxis_z(const v3f& p_left_center, const v3f& p_left_radii, const v3f& p_right_center, const v3f& p_right_radii);

	inline static char Loriented_Roriented(
		const Slice<v3f>& p_left_vertices, const m33f& p_left_rotation,
		const Slice<v3f>& p_right_vertices, const m33f& p_right_rotation,
		const v3f& p_tested_axis
	);

	//TODO -> We can create a more performant version by transforming right in left reference
	/* This SAT test is true only if left and right centers are at the exact center of the tested shapes.  */
	inline static char symetricalshapes_Loriented_Roriented(
		const v3f& p_left_center, const v3f& p_left_radii, const m33f& p_left_rotation,
		const v3f& p_right_center, const v3f& p_right_radii, const m33f& p_right_rotation, 
		const v3f& p_tested_axis
	);
};

struct GeometryUtil
{
	inline static void collapse_points_to_axis_min_max(const v3f& p_axis, const Slice<v3f>& p_points, float* out_min, float* out_max);
};


inline char aabb::overlap(const aabb& p_other) const
{
	if (
		  SAT::mainaxis_x(this->center, this->radiuses, p_other.center, p_other.radiuses)
		| SAT::mainaxis_y(this->center, this->radiuses, p_other.center, p_other.radiuses)
		| SAT::mainaxis_z(this->center, this->radiuses, p_other.center, p_other.radiuses)
	   )
	{
		return 0;
	};

	return 1;
};

inline aabb aabb::add_position(const v3f& p_position) const
{
	return aabb{this->center + p_position, this->radiuses};
};

inline obb aabb::add_position_rotation(const transform_pa& p_position_rotation) const
{
	return obb
	{
		this->add_position(p_position_rotation.position),
		p_position_rotation.axis
	};
};

inline void obb::extract_vertices(Slice<v3f>* in_out_vertices) const
{
	v3f l_rad_delta[3];
	l_rad_delta[0] = this->axis.Points2D[0] * this->box.radiuses.Points[0];
	l_rad_delta[1] = this->axis.Points2D[1] * this->box.radiuses.Points[1];
	l_rad_delta[2] = this->axis.Points2D[2] * this->box.radiuses.Points[2];

	in_out_vertices->get(0) = this->box.center + l_rad_delta[0] + l_rad_delta[1] + l_rad_delta[2];
	in_out_vertices->get(1) = this->box.center + l_rad_delta[0] - l_rad_delta[1] + l_rad_delta[2];
	in_out_vertices->get(2) = this->box.center + l_rad_delta[0] + l_rad_delta[1] - l_rad_delta[2];
	in_out_vertices->get(3) = this->box.center + l_rad_delta[0] - l_rad_delta[1] - l_rad_delta[2];

	in_out_vertices->get(4) = this->box.center - l_rad_delta[0] + l_rad_delta[1] + l_rad_delta[2];
	in_out_vertices->get(5) = this->box.center - l_rad_delta[0] - l_rad_delta[1] + l_rad_delta[2];
	in_out_vertices->get(6) = this->box.center - l_rad_delta[0] + l_rad_delta[1] - l_rad_delta[2];
	in_out_vertices->get(7) = this->box.center - l_rad_delta[0] - l_rad_delta[1] - l_rad_delta[2];
};


inline char obb::overlap(const obb& p_other) const
{
	v3f p_left_points_arr[8];
	v3f p_right_points_arr[8];
	Slice<v3f> p_left_points = Slice<v3f>::build_memory_elementnb(p_left_points_arr, 8);
	Slice<v3f> p_right_points = Slice<v3f>::build_memory_elementnb(p_right_points_arr, 8);

	this->extract_vertices(&p_left_points);
	p_other.extract_vertices(&p_right_points);

	if (
		  SAT::Loriented_Roriented(p_left_points, this->axis, p_right_points, p_other.axis, this->axis.Right)
		| SAT::Loriented_Roriented(p_left_points, this->axis, p_right_points, p_other.axis, this->axis.Up)
		| SAT::Loriented_Roriented(p_left_points, this->axis, p_right_points, p_other.axis, this->axis.Forward)
		| SAT::Loriented_Roriented(p_left_points, this->axis, p_right_points, p_other.axis, p_other.axis.Right)
		| SAT::Loriented_Roriented(p_left_points, this->axis, p_right_points, p_other.axis, p_other.axis.Up)
		| SAT::Loriented_Roriented(p_left_points, this->axis, p_right_points, p_other.axis, p_other.axis.Forward)

		| SAT::Loriented_Roriented(p_left_points, this->axis, p_right_points, p_other.axis, this->axis.Right.cross(p_other.axis.Right))
		| SAT::Loriented_Roriented(p_left_points, this->axis, p_right_points, p_other.axis, this->axis.Right.cross(p_other.axis.Up))
		| SAT::Loriented_Roriented(p_left_points, this->axis, p_right_points, p_other.axis, this->axis.Right.cross(p_other.axis.Forward))
		| SAT::Loriented_Roriented(p_left_points, this->axis, p_right_points, p_other.axis, this->axis.Up.cross(p_other.axis.Right))
		| SAT::Loriented_Roriented(p_left_points, this->axis, p_right_points, p_other.axis, this->axis.Up.cross(p_other.axis.Up))
		| SAT::Loriented_Roriented(p_left_points, this->axis, p_right_points, p_other.axis, this->axis.Up.cross(p_other.axis.Forward))
		| SAT::Loriented_Roriented(p_left_points, this->axis, p_right_points, p_other.axis, this->axis.Forward.cross(p_other.axis.Right))
		| SAT::Loriented_Roriented(p_left_points, this->axis, p_right_points, p_other.axis, this->axis.Forward.cross(p_other.axis.Up))
		| SAT::Loriented_Roriented(p_left_points, this->axis, p_right_points, p_other.axis, this->axis.Forward.cross(p_other.axis.Forward))
		)
	{
		return 0;
	}

	return 1;
};

inline char obb::overlap1(const obb& p_other) const
{
	if (
		  SAT::symetricalshapes_Loriented_Roriented(this->box.center, this->box.radiuses, this->axis, p_other.box.center, p_other.box.radiuses, p_other.axis, this->axis.Right)
		| SAT::symetricalshapes_Loriented_Roriented(this->box.center, this->box.radiuses, this->axis, p_other.box.center, p_other.box.radiuses, p_other.axis, this->axis.Up)
		| SAT::symetricalshapes_Loriented_Roriented(this->box.center, this->box.radiuses, this->axis, p_other.box.center, p_other.box.radiuses, p_other.axis, this->axis.Forward)
		| SAT::symetricalshapes_Loriented_Roriented(this->box.center, this->box.radiuses, this->axis, p_other.box.center, p_other.box.radiuses, p_other.axis, p_other.axis.Right)
		| SAT::symetricalshapes_Loriented_Roriented(this->box.center, this->box.radiuses, this->axis, p_other.box.center, p_other.box.radiuses, p_other.axis, p_other.axis.Up)
		| SAT::symetricalshapes_Loriented_Roriented(this->box.center, this->box.radiuses, this->axis, p_other.box.center, p_other.box.radiuses, p_other.axis, p_other.axis.Forward)

		| SAT::symetricalshapes_Loriented_Roriented(this->box.center, this->box.radiuses, this->axis, p_other.box.center, p_other.box.radiuses, p_other.axis, this->axis.Right.cross(p_other.axis.Right))
		| SAT::symetricalshapes_Loriented_Roriented(this->box.center, this->box.radiuses, this->axis, p_other.box.center, p_other.box.radiuses, p_other.axis, this->axis.Right.cross(p_other.axis.Up))
		| SAT::symetricalshapes_Loriented_Roriented(this->box.center, this->box.radiuses, this->axis, p_other.box.center, p_other.box.radiuses, p_other.axis, this->axis.Right.cross(p_other.axis.Forward))
		| SAT::symetricalshapes_Loriented_Roriented(this->box.center, this->box.radiuses, this->axis, p_other.box.center, p_other.box.radiuses, p_other.axis, this->axis.Up.cross(p_other.axis.Right))
		| SAT::symetricalshapes_Loriented_Roriented(this->box.center, this->box.radiuses, this->axis, p_other.box.center, p_other.box.radiuses, p_other.axis, this->axis.Up.cross(p_other.axis.Up))
		| SAT::symetricalshapes_Loriented_Roriented(this->box.center, this->box.radiuses, this->axis, p_other.box.center, p_other.box.radiuses, p_other.axis, this->axis.Up.cross(p_other.axis.Forward))
		| SAT::symetricalshapes_Loriented_Roriented(this->box.center, this->box.radiuses, this->axis, p_other.box.center, p_other.box.radiuses, p_other.axis, this->axis.Forward.cross(p_other.axis.Right))
		| SAT::symetricalshapes_Loriented_Roriented(this->box.center, this->box.radiuses, this->axis, p_other.box.center, p_other.box.radiuses, p_other.axis, this->axis.Forward.cross(p_other.axis.Up))
		| SAT::symetricalshapes_Loriented_Roriented(this->box.center, this->box.radiuses, this->axis, p_other.box.center, p_other.box.radiuses, p_other.axis, this->axis.Forward.cross(p_other.axis.Forward))
	   )
	{
		return 0;
	}

	return 1;
};

inline char obb::overlap2(const obb& p_other) const
{
	float l_left_radii_projected, l_right_radii_projected;
	m33f l_radii, l_radii_abs;
	v3f l_t;

	for (unsigned char i = 0; i < 3; i++)
	{
		for (unsigned char j = 0; j < 3; j++)
		{
			l_radii.Points2D[i].Points[j] = this->axis.Points2D[i].dot(p_other.axis.Points2D[j]);
			l_radii_abs.Points2D[i].Points[j] = fabsf(l_radii.Points2D[i].Points[j]) + v2::Math::tol_f;
		}
	}

	l_t = p_other.box.center - this->box.center;
	l_t = v3f{ l_t.dot(this->axis.Points2D[0]), l_t.dot(this->axis.Points2D[1]), l_t.dot(this->axis.Points2D[2]) };

	for (unsigned char i = 0; i < 3; i++)
	{
		l_left_radii_projected = this->box.radiuses.Points[i];
		l_right_radii_projected = (p_other.box.radiuses.Points[0] * l_radii_abs.Points2D[i].Points[0]) + (p_other.box.radiuses.Points[1] * l_radii_abs.Points2D[i].Points[1]) + (p_other.box.radiuses.Points[2] * l_radii_abs.Points2D[i].Points[2]);
		if (fabsf(l_t.Points[i]) > (l_left_radii_projected + l_right_radii_projected)) return false;
	}

	for (unsigned char i = 0; i < 3; i++)
	{
		l_left_radii_projected = (this->box.radiuses.Points[0] * l_radii_abs.Points2D[0].Points[i]) + (this->box.radiuses.Points[1] * l_radii_abs.Points2D[1].Points[i]) + (this->box.radiuses.Points[2] * l_radii_abs.Points2D[2].Points[i]);
		l_right_radii_projected = p_other.box.radiuses.Points[i];
		if (fabsf((l_t.Points[0] * l_radii.Points2D[0].Points[i]) + (l_t.Points[1] * l_radii.Points2D[1].Points[i]) + (l_t.Points[2] * l_radii.Points2D[2].Points[i])) > (l_left_radii_projected + l_right_radii_projected)) return false;
	}


	l_left_radii_projected = (this->box.radiuses.Points[1] * l_radii_abs.Points2D[2].Points[0]) + (this->box.radiuses.Points[2] * l_radii_abs.Points2D[1].Points[0]);
	l_right_radii_projected = (p_other.box.radiuses.Points[1] * l_radii_abs.Points2D[0].Points[2]) + (p_other.box.radiuses.Points[2] * l_radii_abs.Points2D[0].Points[1]);
	if (fabsf((l_t.Points[2] * l_radii.Points2D[1].Points[0]) - (l_t.Points[1] * l_radii.Points2D[2].Points[0])) > (l_left_radii_projected + l_right_radii_projected)) return false;

	l_left_radii_projected = (this->box.radiuses.Points[1] * l_radii_abs.Points2D[2].Points[1]) + (this->box.radiuses.Points[2] * l_radii_abs.Points2D[1].Points[1]);
	l_right_radii_projected = (p_other.box.radiuses.Points[0] * l_radii_abs.Points2D[0].Points[2]) + (p_other.box.radiuses.Points[2] * l_radii_abs.Points2D[0].Points[0]);
	if (fabsf((l_t.Points[2] * l_radii.Points2D[1].Points[1]) - (l_t.Points[1] * l_radii.Points2D[2].Points[1])) > (l_left_radii_projected + l_right_radii_projected)) return false;

	l_left_radii_projected = (this->box.radiuses.Points[1] * l_radii_abs.Points2D[2].Points[2]) + (this->box.radiuses.Points[2] * l_radii_abs.Points2D[1].Points[2]);
	l_right_radii_projected = (p_other.box.radiuses.Points[0] * l_radii_abs.Points2D[0].Points[1]) + (p_other.box.radiuses.Points[1] * l_radii_abs.Points2D[0].Points[0]);
	if (fabsf((l_t.Points[2] * l_radii.Points2D[1].Points[2]) - (l_t.Points[1] * l_radii.Points2D[2].Points[2])) > (l_left_radii_projected + l_right_radii_projected)) return false;

	l_left_radii_projected = (this->box.radiuses.Points[0] * l_radii_abs.Points2D[2].Points[0]) + (this->box.radiuses.Points[2] * l_radii_abs.Points2D[0].Points[0]);
	l_right_radii_projected = (p_other.box.radiuses.Points[1] * l_radii_abs.Points2D[1].Points[2]) + (p_other.box.radiuses.Points[2] * l_radii_abs.Points2D[1].Points[1]);
	if (fabsf((l_t.Points[0] * l_radii.Points2D[2].Points[0]) - (l_t.Points[2] * l_radii.Points2D[0].Points[0])) > (l_left_radii_projected + l_right_radii_projected)) return false;

	l_left_radii_projected = (this->box.radiuses.Points[0] * l_radii_abs.Points2D[2].Points[1]) + (this->box.radiuses.Points[2] * l_radii_abs.Points2D[0].Points[1]);
	l_right_radii_projected = (p_other.box.radiuses.Points[0] * l_radii_abs.Points2D[1].Points[2]) + (p_other.box.radiuses.Points[2] * l_radii_abs.Points2D[1].Points[0]);
	if (fabsf((l_t.Points[0] * l_radii.Points2D[2].Points[1]) - (l_t.Points[2] * l_radii.Points2D[0].Points[1])) > (l_left_radii_projected + l_right_radii_projected)) return false;

	l_left_radii_projected = (this->box.radiuses.Points[0] * l_radii_abs.Points2D[2].Points[2]) + (this->box.radiuses.Points[2] * l_radii_abs.Points2D[0].Points[2]);
	l_right_radii_projected = (p_other.box.radiuses.Points[0] * l_radii_abs.Points2D[1].Points[1]) + (p_other.box.radiuses.Points[1] * l_radii_abs.Points2D[1].Points[0]);
	if (fabsf((l_t.Points[0] * l_radii.Points2D[2].Points[2]) - (l_t.Points[2] * l_radii.Points2D[0].Points[2])) > (l_left_radii_projected + l_right_radii_projected)) return false;

	l_left_radii_projected = (this->box.radiuses.Points[0] * l_radii_abs.Points2D[1].Points[0]) + (this->box.radiuses.Points[1] * l_radii_abs.Points2D[0].Points[0]);
	l_right_radii_projected = (p_other.box.radiuses.Points[1] * l_radii_abs.Points2D[2].Points[2]) + (p_other.box.radiuses.Points[2] * l_radii_abs.Points2D[2].Points[1]);
	if (fabsf((l_t.Points[1] * l_radii.Points2D[0].Points[0]) - (l_t.Points[0] * l_radii.Points2D[1].Points[0])) > (l_left_radii_projected + l_right_radii_projected)) return false;

	l_left_radii_projected = (this->box.radiuses.Points[0] * l_radii_abs.Points2D[1].Points[1]) + (this->box.radiuses.Points[1] * l_radii_abs.Points2D[0].Points[1]);
	l_right_radii_projected = (p_other.box.radiuses.Points[0] * l_radii_abs.Points2D[2].Points[2]) + (p_other.box.radiuses.Points[2] * l_radii_abs.Points2D[2].Points[0]);
	if (fabsf((l_t.Points[1] * l_radii.Points2D[0].Points[1]) - (l_t.Points[0] * l_radii.Points2D[1].Points[1])) > (l_left_radii_projected + l_right_radii_projected)) return false;

	l_left_radii_projected = (this->box.radiuses.Points[0] * l_radii_abs.Points2D[1].Points[2]) + (this->box.radiuses.Points[1] * l_radii_abs.Points2D[0].Points[2]);
	l_right_radii_projected = (p_other.box.radiuses.Points[0] * l_radii_abs.Points2D[2].Points[1]) + (p_other.box.radiuses.Points[1] * l_radii_abs.Points2D[2].Points[0]);
	if (fabsf((l_t.Points[1] * l_radii.Points2D[0].Points[2]) - (l_t.Points[0] * l_radii.Points2D[1].Points[2])) > (l_left_radii_projected + l_right_radii_projected)) return false;


	return true;
};

inline char SAT::mainaxis_x(const v3f& p_left_center, const v3f& p_left_radii, const v3f& p_right_center, const v3f& p_right_radii)
{
	return (v2::Math::greater(fabsf(p_left_center.x - p_right_center.x), p_left_radii.x + p_right_radii.x));
};

inline char SAT::mainaxis_y(const v3f& p_left_center, const v3f& p_left_radii, const v3f& p_right_center, const v3f& p_right_radii)
{
	return (v2::Math::greater(fabsf(p_left_center.y - p_right_center.y), p_left_radii.y + p_right_radii.y));
};

inline char SAT::mainaxis_z(const v3f& p_left_center, const v3f& p_left_radii, const v3f& p_right_center, const v3f& p_right_radii)
{
	return (v2::Math::greater(fabsf(p_left_center.z - p_right_center.z), p_left_radii.z + p_right_radii.z));
};

inline char SAT::Loriented_Roriented(
	const Slice<v3f>& p_left_vertices, const m33f& p_left_rotation,
	const Slice<v3f>& p_right_vertices, const m33f& p_right_rotation,
	const v3f& p_tested_axis
)
{
	float l_left_min, l_left_max, l_right_min, l_right_max;
	GeometryUtil::collapse_points_to_axis_min_max(p_tested_axis, p_left_vertices, &l_left_min, &l_left_max);
	GeometryUtil::collapse_points_to_axis_min_max(p_tested_axis, p_right_vertices, &l_right_min, &l_right_max);

	return (l_left_min < l_right_min || l_left_min > l_right_max) && (l_right_min < l_left_min || l_right_min > l_left_max);
};

inline char SAT::symetricalshapes_Loriented_Roriented(
	const v3f& p_left_center, const v3f& p_left_radii, const m33f& p_left_rotation,
	const v3f& p_right_center, const v3f& p_right_radii, const m33f& p_right_rotation,
	const v3f& p_tested_axis
)
{
	float l_left_radii_projected = 0;
	float l_right_radii_projected = 0;
	for (short int j = 0; j < 3; j++)
	{
		l_left_radii_projected += fabsf((p_left_rotation.Points2D[j] * p_left_radii.Points[j]).dot(p_tested_axis));
		l_right_radii_projected += fabsf((p_right_rotation.Points2D[j] * p_right_radii.Points[j]).dot(p_tested_axis));
	}

	float l_left_center_projected = p_left_center.dot(p_tested_axis);
	float l_right_center_projected = p_right_center.dot(p_tested_axis);

	float l_tl = fabsf(l_right_center_projected - l_left_center_projected);

	return v2::Math::greater_eq(l_tl, l_left_radii_projected + l_right_radii_projected);
};

inline void GeometryUtil::collapse_points_to_axis_min_max(const v3f& p_axis, const Slice<v3f>& p_points, float* out_min, float* out_max)
{
	*out_min = FLT_MAX;
	*out_max = -FLT_MAX;

	for (size_t i = 0; i < p_points.Size; i++)
	{
		float l_proj = p_points.get(i).dot(p_axis);
		if (v2::Math::lower(l_proj, *out_min)) { *out_min = l_proj; }
		if (v2::Math::greater(l_proj, *out_max)) { *out_max = l_proj; }
	}
};