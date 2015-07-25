/* rtr - tute-collision base 9/09/2009 pknowles */
#include "util.h"
#include "sdl-base.h"
#include "game-obj.h"
#include "collision.h"
#include "texture.h"

#include <GL/glut.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

#define FULL_LIFE           5
#define BRUTEFORCE          0
#define UNIFORMGRID         1

#define STATUS_ZERO       0
#define STATUS_PRE_START  1
#define STATUS_READY      10  /* Less than STATUS_READY means game has not started yet */
#define STATUS_PLAYING    20
#define STATUS_GAMEOVER   50
#define STATUS_WON        100 /* Won for the current level */
#define STATUS_COMPLETE   1000

#define GAME_COMPLETE_TEXTURE 3
#define LEVEL2_TEXTURE        2
#define LEVEL1_TEXTURE        1
#define GAMEOVER_TEXTURE      0

#define COMPLETE_LEVEL        2
#define MAX_BOWLPAD_SPEED     1.0f
#define MIN_BOWLPAD_SPEED     0.01f
#define MAX_PAD_ANGLE         60.0f
#define MIN_PAD_ANGLE        -60.0f
#define BOWLPAD_LOCATION      0.35f
#define TRIGGERED             3
#define LEVELONE              1
#define NUM_KEYS 1024 /* FIXME: replace with maximum SDLKey enumerator value */
bool keys[NUM_KEYS]; /* key states - up/down */

const int     numBricks = 28;
brickShape    brick[28];
AabbBox       bowlPadAABB;
AabbBox       aabb[28];
GLfloat       m[16];
float         paddleVel;
vec2f         bowlPadVec[7];
vec2f         leftPadEdge, rightPadEdge;
vec2f         paddle;
vec2f         pa, pb, midPoint, bowlLCorner,bowlRCorner;
Ball          ballArr[10];
LevelInfo     levelInfo;
int           numOfBall;
int           score;
int           prevScore   = 1;
int           currJ       = -1;
int           displayAxes = OFF;
int           displayAABB = OFF;
int           displayCTT  = OFF;
float         brickRadius;
float         padStepAng;
float         bowlCTRad;
float         bowlPadSpeed = 0.01f;
static float  padStep = 0;
static        GLuint infoBar, statusBar, lifeIcon;
static        GLuint numArr[10], collisionType[2];
static        GLuint gameStatusTexture[4];
static        GLuint emptyNum;
static int    currScoreRange = 1;
const float scoreRange[10] ={0.0,0.100000f,0.050000f,0.033333f,0.025000f,
                       0.020000,0.016666,0.014285,0.012500,0.011111};
int algorithm = BRUTEFORCE;

int **count, **offset;
int *brickIndex;
int brickIndexNum;
int gridSize;
int gridWNum,gridHNum;
float cellW, cellH;
float startY;
int   calTime;

void completedGame(LevelInfo *levelInfo)
{
  /* level 2 */
  if (levelInfo->currentLevel >= COMPLETE_LEVEL &&  levelInfo->gameStatus == STATUS_WON) {
    levelInfo->gameStatus = STATUS_COMPLETE;
  }

  /* level 2 display */
  if (levelInfo->currentLevel >= COMPLETE_LEVEL && levelInfo->gameStatus == STATUS_COMPLETE) {
      /* Game over texture */
      glEnable(GL_TEXTURE_2D);
       glBindTexture (GL_TEXTURE_2D, gameStatusTexture[GAME_COMPLETE_TEXTURE]);
      glBegin (GL_QUADS);
        gltexcoord2f (0.0, 0.0);
        glvertex3f (1.0, 1.5, 0.0);

        gltexcoord2f (1.0, 0.0);
        glvertex3f (3.0, 1.5, 0.0);

        gltexcoord2f (1.0, 1.0);
        glvertex3f (3.0, 2.0, 0.0);

        gltexcoord2f (0.0, 1.0);
        glvertex3f (1.0, 2.0, 0.0);
      glEnd ();
      glDisable(GL_TEXTURE_2D);

  }
}

void levelTwo(LevelInfo *levelInfo)
{
  time_t seconds;
  time(&seconds);
  static unsigned int startTime;
  unsigned int currTime;
  /* level 2 */
  if (levelInfo->currentLevel == 2 &&  levelInfo->gameStatus == STATUS_ZERO) {
    startTime = (unsigned int) seconds;
  levelInfo->gameStatus = STATUS_PRE_START;
  }
  /* set player life to max and reset timer's flag */
  if (levelInfo->currentLevel == 2 && levelInfo->gameStatus == STATUS_PRE_START) {
    currTime = (unsigned int)seconds;
    if (currTime - startTime >= 10) {
      levelInfo->gameStatus = STATUS_READY;
     }
  }

  /* level 2 display */
  if (levelInfo->currentLevel == 2 && levelInfo->gameStatus == STATUS_PRE_START) {
      /* Game over texture */
      glEnable(GL_TEXTURE_2D);
      glBindTexture (GL_TEXTURE_2D, gameStatusTexture[LEVEL2_TEXTURE]);
      glBegin (GL_QUADS);
        glTexCoord2f (0.0, 0.0);
        glVertex3f (1.0, 1.5, 0.0);

        glTexCoord2f (1.0, 0.0);
        glVertex3f (3.0, 1.5, 0.0);

        glTexCoord2f (1.0, 1.0);
        glVertex3f (3.0, 2.0, 0.0);

        glTexCoord2f (0.0, 1.0);
        glVertex3f (1.0, 2.0, 0.0);
      glEnd ();
      glDisable(GL_TEXTURE_2D);
  }
}

void levelOne(LevelInfo *levelInfo)
{
  time_t seconds;
  time(&seconds);
  static unsigned int startTime;
  unsigned int currTime;

  /* level 1 */
  if (levelInfo->currentLevel == LEVELONE && levelInfo->gameStatus == STATUS_ZERO) {
    startTime = (unsigned int) seconds;
    levelInfo->gameStatus = STATUS_PRE_START;
  }
  /* set player life to max and reset timer's flag */
  if (levelInfo->currentLevel == LEVELONE &&  levelInfo->gameStatus == STATUS_PRE_START) {
    currTime = (unsigned int)seconds;
    if (currTime - startTime >= 5) {
      levelInfo->gameStatus = STATUS_READY;
    }
  }

  /* level 1 display */
  if (levelInfo->currentLevel == LEVELONE && levelInfo->gameStatus == STATUS_PRE_START) {
      /* Game over texture */
      glEnable(GL_TEXTURE_2D);
      glBindTexture (GL_TEXTURE_2D, gameStatusTexture[LEVEL1_TEXTURE]);
      glBegin (GL_QUADS);
        glTexCoord2f (0.0, 0.0);
        glVertex3f (1.0, 1.5, 0.0);

        glTexCoord2f (1.0, 0.0);
        glVertex3f (3.0, 1.5, 0.0);

        glTexCoord2f (1.0, 1.0);
        glVertex3f (3.0, 2.0, 0.0);

        glTexCoord2f (0.0, 1.0);
        glVertex3f (1.0, 2.0, 0.0);
      glEnd ();
      glDisable(GL_TEXTURE_2D);
  }
}

