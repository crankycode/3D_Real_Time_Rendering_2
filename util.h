#ifndef _UTIL_H
#define _UTIL_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define TRUE  1
#define FALSE 0
#define PI_OVER_180 0.0174532f	/* Pi/180 (convert deg to rad) */

/* stuff math.h misses */
#ifndef fabs
#define fabs(x) ((x)<0?-(x):x)
#endif
#ifndef M_PI
#define M_PI 3.1416f
#endif

/* add the bool type if not found */
#ifndef bool
typedef char bool;
#define true 1
#define false 0
#endif

typedef struct vec2f
{
	float x;
	float y;
} vec2f;

typedef struct vec3f
{
	float x;
	float y;
  float z;
} vec3f;

typedef struct matrix
{
  float m[4][4];
} matrix;

float dotProduct(vec2f a, vec2f b);
void  normalize(vec2f *a, float scale);
float vecLength(vec2f *a);
void  crossProduct(vec2f a, vec2f b, vec2f result);
#endif
