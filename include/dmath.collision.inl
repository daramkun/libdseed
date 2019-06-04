#ifndef __DMATH_COLLISION_INL__
#define __DMATH_COLLISION_INL__

namespace dseed
{
	struct plane;
	struct ray;

	struct bounding_box;
	struct bounding_sphere;
	struct bounding_frustum;

	enum intersect_t { intersect_disjoint = 0, intersect_intersects, intersect_contains };
	enum plane_intersect_t { plane_intersect_front, plane_intersect_end, plane_intersect_intersecting };

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Bounding shape Type definitions
	//
	////////////////////////////////////////////////////////////////////////////////////////
	struct plane
	{
	public:
		float x, y, z, w;

	public:
		plane () = default;
		inline plane (const float3& p1, const float3& p2, const float3& p3);
		inline plane (const float3& p, const float3& n);
		inline plane (float x, float y, float z, float w) : x (x), y (y), z (z), w (w) { }
		inline plane (const float4& v) : plane (v.x, v.y, v.z, v.w) { }

	public:
		inline operator float4 () const noexcept { return float4 (x, y, z, w); }

	public:
		inline vectorf normalize ();

	public:
		inline bool intersects (const plane& p);
		inline void intersects (const plane& p, float3* ip1, float3* ip2);

		inline bool intersects (const float3& lp1, const float3& lp2);
		inline void intersects (const float3& lp1, const float3& lp2, float3* ip);
	};

	struct ray
	{
	public:
		float3 position, direction;

	public:
		ray () = default;
		inline ray (const float3& position, const float3& direction)
			: position (position), direction (direction)
		{ }
	};

	struct bounding_box
	{
	public:
		float3 center, extents;
		quaternion orientation;

	public:
		bounding_box () = default;
		inline bounding_box (const float3& center, const float3& extents,
			const quaternion& orientation)
			: center (center), extents (extents), orientation (orientation)
		{ }

	public:
		intersect_t intersects (const bounding_box& bb) const;
		intersect_t intersects (const bounding_sphere& bs) const;
		intersect_t intersects (const bounding_frustum& bf) const;
	};

	struct bounding_sphere
	{
	public:
		float3 center;
		float radius;

	public:
		bounding_sphere () = default;
		inline bounding_sphere (const float3& center, float radius)
			: center (center), radius (radius)
		{ }

	public:
		intersect_t intersects (const bounding_box& bb) const;
		intersect_t intersects (const bounding_sphere& bs) const;
		intersect_t intersects (const bounding_frustum& bf) const;
	};

	struct bounding_frustum
	{
	public:
		float3 position;
		quaternion orientation;

		float right, left, top, bottom;
		float znear, zfar;

	public:
		bounding_frustum () = default;
		inline bounding_frustum (const float3& position, const quaternion& orientation,
			float left, float top, float bottom, float right, float znear, float zfar)
			: position (position), orientation (orientation)
			, left (left), top (top), bottom (bottom), right (right)
			, znear (znear), zfar (zfar)
		{}
		inline bounding_frustum (const float4x4& projection);