void gameState(LevelInfo *levelInfo)
{
  /* Game over */
  if (levelInfo->life <= 0 && levelInfo->gameStatus == STATUS_PLAYING) {
    levelInfo->gameStatus = STATUS_GAMEOVER;
    cleanup();
    init();
  }

  /* Ready to trigger game */
  if (levelInfo->gameStatus == STATUS_READY) {
    levelInfo->life = FULL_LIFE;
    levelInfo->gameStatus = STATUS_PLAYING;
  }

  /* Display Gameover texture */
  if (levelInfo->life <= 0 && levelInfo->gameStatus == STATUS_GAMEOVER) {
    /* Game over texture */
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gameStatusTexture[GAMEOVER_TEXTURE]);
    glBegin (GL_QUADS);
          glTexCoord2f (0.0, 0.0);
          glVertex3f (1.0, 1.5, 0.0);

          glTexCoord2f (1.0, 0.0);
          glVertex3f (3.0, 1.5, 0.0);

          glTexCoord2f (1.0, 1.0);
          glVertex3f (3.0, 2.0, 0.0);

          glTexCoord2f (0.0, 1.0);
          glVertex3f (1.0, 2.0, 0.0);
    glEnd ();
    glDisable(GL_TEXTURE_2D);
  }
  levelOne(levelInfo);
  levelTwo(levelInfo);
  completedGame(levelInfo);
}
/* Ball and Bowl AABB check */
bool bowlPadDetection(AabbBox *ab,Ball *inBall)
{
  if ( inBall->pos.x >= ab->x1 - inBall->ballRadius && inBall->pos.x <= ab->x2 + inBall->ballRadius )
    if ( inBall->pos.y <= ab->y1 - inBall->ballRadius && inBall->pos.y >= ab->y2 - inBall->ballRadius )
      return true;

  if ( inBall->pos.x <= ab->x1 - inBall->ballRadius && inBall->pos.x >= ab->x2 + inBall->ballRadius )
    if ( inBall->pos.y <= ab->y1 - inBall->ballRadius && inBall->pos.y >= ab->y2 - inBall->ballRadius )
      return true;
  return false;
}

/* Level 1 paddle processing */
void levelOnePaddleCollision(Ball *ball)
{
  int result;
  /* normal paddle collision detection */
  if (paddle.x < (paddle.x - PADDLE_XSIZE/2 < 0)) paddle.x = PADDLE_XSIZE/2.0 +0.045;
  if (paddle.x + PADDLE_XSIZE/2 > (levelInfo.lvlBound.x))paddle.x = levelInfo.lvlBound.x - PADDLE_XSIZE/2 - 0.045;

    /* Collide ball with paddle top */
  if (ball->pos.x > paddle.x - PADDLE_XSIZE*0.5f && ball->pos.x < paddle.x + PADDLE_XSIZE*0.5f &&
    ball->pos.y < paddle.y + ball->ballRadius && ball->pos.y > paddle.y)
  {
    /* reflect ball's position */
    ball->pos.y += 2.0 * (paddle.y + ball->ballRadius - ball->pos.y);
    /* reflect ball's velocity */
    ball->ballVel.y = ball->ballVel.y * -1.0f;
    ball->ballVel.y = ball->ballVel.y <= 0 ? 1.4 : ball->ballVel.y;
    /* add paddle's velocity. you may want to tweak this */
    if (paddleVel != 0.0f)
      ball->ballVel.x = (ball->ballVel.x + paddleVel) * 0.5;
  }
    /* collide ball with paddle bottom */
  if (ball->pos.x > paddle.x - PADDLE_XSIZE*0.5f && ball->pos.x < paddle.x + PADDLE_XSIZE*0.5f &&
      ball->pos.y < paddle.y - 0.05 + ball->ballRadius && ball->pos.y > paddle.y - 0.05)
  {
    /* reflect ball's position */
    ball->pos.y -= 2.0 * (paddle.y + ball->ballRadius - ball->pos.y);
    /* reflect ball's velocity */
    ball->ballVel.y = -abs(ball->ballVel.y);
    ball->ballVel.y = ball->ballVel.y <= 0 ? -1 : ball->ballVel.y;
    /* add paddle's velocity. you may want to tweak this */
    if (paddleVel != 0.0f)
      ball->ballVel.x = (ball->ballVel.x + paddleVel) * 0.5;
  }

  /* Collide ball with paddle edge */
  if ((result = sphereIntersectPadEdge(ball,&leftPadEdge, LEFT))) {
    if (result == EDGE_REBOUND) {
      ball->ballVel.x = ball->ballVel.x * -1.0; ball->ballVel.y = ball->ballVel.y * -1.0;
      ball->ballVel.x = ball->ballVel.x <= 0 ? -1 : ball->ballVel.x;
      ball->ballVel.y = ball->ballVel.y <= 0 ?  1 : ball->ballVel.y;
    }
    else if (result == NORMAL_REBOUND) {
      /* reflect ball's position */
      ball->pos.y += 2.0 * (paddle.y + ball->ballRadius - ball->pos.y);
      /* reflect ball's velocity */
      ball->ballVel.y = abs(ball->ballVel.y);
      ball->ballVel.y = ball->ballVel.y <= 0 ? 1 : ball->ballVel.y;
      /* add paddle's velocity. you may want to tweak this */
      if (paddleVel != 0.0f)
        ball->ballVel.x = (ball->ballVel.x + paddleVel) * 0.5;
    }
  }
  if ((result = sphereIntersectPadEdge(ball,&rightPadEdge, RIGHT))) {
    if (result == EDGE_REBOUND) {
      ball->ballVel.x = ball->ballVel.x * -1.0; ball->ballVel.y = ball->ballVel.y * -1.0;
      ball->ballVel.x = ball->ballVel.x <= 0 ? 1 : ball->ballVel.x;
      ball->ballVel.y = ball->ballVel.y <= 0 ? 1 : ball->ballVel.y;
    }
    else if (result == NORMAL_REBOUND) {
      /* reflect ball's position */
      ball->pos.y += 2.0 * (paddle.y + ball->ballRadius - ball->pos.y);
      /* reflect ball's velocity */
      ball->ballVel.y = abs(ball->ballVel.y);
      ball->ballVel.y = ball->ballVel.y <= 0 ? 1 : ball->ballVel.y;
      /* add paddle's velocity. you may want to tweak this */
      if (paddleVel != 0.0f)
        ball->ballVel.x = (ball->ballVel.x + paddleVel) * 0.5;
    }
  }
}

