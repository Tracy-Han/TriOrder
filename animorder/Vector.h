#ifndef VECTOR_H
#define VECTOR_H

#include <cmath>
//a simple Vector class to keep the project self-contained

class Vector
{
public:
	float v[3];
	Vector() {}
	Vector(float a, float b, float c) { v[0] = a; v[1] = b; v[2] = c; }
	Vector(float *a) { v[0] = a[0]; v[1] = a[1]; v[2] = a[2]; }
	float& operator[](int i){ return v[i]; }
	const float& operator[](int i) const { return v[i]; }
	void operator+=(const Vector a) { v[0] += a.v[0]; v[1] += a.v[1]; v[2] += a.v[2]; }
	void operator-=(const Vector a) { v[0] -= a.v[0]; v[1] -= a.v[1]; v[2] -= a.v[2]; }
	Vector operator+(const Vector a) { return Vector(v[0] + a.v[0], v[1] + a.v[1], v[2] + a.v[2]); }
	Vector operator-(const Vector a) { return Vector(v[0] - a.v[0], v[1] - a.v[1], v[2] - a.v[2]); }
	Vector operator/(const float n) { return Vector(v[0] / n, v[1] / n, v[2] / n); }
	void operator/=(const int n) { v[0] /= (float)n; v[1] /= (float)n; v[2] /= (float)n; }
	void operator/=(const float n) { v[0] /= n; v[1] /= n; v[2] /= n; }
	Vector operator*(const float a) { return Vector(v[0] * a, v[1] * a, v[2] * a); }
	Vector operator*(const Vector a) { return Vector(v[0] * a.v[0], v[1] * a.v[1], v[2] * a.v[2]); }
	Vector operator/(const Vector a) { return Vector(v[0] / a.v[0], v[1] / a.v[1], v[2] / a.v[2]); }
	float length() { float w = v[0] * v[0] + v[1] * v[1] + v[2] * v[2]; return w > 0.f ? sqrtf(w) : 0.f; }
	void normalize() { float w = v[0] * v[0] + v[1] * v[1] + v[2] * v[2]; if (w > 0.f) *this /= sqrtf(w); }
};

static float dot(const Vector a, const Vector b)
{
	return (a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2]);
}

static Vector cross(const Vector a, const Vector b)
{
	return Vector(a.v[1] * b.v[2] - a.v[2] * b.v[1], a.v[2] * b.v[0] - a.v[0] * b.v[2], a.v[0] * b.v[1] - a.v[1] * b.v[0]);
}

static float dist(const Vector a, const Vector b)
{
	Vector c = Vector(a);
	return (c - b).length();
}
#endif