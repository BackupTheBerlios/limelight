//this is the main file
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#include <GL/glut.h>
#endif
#ifdef WIN32
#include <windows.h>
else
#include <unistd.h>
#endif
#include <math.h>
#include <plib/pu.h>
#include <iostream>
#include <vector.h>
#include <string>
#include "functRead.h" //what a bad name for this header file
#include "dspWin.h" // a much better job of naming is going on here
#include "functionFileCreator.h"

using namespace std;

//prototypes
void createParamsWin(int num);

//globals

puMenuBar *mainMenu;
puFileSelector *openDialogBox;
int mainWin;
vector<puObject*> paramsWinObjects; 
int curImg; 
int curFunction; //set this when a function menu item is called
dspWin* loadedImg; //this is the loaded image woohoo!
int mainWinWidth, mainWinHeight; //these aren't used right now, but they should eventually be used to increase the size if the params are very large
int winA, winB; //these will be the image windows, ROCK AND ROLL

/*********************** pui globals (globally defined gui elements are not my friend *****/
puFileSelector *addFuncPath;
puInput *addFuncName, *addFuncParamsNum;
vector<puObject*> addFuncParams; //the entires will be name, then type, ok cool 
char *addFuncPathValue = new char[80]; 
char *addFuncNameValue = new char[80]; 
int *addFuncParamNumValue = new int;
void addFuncCB(puObject*);
void addFuncPathCB(puObject*);
void addFuncNameCB(puObject*);
void addFuncFinalCB(puObject*);
puOneShot *ok;
/************end pui shit ******* fuck you pui*/

#ifndef PARAM_S
#define PARAM_S
struct parameter {
  string name;
  string type;
};
#endif

list<parameter> addFuncParamsValues; //the entires will be name, then type, ok not cool, pui is lame

void motionfn ( int x, int y ){
  puMouse ( x, y ) ;
  glutPostRedisplay () ;
}

void mousefn ( int button, int updown, int x, int y ){
  puMouse ( button, updown, x, y ) ;
  glutPostRedisplay () ;
}

void dispfnWinA(){ 
  glutSetWindow(winA);
  glClearColor( 0.9, 1.0, 0.9, 1.0 );
  glClear(GL_COLOR_BUFFER_BIT);
  
  glRasterPos2i(-1,-1 );
  glDrawPixels(loadedImg->A->width(), 
	       loadedImg->A->height(),
	       GL_RGB,
	       GL_UNSIGNED_BYTE,
	       loadedImg->D);
  glutSwapBuffers();
  glutPostRedisplay();
}

void dispfnWinB(){
  glutSetWindow(winB);
  glClearColor( 0.9, 1.0, 0.9, 1.0 );
  glClear(GL_COLOR_BUFFER_BIT);
  
  glRasterPos2i(-1,-1 );
  glDrawPixels(loadedImg->A->width(), 
	       loadedImg->A->height(),
	       GL_RGB,
	       GL_UNSIGNED_BYTE,
	       loadedImg->C);
  glutSwapBuffers();
  glutPostRedisplay();
}

void displayfn(){
  glClearColor( 0.9, 1.0, 0.9, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  puDisplay();
    
  glutSwapBuffers();
  glutPostRedisplay();
}

void keyb(unsigned char key, int x, int y){
  puKeyboard(key, PU_DOWN); //so we need this on our keyboard thing to have that work....
  glutPostRedisplay();
}

//FILE MENU -- OPEN DIALOG BOX CALLBACK
void openFileCB(puObject*){
  char* fileName;
  openDialogBox->getValue(&fileName);
  loadedImg = initDspWin(fileName); //from dspWin.h

  //create the windows for it, booh ya ka-sha!
  glutInitWindowSize(loadedImg->A->height(), loadedImg->A->width());

  winA = glutCreateWindow(fileName);
  glutDisplayFunc(dispfnWinA);

  glutInitWindowSize(loadedImg->A->height(), loadedImg->A->width());
  winB = glutCreateWindow(fileName);
  glutDisplayFunc(dispfnWinB);
   
  puDeleteObject(openDialogBox);
  
  //we need to add some kind of if clause for the cancel button
  //but the cancel button doesn't have it's own callback

  cout << "open the file: " << fileName << endl;
  //we don't want to delete the filName, cuz the glut windows use it
}

//FILE MENU -- OPEN CALLBACK
void openCB(puObject*){
  openDialogBox = new puFileSelector(0, 0, 252, 324, "", "Please select an image");
  openDialogBox->setInitialValue(" "); //make this pretty later
  openDialogBox->setChildBorderThickness(PUCLASS_INPUT, 1);
  openDialogBox->setCallback(openFileCB);
}

//PARAMS WINDOW -- OK CALLBACK
void paramsWinOKCB(puObject*){
  //check to make sure there's an image loaded
  if(loadedImg == NULL) return;

  //create a list of the param values (in the right order) for the function call
  char *params[paramsWinObjects.size()+4]; //we add 2 to the size cuz the first element is the function name, and the last element is a null value
  
  int i = 1; //remember, 0 is the func name
  unsigned int j = 0;
  //loop over the param values from the window
  while(j < paramsWinObjects.size()){
    char *tmp = new char[80]; //PUI only lets an input string be 80 chars long anyway...
    paramsWinObjects[j]->getValue(tmp);
    cout << "ok: " << tmp << endl;
    params[i++] = tmp;
    j++;
  }

  params[0] = loadedFunctions[curFunction].name;
  params[paramsWinObjects.size()+1] = "/tmp/tmp.fuk"; //infile -- this will be trashed
  params[paramsWinObjects.size()+2] ="/tmp/tmp2.fuk"; //outfile -- this will be trashed
  params[paramsWinObjects.size()+3] = '\0';

  //TO DO: clear the params so the next loaded function will be able to make them
  //it's true, this needs to be done, but only after a new function is loaded
  //paramsWinObjects.clear();

  glutSetCursor(GLUT_CURSOR_WAIT); //make a cool wait cursor for while the function is running
  callFunct(loadedImg, curFunction, params); //from dspWin.h
  glutSetCursor(GLUT_CURSOR_INHERIT);
    
  //now redisplay c in winb
  glutSetWindow(winB);
  glutPostRedisplay();
  glutSwapBuffers();
}

//FUNCTION MENU -- CALLBACK
void createParamsWin(int num){

  //otherwise do normal thing
  curFunction = num;
  
  puFrame *box;
  box = new puFrame(0,0,252,324);
  box->setColour(PUCOL_BACKGROUND, 1,1,1, 1);
  box->setStyle(PUSTYLE_BOXED);
  box->setBorderThickness(1);
  
  //loop over the params of the function  
  vector<pairBuff10>::const_iterator it = loadedFunctions[num].params.begin();
  
  //loop over the params vector, creating fun widgets as we go
  int y=280; 
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
    //TO DO: add enum stuff
    it++;
  }
  puOneShot *ok = new puOneShot(x,y,"Run");
  ok->setBorderThickness(2);    
  ok->setCallback(paramsWinOKCB);
}

