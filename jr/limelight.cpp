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
#include <map>
#include "functRead.h" 
#include "dspWin.h" 
#include "functionFileCreator.h"

/******************the great to do list: ****************

 *make limelight take in an environmental arg to open an img -- DONE ts (3/11)
 *make menus work -- DONE ts (3/11)
 *make drop down function changer work -- DONE ts (3/11)
 *make add function stuff work
 *make the open image "cancel" button work -- DONE ts (3/11)
 *'save'/ 'save as' needs to reload B -- DONE ts (3/11)
 *kill the glut menus -- DONE ts (3/11)
 *hmmmm... i think that we can totally get rid of c and d... (glPixelZoom(1.0,-1.0)), haha. except giving it a neg num doesnt work (WTF??)
 *check out disabling gl states we dont use -- DONE ts (3/12)

 *other improvements: 
 *'close' menu item -- DONE ts (3/11)
 *add a printout from the stdout (this one is a little hard, we need to make it pipe the exec)
 *add a function remove feature...
 *rename func.fuk to configure something or other
 *rename enviro to limelight -- DONE ts (3/12)
 *make the code readable
 *enable tab for inputs and return for buttons (possible?)
 *after adding a new function it should tell you to restart the program / if someone's feeling ambitious reload the functions
 *add keyboard shortcuts (cuz that's cool) -- kinda done, except cant get ctrl or shift to work... (whats up with that?)
 *itd be cool to add errors and popups / disable menu items for when you cannot do things
 *load the first function on the list into the window -- DONE ts (3/12)
*/

using namespace std;

//prototypes
void createParamsWin(int num);
void openFile(char* fileName);
void mousefn ( int button, int updown, int x, int y );
void dispfnWinA();
void dispfnWinB();
void displayfn();
void keyb(unsigned char key, int x, int y);
void openFileCB(puObject*); //this is for the open dialog box callback
void openFile(char* fileName);
void openCB(puObject*); //this is for the menu (open dialog box)
void paramsWinOKCB(puObject*);
void createParamsWin(int num);
//crapfunctions
void addFuncCB(puObject*);
void addFuncPathCB(puObject*);
void addFuncNameCB(puObject*);
void addFuncFinalCB(puObject*);
//end crap functions
void exitCB(puObject*);
void closeCB(puObject*);
void saveCB(puObject*);
void saveFileAsCB(puObject*);
//pop up
void go_away_callback(puObject *);
void make_dialog(const char *txt);
//end pop up
void helpCB(puObject*);
void aboutCB(puObject*);
void saveAsCB(puObject*);
void funcReload(puObject*);

//globals
struct ltstr{ //used for the funcMap
  bool operator()(const char* s1, const char* s2) const{
    return strcmp(s1, s2) < 0;
  }
};

map<const char *, int, ltstr> funcMap; // this is here for the drop down menu, cuz it only return char* (never again pui, never again)
puMenuBar *mainMenu;
puFileSelector *openDialogBox;
int mainWin;
int curWidget = 0;
vector<puObject*> paramsWinObjects; 
int curImg; 
int curFunction; //set this when a function menu item is called
dspWin* loadedImg; //this is the loaded image woohoo!
int mainWinWidth, mainWinHeight; //these aren't used right now, but they should eventually be used to increase the size if the params are very large
int winA, winB; //these will be the image windows, ROCK AND ROLL
//stuff for zoom
int mouseOn = 0;
int posWidth, posHeight;

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
//end pui globals (you're dumb pui)

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

void motionfnWinA (/* int button, int updown, */int x, int y ){
  if(mouseOn == 1){
    
    int offsetX, offsetY;
    offsetX = (-posWidth-x);
    offsetY = (-posHeight-y);
    if(offsetX < 0) offsetX = -offsetX;
    if(offsetY < 0) offsetY = -offsetY;
    //if(offsetX > loadedImg->A->width()/2) offsetX = loadedImg->A->width()/2;
    //if(offsetY > loadedImg->A->height()/2) offsetY = loadedImg->A->height()/2;

    cout << "offset y: " << (posHeight-y) << " offset x: " << (posWidth-x) << endl;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, offsetY);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, offsetX);
    glRasterPos2i(-1,-1 );
    glPixelZoom(2.0,2.0);
    glDrawPixels(loadedImg->A->width(),
		 loadedImg->A->height(),
		 GL_RGB,
		 GL_UNSIGNED_BYTE,
		 loadedImg->D);
    glutPostRedisplay();
    glutSwapBuffers();
  }
  else{
    glutPostRedisplay();
    glutSwapBuffers();
  }
}

void mousefnWinA ( int button, int updown, int x, int y ){
  if(updown == GLUT_DOWN){
    if(mouseOn == 1)
      return;
    else{
      cout << "setting posWidth " << x << " posHeight " << y << endl;
      mouseOn = 1;
      posWidth = x/2;
      posHeight = y/2;
    }
  }
  else if (updown == GLUT_UP) 
    mouseOn = 0;
}

