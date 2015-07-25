
#ifndef SDL_BASE_H
#define SDL_BASE_H

#include <SDL/SDL.h>

/* HACK: fix for SDL breaking our main function */
#ifdef main
#undef main
#endif

#ifdef WIN32
#include <windows.h>
#endif

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

/* Implement these yourself */
void init();
void reshape(int w, int h);
void update(int milliseconds);
void display(SDL_Surface *screen);
void event(SDL_Event *event);
void cleanup();
double round(double x);
/* This is updated every second by the main loop -- no need to calculate it
 * yourself.*/
extern int fps;

/* Call this to quit. */
void quit();

#endif
