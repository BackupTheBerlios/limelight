#ifndef DSPWIN_H
#define DSPWIN_H

#ifndef PNM_FILE_H
#include "pnmfile.h"
#endif

#include <fstream>
#include <unistd.h>
#include <sys/wait.h>

/*
  This header holds the struct that will be put into a vector
  of all currently loaded images.  A dspWin is the structure of:
  Stage A : Image X as it is currently on disk - Will be displayed in "Original" window
  Stage B : Copy of Stage A that will be edited by 
  Stage C : The data in B as OpenGL needs it
  Stage D : The data in A as OpenGL needs it
*/


/****************************Globals******************/

//vector of functions, as defined in functRead.h, needed for menu and for forking below
vector<function> loadedFunctions; 

//This is the struct for holding the parts of the image process
struct dspWin {
  char *path;
  image* A;  //in image
  image* B;  //out image
  unsigned char *C;  //in image in gl form
  unsigned char *D;  //out image in gl form
  int winNum;  
};

/************************End Globals**************/



/***********************Prototypes***************/
dspWin* initDspWin(char *filePath);
void get_C_ready (dspWin *target);
void get_D_ready (dspWin *target);
void saveDspWin(dspWin *src, char *filePath);
void saveTMP(dspWin *src);
void deleteDspWin(dspWin *target);
void callFunct(dspWin *target, int i, char **par);
void load2B(dspWin *target, char *filePath);
void getCHelpPPM(dspWin *target);
void getCHelpPGM(dspWin *target);
void getCHelpPBM(dspWin *target);
void getDHelpPPM(dspWin *target);
void getDHelpPGM(dspWin *target);
void getDHelpPBM(dspWin *target);

/********************End Prototypes*************/



/********************Function definitions******************/

//initialize dspWin from disk, using specified path
dspWin* initDspWin(char *filePath) {
  dspWin *tmp = new dspWin;
  tmp->path = new char[70];

  //loop to copy filePath into dspWin's path
  for (int i = 0; filePath[i]!='\0'; i++)
    tmp->path[i] = filePath[i];

  //initialize C and D to NULL
  tmp->C=0; tmp->D=0;

  //open file
  ifstream input(filePath, ios::in);

  //make sure we got a good file
  if(!input.is_open()){
    cout << "ERROR OPENING FILE\n";
    return 0;
  }

  //check second character (P_) - if it's 6, then this is a ppm, if 5, then pgm, 4 - pbm
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
  case '4' :
    tmp->A = loadPBM(filePath);
  }
  
  //tmp B is initially a copy of A
  tmp->B = tmp->A->copy();
  
  //prepares C in GL format
  get_C_ready(tmp);
  
  //prepares D in GL format
  get_D_ready(tmp);
  return tmp;
}

//function that moves data in B to GL format in C
void get_C_ready (dspWin *target) {
  //get height, width, number of pixels
  int width = target->B->width();
  int height = target->B->height();
  
  //in case C is already initialized, delete it's value
  if (target->C!=0) delete[] target->C;
  
  //All images for GL are RGB, so we need 3*pixels to store data
  target->C = new unsigned char[width*height*3];
  
  //check if image is already RGB or not
  if (target->A->isRGB==3) getCHelpPPM(target);
  else if (target->A->isRGB==2) getCHelpPGM(target);
  else getCHelpPBM(target);
  return;
}



//function that moves data in B to GL format in D
void get_D_ready (dspWin *target) {
  //
  int width = target->B->width();
  int height = target->B->height();
  
  //in case D is already initialized, delete it's value
  if (target->D!=0) delete[] target->C;
  
  //All images for GL are RGB, so we need 3*pixels to store data
  target->D = new unsigned char[width*height*3];
  
  //check if image is already RGB
  if (target->A->isRGB==3) getDHelpPPM(target);
  else if (target->A->isRGB==2) getDHelpPGM(target);
  else getDHelpPBM(target);
  return;
}

//Loads PPM image into C
void getCHelpPPM(dspWin *target) {
  //counter variables  
  int width = target->B->width();
  int height = target->B->height();
  int count=0;  //counter for loop
  int loc = 0; //counter
  
  //loop to load data
  for (int i=height-1; i>=0; i--) {
    loc = i*width*3;
    for (int j=0; j<width*3; j++)
      target->C[count++] = target->B->data[loc++];
  }
  return;
}

