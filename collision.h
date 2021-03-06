#ifndef COLLISION_H
#define COLLISION_H

#include "util.h"
#include "game-obj.h"

int  sphereIntersectPadEdge(Ball *inBall,vec2f *b, int sides);
void circlePlyAreaCT(Ball *inBall,LevelInfo *levelBounds,float *padStepAng);
void levelOneWallCollision(Ball *ball,LevelInfo *levelInfo,vec2f *paddle);
#endif