void levelTwoBowlPaddle(Ball *ball)
{
  float cornerLDist, cornerRDist;
  vec2f bowlLoc;
  /* Update bowlPad AABB box */
  if (padStepAng >= MIN_PAD_ANGLE && padStepAng <= 0) {
    bowlPadAABB.x1 =  m[12] + 0.3 * cos(( 210.0 +padStepAng)/180.0*M_PI);
    bowlPadAABB.y1 =  m[13] + 0.3 * sin(( 210.0 +padStepAng)/180.0*M_PI);
    bowlPadAABB.x2 =  m[12] + 0.3 * cos(((210.0 +padStepAng)/180.0*M_PI) +
                      (1.0/9.0*M_PI*6.0));
    if ( padStepAng >= -35)
      bowlPadAABB.y2 =  m[13] + 0.3 * sin(((210.0+padStepAng)/180.0*M_PI) +
                        (1.0/9.0*M_PI*3.0));
    else
      bowlPadAABB.y2 =  m[13] + 0.3 * sin(((210.0+padStepAng)/180.0*M_PI) +
                        (1.0/9.0*M_PI*6.0));
  }

  if (padStepAng <= MAX_PAD_ANGLE && padStepAng >= 0) {
    bowlPadAABB.x1 =  m[12] + 0.3 * cos(((210.0  + padStepAng)/180.0*M_PI)+(1.0/9.0*M_PI*6.0));
    bowlPadAABB.y1 =  m[13] + 0.3 * sin(((210.0  + padStepAng)/180.0*M_PI)+(1.0/9.0*M_PI*6.0));
    bowlPadAABB.x2 =  m[12] + 0.3 * cos(( 210.0  + padStepAng)/180.0*M_PI);
    bowlPadAABB.y2 =  m[13] + 0.3 * sin(((210.0  + padStepAng)/180.0*M_PI)+(1.0/9.0*M_PI*3.0));
    if ( padStepAng <= 35)
      bowlPadAABB.y2 =  m[13] + 0.3 * sin(((210.0  + padStepAng)/180.0*M_PI)+(1.0/9.0*M_PI*3.0));
    else
      bowlPadAABB.y2 =  m[13] + 0.6 * sin(((210.0  + padStepAng)/180.0*M_PI)+(1.0/9.0*M_PI*3.0));
  }

  /* Collision detection for bowlpaddle */
  bowlLCorner.x = m[12] + 0.3 * cos((210+padStepAng)/180.0*M_PI);
  bowlLCorner.y = m[13] + 0.3 * sin((210+padStepAng)/180.0*M_PI);
  bowlLCorner.x = ball->pos.x - bowlLCorner.x;
  bowlLCorner.y = ball->pos.y - bowlLCorner.y ;
  cornerLDist = dotProduct((bowlLCorner),(bowlLCorner));

  /* bowl Left corner detection */
  if ( (cornerLDist < ((ball->ballRadius + 0.027) * (ball->ballRadius + 0.027)))) {
    ball->ballVel.x = ball->ballVel.x * -1.0f; ball->ballVel.y = ball->ballVel.y * -1.0f;
    ball->ballVel.x = ball->ballVel.x <= 0 ? -1.4 : ball->ballVel.x;
    ball->ballVel.y = ball->ballVel.y <= 0 ?  1.4 : ball->ballVel.y;
  }
  /* bowl right corner detection */
  bowlRCorner.x = m[12] + 0.3 * cos(((210.0+padStepAng)/180.0*M_PI)+(1.0/9.0*M_PI*6));
  bowlRCorner.y = m[13] + 0.3 * sin(((210.0+padStepAng)/180.0*M_PI)+(1.0/9.0*M_PI*6));
  bowlRCorner.x = ball->pos.x - bowlRCorner.x;
  bowlRCorner.y = ball->pos.y - bowlRCorner.y ;
  cornerRDist = dotProduct((bowlRCorner),(bowlRCorner));

  if ( (cornerRDist < ((ball->ballRadius + 0.027) * (ball->ballRadius + 0.027)))) {
    ball->ballVel.x = ball->ballVel.x * -1.0f;   ball->ballVel.y = ball->ballVel.y * -1.0f;
    ball->ballVel.x = ball->ballVel.x <= 0 ? -1.4 : ball->ballVel.x;
    ball->ballVel.y = ball->ballVel.y <= 0 ?  1.4 : ball->ballVel.y;
  }

  if (bowlPadDetection(&bowlPadAABB,ball) == true) {
    float intersectRad, vp;
    vec2f P, ballInCir;
    bowlLoc.x    = m[12]; bowlLoc.y = m[13];
    ballInCir.x  = fabs(ball->pos.x) - fabs(m[12]);
    ballInCir.y  = fabs(ball->pos.y) - fabs(m[13]);
    intersectRad = vecLength(&ballInCir);

    if (fabs((intersectRad)) >= bowlCTRad) {
      P.x = bowlLoc.x - ball->pos.x;
      P.y = bowlLoc.y - ball->pos.y;
      normalize (&P,1);
      vp = dotProduct(ball->ballVel,P);
      ball->ballVel.x -= 2 * vp * P.x;
      ball->ballVel.y -= 2 * vp * P.y;
      ball->pos.x += ball->ballVel.x * 0.05;
      ball->pos.y += ball->ballVel.y * 0.05;
    }
  }
}

/* generate random edge number in range of (3 to 8, or 32) */
void generateEdgeNum(brickShape *brick, int numBricks)
{
    int i;
    time_t seconds;
    time(&seconds);
    int r;

    srand((unsigned int) seconds);

    for (i = 0; i < numBricks; i++)
    {
        r = rand() % 7;
        if (r == 6)
            brick[i].s.edgeNum = 32;
        else
            brick[i].s.edgeNum = r + 3;
    }
}

/* generate rot angle - 8 options */
void generateRotAngle(brickShape *brick, int numBricks)
{
    int i;
    time_t seconds;
    time(&seconds);
    int r;

    srand((unsigned int) seconds);

    for (i = 0; i < numBricks; i++)
    {
        r = rand() % 8 ;
        brick[i].s.rotAngle = 2 * M_PI * ( r * 45) / 360;

    }
}

void generateColor(brickShape *brick, int numBricks)
{
    int i;
    time_t seconds;
    time(&seconds);
    int r;

    srand((unsigned int) seconds);

    for (i = 0; i < numBricks; i++)
    {
        r = rand() % 9 ;
        brick[i].s.color = r;

    }
}

void drawAxes()
{
  glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT);
  glLineWidth(1.0);
  glBegin(GL_LINES);
  glColor3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(2, 0, 0);
  glColor3f(0, 1, 0); glVertex3f(0, 0, 0); glVertex3f(0, 2, 0);
  glColor3f(0, 0, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, 2);
  glEnd();
  glPopAttrib();
}

void createAABB(brickShape *bric, AabbBox *ab )
{
  int i;
  float radius = BRICK_RADIUS;
  float currentX, currentY;
  float rad = bric->s.rotAngle;
  ab->x1 = radius * cos(rad);
  ab->y1 = radius * sin(rad);
  ab->x2 = radius * cos(rad);
  ab->y2 = radius * sin(rad);
  for (i = 1; i < bric->s.edgeNum; ++i)
  {
      rad = 2*i*M_PI/ bric->s.edgeNum + bric->s.rotAngle;
      currentX = radius * cos(rad);
      currentY = radius * sin(rad);
      if ( currentX < ab->x1 )
          ab->x1 = currentX;
      if ( currentX > ab->x2 )
          ab->x2 = currentX;
      if ( currentY < ab->y1 )
          ab->y1 = currentY;
      if ( currentY > ab->y2 )
          ab->y2 = currentY;
  }
  /*store the absolute position of aabb */
  ab->x1 += bric->pos.x;
  ab->y1 += bric->pos.y;
  ab->x2 += bric->pos.x;
  ab->y2 += bric->pos.y;

}

/* Call this funciton only you have detected a collision
 * This function tries to get normal which will be used to calculate reflection velocity.
 * Normal will be output by v.
 */