//Loads PGM image into C
void getCHelpPGM(dspWin *target) {
  //counter variables
  int width = target->B->width();
  int height = target->B->height();
  int count = 0;  //counter for loop
  char helper;
  
  //loop to load data - throws greyscale value of pixel into R, G, and B
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


//Loads PBM image into C
void getCHelpPBM(dspWin *target) {
  //counter variables
  int width = target->B->width();
  int height = target->B->height();
  int count = 0;  //counter for loop
  char helper;
  
  //loop to load data - throws black or white value of pixel into R, G, and B
  for (int i=height-1; i>=0; i--) {
    for (int j=0; j<width; j++) {
      helper = target->B->data[(i*width)+j]==1?0:255;
      target->C[count++] = helper;
      target->C[count++] = helper;
      target->C[count++] = helper;
    }
  }
  return;
}

//Loads PPM image into D
void getDHelpPPM(dspWin *target) {
  //counter variables
  int width = target->A->width();
  int height = target->A->height();
  int count = 0;  //counter for loop
  int loc = 0; //counter
  
  //loop to load data
  for (int i=height-1; i>=0; i--) {
    loc = i*width*3;
    for (int j=0; j<width*3; j++)
      target->D[count++] = target->A->data[loc++];
  }
  return;
}

//Loads PGM image into D
void getDHelpPGM(dspWin *target) {
  //counter variables
  int width = target->A->width();
  int height = target->A->height();
  int count = 0;  //counter for loop
  unsigned char helper;
  
  //loop to load data - throws greyscale value of pixel into R, G, and B
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


//Loads PBM image into D
void getDHelpPBM(dspWin *target) {
  //counter variables
  int width = target->A->width();
  int height = target->A->height();
  int count = 0;  //counter for loop
  unsigned char helper;
  
  //loop to load data - throws black or white value of pixel into R, G, and B
  for (int i=height-1; i>=0; i--) {
    for (int j=0; j<width; j++) {
      helper = target->A->data[(i*width)+j]==1?0:255;
      target->D[count++] = helper;
      target->D[count++] = helper;
      target->D[count++] = helper;
    }
  }
  return;
}


//function that saves B
void saveDspWin(dspWin *src, char *filePath) {
  if (src->A->isRGB==3)
    savePPM(src->B, filePath);
  else if (src->A->isRGB==2)
    savePGM(src->B, filePath);
  else
    savePBM(src->B, filePath);
  return;
}

//function that saves B as tmp
void saveTMP(dspWin *src) {
  if (src->A->isRGB==3)
    savePPM(src->A, "/tmp/tmp.lime");
  else if (src->A->isRGB==2)
    savePGM(src->A, "/tmp/tmp.lime");
  else
    savePBM(src->A, "/tmp/tmp.lime");
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
//file is stored at ./tmp.lime (in) and tmp2.lime (out)
//these are imbedded in params
void callFunct(dspWin *target, int i, char **par){
  //save temporary copy
  saveTMP(target);
    
  //forking
  int pid, childStatus;
  int died, status; //status is set by the call to exec
  
  switch(pid=fork()){
  case -1: //error
    exit(-1);
  case 0: //0 means a child process
    childStatus = execv(loadedFunctions[i].path, par);
  default:
    died = wait(&status);
  }
  //end fork
  
  //error with the executable
  if(childStatus == -1){
    cout << "ERROR WITH BINARY FILE\n(you probably just wrote the wrong path name in your functions definitions file)\n";
    return ;
  }

  //load file to B
  load2B(target, "/tmp/tmp2.lime");

  //delete tmp files
  //FORK
  char* temp[4];
  temp[0] = "rm";
  temp[1] = "/tmp/tmp.lime";
  temp[2] = "/tmp/tmp2.lime";
  temp[3] = '\0';
  
  switch(pid=fork()){
  case -1: //error
    exit(-1);
  case 0: //0 means a child process
    execv("/bin/rm", temp);
  default:
    wait(&status);
  }
  //END FORK

  //load current contents of B into C
  get_C_ready(target);
  return;
}

//Function that loads file into B
void load2B(dspWin *target, char *filePath) {
  delete target->B;
  if (target->A->isRGB==3)
    target->B = loadPPM(filePath);
  else if (target->A->isRGB==2)
    target->B = loadPGM(filePath);
  else
    target->B = loadPBM(filePath);
  return;
}

#endif
