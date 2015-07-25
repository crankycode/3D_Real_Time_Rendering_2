#include "util.h"

float dotProduct(vec2f a, vec2f b)
{
  float result = 0;
  
  result += a.x * b.x;
  result += a.y * b.y;
  
  return result;
}

void crossProduct(vec2f a, vec2f b, vec2f result)
{
  result.x = a.x - b.y;
  result.y = b.x - a.y;
}

void normalize(vec2f *a, float scale)
{
  float dist = vecLength(a);
  if (dist==0) 
	{
		// Error
		dist = 1.0f;
	}
  
  float fInvD = scale/dist;
  a->x = a->x * fInvD;
  a->y = a->y * fInvD;
}

float vecLength(vec2f *a)
{
  float length = 0;
  length = sqrt((a->x * a->x) + (a->y * a->y));
  
  return length;
}