void getCollisionNormal(int num, Ball *ball, vec2f *v)
{
  vec2f midPoint[8];
  vec2f conner[8];
  vec2f normal;
  float lenNormal;
  float len[8];
  float min, x0, y0, x1, y1;
  float rad, step;
  int edge1, edge2;
  int i;
  float totalR;

  if ( brick[num].s.edgeNum != 32 )
  {
  /* non-circle .  Notes: think 32 as a circle*/
  /* In this game, the length of all edges for one brick equals.
         * Therefore, get the middle point position of the each edge
         * and find out which point to the ball has the shortest path - the collision edge.
         * Special case: If there exists two shorstest paths, the hit point is the corner.
         */
    rad = brick[num].s.rotAngle;
    step = 2*M_PI/brick[num].s.edgeNum;

    for (i = 0; i < brick[num].s.edgeNum; ++i)
    {
      x0 = brickRadius * cos(rad);
      y0 = brickRadius * sin(rad);
      conner[i].x = x0;
      conner[i].y = y0;
      conner[i].x += brick[num].pos.x;
      conner[i].y += brick[num].pos.y;
      rad += step;
      x1 = brickRadius * cos(rad);
      y1 = brickRadius * sin(rad);
      x1 += brick[num].pos.x;
      y1 += brick[num].pos.y;
      midPoint[i].x = (conner[i].x+x1)/2.0;
      midPoint[i].y = (conner[i].y+y1)/2.0;

    }

    for (i = 0; i < brick[num].s.edgeNum; ++i)
    {
      len[i] = sqrt(pow((midPoint[i].x - ball->pos.x), 2)+pow((midPoint[i].y - ball->pos.y), 2));
    }
    min = len[0];
    edge1 = 0;
    edge2 = -1;
    for ( i = 1; i < brick[num].s.edgeNum; i++)
    {
      if (len[i] < min && fabs(min - len[i]) > 0.0001)
      {
        min = len[i];
        edge1 = i;
        edge2 = -1;
      }
      else if (fabs(min - len[i]) < 0.0001)
      {
        edge2 = i;
      }
    }
    if (edge2 == -1)
    {
      /* Get normal for hitting one edge */
      if (edge1 == brick[num].s.edgeNum - 1 )
      {
        lenNormal = sqrt(pow((conner[edge1].x - conner[0].x ), 2) +
                    pow((conner[edge1].y - conner[0].y), 2));
        normal.x = (conner[edge1].x - conner[0].x) / lenNormal;
        normal.y = (conner[edge1].y - conner[0].y) / lenNormal;
      }
      else
      {
        lenNormal = sqrt(pow((conner[edge1+1].x - conner[edge1].x ), 2) +
                    pow((conner[edge1+1].y - conner[edge1].y), 2));
        normal.x = (conner[edge1+1].x - conner[edge1].x) / lenNormal;
        normal.y = (conner[edge1+1].y - conner[edge1].y) / lenNormal;
      }
      v->x = -normal.y;
      v->y = normal.x;
    }
    else
    {
      /* Get normal for ball hitting on the conner of the brick */
      lenNormal = sqrt(pow((ball->pos.x - brick[num].pos.x ), 2) +
                  pow((ball->pos.y - brick[num].pos.y), 2));
      v->x = (ball->pos.x - brick[num].pos.x)/lenNormal;
      v->y = (ball->pos.y - brick[num].pos.y)/lenNormal;
    }
  }
  else
  {
    /* Get normal for hitting circle */
    totalR = brickRadius + ball->ballRadius;
    v->x = (ball->pos.x - brick[num].pos.x)/totalR;
    v->y = (ball->pos.y - brick[num].pos.y)/totalR;
  }
}

/* Detect if the ball hit the current brick
 */
int intersection(AabbBox *ab, brickShape *b, Ball *ball)
{
  float totalR;
  if (b->show == ON)
  {
    if ( b->s.edgeNum != 32)
    {
      /* non-circle .  Notes: think 32 as a circle*/
      if ( ball->pos.x >= ab->x1 - ball->ballRadius && ball->pos.x <= ab->x2 + ball->ballRadius )
      {
        if ( ball->pos.y >= ab->y1 - ball->ballRadius && ball->pos.y <= ab->y2 + ball->ballRadius )
          return true;
      }
    }
    else
    {
      /* circle */
      totalR = brickRadius + ball->ballRadius;
      if ( sqrt(pow((b->pos.x - ball->pos.x ), 2) + pow((b->pos.y - ball->pos.y), 2)) <= totalR)
        return true;
    }
  }
  return false;
}

/* Bruteforce collision detection
   find out which brick was collided
   */
int bruteforceDetection(AabbBox *ab, Ball *ball)
{
  int i;

  for ( i = 0; i < numBricks; ++i)
  {
    if (intersection(&ab[i], &brick[i], ball) == true)
    {
      brick[i].show = OFF;
      return i;
    }
  }
  return -1;
}

int existsIndex(int posI, int posJ, int index[4][2], int cellUsedCount)
{
  int i;
  for (i = 0; i < cellUsedCount; i++)
  {
    if (posI == index[i][0] && posJ == index[i][1])
      return true;
  }

  return false;
}

/*
 * Based on the brick current radius and ball's radius,
 * the input aabb box can only occupies maxmium 4 cells.
 */
void calcGridIndex(AabbBox *ab, int index[4][2], int *cellUsedCount)
{
  int posI = (ab->y1 - startY)/cellH;
  int posJ = ab->x1/cellW;
  index[0][0] = posI;
  index[0][1] = posJ;
  *cellUsedCount = 1;

  posI = (ab->y2 - startY)/cellH;
  posJ = ab->x1/cellW;
  if (existsIndex(posI, posJ, index, *cellUsedCount) == false)
  {
    index[*cellUsedCount][0] = posI;
    index[*cellUsedCount][1] = posJ;
    (*cellUsedCount)++;
  }

  posI = (ab->y2 - startY)/cellH;
  posJ = ab->x2/cellW;
  if (existsIndex(posI, posJ, index, *cellUsedCount) == false)
  {
    index[*cellUsedCount][0] = posI;
    index[*cellUsedCount][1] = posJ;
    (*cellUsedCount)++;
  }

  posI = (ab->y1 - startY)/cellH;
  posJ = ab->x2/cellW;
  if (existsIndex(posI, posJ, index, *cellUsedCount) == false)
  {
    index[*cellUsedCount][0] = posI;
    index[*cellUsedCount][1] = posJ;
    (*cellUsedCount)++;
  }
}

void initUniformGrid()
{
    int i, j, k;
    int posI, posJ, pos;
    int index[4][2];
    int cellUsedCount;
  /*int brickIndexNum;*/

    startY = paddle.y;
    /* Work out grid dimensions */
    gridWNum = (int)(sqrt(numBricks) + 1);

    cellW = ( levelInfo.lvlBound.x ) / gridWNum;
    cellH = cellW;
    gridHNum = ceil((levelInfo.lvlBound.y - startY)/cellH);
    gridSize = gridWNum * gridHNum;

    /* Allocate arrays. */
    if ((count = (int**)malloc(gridHNum * sizeof(int*))) == NULL)
    {
        printf("Cannot allocate memory for count array\n");
        exit(1);
    }
    if ((offset = (int**)malloc(gridHNum * sizeof(int*))) == NULL)
    {
        printf("Cannot allocate memory for offset array\n");
        exit(1);
    }

    for ( i = 0; i < gridHNum; i++)
    {
        if ((count[i] = (int*)malloc(gridWNum * sizeof(int))) == NULL)
        {
            printf("Cannot allocate memory for count array\n");
            exit(1);
        }
        if ((offset[i] = (int*)malloc(gridWNum * sizeof(int))) == NULL)
        {
            printf("Cannot allocate memory for count array\n");
            exit(1);
        }
    }

  /* initiate count and offset array */
    for ( i = 0; i < gridHNum; i++)
        for ( j = 0; j < gridWNum; j++)
        {
            count[i][j] = 0;
            offset[i][j] = 0;
        }

    for ( i = 0; i < numBricks; i++)
    {
        cellUsedCount = 0;
        calcGridIndex(&aabb[i], index, &cellUsedCount);
        for (j = 0; j < cellUsedCount; j++)
        {
          posI = index[j][0];
          posJ = index[j][1];
          count[posI][posJ] += 1;
        }
    }

    for ( i = 0; i < gridHNum; i++)
    {
      for (j = 0; j < gridWNum; j++)
      {
        if (i == 0)
        {
          if (j == 0)
            offset[i][j] = 0;
        }
        else
        {
          if ( j == 0 )
            offset[i][j] = offset[i-1][gridWNum-1] + count[i-1][gridWNum-1];
          else
            offset[i][j] = offset[i][j-1] + count[i][j-1];
        }
      }
    }

  /* create array brickIndex */
  brickIndexNum = offset[gridHNum-1][gridWNum-1] + count[gridHNum-1][gridWNum-1];

  if ((brickIndex =(int*)malloc(brickIndexNum * sizeof(int))) == NULL )
  {
    printf("Cannot allocate memory for count array\n");
    exit(1);
  }

  /* initiate the array brickIndex */
  for ( i = 0; i < brickIndexNum; i++ )
    brickIndex[i] = -1;

  for ( i = 0; i < numBricks; i++)
  {
    cellUsedCount = 0;
    calcGridIndex(&aabb[i], index, &cellUsedCount);
    for (j = 0; j < cellUsedCount; j++)
    {
      posI = index[j][0];
      posJ = index[j][1];
      pos = offset[posI][posJ];

      for ( k = 0; k < count[posI][posJ]; k++ )
      {
        if (brickIndex[pos] == -1)
        {
          brickIndex[pos] = i;
          break;
        }
        pos++;
      }
    }
  }
}