//FILE MENU -- add function callback
//LET ME JUST SAY THAT I AM NOT RESPONSIBLE FOR THIS SHIT
//(it was written between 3 and something in the morning, and it's all crap)
void addFuncCB(puObject*){
  //grab the path to the binary
  addFuncPath = new puFileSelector(0, 0, 252, 324, "", "Please select a binary");
  addFuncPath->setInitialValue("");
  addFuncPath->setChildBorderThickness(PUCLASS_INPUT, 1);
  addFuncPath->setCallback(addFuncPathCB);  
}

void addFuncPathCB(puObject*){
 
  addFuncPath->getValue(addFuncPathValue); //store value of the filepath
  puDeleteObject(addFuncPath);
 
  //TO DO: check for cancel button (cuz it doesn't have its own callback)
  //TO DO: these labels don't show up, i have NO idea why...
  
  //this guy needs to be deleted (aka globally declared)
  puText *title = new puText(50, 200);
  title->setLabelPlace(PUPLACE_CENTERED_LEFT);
  title->setLabel("Add new function (2 of 3)");

  addFuncName = new puInput(50, 150, 100, 170);
  addFuncName->setLabel("Name: ");
  addFuncName->setBorderThickness(1);
  addFuncName->setLabelPlace(PUPLACE_CENTERED_LEFT);
  
  addFuncParamsNum = new puInput(50, 100, 100, 120);
  addFuncParamsNum->setBorderThickness(1);
  addFuncParamsNum->setLabelPlace(PUPLACE_CENTERED_LEFT);
  addFuncParamsNum->setLabel("# of params: ");
  
  ok = new puOneShot(50,50,"Ok");
  ok->setBorderThickness(2);    
  ok->setCallback(addFuncNameCB);
}

void addFuncNameCB(puObject*){
  addFuncName->getValue(addFuncNameValue); //get this value
  cout << "name: " << addFuncNameValue << endl;
  addFuncParamsNum->getValue(addFuncParamNumValue);
  puDeleteObject(addFuncName);
  puDeleteObject(addFuncParamsNum);
  puDeleteObject(ok);
  
  //loop over the params vector, creating fun widgets as we go
  int y=280; 
  int x=100;
  int i =0; 
  while(i != *addFuncParamNumValue){
    puInput *tmp = new puInput ( x, y, x+50,y+20 ) ;
    tmp->setBorderThickness(1);
    tmp->setLabelPlace(PUPLACE_CENTERED_LEFT);
    tmp->setLabel("Name:");
    addFuncParams.push_back(tmp); //push these onto the vector so we can grab their values later
    y-=30;
    //add type boxes
    tmp = new puInput ( x, y, x+50,y+20 ) ;
    tmp->setBorderThickness(1);
    tmp->setLabelPlace(PUPLACE_CENTERED_LEFT);
    tmp->setLabel("Type:");
    addFuncParams.push_back(tmp); //push these onto the vector so we can grab their values later
    y-=30;
    i++;
  }       
  ok = new puOneShot(x,y,"Run");
  ok->setBorderThickness(2);    
  ok->setCallback(addFuncFinalCB);
}

