//this is the main file

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

/*********************ok here we go: this will be a class but i dont feel like making a class right now *************/

vector<function> loadedFunctions; //this is read in from the funct.fuk file
puMenuBar *mainMenu;
puFileSelector *openDialogBox;
int mainWin;
vector<puObject*> paramsWinObjects;
puDialogBox *paramsWin; //this is an ugly hack for now

//???
void(*funct)();

vector< void(*funct)() > cbs;  

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
  
  //we need to add some kind of if clause for the cancel button
  //but the cancel button doesn't have it's own callback

  cout << "open the file: " << fileName << "|" << endl;
}

//FILE MENU -- OPEN CALLBACK
void openCB(puObject*){
  openDialogBox = new puFileSelector(50, 50, "./", "Please select a file");
  openDialogBox->setInitialValue(" ");
  openDialogBox->setChildBorderThickness(PUCLASS_INPUT, 1);
  openDialogBox->setCallback(openFileCB);
}

//PARAMS WINDOW -- CANCEL CALLBACK
void paramsWinCancelCB(puObject*){
  delete paramsWin;
}

//CBS FOR FUNCTIONS
void funcCBS(){
  for(int i=0;i<loadedFunctions.size();i++){
    void CB(puObject*){ createParamsWin(i);}
    func = CB(puObject*);
    cbs.push_back(func);
  }
}

//FUNCTION MENU -- CALLBACK
void createParamsWin(int num){
  //this is lame, but we cannot pass anything in here, so we need to gather what function is being called
  //from somewhere else.... (thundercrash) but where?
  
 //this is most definetly not the ideal way to do this, but for now....
  paramsWin = new puDialogBox(0, 0);
  {//this is weird syntax
    //this is the background
    puFrame *box;
    box = new puFrame(25,50,250,350);
    box->setColour(PUCOL_BACKGROUND, 1,1,1, 1);
    box->setStyle(PUSTYLE_BOXED);
    box->setBorderThickness(1);

    //loop over the params of the function
    //hardcoded for now 
    vector<pairBuff10>::const_iterator it = loadedFunctions[num].params.begin();
    
    //loop over the params vector, creating fun widgets as we go
    int y=300; 
    int x=100;
    while(it!=loadedFunctions[num].params.end()){
      //if it's an int make a box for it
      if((string)it->second == (string)"int"){
	puInput *tmp = new puInput ( x, y, x+50,y+20 ) ;
	tmp->setBorderThickness(1);
	tmp->setLabelPlace(PUPLACE_CENTERED_LEFT);
	tmp->setLabel(it->first);
	paramsWinObjects.push_back(tmp); //push these onto the vector so we can grab their values later
	y-=30;
      }     
      //if it's a boolean make a checkbox
      else if((string)it->second == (string)"bool"){
	puButton *tmp = new puButton(x, y, x+10, y+10, PUBUTTON_XCHECK);
	tmp->setLabelPlace(PUPLACE_CENTERED_LEFT);
	tmp->setLabel(it->first);
	paramsWinObjects.push_back(tmp); //push these onto the vector so we can grab their values later
	y-=30;
      }
      //add enum stuff here
      it++;
    }
    y-=30;
    puOneShot *ok = new puOneShot(x,y,"OK");
    ok->setBorderThickness(2);    
    puOneShot *cancel = new puOneShot(x+45, y, "Cancel");
    cancel->setBorderThickness(2);
    cancel->setCallback(paramsWinCancelCB);
  }
  paramsWin->close();
  paramsWin->reveal();  
}

//FILE MENU -- EXIT CALLBACK
void exitCB(puObject*){ //work on this later, we need to pass all of this shyte
  //delete all of this memory
  puDeleteObject(mainMenu);

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
  puDisplay();
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
  while(i<(const int)loadedFunctions.size())
    functSubmenuCB[i++] = cbs[i]; //WHAT GOES HERE???
  //end the a-going-down-ness

  mainMenu = new puMenuBar ( -1 );
  mainMenu->add_submenu ( "File", file_submenu, file_submenu_cb ) ;
  mainMenu->add_submenu ( "Functions", functSubmenu, functSubmenuCB);
  mainMenu->close ();

  glutMainLoop () ;

  return 0 ;
}
