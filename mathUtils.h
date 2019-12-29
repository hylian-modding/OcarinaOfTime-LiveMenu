#ifndef MATHUTILS_H
#define MATHUTILS_H

void interpolateInt(float deltaTime, float d, int* v, int* p, int t) {
	float ddt = d * deltaTime;
	float ddd = d * ddt;

	int force = *v - (*p - t) * ddd;

	float e = 1 + ddt;
	e *= e;

	*v = force / e;

	*p += *v * deltaTime;
}

void interpolateFloat(float deltaTime, float d, float* v, float* p, float t) {
	float ddt = d * deltaTime;
	float ddd = d * ddt;

	float force = *v - (*p - t) * ddd;

	float e = 1 + ddt;
	e *= e;

	*v = force / e;

	*p += *v * deltaTime;
}

typedef struct {
    float p;
    float t;
    float v;
    float d;
} interpolator_float_t;

typedef struct {
    int p;
    int t;
    int v;
    float d;
} interpolator_int_t;


#endif


