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
#include <vector>
#include <string>
#include <map>

//our includes
#include "functRead.h" 
#include "dspWin.h" 

/******************the great to do list: ****************

 *check out disabling gl states we dont use -- for macs and linux -- DONE
 *remeber to not compile with -g in the final makefile
 *kill save -- DONE
 *on linux kill the windows thing (when someone hits a child window's X don't make it close), also make it so you cant resize the image windows
 *error checking for this entire program, specifically if the executable isn't there for a function call, if an image didn't load correctly,
  or wasn't there, etc.. -- DONE
 *dspWin.h doesn't need save or save as distinction anymore

 *wish list:
 *hmmmm... i think that we can totally get rid of c and d... (glPixelZoom(1.0,-1.0)), haha. except giving it a neg num doesnt work (WTF??)
 *add a printout from the stdout (this one is a little hard, we need to make it pipe the exec) -- dup2 to a text file so people can look at it later
 *enable tab for inputs and return for buttons (possible?)
 *make pan amount not so slow when zoomAmount < 5
 *TO DO: we should probably add some check to make sure there are values in the params...
 *currently, when saving a file it will overwrite if one exists with the specified name, we should ask if this is the case
 *see what gl states macs can actually use

*/

using namespace std;

/* ************************** PROTOTYPES ************************** */

/* ******************** GLUT WINDOW CALL BACKS ******************** */

//main window
void dspfnMain();
void mousefnMain ( int button, int updown, int x, int y );
void motionfnMain (int x, int y);
//window a (original image)
void dispfnWinA();
void mousefnWinA(int button, int updown, int x, int y);
void motionfnWinA(int x, int y );
//window b (output image)
void dispfnWinB();
void mousefnWinB(int button, int updown, int x, int y);
void motionfnWinB(int x, int y );
//shared
void keyb(unsigned char key, int x, int y);

/* ************************* PUI CALLBACKS ************************* */

//file menu callbacks
void closeCB(puObject*);
void exitCB(puObject*);
void openCB(puObject*);
void saveCB(puObject*);

//help menu callbacks
void aboutCB(puObject*);
void helpCB(puObject*);

//pop up box callbacks
void openFileWinCB(puObject*);
void saveWinCB(puObject*);

//main window
void callFuncCB(puObject*);
void chngFuncCB(puObject*);

//misc
void hidePopupCB(puObject *);

/* ************************** FUNCTIONS *************************** */

void dspFuncParams(int num);
void exitProgram(void);
void makePopup(const char *txt);
void openFile(char* fileName);

/* **************************** GLOBALS *************************** */

struct ltstr{ //used for the funcMap
  bool operator()(const char* s1, const char* s2) const{
    return strcmp(s1, s2) < 0;
  }
};

map<const char *, int, ltstr> funcMap; // this is here for the drop down menu, cuz it only return char* (never again pui, never again)
puMenuBar *mainMenu;
puFileSelector *openDialogBox = 0;
int mainWin;
int curWidget = 0;
vector<puObject*> paramsWinObjects; 
int curImg; 
int curFunction; //set this when a function menu item is called
dspWin* loadedImg; //this is the loaded image woohoo!
int winA, winB; //these will be the image windows, ROCK AND ROLL
//vector<function> loadedFunctions is where all of our functions are stored, it is (sadly) declared in the dspWin.h file

//for zoom and pan winA
int mouseOnA = 0;
int posWidthA, posHeightA;
double newOffXA=0.0;
double newOffYA=0.0;
double offSetXA=0.0;
double offSetYA=0.0;
int zoomA=0; //whether or not we are zooming
double zoomAmountA=1.0;
double zoomOffSetYA = 0.0;

//for zoom and pan winB
int mouseOnB = 0;
int posWidthB, posHeightB;
double newOffXB=0.0;
double newOffYB=0.0;
double offSetXB=0.0;
double offSetYB=0.0;
int zoomB=0; //whether or not we are zooming
double zoomAmountB=1.0;
double zoomOffSetYB = 0.0;

//pui globals
puOneShot *ok;
puDialogBox *popupBox=NULL;
puComboBox *funcItems;
puOneShot *chngFuncBtn;

//etc
#ifndef PARAM_S
#define PARAM_S
struct parameter {
  string name;
  string type;
};
#endif

