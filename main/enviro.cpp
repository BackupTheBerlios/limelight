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
#include "dspWin.h" // a much better job of naming is going on here


using namespace std;

/*********************ok here we go: this will be a class but i dont feel like making a class right now *************/

//vector<function> loadedFunctions; //this is read in from the funct.fuk file UGLINESS!!!
puMenuBar *mainMenu;
puFileSelector *openDialogBox;
int mainWin;
vector<puObject*> paramsWinObjects;
puDialogBox *paramsWin; //this is an ugly hack for now
vector<dspWin*> imgsOnScr; //this is the images currently on the screen
int curImg;
int w2; // tsk, tsk, so messy, we'll take care of this soon
int curFunction; //set this when a function menu item is called

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
  
  //this displays the images on the screen
  if(!imgsOnScr.empty() && glutGetWindow() == imgsOnScr[curImg]->winNum){
    //cout << "does this get called?\n";
    glCallList(curImg+1);
  }

  /*the above needs to be changed to lists of display lists... check it out below
  it's not really intuitive
  glListBase(0);
  GL_INT lists[] = {1,1,3,5};
  glCallLists(4,GL_INT,lists);
  ok, so that code calls the lists 1, 2, 5 and 10 that were created using glNewList
  just like normal
  */
  
  glutSwapBuffers () ;
  glutPostRedisplay () ;
}

void keyb(unsigned char key, int x, int y){
  puKeyboard(key, PU_DOWN); //so we need this on our keyboard thing to have that work....
  glutPostRedisplay();
}

//file menu -- revertCB
void revertCB(puObject*){
  revert2A(imgsOnScr[curImg]);
  glutSetWindow(imgsOnScr[curImg]->winNum);
  glNewList(curImg+1, GL_COMPILE);
  glRasterPos2i(-1,-1 ); //what is up with this? 0,0 supposed to be lower left corner...
  glDrawPixels(imgsOnScr[curImg]->A->width(), 
	       imgsOnScr[curImg]->A->height(),
	       GL_RGB,
	       GL_UNSIGNED_BYTE,
	       imgsOnScr[curImg]->C);
  glEndList();
  
  cout << glutGetWindow() << "should be : " << imgsOnScr[curImg]->winNum << endl; 
  glutPostRedisplay();

//FILE MENU -- OPEN DIALOG BOX CALLBACK
void openFileCB(puObject*){
  char* fileName;
  openDialogBox->getValue(&fileName);
  
  imgsOnScr.push_back(initDspWin(fileName)); //from dspWin.h
  
  curImg = imgsOnScr.size() - 1;

  //display it??
  glutInitWindowSize(imgsOnScr[curImg]->A->width(), imgsOnScr[curImg]->A->height());
  glutInitWindowPosition(0,0);   
  imgsOnScr[curImg]->winNum = glutCreateWindow(fileName);
 
  //lets go man, out of the frying pan
  
  //OK THIS WORKS NICE FOR A SINGLE FILE
  //BUT WE NEED TO REDO IT TO WORK ON LISTS OF LISTS (glCallLists)
  //FOR MULTIPLE FILES
  
  glNewList(curImg+1, GL_COMPILE);
  glRasterPos2i(-1,-1 ); //what is up with this? 0,0 supposed to be lower left corner...
  glDrawPixels(imgsOnScr[curImg]->A->width(), 
	       imgsOnScr[curImg]->A->height(),
	       GL_RGB,
	       GL_UNSIGNED_BYTE,
	       imgsOnScr[curImg]->C);
  glEndList();
 
  glutDisplayFunc(displayfn);
  glutPostRedisplay();
  
  //the following note was in a pui ex file, it's probably important

  //NOTE: interface creation/deletion must be nested
  //the old interface must be deleted *before* a new one is created
  //otherwise the interface stack will be messed up
  
  puDeleteObject(openDialogBox);
  
  //we need to add some kind of if clause for the cancel button
  //but the cancel button doesn't have it's own callback

  cout << "open the file: " << fileName << endl;
}

//FILE MENU -- OPEN CALLBACK
void openCB(puObject*){
  openDialogBox = new puFileSelector(50, 50, "", "Please select a file");
  openDialogBox->setInitialValue(" ");
  openDialogBox->setChildBorderThickness(PUCLASS_INPUT, 1);
  openDialogBox->setCallback(openFileCB);
}

//PARAMS WINDOW -- CANCEL CALLBACK
void paramsWinCancelCB(puObject*){
  //free up all that memory...
  paramsWinObjects.clear();
  delete paramsWin;
}

//PARAMS WINDOW -- OK CALLBACK
//REMINDER: this is the beef of the program, the reason for its living, make 
//sure that its done well.
void paramsWinOKCB(puObject*){
  
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
  
  callFunct(imgsOnScr[curImg], curFunction, params); //from dspWin.h
  //callFunct runs the function on dspWin, deletes the tmp files, displays the result, DAMN

  

  glutSetWindow(imgsOnScr[curImg]->winNum);
 
  glNewList(curImg+1, GL_COMPILE);
  glRasterPos2i(-1,-1 ); //what is up with this? 0,0 supposed to be lower left corner...
  glDrawPixels(imgsOnScr[curImg]->A->width(), 
	       imgsOnScr[curImg]->A->height(),
	       GL_RGB,
	       GL_UNSIGNED_BYTE,
	       imgsOnScr[curImg]->C);
  glEndList();
  
  cout << glutGetWindow() << "should be : " << imgsOnScr[curImg]->winNum << endl; 
  glutPostRedisplay();

  //make it ready for the next kid
  paramsWinObjects.clear();
  delete paramsWin;
}

//FUNCTION MENU -- CALLBACK
void createParamsWin(int num){
  //this is lame, but we cannot pass anything in here, so we need to gather what function is being called
  //from somewhere else.... (thundercrash) but where?

  curFunction = num;
  
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
    ok->setCallback(paramsWinOKCB);
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

  //read in functions and make a glut style menu
  createMenu("funct.fuk", loadedFunctions); //load 'er up
  vector<function>::const_iterator it = loadedFunctions.begin();
  glutCreateMenu(createParamsWin); //in the future, if we want, this returns an int as an id

  int i=0;
  while(it != loadedFunctions.end()){
    glutAddMenuEntry((char*)it->name, i++);
    it++;
  }

  glutAttachMenu(GLUT_RIGHT_BUTTON);
  //end menu creation

  /*DOESN'T WORK, HOPEFULLY FIX LATER
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
    functSubmenuCB[i++] = createParamsWin; //WHAT GOES HERE???
  //end the a-going-down-ness
  */

  mainMenu = new puMenuBar ( -1 );
  mainMenu->add_submenu ( "File", file_submenu, file_submenu_cb ) ;
  //  mainMenu->add_submenu ( "Functions", functSubmenu, functSubmenuCB);
  mainMenu->close ();

  glutMainLoop () ;

  return 0 ;
}
