#ifndef DSPWIN_H
#define DSPWIN_H

#ifndef PNM_FILE_H
#include "pnmfile.h"
#endif

#include <fstream>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;
/*
  This header holds the struct that will probably be put into a vector
  of all currently loaded images.  A dspWin is the structure of:
  Stage A : Image X as it is currently on disk
  Stage B : Unsaved modifications of Stage A
  Stage C : The data in B as OpenGL needs it
  Stage D : The date in A as OpenGL needs it
*/

/*
  Modifications:
  Update Pedro's image class to:
  1) Not be templatized
  2) Have a constructor that opens from a filePath
  3) Have a copy constructor
  4) ???

  Deal with unpacking stuff for PBM files into Stage C

*/

vector<function> loadedFunctions; //this is read in from the funct.fuk file UGLINESS!!!

//This is the struct for holding the parts of the image process

struct dspWin {
  char *path;
  image* A;  //in image
  image* B;  //out image
  unsigned char *C;  //in image in gl form
  unsigned char *D;  //out image in gl form
  int winNum;  
};

//prototypes
dspWin* initDspWin(char *filePath);
void get_C_ready (dspWin *target);
void get_D_ready (dspWin *target);
void saveDspWin(dspWin *src, char *filePath);
void saveDspWin(dspWin *src);
void saveTMP(dspWin *src);
void deleteDspWin(dspWin *target);
void callFunct(dspWin *target, int i, char **par);
void load2B(dspWin *target, char *filePath);
void revert2A(dspWin *target);
void getCHelpPPM(dspWin *target);
void getCHelpPGM(dspWin *target);
void getDHelpPPM(dspWin *target);
void getDHelpPGM(dspWin *target);



//initialize dspWin from disk
dspWin* initDspWin(char *filePath) {
  dspWin *tmp = new dspWin;
  tmp->path = new char[70];

  for (int i = 0; filePath[i]!='\0'; i++)
    tmp->path[i] = filePath[i];

  /*check to see if image is ppm or pgm*/
  //5 is raw pgm, 6 is raw ppm, ...

  //open file
  ifstream input(filePath, ios::in);

  //check second character (P_)
  input.seekg(1, ios::beg);
  char c = input.peek();
  input.close();

  switch (c) {
  case '6' :
    tmp->A = loadPPM(filePath);
    break;
  case '5' :
    tmp->A = loadPGM(filePath);
    break;
  }
  
  //tmp B is initially a copy of A
  tmp->B = tmp->A->copy();

  //prepares C in GL format
  
  get_C_ready(tmp);
  get_D_ready(tmp);
  return tmp;
}



//function that moves data in B to GL format in C
void get_C_ready (dspWin *target) {
  int width = target->B->width();
  int height = target->B->height();
  int pixels = target->B->pixelCount;

  //in case C is already initialized, delete it's value
  //delete[] target->C;

  //for RGB, it is three bigger
  target->C = new unsigned char[width*height*3];
    
  if (pixels==3)
    getCHelpPPM(target);
  else
    getCHelpPGM(target);
  return;
}



//function that moves data in B to GL format in D
void get_D_ready (dspWin *target) {
  int width = target->B->width();
  int height = target->B->height();

  //in case C is already initialized, delete it's value
  //delete[] target->C;

  //for RGB, it is three bigger
  target->D = new unsigned char[width*height*3];

  if (target->A->isRGB) {
    getDHelpPPM(target);
    return;
  }
  else
    getDHelpPGM(target);
  return;
}


void getCHelpPPM(dspWin *target) {
  int width = target->B->width();
  int height = target->B->height();
  int count=0;  //counter for loop
  int loc = 0; //counter
  for (int i=height-1; i>=0; i--) {
    loc = i*width*3;
    for (int j=0; j<width*3; j++)
      target->C[count++] = target->B->data[loc++];
  }
  return;
}

