/*
 * Copyright 2011-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#ifndef BX_BOUNDS_H_HEADER_GUARD
#define BX_BOUNDS_H_HEADER_GUARD

#include <bx/math.h>

namespace bx
{
	///
	struct Line
	{
		Vec3 pos = InitNone;
		Vec3 dir = InitNone;
	};

	///
	struct LineSegment
	{
		Vec3 pos = InitNone;
		Vec3 end = InitNone;
	};

	///
	struct Aabb
	{
		Vec3 min = InitNone;
		Vec3 max = InitNone;
	};

	///
	struct Capsule
	{
		Vec3  pos = InitNone;
		Vec3  end = InitNone;
		float radius;
	};

	///
	struct Cone
	{
		Vec3  pos = InitNone;
		Vec3  end = InitNone;
		float radius;
	};

	///
	struct Cylinder
	{
		Vec3  pos = InitNone;
		Vec3  end = InitNone;
		float radius;
	};

	///
	struct Disk
	{
		Vec3  center = InitNone;
		Vec3  normal = InitNone;
		float radius;
	};

	///
	struct Obb
	{
		float mtx[16];
	};

	///
	struct Sphere
	{
		Vec3  center = InitNone;
		float radius;
	};

	///
	struct Triangle
	{
		Vec3 v0 = InitNone;
		Vec3 v1 = InitNone;
		Vec3 v2 = InitNone;
	};

	///
	struct Ray
	{
		Vec3 pos = InitNone;
		Vec3 dir = InitNone;
	};

	///
	struct Hit
	{
		Vec3  pos   = InitNone;
		Plane plane = InitNone;
	};

	///
	struct Interval
	{
		///
		Interval(float _val);

		///
		Interval(float _min, float _max);

		///
		void set(float _val);

		///
		void setCenter(float _val);

		///
		void expand(float _val);

		float min;
		float max;
	};

	///
	Vec3 getPointAt(const Ray& _ray, float _t);

	///
	Vec3 getPointAt(const Line& _line, float _t);

	///
	Vec3 getPointAt(const LineSegment& _line, float _t);

	///
	Vec3 getCenter(const Aabb& _aabb);

	///
	Vec3 getExtents(const Aabb& _aabb);

	///
	Vec3 getCenter(const Triangle& _triangle);

	///
	void toAabb(Aabb& _outAabb, const Vec3& _extents);

	///
	void toAabb(Aabb& _outAabb, const Vec3& _center, const Vec3& _extents);

	/// Convert cylinder to axis aligned bounding box.
	void toAabb(Aabb& _outAabb, const Cylinder& _cylinder);

	/// Convert disk to axis aligned bounding box.
	void toAabb(Aabb& _outAabb, const Disk& _disk);

	/// Convert oriented bounding box to axis aligned bounding box.
	void toAabb(Aabb& _outAabb, const Obb& _obb);

	/// Convert sphere to axis aligned bounding box.
	void toAabb(Aabb& _outAabb, const Sphere& _sphere);

	/// Convert triangle to axis aligned bounding box.
	void toAabb(Aabb& _outAabb, const Triangle& _triangle);

	/// Calculate axis aligned bounding box.
	void toAabb(Aabb& _outAabb, const void* _vertices, uint32_t _numVertices, uint32_t _stride);

	/// Transform vertices and calculate axis aligned bounding box.
	void toAabb(Aabb& _outAabb, const float* _mtx, const void* _vertices, uint32_t _numVertices, uint32_t _stride);

	/// Expand AABB.
	void aabbExpand(Aabb& _outAabb, float _factor);

	/// Expand AABB with xyz.
	void aabbExpand(Aabb& _outAabb, const Vec3& _pos);

	/// Calculate surface area of axis aligned bounding box.
	float calcAreaAabb(const Aabb& _aabb);

	/// Convert axis aligned bounding box to oriented bounding box.
	void toObb(Obb& _outObb, const Aabb& _aabb);

	/// Calculate oriented bounding box.
	void calcObb(Obb& _outObb, const void* _vertices, uint32_t _numVertices, uint32_t _stride, uint32_t _steps = 17);

	/// Calculate maximum bounding sphere.
	void calcMaxBoundingSphere(Sphere& _outSphere, const void* _vertices, uint32_t _numVertices, uint32_t _stride);

	/// Calculate minimum bounding sphere.
	void calcMinBoundingSphere(Sphere& _outSphere, const void* _vertices, uint32_t _numVertices, uint32_t _stride, float _step = 0.01f);

	/// Returns 6 (near, far, left, right, top, bottom) planes representing frustum planes.
	void buildFrustumPlanes(Plane* _outPlanes, const float* _viewProj);

	/// Returns point from 3 intersecting planes.
	Vec3 intersectPlanes(const Plane& _pa, const Plane& _pb, const Plane& _pc);

	/// Make screen space ray from x, y coordinate and inverse view-projection matrix.
	Ray makeRay(float _x, float _y, const float* _invVp);

	/// Intersect ray / AABB.
	bool intersect(const Ray& _ray, const Aabb& _aabb, Hit* _hit = NULL);

	/// Intersect ray / OBB.
	bool intersect(const Ray& _ray, const Obb& _obb, Hit* _hit = NULL);

	/// Intersect ray / cylinder.
	bool intersect(const Ray& _ray, const Cylinder& _cylinder, Hit* _hit = NULL);

	/// Intersect ray / capsule.
	bool intersect(const Ray& _ray, const Capsule& _capsule, Hit* _hit = NULL);

	/// Intersect ray / cone.
	bool intersect(const Ray& _ray, const Cone& _cone, Hit* _hit = NULL);

	/// Intersect ray / disk.
	bool intersect(const Ray& _ray, const Disk& _disk, Hit* _hit = NULL);

	/// Intersect ray / plane.
	bool intersect(const Ray& _ray, const Plane& _plane, Hit* _hit = NULL);

	/// Intersect ray / plane.
	bool intersect(const Ray& _ray, const Plane& _plane, bool _doublesided, Hit* _hit = NULL);

	/// Intersect ray / sphere.
	bool intersect(const Ray& _ray, const Sphere& _sphere, Hit* _hit = NULL);

	/// Intersect ray / triangle.
	bool intersect(const Ray& _ray, const Triangle& _triangle, Hit* _hit = NULL);

	///
	Vec3 closestPoint(const Line& _line, const Vec3& _point);

	///
	Vec3 closestPoint(const LineSegment& _line, const Vec3& _point);

	///
	Vec3 closestPoint(const Plane& _plane, const Vec3& _point);

	///
	Vec3 closestPoint(const Aabb& _aabb, const Vec3& _point);

	///
	Vec3 closestPoint(const Obb& _obb, const Vec3& _point);

	///
	Vec3 closestPoint(const Triangle& _triangle, const Vec3& _point);

	///
	bool overlap(const Interval& _interval, float _pos);

	///
	bool overlap(const Interval& _intervalA, const Interval& _intervalB);

	///
	bool overlap(const Aabb& _aabb, const Vec3& _pos);

	///
	bool overlap(const Aabb& _aabb, const Sphere& _sphere);

	///
	bool overlap(const Aabb& _aabbA, const Aabb& _aabbB);

	///
	bool overlap(const Aabb& _aabb, const Plane& _plane);

	///
	bool overlap(const Aabb& _aabb, const Triangle& _triangle);

	///
	bool overlap(const Aabb& _aabb, const Cylinder& _cylinder);

	///
	bool overlap(const Aabb& _aabb, const Capsule& _capsule);

	///
	bool overlap(const Aabb& _aabb, const Cone& _cone);

	///
	bool overlap(const Aabb& _aabb, const Disk& _disk);

	///
	bool overlap(const Aabb& _aabb, const Obb& _obb);

	///
	bool overlap(const Capsule& _capsule, const Vec3& _pos);

	///
	bool overlap(const Capsule& _capsule, const Sphere& _sphere);

	///
	bool overlap(const Capsule& _capsule, const Aabb& _aabb);

	///
	bool overlap(const Capsule& _capsule, const Plane& _plane);

	///
	bool overlap(const Capsule& _capsule, const Triangle& _triangle);

	///
	bool overlap(const Capsule& _capsule, const Cylinder& _cylinder);

	///
	bool overlap(const Capsule& _capsuleA, const Capsule& _capsuleB);

	///
	bool overlap(const Capsule& _capsule, const Cone& _cone);

	///
	bool overlap(const Capsule& _capsule, const Disk& _disk);

	///
	bool overlap(const Capsule& _capsule, const Obb& _obb);

	///
	bool overlap(const Cone& _cone, const Vec3& _pos);

	///
	bool overlap(const Cone& _cone, const Sphere& _sphere);

	///
	bool overlap(const Cone& _cone, const Aabb& _aabb);

	///
	bool overlap(const Cone& _cone, const Plane& _plane);

	///
	bool overlap(const Cone& _cone, const Triangle& _triangle);

	///
	bool overlap(const Cone& _cone, const Cylinder& _cylinder);

	///
	bool overlap(const Cone& _cone, const Capsule& _capsule);

	///
	bool overlap(const Cone& _coneA, const Cone& _coneB);

	///
	bool overlap(const Cone& _cone, const Disk& _disk);

	///
	bool overlap(const Cone& _cone, const Obb& _obb);

	///
	bool overlap(const Cylinder& _cylinder, const Vec3& _pos);

	///
	bool overlap(const Cylinder& _cylinder, const Sphere& _sphere);

	///
	bool overlap(const Cylinder& _cylinder, const Aabb& _aabb);

	///
	bool overlap(const Cylinder& _cylinder, const Plane& _plane);

	///
	bool overlap(const Cylinder& _cylinder, const Triangle& _triangle);

	///
	bool overlap(const Cylinder& _cylinderA, const Cylinder& _cylinderB);

	///
	bool overlap(const Cylinder& _cylinder, const Capsule& _capsule);

	///
	bool overlap(const Cylinder& _cylinder, const Cone& _cone);

	///
	bool overlap(const Cylinder& _cylinder, const Disk& _disk);

	///
	bool overlap(const Cylinder& _cylinder, const Obb& _obb);

	///
	bool overlap(const Disk& _disk, const Vec3& _pos);

	///
	bool overlap(const Disk& _disk, const Sphere& _sphere);

	///
	bool overlap(const Disk& _disk, const Aabb& _aabb);

	///
	bool overlap(const Disk& _disk, const Plane& _plane);

	///
	bool overlap(const Disk& _disk, const Triangle& _triangle);

	///
	bool overlap(const Disk& _disk, const Cylinder& _cylinder);

	///
	bool overlap(const Disk& _disk, const Capsule& _capsule);

	///
	bool overlap(const Disk& _disk, const Cone& _cone);

	///
	bool overlap(const Disk& _diskA, const Disk& _diskB);

	///
	bool overlap(const Disk& _disk, const Obb& _obb);

	///
	bool overlap(const Obb& _obb, const Vec3& _pos);

	///
	bool overlap(const Obb& _obb, const Sphere& _sphere);

	///
	bool overlap(const Obb& _obb, const Aabb& _aabb);

	///
	bool overlap(const Obb& _obb, const Plane& _plane);

	///
	bool overlap(const Obb& _obb, const Triangle& _triangle);

	///
	bool overlap(const Obb& _obb, const Cylinder& _cylinder);

	///
	bool overlap(const Obb& _obb, const Capsule& _capsule);

	///
	bool overlap(const Obb& _obb, const Cone& _cone);

	///
	bool overlap(const Obb& _obb, const Disk& _disk);

	///
	bool overlap(const Obb& _obbA, const Obb& _obbB);

	///
	bool overlap(const Plane& _plane, const Vec3& _pos);

	///
	bool overlap(const Plane& _plane, const Sphere& _sphere);

	///
	bool overlap(const Plane& _plane, const Aabb& _aabb);

	///
	bool overlap(const Plane& _planeA, const Plane& _planeB);

	///
	bool overlap(const Plane& _plane, const Triangle& _triangle);

	///
	bool overlap(const Plane& _plane, const Cylinder& _cylinder);

	///
	bool overlap(const Plane& _plane, const Capsule& _capsule);

	///
	bool overlap(const Plane& _plane, const Cone& _cone);

	///
	bool overlap(const Plane& _plane, const Disk& _disk);

	///
	bool overlap(const Plane& _plane, const Obb& _obb);

	///
	bool overlap(const Sphere& _sphere, const Vec3& _pos);

	///
	bool overlap(const Sphere& _sphereA, const Sphere& _sphereB);

	///
	bool overlap(const Sphere& _sphere, const Aabb& _aabb);

	///
	bool overlap(const Sphere& _sphere, const Plane& _plane);

	///
	bool overlap(const Sphere& _sphere, const Triangle& _triangle);

	///
	bool overlap(const Sphere& _sphere, const Cylinder& _cylinder);

	///
	bool overlap(const Sphere& _sphere, const Capsule& _capsule);

	///
	bool overlap(const Sphere& _sphere, const Cone& _cone);

	///
	bool overlap(const Sphere& _sphere, const Disk& _disk);

	///
	bool overlap(const Sphere& _sphere, const Obb& _obb);

	///
	bool overlap(const Triangle& _triangle, const Vec3& _pos);

	///
	bool overlap(const Triangle& _triangle, const Sphere& _sphere);

	///
	bool overlap(const Triangle& _triangle, const Aabb& _aabb);

	///
	bool overlap(const Triangle& _triangle, const Plane& _plane);

	///
	bool overlap(const Triangle& _triangleA, const Triangle& _triangleB);

	///
	bool overlap(const Triangle& _triangle, const Cylinder& _cylinder);

	///
	bool overlap(const Triangle& _triangle, const Capsule& _capsule);

	///
	bool overlap(const Triangle& _triangle, const Cone& _cone);

	///
	bool overlap(const Triangle& _triangle, const Disk& _disk);

	///
	bool overlap(const Triangle& _triangle, const Obb& _obb);

} // namespace bx

#include "inline/bounds.inl"

#endif // BX_BOUNDS_H_HEADER_GUARD
