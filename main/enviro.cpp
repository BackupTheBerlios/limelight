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
#include <vector>

#include "functRead.h" //what a bad name for this header file
using namespace std;

/*********************ok here we go this will be a class but i dont feel like making a class right now *************/

vector<function> loadedFunctions; //this is read in from the funct.fuk file
puMenuBar *mainMenu;
puFileSelector *openDialogBox;
int mainWin;

/********/


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

//FILE MENU -- OPEN DIALOG BOX CALLBACK
void openFileCB(puObject*){
  char* fileName;
  openDialogBox->getValue(&fileName);
  
  //the following note was in a pui ex file, it's probably important

  //NOTE: interface creation/deletion must be nested
  //the old interface must be deleted *before* a new one is created
  //otherwise the interface stack will be messed up
  
  puDeleteObject(openDialogBox);
  
  cout << "open the file: " << fileName << endl;
}

//FILE MENU -- OPEN CALLBACK
void openCB(puObject*){
  openDialogBox = new puFileSelector(50, 50, "./", "Please select a file");
  openDialogBox->setChildBorderThickness(PUCLASS_INPUT, 1);
  openDialogBox->setCallback(openFileCB);
}

void exitCB(puObject*){ //work on this later, we need to pass all of this shyte
  glutDestroyWindow(mainWin);
  exit(0);
}

int main ( int argc, char **argv )
{
  glutInitWindowSize ( 750, 550 ) ;
  glutInit ( &argc, argv ) ;
  glutInitDisplayMode ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;
  mainWin = glutCreateWindow ("limelight ALPHA" ) ;
  glutDisplayFunc ( displayfn ) ;
  glutMouseFunc ( mousefn ) ;
  glutMotionFunc ( motionfn ) ;
  glutKeyboardFunc(keyb);
  
  puInit();

  //ok let's try this shit
  //menus must be declared backwards and we get seg faults if we don't make the char of a specified array length

  char **file_submenu = new (char*)[10];
  file_submenu[2] = "Open";
  file_submenu[1] = "Save";
  file_submenu[0] = "Exit";

  puCallback file_submenu_cb [3] = { exitCB, NULL, openCB};

  //here's where it a-goes down
  createMenu("funct.fuk", loadedFunctions); //load 'er up
  
  char** functSubmenu = new (char*)[loadedFunctions.size()];
  
  vector<function>::const_iterator it = loadedFunctions.begin();
  int i=0;
  while(it != loadedFunctions.end()){
    functSubmenu[i++] = (char*)it->name;
    it++;
  }
  puCallback functSubmenuCB [loadedFunctions.size()];
  i=0;
  while(i<loadedFunctions.size())
    functSubmenuCB[i++] = NULL;
  //end the a-going-down-ness

  mainMenu = new puMenuBar ( -1 );
  mainMenu->add_submenu ( "File", file_submenu, file_submenu_cb ) ;
  mainMenu->add_submenu ( "Functions", functSubmenu, functSubmenuCB);
  mainMenu->close ();

  glutMainLoop () ;

  return 0 ;
}
