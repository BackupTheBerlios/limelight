#include <iostream>
// for linux#include <GL/glut.h>
#include <GLUT/glut.h> // for macs
#include "pnmfile.h"
using namespace std;

image<rgb> *image1;
int win;

void init(char *fileName){
  glClearColor(0.5,0.5,0.5,0.0);
  glShadeModel(GL_FLAT);
  image1 = loadPPM(fileName); // currently this is hardcoded for ppm, change this
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //this wants packed pixels
}

//this controls what is being displayed
void disp(void){
  glClear(GL_COLOR_BUFFER_BIT);
  glRasterPos2i(-1, -1); //what is up with this? 0,0 supposed to be lower left corner...
  glDrawPixels(image1->width(), 
	       image1->height(),
	       GL_RGB,
	       GL_UNSIGNED_BYTE,
	       image1->data);
  glFlush();
}

//keyboard stuff
void keyb(unsigned char key, int x, int y){
  if(key=='q'){
    std::cout << "goodbye" << endl;    
    glutDestroyWindow(win);
    exit(0);
  }
}

int main(int argc, char **argv){
  if(argc < 2){ //there's always one argument which is the name of the program
    std::cout << "Usage: <image>" << endl;
    return 1;
   }
  cout << "Whats going one?\n";
  
  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_RGB|GLUT_SINGLE|GLUT_DEPTH);
  
  glutInitWindowSize(507,509);
  glutInitWindowPosition(0,0);
 
  win = glutCreateWindow("a malicious god?");
  
  init(argv[1]); //the second argument will be the image filename  
   
  //event stuff
  glutDisplayFunc(disp);
  glutKeyboardFunc(keyb);
  
  glutMainLoop();

  return 0;
}