/* UniformGrid detection
   find out which brick was collided
   */
int uniformGridDetection(AabbBox *ab, Ball *ball)
{
  AabbBox ballAABB;
  int index[4][2];
  int cellUsedCount;
  int i, j;
  int posI, posJ, pos;
  int brickNum;

  /* get the ball's AABB box */
  ballAABB.x1 = ball->pos.x - ball->ballRadius;
  ballAABB.y1 = ball->pos.y - ball->ballRadius;
  ballAABB.x2 = ball->pos.x + ball->ballRadius;
  ballAABB.y2 = ball->pos.y + ball->ballRadius;

  /* get the cell(s) which the ball is in */
  calcGridIndex(&ballAABB, index, &cellUsedCount);

  /* check the cell(s) which the ball shares with any AABB boxes */
  for (i = 0; i < cellUsedCount; i++)
  {
    posI = index[i][0];
    posJ = index[i][1];
    /* the ball out of range, just igonore the case */
    if ( posI > gridHNum - 1 || posJ > gridWNum - 1 )
      continue;

    pos = offset[posI][posJ];
    if ( count[posI][posJ] > 0)
    {
      for (j = 0; j < count[posI][posJ]; j++)
      {
        brickNum = brickIndex[pos];
        if ( intersection(&ab[brickNum], &brick[brickNum], ball) )
        {
          brick[brickNum].show = OFF;
          return brickNum;
        }
        pos++;
      }
    }
  }
  return -1;
}

void initialiseBrickDisplay()
{
  int i, numOfEnable;
  float maxR = levelInfo.radius;

  /** If it is a round field, hide all bricks out of range*/
  if (levelInfo.currentLevel == 2)
  {
    numOfEnable = 0;
    for (i = 0; i < numBricks; i++)
    {
      if (sqrt(pow((levelInfo.center.x - brick[i].pos.x), 2) + pow((levelInfo.center.y - brick[i].pos.y), 2)) > maxR)
        brick[i].show = OFF;

      if (brick[i].show  == ON)
        numOfEnable++;
    }
    levelInfo.totalBrick = numOfEnable;
  }
}

void init()
{
  int i;
  char image[15];
  vec2f levelBounds;
  float step    = 0.0;

  glEnable(GL_TEXTURE_2D);
  infoBar   = texture_load("image/score.jpg");
  statusBar = texture_load("image/status_bar.jpg");
  lifeIcon  = texture_load("image/life.jpg");
  collisionType[0] = texture_load("image/bruteforce.jpg");
  collisionType[1] = texture_load("image/uniform.jpg");
  gameStatusTexture[GAMEOVER_TEXTURE] = texture_load("image/game_over.jpg");
  gameStatusTexture[LEVEL2_TEXTURE] = texture_load("image/level2.jpg");
  gameStatusTexture[LEVEL1_TEXTURE] = texture_load("image/level1.jpg");
  gameStatusTexture[GAME_COMPLETE_TEXTURE] = texture_load("image/complete.jpg");
  glBindTexture(GL_TEXTURE_2D, infoBar);

  /* Load score image 0 to 9 */
  for(i = 0; i< 10; i++) {
    memset(image,'\0',16);
    sprintf(image,"%s%d%s","image/",i,".jpg");
    numArr[i] = texture_load(image);
  }
  emptyNum = texture_load("image/empty.jpg");

  /* Init level data */
  levelInfo.currentLevel = 1;
  levelInfo.gameStatus   = STATUS_ZERO;  /* initialize game status */
  levelInfo.totalBrick   = numBricks;
  levelInfo.center.x     = 2.0;
  levelInfo.center.y     = 1.5;
  levelInfo.radius       = 1.5;
  levelInfo.numPoints    = 64;
  levelInfo.lvlBound.x   = 4;
  levelInfo.lvlBound.y   = 3;
  levelInfo.life         = FULL_LIFE;
  levelBounds = levelInfo.lvlBound;
  score                  = 0;
  numOfBall              = 1;

  /* Init ball data */
  for (i =0; i < numOfBall; i++) {
    ballArr[i].pos.x      = levelBounds.x * 0.5f + i % 4;
    ballArr[i].pos.y      = 1.0f;
    ballArr[i].ballVel.x  = 0.0f;
    ballArr[i].ballVel.y  = -1.0f;
    ballArr[i].ballRadius = BALL_RADIUS;
  }

  brickRadius = BRICK_RADIUS;

  /* get the position of the bricks */
  brick[0].show = ON;
  brick[0].pos.x = levelBounds.x * 0.1f;
  brick[0].pos.y = 1.5f;

  step = (4.0-2*brick[0].pos.x)/6;
  for (i = 1; i < numBricks; i++)
  {
      brick[i].show = ON;
      if ((i % 7) == 0)
      {
          brick[i].pos.x = brick[i-7].pos.x;
        brick[i].pos.y = brick[i-7].pos.y + 0.4f;
      }
      else
      {
          brick[i].pos.x = brick[i-1].pos.x + step;
        brick[i].pos.y = brick[i-1].pos.y;
      }
  }
  generateEdgeNum(brick, numBricks);
  generateRotAngle(brick, numBricks);
  generateColor(brick, numBricks);

  initialiseBrickDisplay();

  for (i = 0; i < numBricks; i++)
      createAABB(&brick[i], &aabb[i]);

  initUniformGrid();

  paddle.x = levelBounds.x * 0.5f;
  paddle.y = 0.5f;
}

void reshape(int w, int h)
{
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, levelInfo.lvlBound.x, 0, levelInfo.lvlBound.y, -1, 1);
  glMatrixMode(GL_MODELVIEW);
}

