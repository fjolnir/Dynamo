// Performs initialization when running in a browser

#ifndef EMSCRIPTEN
	#error "This file is for use with emscripten"
#endif

#include <stdio.h>
#include <GL/glut.h>
#include "drawutils.h"

static Renderer_t *renderer;

static void draw()
{
	glClearColor(1,1,1,1);
	glClear(GL_COLOR_BUFFER_BIT);
	draw_ellipse(vec2_create(10,10),
				 vec2_create(50, 50), 50, 0, vec4_create(0,1,0,1), true);
	/*renderer_display(renderer, 0, 0);*/
	glutSwapBuffers();
}
static void idle()
{
	glutPostRedisplay();
}
int main(int argc, char *argv[])
{
   glutInit(&argc, argv);
   glutInitWindowSize(300, 300);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

   glutCreateWindow("dynamo");

   glutIdleFunc(idle);
   /*glutReshapeFunc(gears_reshape);*/
   glutDisplayFunc(draw);
   /*glutSpecialFunc(gears_special);*/

   renderer = renderer_create(vec2_create(640, 480), vec3_create(0,0,0));
   obj_retain(renderer);
   draw_init(renderer);
   
   glutMainLoop();

   return 0;
}