	public:
		intersect_t intersects (const bounding_box& bb) const;
		intersect_t intersects (const bounding_sphere& bs) const;
		intersect_t intersects (const bounding_frustum& bf) const;
	};

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Bounding Shape Type Operators
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline float dot_plane_normal (const vectorf& p, const vectorf& n)
	{
		return dot3 (p, n);
	}
	inline float dot_plane_coord (const vectorf& p, const vectorf& v)
	{
		return dot_plane_normal (p, v) + p.w ();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// Bounding Shape Type Implement
	//
	////////////////////////////////////////////////////////////////////////////////////////
	inline plane::plane (const float3& p1, const float3& p2, const float3& p3)
	: plane (p1, normalize3 (cross3 ((vectorf)p2 - p1, (vectorf)p3 - p1)))
	{ }
	inline plane::plane (const float3& p, const float3& n)
		: plane (n.x, n.y, n.z, -dot3 (p, n))
	{ }
	inline vectorf plane::normalize () { return normalize_plane ((float4)* this); }

	inline bool plane::intersects (const plane& p)
	{
		float3 ip1, ip2;
		intersects (p, &ip1, &ip2);
		return !(ip1 == ip2 && ip1 == float3 (dseed::single_nan));
	}

	inline void plane::intersects (const plane& p, float3* ip1, float3* ip2)
	{
		if (ip1 == nullptr || ip2 == nullptr) return;

		vectorf v1 = cross3 ((float4)p, (float4)* this);
		float lengthsq = length_squared3 (v1);

		vectorf v2 = cross3 ((float4)p, v1);
		vectorf point = v1 * float4 (w, w, w, w);

		point = fmavf (cross3 (v1, (float4)* this), float4 (p.w, p.w, p.w, p.w), point);

		if (dseed::equals (lengthsq, 0))
			* ip2 = (*ip1 = point / lengthsq) + v1;
		else
			*ip1 = *ip2 = float3 (dseed::single_nan);
	}

	inline bool plane::intersects (const float3& lp1, const float3& lp2)
	{
		float3 ip;
		intersects (lp1, lp2, &ip);
		return ip != float3 (dseed::single_nan);
	}
	inline void plane::intersects (const float3& lp1, const float3& lp2, float3* ip)
	{
		if (ip == nullptr)
			return;

		float d = dot3 ((float4)* this, lp1) - dot3 ((float4)* this, lp2);
		*ip = dseed::equals (d, 0)
			? float3 (fmavf (lp2 - lp1, float3 (dot_plane_coord ((float4)* this, lp1) / d), lp1))
			: float3 (dseed::single_nan);
	}

	const static vectorf ____bounding_box_polygon[] = {
		float3 (-1.0f, -1.0f, +1.0f),
		float3 (+1.0f, -1.0f, +1.0f),
		float3 (+1.0f, +1.0f, +1.0f),
		float3 (-1.0f, +1.0f, +1.0f),
		float3 (-1.0f, -1.0f, -1.0f),
		float3 (+1.0f, -1.0f, -1.0f),
		float3 (+1.0f, +1.0f, -1.0f),
		float3 (-1.0f, +1.0f, -1.0f),
	};

	inline intersect_t bounding_box::intersects (const bounding_box& bb) const
	{
		// TODO
	}
	inline intersect_t bounding_box::intersects (const bounding_sphere& bs) const
	{
		// TODO
	}
	inline intersect_t bounding_box::intersects (const bounding_frustum& bf) const
	{
		return bf.intersects (*this);
	}

	inline intersect_t bounding_sphere::intersects (const bounding_box& bb) const
	{
		// TODO
	}
	inline intersect_t bounding_sphere::intersects (const bounding_sphere& bs) const
	{
		vectorf center1 = center;
		vectorf center2 = bs.center;

		vectorf v = center2 - center1;
		float distance = length3 (v);

		return (radius + bs.radius >= distance)
			? (radius - bs.radius >= distance ? intersect_contains : intersect_intersects)
			: intersect_disjoint;
	}
	inline intersect_t bounding_sphere::intersects (const bounding_frustum& bf) const
	{
		return bf.intersects (*this);
	}

	inline bounding_frustum::bounding_frustum (const float4x4& projection)
	{
		static vectorf homogenousPoints[6] =
		{
			float4 (1.0f,  0.0f, 1.0f, 1.0f),
			float4 (-1.0f,  0.0f, 1.0f, 1.0f),
			float4 (0.0f,  1.0f, 1.0f, 1.0f),
			float4 (0.0f, -1.0f, 1.0f, 1.0f),

			float4 (0.0f, 0.0f, 0.0f, 1.0f),
			float4 (0.0f, 0.0f, 1.0f, 1.0f)
		};

		matrixf matInverse = projection.invertmf ();
		vectorf points[6] = {
			transform4 (homogenousPoints[0], matInverse),
			transform4 (homogenousPoints[1], matInverse),
			transform4 (homogenousPoints[2], matInverse),
			transform4 (homogenousPoints[3], matInverse),

			transform4 (homogenousPoints[4], matInverse),
			transform4 (homogenousPoints[5], matInverse),
		};

		position = float3 (0.0f, 0.0f, 0.0f);
		orientation = float4 (0.0f, 0.0f, 0.0f, 1.0f);

		points[0] = points[0] * (float4 (1) / points[0].splat_z ());
		points[1] = points[1] * (float4 (1) / points[1].splat_z ());
		points[2] = points[2] * (float4 (1) / points[2].splat_z ());
		points[3] = points[3] * (float4 (1) / points[3].splat_z ());

		right = points[0].x ();
		left = points[1].x ();
		top = points[2].y ();
		bottom = points[3].y ();

		points[4] = points[4] * (float4 (1) / points[4].splat_w ());
		points[5] = points[5] * (float4 (1) / points[5].splat_w ());

		znear = points[4].z ();
		zfar = points[5].z ();
	}
	inline intersect_t bounding_frustum::intersects (const bounding_box& bb) const
	{
		// TODO
	}
	inline intersect_t bounding_frustum::intersects (const bounding_sphere& bs) const
	{
		// TODO
	}
	inline intersect_t bounding_frustum::intersects (const bounding_frustum& bf) const
	{
		// TODO
	}
}

#endif