void update(int milliseconds)
{
  int nowTime;
  float elapsed_time;
  static int lastTime;
  int i = 0;
  int collisionBrick;
  Ball *ball;
  float dt = (float)milliseconds * 0.001f;
  /* move the ball */
  for (i = 0; i < numOfBall; i++)
  {
    ball = &ballArr[i];
    ball->pos.x += ball->ballVel.x * dt;
    ball->pos.y += ball->ballVel.y * dt;

    if ( levelInfo.gameStatus < STATUS_READY )
    {
      ball->ballVel.x = 0;
      if ( ball->pos.y > 1.2 )
      {
        ball->pos.y = 1.18;
        ball->ballVel.y = -ball->ballVel.y;
      }
    }

    /* 'level 2 bowl play area'. Radius for paddle to do collision*/
      bowlCTRad = 0.3 - (ball->ballRadius*2);

    /* Collision Dection */
    if ( algorithm == BRUTEFORCE)
    {
      nowTime  = glutGet(GLUT_ELAPSED_TIME);
      collisionBrick = bruteforceDetection(aabb, ball);
      elapsed_time = (nowTime - lastTime);
      lastTime = nowTime;
      calTime += elapsed_time;
      if (calTime >= 1000 && displayCTT == true)
      {
        calTime = 0;
        printf("bruteforce detection time:%.3f\n",elapsed_time * 0.001);
      }
    }
    else
    {
      nowTime  = glutGet(GLUT_ELAPSED_TIME);
      collisionBrick = uniformGridDetection(aabb, ball);
      elapsed_time = (nowTime - lastTime);
      lastTime = nowTime;
      calTime += elapsed_time;
      if (calTime >= 1000 && displayCTT == true)
      {
        calTime = 0;
        printf("uniform-grid detection time:%.3f\n",elapsed_time * 0.001);
      }
    }
    if (collisionBrick >= 0)
    {
      vec2f v;
      getCollisionNormal(collisionBrick, ball, &v);
      float x0 = v.x, y0 = v.y;
      float curX = ball->ballVel.x, curY = ball->ballVel.y;

      ball->ballVel.x = curX - 2 * (curX * x0 + curY * y0) * x0;
      ball->ballVel.y = curY - 2 * (curX * x0 + curY * y0) * y0;
      if ( levelInfo.gameStatus == STATUS_PLAYING )
        score++;
    }

    /*level 1 normalPaddle collision detection and squre  Wall*/
    if (levelInfo.currentLevel == 1) {
      levelOneWallCollision(ball,&levelInfo,&paddle);
      levelOnePaddleCollision(ball);
    }
    /* level 2 play area, bounce ball off circlura sides */
    else if (levelInfo.currentLevel == 2) {
      levelTwoBowlPaddle(ball);
      circlePlyAreaCT(ball,&levelInfo,&padStepAng);
    }

    /* Check if play have completed current level */
    if (score >= levelInfo.totalBrick)
    {
      if (levelInfo.currentLevel < COMPLETE_LEVEL)
      {
        /* Get into the next level */
        levelInfo.currentLevel++;

        /* Reset game status for the next level */
        levelInfo.gameStatus = STATUS_ZERO;
        score = 0;

        for (i = 1; i < numBricks; i++)
          brick[i].show = ON;

        //hide bricks out of range
        initialiseBrickDisplay();

        //initilise ball(s)
        for (i =0; i < numOfBall; i++) {
          ballArr[i].pos.x      = levelInfo.lvlBound.x * 0.5f + i % 4;
          ballArr[i].pos.y      = 1.0f;
          ballArr[i].ballVel.x  = 0.0f;
          ballArr[i].ballVel.y  = -1.0f;
        }
      }
      else
      {
        /* You won the game */
        levelInfo.gameStatus = STATUS_WON;
      }
    }
  }

  /* move paddle if game starts */
  if ( levelInfo.gameStatus >= STATUS_READY && levelInfo.gameStatus != STATUS_GAMEOVER )
  {
    if (keys[SDLK_LEFT])
    {
      paddleVel = -PADDLE_SPEED;
      if (levelInfo.currentLevel == 1)
        padStep -= padStep > -8.69997 ? 0.1: 0;
      else
        padStepAng -= padStepAng >= MIN_PAD_ANGLE ? bowlPadSpeed: 0;
    }
    else if (keys[SDLK_RIGHT])
    {
      paddleVel   = PADDLE_SPEED;
      if (levelInfo.currentLevel == 1)
        padStep    += padStep < 8.69997 ? 0.1: 0.0;
      else
        padStepAng += padStepAng <= MAX_PAD_ANGLE ? bowlPadSpeed: 0.0;
    }
    else
      paddleVel = 0.0f;
    paddle.x += paddleVel * dt;
  }
}

void drawRegularPolygon(float radius, int subs)
{
  /* draws a solid polygon with same length sides */
  int i;
  glBegin(GL_TRIANGLE_FAN);
  glVertex2f(0.0f, 0.0f);
  for (i = 0; i <= subs; ++i)
    glVertex2f(radius * cos(2*i*M_PI/subs), radius * sin(2*i*M_PI/subs));
  glEnd();

    if (displayAABB == ON)
    {
      glBegin(GL_LINE_LOOP);
      glVertex2f(-radius, -radius);
      glVertex2f(-radius, radius);
      glVertex2f(radius, radius);
      glVertex2f(radius, -radius);
      glEnd();
    }
}

/* Draw bowlpaddle */
void drawHalfRegularPolygon(float radius)
{
  int i;
  float d = ((float)(7.0/6.0)*M_PI);
  float step = (1.0f/9.0f *M_PI);

  glBegin(GL_LINE_STRIP);
    for (i = 0; i < 7; ++i)
    {
      bowlPadVec[i].x = radius * cos(d);
      bowlPadVec[i].y = radius * sin(d);
      glVertex2f(bowlPadVec[i].x, bowlPadVec[i].y);
      d = d + step;
    }
    d = d - step;
    radius -=0.02;

    for (i = 7; i > 0; --i)
    {
      glVertex2f(radius * cos(d), radius * sin(d));
      d = d - step;
    }
    d = d + step;
    radius =0.3;
    glVertex2f(radius * cos(d), radius * sin(d));
  glEnd();

  if (displayAABB == ON)
  {
    glBegin(GL_LINE_LOOP);
    glVertex2f(-radius, -radius);
    glVertex2f(-radius, radius);
    glVertex2f(radius,  radius);
    glVertex2f(radius, -radius);
    glEnd();
  }
}

void drawBricks(float radius, int num)
{
  int i;
    int subs = brick[num].s.edgeNum;
    float rad;

  switch (brick[num].s.color){
    case 0: glColor3f(205.0/255.0, 140.0/255.0, 149.0/255.0);
      break;
    case 1: glColor3f(238.0/255.0, 201.0/255.0, 0/255.0);
      break;
    case 2: glColor3f(113.0/255.0, 198.0/255.0, 113.0/255.0);
      break;
    case 3: glColor3f(139.0/255.0, 58.0/255.0, 58.0/255.0);
      break;
    case 4: glColor3f(113.0/255.0, 198.0/255.0, 198.0/255.0);
      break;
    case 5: glColor3f(105.0/255.0, 89.0/255.0, 205.0/255.0);
      break;
    case 6: glColor3f(100.0/255.0, 147.0/255.0, 237.0/255.0);
      break;
    case 7: glColor3f(0.0/255.0, 206.0/255.0, 209.0/255.0);
      break;
  }

  glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0f, 0.0f);

    for (i = 0; i <= subs; ++i)
    {
        rad = 2*i*M_PI/subs + brick[num].s.rotAngle;
      glVertex2f(radius * cos(rad), radius * sin(rad));
    }
  glEnd();

  /* draw AABB */
  if (displayAABB == ON)
  {
    glBegin(GL_LINE_LOOP);
    glVertex2f(aabb[num].x1 - brick[num].pos.x, aabb[num].y1 - brick[num].pos.y);
    glVertex2f(aabb[num].x1 - brick[num].pos.x, aabb[num].y2 - brick[num].pos.y);
    glVertex2f(aabb[num].x2 - brick[num].pos.x, aabb[num].y2 - brick[num].pos.y);
    glVertex2f(aabb[num].x2 - brick[num].pos.x, aabb[num].y1 - brick[num].pos.y);
    glEnd();
  }
}

