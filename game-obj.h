#ifndef GAME_OBJ_H
#define GAME_OBJ_H
#include "util.h"

/* game constants */
#define PADDLE_XSIZE 0.6f
#define BALL_RADIUS  0.05f
#define PADDLE_SPEED 1.8f
#define BRICK_RADIUS 0.10f

#define PADDLE_EDGE_RADIUS 0.025f
#define NORMAL_REBOUND     1
#define EDGE_REBOUND       2
#define RADIAN_245         4.2760
#define RADIAN_270         4.7123
#define COS_135           -0.996087835
#define COS_105           -0.2588
#define COS_110           -0.3420
#define COS_210           -0.883877473
#define COS_330           -0.991198822
#define COS_45             0.707
#define LIFE_OFFSET        0.15
#define ON    	1	
#define OFF		0
#define BOTTOM  0
#define LEFT    1
#define UP      2
#define RIGHT   3

enum PlayAreaType {SQUARE,CIRCLE};

typedef struct LevelInfo {
  int   currentLevel;
  int   levelType;
  int   numPoints;
  int   totalBrick;
  int   life;
  int   gameStatus;  /* indicate the progress of the game */ 
  /*int   gameOverFlag;*/
  /*int   levelTwoFlag;*/
  /*int   levelOneFlag;*/
  /*int   completedFlag;*/
  /*int   levelOneTmStarted;*/
  /*int   levelTwoTmStarted;*/
  /*int   gameOverTmStarted;*/
  /*int   completedTmStarted;*/
  float radius;
  vec2f lvlBound;
  vec2f center;
} LevelInfo;

typedef struct Ball
{
  vec2f pos;
  vec2f ballVel;
  float ballRadius;
} Ball;

typedef struct shape
{
	int edgeNum;
	float rotAngle;
	int color;
} shape;

typedef struct brickShape
{
    shape s;
    vec2f pos;
	int show;
} brickShape;

/* define AABBs */
typedef struct AabbBox
{
    float x1;
    float y1;
    float x2;
    float y2;
}AabbBox;

#endif