void getCHelpPGM(dspWin *target) {
  int width = target->B->width();
  int height = target->B->height();
  int count = 0;  //counter for loop
  char helper;
  for (int i=height-1; i>=0; i--) {
    for (int j=0; j<width; j++) {
      helper = target->B->data[(i*width)+j];
      target->C[count++] = helper;
      target->C[count++] = helper;
      target->C[count++] = helper;
    }
  }
  return;
}

void getDHelpPPM(dspWin *target) {
  int width = target->A->width();
  int height = target->A->height();
  int count = 0;  //counter for loop
  int loc = 0; //counter
  for (int i=height-1; i>=0; i--) {
    loc = i*width*3;
    for (int j=0; j<width*3; j++)
      target->D[count++] = target->A->data[loc++];
  }
  return;
}

void getDHelpPGM(dspWin *target) {
  int width = target->A->width();
  int height = target->A->height();
  int count = 0;  //counter for loop
  unsigned char helper;
  for (int i=height-1; i>=0; i--) {
    for (int j=0; j<width; j++) {
      helper = target->A->data[(i*width)+j];
      target->D[count++] = helper;
      target->D[count++] = helper;
      target->D[count++] = helper;
    }
  }
  return;
}


//function that saves B, and loads B into A
//WHAT IS PEDRO'S VLIB THING IN save_image (in pnmfile.h)?
void saveDspWin(dspWin *src, char *filePath) {
  if (src->A->isRGB)
    savePPM(src->B, filePath);
  else
    savePGM(src->B, filePath);
  delete src->A;
  src->A = src->B->copy();
}

//function that saves B to same location as A, then loads image
void saveDspWin(dspWin *src) {
  if (src->A->isRGB)
    savePPM(src->B, src->path);
  else
    savePGM(src->B, src->path);
  delete src->A;
  src->A = src->B->copy();
}

//function that saves B as tmp
void saveTMP(dspWin *src) {
  if (src->A->isRGB)
    savePPM(src->A, "/tmp/tmp.fuk");
  else
    savePGM(src->A, "/tmp/tmp.fuk");
  return;
}

/*Function that deletes a dspWin */
void deleteDspWin(dspWin *target) {
  delete target->A;
  delete target->B;
  delete [] target->C;
  delete [] target->D;
}

/*Calls function on B*/
//file is stored at ./tmp.fuk (in) and tmp2.fuk (out)
//these are imbedded in params
void callFunct(dspWin *target, int i, char **par){
  //save temporary copy
  saveTMP(target);
    
  //forking
  int pid;
  int died, status; //status is set by the call to exec

  switch(pid=fork()){
  case -1: //error
    exit(-1);
  case 0: //0 means a child process
    execv(loadedFunctions[i].path, par);
  default:
    died = wait(&status);
  }
  //end fork

  //load file to B
  load2B(target, "/tmp/tmp2.fuk");

  //delete tmp files - not working
  //FORK
  char* temp[4];
  temp[0] = "rm";
  temp[1] = "/tmp/tmp.fuk";
  temp[2] = "/tmp/tmp2.fuk";
  temp[3] = '\0';

  switch(pid=fork()){
  case -1: //error
    cout << "cannot fork\n";
    exit(-1);
  case 0: //0 means a child process
    cout << "forking... \n" << "pid: " << pid << endl;
    execv("/bin/rm", temp);
  default:
    cout << "i'm the parent\n"<<  "pid: " << pid << endl;;
    wait(&status);
    cout << "completed ok \n";
  }
  //END FORK

  //load current contents of B into C
  get_C_ready(target);
  return;
}

//Function that loads file into B
void load2B(dspWin *target, char *filePath) {
  delete target->B;
  if (target->A->isRGB)
    target->B = loadPPM(filePath);
  else
    target->B = loadPGM(filePath);
  return;
}

/*Function that changes B and C back into copy
  gotten from A */
/*
void revert2A(dspWin *target) {
  delete target->B;
  delete[] target->C;
  target->B = target->A->copy();
  get_C_ready(target);
  return;
}
*/

#endif