void dispfnWinA(){ 
  glClearColor( 0.9, 1.0, 0.9, 1.0 );
  glClear(GL_COLOR_BUFFER_BIT);
  glPixelZoom(2.0,2.0);
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
 
  if(glutGetModifiers() == GLUT_ACTIVE_ALT)
    glutSetCursor(GLUT_CURSOR_DESTROY);

  //keyboard shortcuts
  int KEYCONST = 4; //this means alt key, ctrl is 2, but for some reason it doesnt work...
  
  if(key=='q'){ //quit
    if(glutGetModifiers()==KEYCONST)
      exitCB(mainMenu);//does it matter what we pass it? / will this affect performance? we could just make a null puObject
  }

 if(key=='w'){ //close
    if(glutGetModifiers()==KEYCONST)
      closeCB(mainMenu);
  }

  if(key=='o'){ //open
    if(glutGetModifiers()==KEYCONST)
      openCB(mainMenu);
  }
  
  if(key=='s'){ //save & save as
    if(glutGetModifiers()==KEYCONST)
      saveCB(mainMenu);
    else if(glutGetModifiers()==(GLUT_ACTIVE_ALT|GLUT_ACTIVE_SHIFT)) //for some reason this also doesnt work
      saveAsCB(mainMenu);
  }
}

//FILE MENU -- OPEN DIALOG BOX CALLBACK
void openFileCB(puObject*){
  char* fileName;
  openDialogBox->getValue(&fileName);
  
  //check to see if the cancel button was hit (is this string conv neccesary)
  string tmp = fileName;
  if(tmp.size() == 0){
    puDeleteObject(openDialogBox);
    return;
  }
  
  openFile(fileName); //we don't want to delete the filName, cuz the glut windows use it
}