void updateScore()
{
  int i;
  char result[6];
  float temp;

 glEnable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, statusBar);
    /* Left status bar */
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin (GL_QUADS);
    glTexCoord2f (0.0, 0.0);
    glVertex3f (0.0, 0.0, 0.0);

    glTexCoord2f (1.0, 0.0);
    glVertex3f (0.85, 0.0, 0.0);

    glTexCoord2f (1.0, 1.0);
    glVertex3f (0.85, 0.42, 0.0);

    glTexCoord2f (0.0, 1.0);
    glVertex3f (0.0, 0.42, 0.0);
    glEnd ();

     /* Right status bar */
    glBegin (GL_QUADS);
    glTexCoord2f (0.0, 0.0);
    glVertex3f (3.15, 0.0, 0.0);

    glTexCoord2f (1.0, 0.0);
    glVertex3f (4.0, 0.0, 0.0);

    glTexCoord2f (1.0, 1.0);
    glVertex3f (4.0, 0.42, 0.0);

    glTexCoord2f (0.0, 1.0);
    glVertex3f (3.15, 0.42, 0.0);
  glEnd ();
  /* life icon:, level one update info from levelOneWallCollision() function */
  for(i = 0; i <levelInfo.life; i++)
  {
    glBindTexture(GL_TEXTURE_2D, lifeIcon);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin (GL_QUADS);
      glTexCoord2f (0.0, 0.0);
      glVertex3f (0.05 + (LIFE_OFFSET * i), 0.25, 0.0);

      glTexCoord2f (1.0, 0.0);
      glVertex3f (0.2 + (LIFE_OFFSET * i), 0.25, 0.0);

      glTexCoord2f (1.0, 1.0);
      glVertex3f (0.2 + (LIFE_OFFSET * i), 0.35, 0.0);

      glTexCoord2f (0.0, 1.0);
      glVertex3f (0.05 + (LIFE_OFFSET * i), 0.35, 0.0);
    glEnd ();
 }

  /* update score 10th digits */
  if( score < 10 ) {
    i = score;
  }
  else {
    temp = atof(result);
    for ( i = 0; i < 10; i++)
    {
      sprintf(result,"%3f",(1.0/score));
      temp = atof(result);
      if(temp == scoreRange[i]) {
        currScoreRange = i;
        break;
      }
    }
    i = currScoreRange;
  }
  glBindTexture(GL_TEXTURE_2D, numArr[i]);
  /* Display current score */
  glColor3f(1.0f, 1.0f, 1.0f);

  glBegin (GL_QUADS);
    glTexCoord2f (0.0, 0.0);
    glVertex3f (0.45, 0.02, 0.0);

    glTexCoord2f (1.0, 0.0);
    glVertex3f (0.6, 0.02, 0.0);

    glTexCoord2f (1.0, 1.0);
    glVertex3f (0.6, 0.23, 0.0);

    glTexCoord2f (0.0, 1.0);
    glVertex3f (0.45, 0.230, 0.0);
  glEnd ();

  /* Updates score 1th digits */
  if (score < 10) {
    glBindTexture(GL_TEXTURE_2D, emptyNum);
  }
  else {
    if ( score > prevScore) {
      prevScore = score;
      ++currJ;
      if (currJ == 10)
        currJ = 0;
    }
    glBindTexture(GL_TEXTURE_2D, numArr[currJ]);
  }

  /* display current score */
  glColor3f(1.0f, 1.0f, 1.0f);
  glBegin (GL_QUADS);
    glTexCoord2f (0.0, 0.0);
    glVertex3f (0.615, 0.02, 0.0);

    glTexCoord2f (1.0, 0.0);
    glVertex3f (0.73, 0.02, 0.0);

    glTexCoord2f (1.0, 1.0);
    glVertex3f (0.73, 0.23, 0.0);

    glTexCoord2f (0.0, 1.0);
    glVertex3f (0.615, 0.230, 0.0);
  glEnd ();

  /* Right status bar Collision method*/
  if (algorithm == BRUTEFORCE)
    glBindTexture(GL_TEXTURE_2D, collisionType[BRUTEFORCE]);
  else
     glBindTexture(GL_TEXTURE_2D, collisionType[UNIFORMGRID]);

  glBegin (GL_QUADS);
    glTexCoord2f (0.0, 0.0);
    glVertex3f (3.19, 0.025, 0.0);

    glTexCoord2f (1.0, 0.0);
    glVertex3f (3.98, 0.025, 0.0);

    glTexCoord2f (1.0, 1.0);
    glVertex3f (3.98, 0.3, 0.0);

    glTexCoord2f (0.0, 1.0);
    glVertex3f (3.19, 0.3, 0.0);
  glEnd ();


   glDisable(GL_TEXTURE_2D);
}