void addFuncFinalCB(puObject*){
  unsigned int j = 0;
  //loop over the param values from the window
  while(j < addFuncParams.size()){
    parameter tmp;
    char *tmp1 = new char[80];
    addFuncParams[j]->getValue(tmp1);
    puDeleteObject(addFuncParams[j++]);
    cout << "param name: " << tmp1 << endl;
    tmp.name = tmp1;
    tmp1 = new char[80];
    addFuncParams[j]->getValue(tmp1);
    puDeleteObject(addFuncParams[j++]);
    cout << "param type: " << tmp1 << endl;
    tmp.type = tmp1;
    addFuncParamsValues.push_back(tmp);
  }
  puDeleteObject(ok);
  //ok call this fucker
  //these all need to be changed to string
  string name, path;
  name = addFuncNameValue;//UGLY
  path = addFuncPathValue;//UGLY
  newFunct(name, path, addFuncParamsValues);
  cout << "function added\n";
}

//END SECTION OF CODE NO ONE IS RESPONSIBLE FOR

//FILE MENU -- exit function callback
void exitCB(puObject*){
  puDeleteObject(mainMenu);
  //TO DO: is this deallocating everything? 
  
  if(loadedImg != NULL){
    glutDestroyWindow(winA);
    glutDestroyWindow(winB); 
    deleteDspWin(loadedImg); 
  }
  glutDestroyWindow(mainWin);
  exit(0);
}

//FILE MENU -- SAVE CALLBACK
void saveCB(puObject*){
  saveDspWin(loadedImg);
}

//FILE MENU -- SAVE FILE AS CALLBACK
void saveFileAsCB(puObject*){
  char *tmp = new char[200];
  openDialogBox->getValue(tmp);
  
  puDeleteObject(openDialogBox);
  
  saveDspWin(loadedImg, tmp);
  cout << "file saved as successfully\n";
  delete[] tmp;
}

//HELP MENU -- help callback
void helpCB(puObject*){
  cout << "help me!\n";
}

//HELP MENU -- about callback
void aboutCB(puObject*){
  cout << "about everything\n";
}

//FILE MENU -- SAVE AS CALLBACK
void saveAsCB(puObject*){
  openDialogBox = new puFileSelector(0, 0, 252, 324, "", "Please print to where you want to save");
  openDialogBox->setInitialValue(""); //make this pretty later
  openDialogBox->setChildBorderThickness(PUCLASS_INPUT, 1);
  openDialogBox->setCallback(saveFileAsCB);
}

int main ( int argc, char **argv ){
  mainWinHeight = 250;
  mainWinWidth = 350;
  glutInitWindowSize ( mainWinHeight, mainWinWidth ) ;
  glutInit ( &argc, argv ) ;
  glutInitDisplayMode ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;
  mainWin = glutCreateWindow ("limelight jr" ) ;
  glutDisplayFunc ( displayfn ) ;
  glutMouseFunc ( mousefn ) ;
  glutMotionFunc ( motionfn ) ;
  glutKeyboardFunc(keyb);

  glutSetWindow(mainWin);
  puInit();
  puDisplay();  

  char **file_submenu = new (char*)[10];
  file_submenu[7] = "Open";
  file_submenu[6] = "-------";
  file_submenu[5] = "Add new function";
  file_submenu[4] = "-------";
  file_submenu[3] = "Save";
  file_submenu[2] = "Save As";
  file_submenu[1] = "-------";
  file_submenu[0] = "Exit";
  puCallback file_submenu_cb [8] = { exitCB, NULL, saveAsCB, saveCB, NULL, addFuncCB, NULL, openCB};

  //read in functions and make a glut style menu
  createMenu("funct.fuk", loadedFunctions); //load 'er up
  vector<function>::const_iterator it = loadedFunctions.begin();
  glutCreateMenu(createParamsWin); //in the future, if we want, this returns an int as an id

  int i=0;
  while(it != loadedFunctions.end()){
    glutAddMenuEntry((char*)it->name, i++);
    it++;
  }

  glutAttachMenu(GLUT_RIGHT_BUTTON); //end menu creation

  //draw a line for the top of the screen
  puFrame *top =  new puFrame(0,350,250,326.5);
  top->setColour(PUCOL_BACKGROUND, 1,1,1,1);
  top->setStyle(PUSTYLE_BOXED);
  top->setBorderThickness(1);  

  //make the menu
 
  //help submenu
  char **help_submenu = new (char*)[5];
  help_submenu[3] = "Help";
  help_submenu[2] = "-------";
  help_submenu[1] = "About";
  help_submenu[0] = NULL;
  puCallback help_submenu_cb [4] = {NULL, aboutCB, NULL, helpCB};

  mainMenu = new puMenuBar ( -1 );
  mainMenu->add_submenu ( "File", file_submenu, file_submenu_cb );
  //TO DO: this menu won't display and I don't know why (hmmm, there's a pattern here, of things that are happening for reasons i do 
  //not understand, and defy rules of logic, i think that this must have something to do with lack of sleep
  mainMenu->add_submenu( "Help", help_submenu, help_submenu_cb);
  mainMenu->close ();

  glutMainLoop () ;

  return 0 ;
}