//OPEN FILE
void openFile(char* fileName){

  //check to see if we've already got an image, if so delete it
  if(loadedImg != NULL){
    glutDestroyWindow(winA);
    glutDestroyWindow(winB);
    deleteDspWin(loadedImg);
  }

  loadedImg = initDspWin(fileName); //from dspWin.h
  
  //create the windows for it, booh ya ka-sha!
  glutInitWindowSize(loadedImg->A->height(), loadedImg->A->width());
  string nameA = "Original: ";
  nameA += fileName;
  winA = glutCreateWindow(nameA.c_str());
  //for zoom stuff
  glutMotionFunc(motionfnWinA);
  glutMouseFunc(mousefnWinA);
  glutDisplayFunc(dispfnWinA);
  glutKeyboardFunc(keyb);

  glutInitWindowSize(loadedImg->A->height(), loadedImg->A->width());
  string nameB = "Output: ";
  nameB += fileName;
  winB = glutCreateWindow(nameB.c_str());
  glutDisplayFunc(dispfnWinB);
  glutKeyboardFunc(keyb);

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
  //TO DO: we should probably add some check to make sure there are values in the params...
  //check to make sure there's an image loaded
  cout << "is paramsWINCB called?\n";
  if(loadedImg == NULL) return;

  //create a list of the param values (in the right order) for the function call
  char *params[paramsWinObjects.size()+4]; //we add 4 to the size cuz the first element is the function name, the last element is a null value, plus in and out files
  
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
  //we need to free up the memory that was allocated for the last function that was displayed
  vector<puObject*>::iterator start = paramsWinObjects.begin();
  while(start!=paramsWinObjects.end()){
    puDeleteObject(*start);
    start++;
  }
  paramsWinObjects.clear();

  //ok, now for the new function
  curFunction = num;
  cout << "curFunction: " << num << endl;
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

//FILE MENU -- CLOSE CALLBACK
void closeCB(puObject*){
  if(loadedImg!=NULL){
    glutDestroyWindow(winA);
    glutDestroyWindow(winB); 
    deleteDspWin(loadedImg);
    loadedImg = NULL;
  }
}

//FILE MENU -- SAVE CALLBACK
void saveCB(puObject*){
  if(loadedImg != NULL){
  saveDspWin(loadedImg);//this saves the image
  get_C_ready(loadedImg);//nice job josh, this is fuckin beautiful
  get_D_ready(loadedImg);
  }
}

//FILE MENU -- SAVE FILE AS CALLBACK
void saveFileAsCB(puObject*){
  char *tmp = new char[200];
  openDialogBox->getValue(tmp);    
  
  //check to see if the cancel button was hit (is this string conv neccesary?)
  string fileName = tmp;
  if(fileName.size() == 0){
    puDeleteObject(openDialogBox);
    return;
  }

  saveDspWin(loadedImg, tmp);
  delete[] tmp;
  //so here, we should actually reload the entire img, so as to rename the win names, etc.
  //in the future this could be a place for improvement, cuz we don't actually need to call
  //the entire open sequence...
  openFile(loadedImg->path);
}

//stuff for making a pop-up
void go_away_callback(puObject *){
  delete popupBox ;
  popupBox = NULL ;
}

void make_dialog(const char *txt){
  if(popupBox != NULL)
    return ;
  
  popupBox = new puDialogBox(25, 25);
  {
    puFrame *tmp1 = new puFrame(0, 0, 200, 225);
    tmp1->setBorderThickness(3);
    puText *tmp = new puText(10, 200);
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
  make_dialog("This program is the \nculmination of 10 \nweeks of work by \nJosh Schwartz and \nTrevor Smith. Thanks \ngo to Yitz and Rio, \nfor answering our \nmultitude of questions,\nand especially to Yitz \nfor suggesting that \nwriting code for \nXServer is a bad \nidea. Thanks to Pedro \nfor believing that we \ncould create this \nprogram.");
}

//FILE MENU -- SAVE AS CALLBACK
void saveAsCB(puObject*){
  if(loadedImg != NULL){
    openDialogBox = new puFileSelector(0, 0, 252, 324, "", "Please print to where you want to save");
    openDialogBox->setInitialValue(""); //make this pretty later
    openDialogBox->setChildBorderThickness(PUCLASS_INPUT, 1);
    openDialogBox->setCallback(saveFileAsCB);
  }
}

//FUNC RELOAD CALLBACK
void funcReload(puObject*){
  char *tmp = new char[80];
  funcItems->getValue(tmp);
  int num =  funcMap[tmp];
  cout << "drop down: " << num << endl;
  createParamsWin(num);
}

int main ( int argc, char **argv ){

  //here's some performance improvements (WOW!, that's a LOT quicker)
  //i think that we only need to disable them in main (not for every window)
  //framebuffer ops we don't need
  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_STENCIL_TEST);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_DITHER);
  glDisable(GL_INDEX_LOGIC_OP);
  //gl states we don't need
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_3D);
  glDisable(GL_LIGHTING);
  glDisable(GL_FOG);


  mainWinHeight = 250;
  mainWinWidth = 400;
  glutInitWindowSize ( mainWinHeight, mainWinWidth ) ;
  glutInit ( &argc, argv ) ;
  glutInitDisplayMode ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;
  mainWin = glutCreateWindow ("limelight" ) ;
  glutDisplayFunc ( displayfn ) ;
  glutMouseFunc ( mousefn ) ;
  glutMotionFunc ( motionfn ) ;
  glutKeyboardFunc(keyb);

  glutSetWindow(mainWin);
  puInit();
  puDisplay();  

  //read in functions and make the drop down menu
  createMenu("funct.fuk", loadedFunctions); //load 'er up
  vector<function>::const_iterator it = loadedFunctions.begin();
  char **functionList = new (char*)[loadedFunctions.size()];

  int i=0;
  while(it != loadedFunctions.end()){
    functionList[i] = (char*)it->name;
    funcMap[(char*)it->name] = i++; //map for drop down callback
    it++;
  }
  
  functionList[loadedFunctions.size()] = NULL; //again, this needs to be set to null for pui...
 
  funcItems = new puComboBox( 15, 335, 215, 360, functionList, FALSE ) ;
 
  funcOk = new puOneShot(15, 300, "Change");
  funcOk->setBorderThickness(2);
  funcOk->setCallback(funcReload);
  
  //draw a line for the top of the screen
  puFrame *top =  new puFrame(0,375,250,401.5);
  top->setColour(PUCOL_BACKGROUND, 1,1,1,1);
  top->setStyle(PUSTYLE_BOXED);
  top->setBorderThickness(1);  

  //help -- here's the vedict on these arrays, they NEED to be one longer than the length, and the last entry needs to be a NULL
  //otherwise we get weird seg faults, etc.
  char *help_submenu[4] = {"Help", "-----", "About", NULL};
  puCallback help_submenu_cb [4] = {helpCB, NULL, aboutCB, NULL};

  //file menu 
  //for now, we have now "add function" menu item, cuz i dont feel like making it good.
  char *file_submenu[8] = {"Exit           Alt+Q", "--------------------", "Save as   Alt+Shft+S", "Save           Alt+S",/* "--------------------", "Add new function",*/ "--------------------","Close          Alt+W", "Open           Alt+O", NULL};
  puCallback file_submenu_cb [8] = { exitCB, NULL, saveAsCB, saveCB, /*NULL, addFuncCB,*/ NULL, closeCB, openCB, NULL};
  
  //make the main menu
  mainMenu = new puMenuBar ( -1 );
  mainMenu->add_submenu ( "File", file_submenu, file_submenu_cb);
  mainMenu->add_submenu( "Help", help_submenu, help_submenu_cb);
  mainMenu->close ();

  //load the first function on the list into the main window
  createParamsWin(0);

  //the program was called with an image, so open it
  if(argc > 1)
    openFile(argv[1]);
 
  glutMainLoop () ;

  return 0 ;
}