void display(SDL_Surface *screen)
{
  int i;
  glClear(GL_COLOR_BUFFER_BIT);
  glLoadIdentity();

  glBegin(GL_QUADS);
    glColor3f(0.8f, 0.8f, 0.8f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(levelInfo.lvlBound.x, 0.0f);
    glColor3f(0.2f, 0.2f, 0.2f);
    glVertex2f(levelInfo.lvlBound.x, levelInfo.lvlBound.y);
    glVertex2f(0.0f, levelInfo.lvlBound.y);
  glEnd();

  if (displayAxes == ON)
    drawAxes();

  /* level 1 draw backdrop */
  if (levelInfo.currentLevel == 1)
  {
    /* Draw level 1 paddle */
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
      glVertex2f(paddle.x - PADDLE_XSIZE*0.5f, paddle.y);
      glVertex2f(paddle.x + PADDLE_XSIZE*0.5f, paddle.y);
      glVertex2f(paddle.x + PADDLE_XSIZE*0.5f, paddle.y - 0.05);
      glVertex2f(paddle.x - PADDLE_XSIZE*0.5f, paddle.y - 0.05);
    glEnd();

    leftPadEdge.x  = paddle.x - PADDLE_XSIZE*0.5f;
    leftPadEdge.y  = paddle.y - 0.025;
    rightPadEdge.x = paddle.x + PADDLE_XSIZE*0.5f;
    rightPadEdge.y = paddle.y - 0.025;

    glPushMatrix();
      glTranslatef(leftPadEdge.x,leftPadEdge.y, 0.0f);
      drawRegularPolygon(PADDLE_EDGE_RADIUS, 32);
      glPopMatrix();
      glPushMatrix();
      glTranslatef(rightPadEdge.x,rightPadEdge.y, 0.0f);
      drawRegularPolygon(PADDLE_EDGE_RADIUS, 32);
    glPopMatrix();

    glColor3f(0,1,0);
  }

  /* Draw level 2 round play area */
  if (levelInfo.currentLevel == 2) {

    glPushMatrix();
      glTranslatef(levelInfo.center.x, levelInfo.center.y, 0.0f);
      drawRegularPolygon(levelInfo.radius, levelInfo.numPoints);
    glPopMatrix();
    if (displayAxes == ON)
      drawAxes();
    /* Dead zone */
    glColor3f(0.88671875,0.328125,0.328125);
    glPushMatrix();
      glPointSize(10);
      glTranslatef(levelInfo.center.x, levelInfo.center.y, 0.0f);
      glRotatef(210 + MIN_PAD_ANGLE*1.5,0,0,1);
      glTranslatef(0,levelInfo.radius,0);
      glBegin(GL_POINTS);
      glVertex2f(0, 0);
      glEnd();
    glPopMatrix();
    glPushMatrix();
      glTranslatef(levelInfo.center.x, levelInfo.center.y, 0.0f);
      glRotatef(210 + MAX_PAD_ANGLE*0.5,0,0,1);
      glTranslatef(0,levelInfo.radius,0);
      glBegin(GL_POINTS);
      glVertex2f(0, 0);
      glEnd();
    glPopMatrix();
    if( displayAABB == ON)
    {
      glPointSize(10);
      /* Display paddle center point */
      glBegin(GL_POINTS);
        glVertex2f(m[12],m[13]);
      glEnd();
      glColor3f(0.88671875,0.328125,0.328125);
      glPushMatrix();
        glTranslatef(levelInfo.center.x, levelInfo.center.y, 0.0f);
        glRotatef(210 + MIN_PAD_ANGLE*1.5,0,0,1);
        glTranslatef(0,levelInfo.radius,0);
        glBegin(GL_POINTS);
          glVertex2f(0, 0);
       glEnd();
     glPopMatrix();
     glPushMatrix();
       glTranslatef(levelInfo.center.x, levelInfo.center.y, 0.0f);
       glRotatef(210 + MAX_PAD_ANGLE*0.5,0,0,1);
       glTranslatef(0,levelInfo.radius,0);
       glBegin(GL_POINTS);
         glVertex2f(0, 0);
       glEnd();
     glPopMatrix();
     /* Bowlpaddle bounding box */
     glColor3f(0.0,1.0,0.0);
     glBegin(GL_LINE_LOOP);
       glVertex2f(bowlPadAABB.x1,bowlPadAABB.y1);
       glVertex2f(bowlPadAABB.x2,bowlPadAABB.y1);
       glVertex2f(bowlPadAABB.x2,bowlPadAABB.y2);
       glVertex2f(bowlPadAABB.x1,bowlPadAABB.y2);
     glEnd();

     /* Left and right bowl corner bounding sphere */
     glPushMatrix();
       glTranslatef(m[12] + 0.3 * cos((210+padStepAng)/180.0*M_PI),
                    m[13] + 0.3 * sin((210+padStepAng)/180.0*M_PI),0.0);
      drawRegularPolygon(0.03,64);
      glPopMatrix();

      glPushMatrix();
        glTranslatef(m[12] + 0.3 * cos(((210.0+padStepAng)/180.0*M_PI)+(1.0/9.0*M_PI*6)),
                     m[13] + 0.3 * sin(((210.0+padStepAng)/180.0*M_PI)+(1.0/9.0*M_PI*6)),0.0);
        drawRegularPolygon(0.03,64);
      glPopMatrix();
    }
    glColor3f(1.50f, 1.5f, 0.0f);
     /* Draw level 2 bowlpaddle */
    glPushMatrix();
      /* Cal bowlpaddle mid point */
      midPoint.x =(levelInfo.radius -BOWLPAD_LOCATION)* cos(2*(45+2.5)*M_PI/64);
      midPoint.y =(levelInfo.radius -BOWLPAD_LOCATION)* sin(2*(45+2.5)*M_PI/64);
      /* Trans to main play area mid point */
      glTranslatef(levelInfo.center.x, (levelInfo.center.y), 0.0f);
      /* Rotate Bowlpaddle */
      glRotatef(padStepAng,0,0,1);
      if (displayAxes == ON)
        drawAxes();
      /* Trans to Bowlpaddle mid point */
      glTranslatef(midPoint.x, midPoint.y,0.0f);
      glGetFloatv (GL_MODELVIEW_MATRIX, m);
      /* Draw level 2 bowlpaddle */
      drawHalfRegularPolygon(0.3);
    glPopMatrix();
  }

  /* draw the ball */
  for (i = 0; i < numOfBall; i++)
  {
    glColor3f(0.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(ballArr[i].pos.x, ballArr[i].pos.y, 0.0f);
    drawRegularPolygon(ballArr[i].ballRadius, 32);
    glPopMatrix();
  }

  glPushMatrix();
  /* Draw the bricks */
  glColor3f(0.1f, 0.8f, 0.1f);
  glTranslatef(brick[0].pos.x, brick[0].pos.y, 0.0f);
  if (displayAxes == ON)
    drawAxes();
  if ( brick[0].show == ON )
    drawBricks(brickRadius, 0);
  for (i = 1; i < numBricks; i++)
  {
    glTranslatef(brick[i].pos.x - brick[i-1].pos.x, brick[i].pos.y - brick[i-1].pos.y, 0.0f);
    if (displayAxes == ON)
      drawAxes();
    if ( brick[i].show == ON)
      drawBricks(brickRadius, i);
  }
  glPopMatrix();
  /* Update score */
  updateScore();
  gameState(&levelInfo);
}

void keyDown(SDLKey key)
{
  int i;
  switch (key)
  {
    case SDLK_ESCAPE: /* exit app on escape */
    case SDLK_q:
      quit();
    break;
    case SDLK_F2:  /* F2 to start or re-start game */
      if (levelInfo.gameStatus < STATUS_READY
        || levelInfo.gameStatus == STATUS_GAMEOVER
        || levelInfo.gameStatus == STATUS_COMPLETE)
      {
        levelInfo.gameStatus = STATUS_READY;
      }
    break;
    case SDLK_a:
        displayAxes = !displayAxes;
    break;
    case SDLK_b:
        displayAABB = !displayAABB;
    break;
    case SDLK_t:
      displayCTT = !displayCTT;
        break;
    case SDLK_g:    /* switch algorithm */
        algorithm = (algorithm + 1)%2;
    break;
    case SDLK_p:    /* cheating, going to the second level directly */
        if (levelInfo.gameStatus == STATUS_PLAYING && levelInfo.currentLevel == 1)
      score = levelInfo.totalBrick;
    break;
    case SDLK_s:    /* accelerate ball speed */
        for (i = 0; i < numOfBall; i++)
        {
          ballArr[i].ballVel.x = ballArr[i].ballVel.x * 2;
          ballArr[i].ballVel.y = ballArr[i].ballVel.y * 2;
        }
    break;
    case SDLK_x:    /* accelerate ball speed */
        for (i = 0; i < numOfBall; i++)
        {
          ballArr[i].ballVel.x = 1.0f;
        }
    break;
    case SDLK_y:    /* accelerate ball speed */
        for (i = 0; i < numOfBall; i++)
        {
          ballArr[i].ballVel.y = 1.0f;
        }
    break;
    case SDLK_l:   /* inclrease bowl paddle speed */
          bowlPadSpeed += bowlPadSpeed <= MAX_BOWLPAD_SPEED ? 0.01f : 0.0; 
    break;
    case SDLK_k:
          bowlPadSpeed -= bowlPadSpeed >= MIN_BOWLPAD_SPEED ? 0.01f : 0.0; 
    default:
    break;
  }

  /* set the current key state as down */
  if (key < NUM_KEYS)
    keys[key] = true;
}

void keyUp(SDLKey key)
{
  /* set the current key state as up */
  if (key < NUM_KEYS)
    keys[key] = false;
}

void event(SDL_Event *event)
{
    switch (event->type)
    {
        case SDL_KEYDOWN:
            keyDown(event->key.keysym.sym);
            break;
        case SDL_KEYUP:
            keyUp(event->key.keysym.sym);
            break;
        default:
        break;
    }
}

void cleanup()
{
  int i;
  if ( count != NULL )
  {
    for ( i = 0; i < gridHNum; i++ )
      free(count[i]);
    free(count);
    count = NULL;
  }

  if ( offset != NULL )
  {
    for ( i = 0; i < gridHNum; i++ )
      free(offset[i]);
    free(offset);
    offset = NULL;
  }


  if (brickIndex != NULL)
    free(brickIndex);
  brickIndex = NULL;
}
