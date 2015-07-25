#include "collision.h"

int sphereIntersectPadEdge(Ball *inBall,vec2f *b, int sides)
{
  vec2f dist, nDist;
  vec2f edgeVec;
  float dSqrt, mag;
  float section;

  edgeVec.x =  1;  edgeVec.y = 0.1;
  dist.x = inBall->pos.x - b->x;  dist.y = inBall->pos.y - b->y;
  dSqrt = dotProduct((dist),(dist));
  // normalize
  mag   = sqrt(dSqrt);
  nDist.x = dist.x/mag; nDist.y = dist.y/mag;

  switch (sides)
  {
    case LEFT:
      section = dotProduct((nDist),edgeVec);
      if ( (dSqrt < ((inBall->ballRadius + PADDLE_EDGE_RADIUS) * (inBall->ballRadius + PADDLE_EDGE_RADIUS)))) {
        if ( section < COS_105)
          return EDGE_REBOUND;
        else
          return NORMAL_REBOUND;
      }
      else {
        return false;
      }
    break;

    case RIGHT:
      section = dotProduct((nDist),edgeVec);
      if ( (dSqrt < ((inBall->ballRadius + PADDLE_EDGE_RADIUS) * (inBall->ballRadius + PADDLE_EDGE_RADIUS)))) {
        if ( section < COS_45) {
          return NORMAL_REBOUND;
        }
        else {
          return EDGE_REBOUND;
        }
      }
      else {
        return false;
      }
    break;
  }
  return false;
}

void circlePlyAreaCT(Ball *inBall,LevelInfo *lvlInfo,float *padStepAng)
{
  vec2f P;
  vec2f center     = lvlInfo->center;
  int   counter    = 0;
  float vp, tan;
  float cx, cy;
  float lengthP;
  float radius     = lvlInfo->radius;
  float cRadLength = sqrtf((radius - inBall->ballRadius) * (radius - inBall->ballRadius));
  cx               = center.x;
  cy               = center.y;

  /* bound ball off the sides of circle play area */
  P.x = inBall->pos.x - cx;
  P.y = inBall->pos.y -  cy;
  lengthP = vecLength(&P);

  if (lengthP >= cRadLength) {
    normalize (&P,1);
    tan = atan2(P.y,P.x) * 180/M_PI;
    /* if hit in the range of 210 to 310 deg, play loss 1 life */
    if (tan > -150.0 && tan < -30.0f) {
      lvlInfo->life += lvlInfo->life > 0 ? -1 : 5;
    inBall->pos.x = center.x;
      inBall->pos.y = center.y;
      inBall->ballVel.y = -1.4;
      inBall->ballVel.x = 0.0f;
      *padStepAng = 0.0f;
    return;
    }
    // Find projection
    vp = dotProduct(inBall->ballVel,P);

    inBall->ballVel.x -= 2 * vp * P.x;
    inBall->ballVel.y -= 2 * vp * P.y;
    normalize (&inBall->ballVel,2);

    P.x = cx - inBall->pos.x;
    P.y = cy - inBall->pos.y;
    lengthP = vecLength(&P);

    /* Move the ball out of wall */
    while (lengthP > sqrt ((radius - inBall->ballRadius) * (radius - inBall->ballRadius)))
    {
      counter++;
      inBall->pos.x += inBall->ballVel.x * 0.05;
      inBall->pos.y += inBall->ballVel.y * 0.05;
      P.x = cx - inBall->pos.x;
      P.y = cy - inBall->pos.y;
      lengthP = vecLength(&P);
      // Prevent infinite loops
      if (counter == 30)
        break;
    }
  }
}

void levelOneWallCollision(Ball *ball,LevelInfo *levelInfo,vec2f *paddle)
{
    vec2f *levelBounds = &levelInfo->lvlBound;
    /* bounce ball off sides */
    if (ball->pos.x < 0.0f + ball->ballRadius)
    {
      ball->pos.x -= 2.0f * (ball->pos.x - ball->ballRadius);
      ball->ballVel.x = fabs(ball->ballVel.x);
    }
    if (ball->pos.x > levelBounds->x - ball->ballRadius)
    {
      ball->pos.x -= 2.0f * (ball->pos.x - levelBounds->x + ball->ballRadius);
      ball->ballVel.x = -fabs(ball->ballVel.x);
    }
    if (ball->pos.y > levelBounds->y - ball->ballRadius)
    {
      ball->pos.y -= 2.0f * (ball->pos.y - levelBounds->y + ball->ballRadius);
      ball->ballVel.y = -fabs(ball->ballVel.y);
    }
    if (ball->pos.y < 0.0f - ball->ballRadius)
    {
      levelInfo->life -= levelInfo->life > 0 ? 1 : 0;
      /* lose one ball, lost one life */
      ball->pos.x = levelBounds->x * 0.5f;
      ball->pos.y = 1.0f;
      ball->ballVel.y = fabs(ball->ballVel.y);
      paddle->x = levelBounds->x * 0.5f;
    }
}
