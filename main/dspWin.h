#ifndef DSPWIN_H
#define DSPWIN_H

#ifndef PNM_FILE_H
#include "pnmfile.h"
#endif


#include <unistd.h>
#include <sys/wait.h>


/*
  This header holds the struct that will probably be put into a vector
  of all currently loaded images.  A dspWin is the structure of:
  Stage A : Image X as it is currently on disk
  Stage B : Unsaved modifications of Stage A
  Stage C : The data in B as OpenGL needs it
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

/*Modified on 3-6 - added ability to call functions on structs by
  saving B, forking into call on saved B, reloading into B, and
  deleting the tmp location of B */

vector<function> loadedFunctions; //this is read in from the funct.fuk file UGLINESS!!!

//This is the struct for holding the parts of the image process
//We need to de-templatize image class from image.h
struct dspWin {
  char *path;
  image<rgb>* A;
  image<rgb>* B;
  unsigned char *C;
  int winNum;
};

//prototypes
void get_C_ready (dspWin *target);
dspWin* initDspWin(char *filePath);
void saveDspWin(dspWin *src, char *filePath);
void deleteDspWin(dspWin *target);
void callFunct(dspWin *target, int i, char **par);
void load2B(dspWin *target, char *filePath);
void revert2A(dspWin *target);
void saveDspWin(dspWin *src);

//initialize dspWin from disk
dspWin* initDspWin(char *filePath) {
  
  dspWin *tmp = new dspWin;
  tmp->path = new char[50];

  for (int i = 0; filePath[i]!='\0'; i++)
    tmp->path[i] = filePath[i];
  //tmp->path = filePath;
  

  /*this won't work unless we change Pedro's  pnmfile.h
    so that it will read any image without caring if they are pgm, ppm, etc.
    ASK HIM ABOUT IT */

  /*For now (3/5/05), this is just going to assume the image is a PPM*/

  //A is an image loaded from filePath
  tmp->A = loadPPM(filePath);
  
  //tmp B is initially a copy of A
  tmp->B = tmp->A->copy();

  //prepares C in GL format
  
  get_C_ready(tmp);
  return tmp;
}

//function that moves data in B to GL format in C
void get_C_ready (dspWin *target) {
  int width = target->B->width();
  int height = target->B->height();
  int count = 0;  //counter for loop

  //in case C is already initialized, delete it's value
  //delete[] target->C;

  //for RGB, it is three bigger
  target->C = new unsigned char[width*height*3];

  //since we want to start at bottom, i starts at height-1, goes to 0
  for (int i=height-1; i>=0; i--) {
    for (int j=0; j<width; j++) {
      target->C[count++] = imRef(target->B, j, i).r;
      target->C[count++] = imRef(target->B, j, i).g;
      target->C[count++] = imRef(target->B, j, i).b;
    }
  }
}

//function that saves B, and loads B into A
//WHAT IS PEDRO'S VLIB THING IN save_image (in pnmfile.h)?
void saveDspWin(dspWin *src, char *filePath) {
  savePPM(src->B, filePath);
  cout <<"file saved succesfully\n";
  delete src->A;
  src->A = src->B->copy();
}

//function that saves B to same location as A, then loads image
void saveDspWin(dspWin *src) {
  savePPM(src->B, src->path);
  cout <<"file saved succesfully\n";
  delete src->A;
  src->A = src->B->copy();
}

//function that saves B as tmp
void saveTMP(dspWin *src) {
  savePPM(src->B, "/tmp/tmp.fuk");
  return;
}

/*Function that deletes a dspWin */
void deleteDspWin(dspWin *target) {
  delete target->A;
  delete target->B;
  delete [] target->C;
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
    //test - REMOVE THIS
    //execv("/home/vegan/seg", par);
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
  target->B = loadPPM(filePath);
  return;
}

/*Function that changes B and C back into copy
  gotten from A */

void revert2A(dspWin *target) {
  delete target->B;
  delete[] target->C;
  target->B = target->A->copy();
  get_C_ready(target);
  return;
}

#endif
