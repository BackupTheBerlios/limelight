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


/******************the great to do list: ****************

 *make limelight take in an environmental arg to open an img
 *make menus work
 *make drop down function changer work
 *make add function drop down work
 *make the open image "cancel" button work

 *other improvements: 
 *add a printout from the stdout
 *rename func.fuk to configure something or other
 *rename enviro to limelight

 */

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

//pui globals
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
puDialogBox *popupBox = NULL ;
puComboBox *funcItems;
puOneShot *funcOk;
//end pui globas (you're dumb pui)

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
  
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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
  
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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

  //check to see if the cancel button was hit (this is ugly)
  if(*(((string)fileName).last()) == '/')
    return;
  
  //check to see if we've already go an image, if so delete it
  if(loadedImg != NULL){
    glutDestroyWindow(winA);
    glutDestroyWindow(winB);
    deleteDspWin(loadedImg);
  }

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
  openDialogBox->setInitialValue(""); //make this pretty later
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
  box = new puFrame(0,0,252,280);
  box->setColour(PUCOL_BACKGROUND, 1,1,1, 1);
  box->setStyle(PUSTYLE_BOXED);
  box->setBorderThickness(1);
  
  //loop over the params of the function  
  vector<pairBuff10>::const_iterator it = loadedFunctions[num].params.begin();
  
  //loop over the params vector, creating fun widgets as we go
  int y=250; 
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
  //TO DO: is this deallocating everything? (check into openGL)
  
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

//stuff for making a pop-up
void go_away_callback ( puObject * ){
  delete popupBox ;
  popupBox = NULL ;
}

void make_dialog ( const char *txt ){
  if ( popupBox != NULL )
    return ;
  
  popupBox = new puDialogBox(25, 100);
  {
    puFrame *tmp1 = new puFrame(0, 0, 200, 125);
    tmp1->setBorderThickness(3);
    puText *tmp = new puText(10, 95);
    tmp->setLabel(txt);

    puOneShot *ok = new puOneShot (160, 10, "OK" ) ;
    ok->makeReturnDefault(TRUE);
    ok->setCallback(go_away_callback);
  }
  popupBox -> close  () ;
  popupBox -> reveal () ;
}
//end pop up stuff

//HELP MENU -- help callback
void helpCB(puObject*){
  make_dialog("help? haha. not here.\n");
}

//HELP MENU -- about callback
void aboutCB(puObject*){
  make_dialog("here's some cool stuff\nabout the program\n");
}

//FILE MENU -- SAVE AS CALLBACK
void saveAsCB(puObject*){
  openDialogBox = new puFileSelector(0, 0, 252, 324, "", "Please print to where you want to save");
  openDialogBox->setInitialValue(""); //make this pretty later
  openDialogBox->setChildBorderThickness(PUCLASS_INPUT, 1);
  openDialogBox->setCallback(saveFileAsCB);
}

//FUNC RELOAD CALLBACK
void funcReload(puObject*){
  
}

int main ( int argc, char **argv ){
  mainWinHeight = 250;
  mainWinWidth = 400;
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
 
  //this will be good shortly
  //read the functions in, add them to the list array
  createMenu("funct.fuk", loadedFunctions); //load 'er up
  it = loadedFunctions.begin();
  i=0;
  
  char **functionList = new (char*)[loadedFunctions.size()+1]; 
  while(it != loadedFunctions.end()){
    functionList[i++] = (char*)it->name;
    it++;
  }
  functionList[loadedFunctions.size()] = NULL;
  
  //ok, here's the functions shit, so you can change them
  funcItems = new puComboBox( 15, 335, 215, 360, functionList, FALSE ) ;
  
  funcOk = new puOneShot(15, 300, "Change");
  funcOk->setBorderThickness(2);
  //funcOk->setCallback(funcReload);
  
  //draw a line for the top of the screen
  puFrame *top =  new puFrame(0,375,250,401.5);
  top->setColour(PUCOL_BACKGROUND, 1,1,1,1);
  top->setStyle(PUSTYLE_BOXED);
  top->setBorderThickness(1);  

  //help submenu -- ok more screwed up pui stuff, these menus won't work if you have an odd number of items...
  /*  char **help_submenu = new (char*)[4];
  for(int i=0;i<3;i++){
    help_submenu[i] = new char[30];
  }
  help_submenu[2] = "Help";
  help_submenu[1] = "---";
  help_submenu[0] = "About";

  int l =0;
  while(l<3)
    cout << help_submenu[l++] <<endl;
  */

  char *help_submenu[] = {"Help", "-----", "About", NULL};
  puCallback help_submenu_cb [] = {helpCB, NULL, aboutCB, NULL};

  //file menu 
  char *file_submenu[] = {"Exit", "-----", "Save as", "Save", "-----", "Add new function", "-----", "Open", NULL};
  puCallback file_submenu_cb [] = { exitCB, NULL, saveAsCB, saveCB, NULL, addFuncCB, NULL, openCB, NULL};
  
  //make the menu
  mainMenu = new puMenuBar ( -1 );
  mainMenu->add_submenu ( "File", file_submenu, file_submenu_cb);
  mainMenu->add_submenu( "Help", help_submenu, help_submenu_cb);
  cout << "after help menu called get here?\n";  
  mainMenu->close ();
 
  glutMainLoop () ;

  return 0 ;
}
