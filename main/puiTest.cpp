//this file is for testing the pui library, here you can see exactly how one uses it.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
else
#include <unistd.h>
#endif
#include <math.h>
#include <GL/glut.h>
#include <plib/pu.h>
#include <iostream>

using namespace std;

//here for now...
static puInput *input1;
static puOneShot *btn1;
static puButton *chck1;
static puComboBox *cbx1;

void motionfn ( int x, int y )
{
  puMouse ( x, y ) ;
  glutPostRedisplay () ;
}

void mousefn ( int button, int updown, int x, int y )
{
  puMouse ( button, updown, x, y ) ;
  glutPostRedisplay () ;
}

void displayfn ()
{
  glClearColor ( 0.8, 0.8, 0.8, 1.0 ) ;
  glClear ( GL_COLOR_BUFFER_BIT ) ;

  puDisplay () ;

  glutSwapBuffers () ;
  glutPostRedisplay () ;
}

void keyb(unsigned char key, int x, int y){
  puKeyboard(key, PU_DOWN); //so we need this on our keyboard thing to have that work....
  glutPostRedisplay();
}

void btnCB(puObject*){ //work on this later, we need to pass all of this shyte
  char *tmp = new char[80]; //PUI only lets an input string be 80 chars long, if we don't set it here then it will seg fault later on
  input1->getValue(tmp);
  cout<< "input1: " << tmp << endl;
  cout << "chck1: " << chck1->getIntegerValue() << endl;
  cbx1->getValue(tmp);
  cout << "cmbx1: " << tmp << endl;
}

int main ( int argc, char **argv )
{
  glutInitWindowSize ( 500, 500 ) ;
  glutInit ( &argc, argv ) ;
  glutInitDisplayMode ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;
  glutCreateWindow ( "limelight ALPHA" ) ;
  glutDisplayFunc ( displayfn ) ;
  glutMouseFunc ( mousefn ) ;
  glutMotionFunc ( motionfn ) ;
  glutKeyboardFunc(keyb);
  
  puInit();
  
  //a check box (button)
  chck1 = new puButton(200, 300, 210, 310, PUBUTTON_XCHECK);
  chck1->setLabelPlace(PUPLACE_CENTERED_LEFT);
  chck1->setLabel("Bool?:");
  
  //combo box list items
  char **cmboList = new (char*)[10]; //ok this needs to be declared as an array with a size, otherwise it seg faults, but not on a mac
  cmboList[0] = "Dolphins";
  cmboList[1] = "Cat";
  cmboList[2] = "People";
     
  //a combo box
  cbx1 = new puComboBox(200, 400, 300, 430, cmboList, FALSE);
  cbx1->setLabel("Who is smarter?: ");
  cbx1->setLabelPlace(PUPLACE_CENTERED_LEFT);
  
  //an input box
  input1 = new puInput ( 200, 200, 300, 225 ) ;
  input1->setBorderThickness(1);
  input1->setLabelPlace(PUPLACE_CENTERED_LEFT);
  input1->setLabel("Name:");
  
  //a button
  btn1 = new puOneShot(200, 100, 300, 150);
  btn1->setBorderThickness(2);
  btn1->setLegend("Run");
  btn1->setCallback(btnCB);
   
  glutMainLoop () ;

  return 0 ;
}