/* ******************** GLUT WINDOW CALL BACKS ******************** */

//main window

//Main Window's display function
void dspfnMain(){
  glClearColor( 0.9, 1.0, 0.9, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  puDisplay();
    
  glutSwapBuffers();
  glutPostRedisplay();
}

//Main Window's motion function
void motionfnMain(int x, int y){
  puMouse(x, y);
  glutPostRedisplay();
}
//Main Window's mouse function
void mousefnMain(int button, int updown, int x, int y){
  puMouse(button, updown, x, y );
  glutPostRedisplay();
}

//window a (original image)

//WinA (original image) display function
void dispfnWinA(){ 
  if(loadedImg!=NULL){
    //take a look at the opengl manual if you don't know whats going on...
    //as for a good explantion of the 
    //loadedImg->A, B, C, D stuff, checkout dspWin.h
    glClearColor( 0.9, 1.0, 0.9, 1.0 );
    glClear(GL_COLOR_BUFFER_BIT);
    glPixelZoom(zoomAmountA,zoomAmountA);
    glPixelStoref(GL_UNPACK_ALIGNMENT, 1);
    glRasterPos2i(-1,-1);
    glDrawPixels(loadedImg->A->width(),
		 loadedImg->A->height(),
		 GL_RGB,
		 GL_UNSIGNED_BYTE,
		 loadedImg->D);
    glutSwapBuffers();
    glutPostRedisplay();
  }
}

//WinA (original image) mouse click function
//This function sets up the zoom and panning
void mousefnWinA(int button, int updown, int x, int y){
  //for zoom
  if(glutGetModifiers() == GLUT_ACTIVE_SHIFT){
    glutSetCursor(GLUT_CURSOR_DESTROY);
    if(updown == GLUT_DOWN){
      if(zoomA!=1){ 
	zoomA = 1; //zoom is for telling if zoom is on or off
	posHeightA=y;
      }
    }
    else{
      glutSetCursor(GLUT_CURSOR_INHERIT);
      zoomA = 0;
    }
    return; //we don't want people panning and zooming at the same time -- that'd just be insane!
  }

   //if someone let's off of the shift, but not on the mouse button we need to stop the zooming
  //NEED A FIX: this takes care of the problem after the fact, so that if someone starts zooming
  //again, after they stopped zooming with a shift letup then a mouse button letup, but it would be nice
  // to make it stop zooming when they let up on the shift key
  if(updown == GLUT_DOWN && zoomA==1 && glutGetModifiers() != GLUT_ACTIVE_SHIFT){
    glutSetCursor(GLUT_CURSOR_INHERIT);
    zoomA = 0;
    return;
  }
  
  //for the pan
  if(updown == GLUT_DOWN){
    if(mouseOnA == 1)
      return;
    else{
      mouseOnA = 1;
      posWidthA = x;
      posHeightA = y;
      glutSetCursor(GLUT_CURSOR_INFO);
    }
  }
  else if (updown == GLUT_UP){ 
    glutSetCursor(GLUT_CURSOR_INHERIT);
    mouseOnA = 0;
  }
}

//WinA (original image) motion function
//This function zooms and pans the image
void motionfnWinA(int x, int y ){
 //for the zoom 
  if(zoomA==1){
    //we moved up a pixel so zoom in
    if((double)(posHeightA-y) > 0){
      posHeightA = y;
      zoomAmountA += .1;
    }
    //we moved down a pixel so zoom out
    if((double)(posHeightA-y)< 0){
      posHeightA = y;
      zoomAmountA -= .1;
      //check to see if we're zooming outside of our bounds
      float tmp = 1.0/zoomAmountA;
      tmp = 1 - tmp;
      if(offSetXA > ((double)loadedImg->A->width() * tmp)) offSetXA = ((double)loadedImg->A->width() * tmp);
      if(offSetYA > ((double)loadedImg->A->height() * tmp)) offSetYA = ((double)loadedImg->A->height() * tmp);
      glPixelStoref(GL_UNPACK_SKIP_ROWS, offSetYA);
      glPixelStoref(GL_UNPACK_SKIP_PIXELS, offSetXA);
    }
    
    //don't zoom too much, or out too far
    if(zoomAmountA < 1.0) zoomAmountA = 1.0;
    if(zoomAmountA > 10.0) zoomAmountA = 10.0;
  
    glPixelZoom(zoomAmountA,zoomAmountA);
    glutPostRedisplay();
    glutSwapBuffers();
    return;
  }
  
  else if (zoomAmountA == 1.0) return; //we cannot pan if zoom is 1
  
  //for the pan 
  else{
    /*REMEMBER TO TEST PAN ON LENA AND STOP (AND WITH LITTLE ZOOM AMOUNTS AND BIG ONES)*/
    /*AND TEST ON aLena.ppm CUZ SHE FUCKS UP ON MACS */
   
    //height -- move up move image cutoff up / reverse for down
    if((double)(posHeightA-y) > 0) offSetYA -= 1*zoomAmountA;
    if((double)(posHeightA-y) < 0) offSetYA += 1*zoomAmountA;

    //width -- move left move image cutoff left / reverse for right
    if((double)(posWidthA-x) > 0) offSetXA += 1*zoomAmountA;
    if((double)(posWidthA-x) < 0) offSetXA -= 1*zoomAmountA;

    //reset current width and height for next round
    posHeightA = y;
    posWidthA = x;
    
    //tmp stuff for now
    float tmp = 1.0/zoomAmountA;
    tmp = 1 - tmp;
    
    //don't try to pan too far...
    if(offSetXA > ((double)loadedImg->A->width() * tmp)) offSetXA = ((double)loadedImg->A->width() * tmp);
    if(offSetYA > ((double)loadedImg->A->height() * tmp)) offSetYA = ((double)loadedImg->A->height() * tmp);
    if(offSetXA<0)offSetXA=0;
    if(offSetYA<0)offSetYA=0;

    glPixelStoref(GL_UNPACK_SKIP_ROWS, offSetYA);
    glPixelStoref(GL_UNPACK_SKIP_PIXELS, offSetXA);
    glutPostRedisplay();
    glutSwapBuffers();
  } 
}

//window b (output image)

//WinB (output image) display function
void dispfnWinB(){
  if(loadedImg!=NULL){
    //take a look at the opengl manual if you don't know whats going on...
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
}

//WinB (output image) mouse click function
//This function sets up the zoom and panning
void mousefnWinB(int button, int updown, int x, int y){
  //for zoom
  if(glutGetModifiers() == GLUT_ACTIVE_SHIFT){
    glutSetCursor(GLUT_CURSOR_DESTROY);
    if(updown == GLUT_DOWN){
      if(zoomB!=1){ 
	zoomB = 1; //zoom is for telling if zoom is on or off
	posHeightB=y;
      }
    }
    else{
      glutSetCursor(GLUT_CURSOR_INHERIT);
      zoomB = 0;
    }
    return; //we don't want people panning and zooming at the same time -- that'd just be insane!
  }

   //if someone let's off of the shift, but not on the mouse button we need to stop the zooming
  //NEED A FIX: this takes care of the problem after the fact, so that if someone starts zooming
  //again, after they stopped zooming with a shift letup then a mouse button letup, but it would be nice
  // to make it stop zooming when they let up on the shift key
  if(updown == GLUT_DOWN && zoomB==1 && glutGetModifiers() != GLUT_ACTIVE_SHIFT){
    glutSetCursor(GLUT_CURSOR_INHERIT);
    zoomB = 0;
    return;
  }
  
  //for the pan
  if(updown == GLUT_DOWN){
    if(mouseOnB == 1)
      return;
    else{
      mouseOnB = 1;
      posWidthB = x;
      posHeightB = y;
      glutSetCursor(GLUT_CURSOR_INFO);
    }
  }
  else if (updown == GLUT_UP){ 
    glutSetCursor(GLUT_CURSOR_INHERIT);
    mouseOnB = 0;
  }
}

//WinB (output image) motion function
//This function zooms and pans the image
void motionfnWinB(int x, int y ){
 //for the zoom 
  if(zoomB==1){
    //we moved up a pixel so zoom in
    if((double)(posHeightB-y) > 0){
      posHeightB = y;
      zoomAmountB += .1;
    }
    //we moved down a pixel so zoom out
    if((double)(posHeightB-y)< 0){
      posHeightB = y;
      zoomAmountB -= .1;
      //check to see if we're zooming outside of our bounds
      float tmp = 1.0/zoomAmountB;
      tmp = 1 - tmp;
      if(offSetXB > ((double)loadedImg->A->width() * tmp)) offSetXB = ((double)loadedImg->A->width() * tmp);
      if(offSetYB > ((double)loadedImg->A->height() * tmp)) offSetYB = ((double)loadedImg->A->height() * tmp);
      glPixelStoref(GL_UNPACK_SKIP_ROWS, offSetYB);
      glPixelStoref(GL_UNPACK_SKIP_PIXELS, offSetXB);
    }
    
    //don't zoom too much, or out too far
    if(zoomAmountB < 1.0) zoomAmountB = 1.0;
    if(zoomAmountB > 10.0) zoomAmountB = 10.0;
  
    glPixelZoom(zoomAmountB,zoomAmountB);
    glutPostRedisplay();
    glutSwapBuffers();
    return;
  }
  
  else if (zoomAmountB == 1.0) return; //we cannot pan if zoom is 1
  
  //for the pan 
  else{
    /*REMEMBER TO TEST PAN ON LENA AND STOP (AND WITH LITTLE ZOOM AMOUNTS AND BIG ONES)*/
    /*AND TEST ON aLena.ppm CUZ SHE FUCKS UP ON MACS */
   
    //height -- move up move image cutoff up / reverse for down
    if((double)(posHeightB-y) > 0) offSetYB -= 1*zoomAmountB;
    if((double)(posHeightB-y) < 0) offSetYB += 1*zoomAmountB;

    //width -- move left move image cutoff left / reverse for right
    if((double)(posWidthB-x) > 0) offSetXB += 1*zoomAmountB;
    if((double)(posWidthB-x) < 0) offSetXB -= 1*zoomAmountB;

    //reset current width and height for next round
    posHeightB = y;
    posWidthB = x;
    
    //tmp stuff for now
    float tmp = 1.0/zoomAmountB;
    tmp = 1 - tmp;
    
    //don't try to pan too far...
    if(offSetXB > ((double)loadedImg->A->width() * tmp)) offSetXB = ((double)loadedImg->A->width() * tmp);
    if(offSetYB > ((double)loadedImg->A->height() * tmp)) offSetYB = ((double)loadedImg->A->height() * tmp);
    if(offSetXB<0)offSetXB=0;
    if(offSetYB<0)offSetYB=0;

    glPixelStoref(GL_UNPACK_SKIP_ROWS, offSetYB);
    glPixelStoref(GL_UNPACK_SKIP_PIXELS, offSetXB);
    glutPostRedisplay();
    glutSwapBuffers();
  } 
}

//shared

//All Window's keyboard function 
void keyb(unsigned char key, int x, int y){
  puKeyboard(key, PU_DOWN); //so we need this on our keyboard thing to have that work....
  glutPostRedisplay(); //JACK THIS OUT OF HERE LATER
   
  //keyboard shortcuts
  int KEYCONST = 4; //this means alt key, ctrl is 2, but for some reason it doesnt work...
  
  if(key=='q'){ //quit
    if(glutGetModifiers()==KEYCONST)
      exitProgram();//does it matter what we pass it? / will this affect performance? we could just make a null puObject
  }

 if(key=='w'){ //close
    if(glutGetModifiers()==KEYCONST)
      closeCB(mainMenu);
  }

  if(key=='o'){ //open
    if(glutGetModifiers()==KEYCONST)
      openCB(mainMenu);
  }
  
  if(key=='s'){ //save
    if(glutGetModifiers()==KEYCONST)
      saveCB(mainMenu);
  }
}

/* ************************* PUI CALLBACKS ************************* */

//file menu callbacks

//FILE MENU -- CLOSE CALLBACK
void closeCB(puObject*){
  if(loadedImg!=NULL){
    glutDestroyWindow(winA);
    glutDestroyWindow(winB); 
    deleteDspWin(loadedImg);
    loadedImg = NULL;
  }
}

//FILE MENU -- EXIT CALLBACK
void exitCB(puObject*){
  exitProgram();
}

//FILE MENU -- OPEN CALLBACK
void openCB(puObject*){
  openDialogBox = new puFileSelector(0, 0, 252, 324, "", "Please select an image");
  openDialogBox->setInitialValue(""); //make this pretty later
  openDialogBox->setChildBorderThickness(PUCLASS_INPUT, 1);
  openDialogBox->setCallback(openFileWinCB);
}

//FILE MENU -- SAVE AS CALLBACK
void saveCB(puObject*){
  if(loadedImg != NULL){
    openDialogBox = new puFileSelector(0, 0, 252, 324, "", "Please type where you want to save");
    openDialogBox->setInitialValue(""); //make this pretty later
    openDialogBox->setChildBorderThickness(PUCLASS_INPUT, 1);
    openDialogBox->setCallback(saveWinCB);
  }
}

//help menu callbacks

//HELP MENU -- about callback
void aboutCB(puObject*){
  makePopup("This program is the \nculmination of 10 \nweeks of work by \nJosh Schwartz and \nTrevor Smith. Thanks \ngo to Yitz and Rio, \nfor answering our \nmultitude of questions,\nand especially to Yitz \nfor suggesting that \nwriting code for \nXServer is a bad \nidea. Thanks to Pedro \nfor believing that we \ncould create this \nprogram.");
}

//HELP MENU -- help callback
void helpCB(puObject*){
  makePopup("help? haha. not here.\n");
}

//pop up box callbacks

//OPEN DIALOG BOX CALLBACK
void openFileWinCB(puObject*){
  char* fileName = new char[80];
  openDialogBox->getValue(&fileName);
  
  //check to see if the cancel button was hit (is this string conv neccesary??)
  string tmp = fileName;
  if(tmp.size() == 0){
    puDeleteObject(openDialogBox);
    return;
  }
  openFile(fileName); //we don't want to delete the filName, cuz the glut windows use it
}

//SAVE FILE AS DIALOG BOX CALLBACK
void saveWinCB(puObject*){
  char *tmp = new char[200];
  openDialogBox->getValue(tmp);    
  
  //check to see if the cancel button was hit (is this string conv neccesary?)
  string fileName = tmp;
  
  //checks for cancel button
  if(fileName.size() == 0){
    puDeleteObject(openDialogBox);
    return;
  }
  
  saveDspWin(loadedImg, tmp);//from dspWin.h -- actually saves the image
  
  puDeleteObject(openDialogBox); //remove dialog box from screen
  delete[] tmp;
}

//main window

//call function callback
void callFuncCB(puObject*){
  //TO DO: we should probably add some check to make sure there are values in the params...

  //check to make sure there's an image loaded
  if(loadedImg == NULL) return;

  //create a list of the param values (in the right order) for the function call
  char *params[paramsWinObjects.size()+4]; //we add 4 to the size cuz the first element is the function name, the last element is a null value, plus in and out files
  
  int i = 1; //remember, 0 is the func name
  unsigned int j = 0;
  //loop over the param values from the window
  while(j < paramsWinObjects.size()){
    char *tmp = new char[80]; //PUI only lets an input string be 80 chars long anyway...
    paramsWinObjects[j]->getValue(tmp);
    params[i++] = tmp;
    j++;
  }

  params[0] = loadedFunctions[curFunction].name;
  params[paramsWinObjects.size()+1] = "/tmp/tmp.lime"; //infile -- this will be trashed
  params[paramsWinObjects.size()+2] ="/tmp/tmp2.lime"; //outfile -- this will be trashed
  params[paramsWinObjects.size()+3] = '\0';

  glutSetCursor(GLUT_CURSOR_WAIT); //make a cool wait cursor for while the function is running
  callFunct(loadedImg, curFunction, params); //from dspWin.h
  glutSetCursor(GLUT_CURSOR_INHERIT);
 
  //now redisplay c in winb
  glutSetWindow(winB);
  glutPostRedisplay();
  glutSwapBuffers();
}

//change function callback
void chngFuncCB(puObject*){
  char *tmp = new char[80];
  funcItems->getValue(tmp);
  int num =  funcMap[tmp];
  dspFuncParams(num);
}

//misc

//popup ok callback -- removes a popup (dialog box) from the screen
void hidePopupCB(puObject *){
  puDeleteObject(popupBox);
  //delete popupBox;
  popupBox = NULL;
}

/* ************************** FUNCTIONS *************************** */

//display a function's parameters on the main window
void dspFuncParams(int num){
  //we need to free up the memory that was allocated for the last function that was displayed
  vector<puObject*>::iterator start = paramsWinObjects.begin();
  while(start!=paramsWinObjects.end()){
    puDeleteObject(*start);
    start++;
  }
  paramsWinObjects.clear();

  //ok, now for the new function
  curFunction = num;

  puFrame *box;
  box = new puFrame(0,0,252,280);
  box->setColour(PUCOL_BACKGROUND, 1,1,1, 1);
  box->setStyle(PUSTYLE_BOXED);
  box->setBorderThickness(1);
  
  //loop over the params of the function  
  vector<pairBuff10>::const_iterator it = loadedFunctions[num].params.begin();
  
  //loop over the params vector, creating fun widgets as we go (pui, like communism, is a nice idea, but in practice it can make people's lives hell)
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
    //so the type of variable wasn't written correctly in the .lime file
    else  cout << "FUNCTION PARSE ERROR IN VARIABLE: " << it->first << " UNKNOWN TYPE: " << it->second << endl;
    it++;
  }
  puOneShot *ok = new puOneShot(x,y,"Run");
  ok->setBorderThickness(2);
  ok->setCallback(callFuncCB);
}

//exit the program
void exitProgram(void){
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

//make a popup (aka dialog box)
void makePopup(const char *txt){
  if(popupBox != NULL) return;
  
  popupBox = new puDialogBox(25, 25); //this is just strange pui syntax
  {
    puFrame *tmp1 = new puFrame(0, 0, 200, 225);
    tmp1->setBorderThickness(3);
    puText *tmp = new puText(10, 200);
    tmp->setLabel(txt);
    
    puOneShot *ok = new puOneShot (160, 10, "OK" ) ;
    ok->makeReturnDefault(TRUE);
    ok->setCallback(hidePopupCB);
  }
  popupBox->close();
  popupBox->reveal();
}

//open a file
void openFile(char* fileName){
  //check to see if we've already got an image, if so delete it
  if(loadedImg != NULL){
    glutDestroyWindow(winA);
    glutDestroyWindow(winB);
    deleteDspWin(loadedImg);
  }
  
  loadedImg = initDspWin(fileName); //from dspWin.h
  if(loadedImg == 0) return;

  //reset the pan and zoom variable BEFORE loading images onto screens
  //for winA
  mouseOnA = 0;
  posWidthA = 0;
  posHeightA = 0;
  newOffXA=0.0;
  newOffYA=0.0;
  offSetXA=0.0;
  offSetYA=0.0;
  zoomA=0;
  zoomAmountA=1.0;
  zoomOffSetYA=0.0;

  //for winB
  mouseOnB = 0;
  posWidthB=0;
  posHeightB=0;
  newOffXB=0.0;
  newOffYB=0.0;
  offSetXB=0.0;
  offSetYB=0.0;
  zoomB=0;
  zoomAmountB=1.0;
  zoomOffSetYB = 0.0;

  //create the windows for it, booh ya ka-sha!
  glutInitWindowSize(loadedImg->A->height(), loadedImg->A->width());
  string nameA = "Original: ";
  nameA += fileName;
  winA = glutCreateWindow(nameA.c_str());
  glutDisplayFunc(dispfnWinA);
  glutKeyboardFunc(keyb);
  //for zoom, pan stuff
  glutMotionFunc(motionfnWinA);
  glutMouseFunc(mousefnWinA);

  glutInitWindowSize(loadedImg->A->height(), loadedImg->A->width());
  string nameB = "Output: ";
  nameB += fileName;
  winB = glutCreateWindow(nameB.c_str());
  glutDisplayFunc(dispfnWinB);
  glutKeyboardFunc(keyb);
  //for zoom, pan stuff
  glutMotionFunc(motionfnWinB);
  glutMouseFunc(mousefnWinB);
 
  if(openDialogBox!=0)
    puDeleteObject(openDialogBox);
  //note: we don't want to delete the fileName variable, cuz the glut windows use it
}

/* ********************** LET'S ROCK AND ROLL ********************* */

int main ( int argc, char **argv ){

  /*deal with incorrect calls*/

  //function must have at least a .lime file, and no more than a .lime and a pnm
  if (argc<2||argc>3) {
    cout << "Error opening limelight, correct syntax is:\nlimelight .lime-absolute-path [PNMimage]\n";
    return 1;
  }
  int i=0;
  //check to make sure configfile is .lime
  while (argv[1][++i]!='\0');
  if (argv[1][i-1]!='e'||argv[1][i-2]!='m'||argv[1][i-3]!='i'||argv[1][i-4]!='l'){
    cout << "ERROR: Configuration file must be .lime file\n";
    return 1;
  }
  /*check to make sure that .lime is actually a valid file - DO THIS.  it needs to happen
    within the function that reads the .lime in, since even something with the correct extension
    could still be messed up*/
  
  if (argc==3){
    i=0;
    //if file doesn't end in .pgm, .ppm, or .pbm, we can't read it
    while (argv[2][++i]!='\0');
    if (argv[2][i-1]!='m'||(argv[2][i-2]!='g'&&argv[2][i-2]!='p'&&argv[2][i-2]!='b'&&argv[2][i-2]!='b')||argv[2][i-3]!='p'){
      cout << "ERROR: The image given is not a valid PNM image\n";
      return 1;
    }
  }
  
  //here's some performance improvements (WOW!, that's a LOT quicker)
  //i think that we only need to disable them once (not for every window)
  //framebuffer ops we don't need
  
#ifndef __APPLE__ //apple doesn't seem to like this guys
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
#endif

  //intialize the limelight function params window aka main window
  glutInitWindowSize ( 250, 400 ) ;
  glutInit ( &argc, argv ) ;
  glutInitDisplayMode ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;
  mainWin = glutCreateWindow ("limelight") ;
  glutDisplayFunc ( dspfnMain ) ;
  glutMouseFunc ( mousefnMain ) ;
  glutMotionFunc ( motionfnMain ) ;
  glutKeyboardFunc(keyb);

  glutSetWindow(mainWin);
  puInit();
  puDisplay();  
  
  //read in functions and make the drop down menu
  createMenu(argv[1], loadedFunctions); //load 'er up
  vector<function>::const_iterator it = loadedFunctions.begin();
  char **functionList = new (char*)[loadedFunctions.size()];

  i=0;
  while(it != loadedFunctions.end()){
    functionList[i] = (char*)it->name;
    funcMap[(char*)it->name] = i++; //map for drop down callback
    it++;
  }
  
  functionList[loadedFunctions.size()] = NULL; //again, this needs to be set to null for pui...
 
  funcItems = new puComboBox( 15, 335, 215, 360, functionList, FALSE ) ;
 
  chngFuncBtn = new puOneShot(15, 300, "Change Function");
  chngFuncBtn->setBorderThickness(2);
  chngFuncBtn->setCallback(chngFuncCB);
  
  //draw a line for the top of the screen
  puFrame *top =  new puFrame(0,375,250,402);
  top->setColour(PUCOL_BACKGROUND, 1,1,1,1);
  top->setStyle(PUSTYLE_BOXED);
  top->setBorderThickness(1);  

  //help menu
  char *help_submenu[4] = {"Help", "-----", "About", NULL};
  puCallback help_submenu_cb [4] = {helpCB, NULL, aboutCB, NULL};

  //file menu 
  char *file_submenu[7] = {"Exit           Alt+Q", "--------------------", "Save Output    Alt+S", "--------------------","Close          Alt+W", "Open           Alt+O", NULL};
  puCallback file_submenu_cb [7] = { exitCB, NULL, saveCB, NULL, closeCB, openCB, NULL};
  
  //make the main menu
  mainMenu = new puMenuBar( -1);
  mainMenu->add_submenu( "File", file_submenu, file_submenu_cb);
  mainMenu->add_submenu( "Help", help_submenu, help_submenu_cb);
  mainMenu->close ();

  //load the first function on the list into the main window
  dspFuncParams(0);
  
  if(argc > 2) openFile(argv[2]);  //the program was called with an image, so open it

  glutMainLoop () ;

  return 0 ;
}
