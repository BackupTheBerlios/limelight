#ifndef DSPWIN_H
#define DSPWIN_H

#ifndef PNM_FILE_H
#include "pnmfile.h"
#endif

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

//This is the struct for holding the parts of the image process
//We need to de-templatize image class from image.h
struct dspWin {
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


//initialize dspWin with Path
dspWin* initDspWin(char *filePath) {
  dspWin *tmp = new dspWin;
  
  /*this won't work unless we change Pedro's  pnmfile.h
    so that it will read any image without caring if they are pgm, ppm, etc.
    ASK HIM ABOUT IT */

  /*For now (3/5/05), this is just going to assume the image is a PPM*/


  //A is an image loaded from filePath
  //this constructor doesn't exist either - damn!
  tmp->A = loadPPM(filePath);

  //tmp B is initially a copy of A
  //But Pedro's image class doesn't have a copy constructor
  tmp->B = tmp->A->copy();;

  /*
    C is the image as we'll want it for GL:
    But, do they need to be in RGB (as I'm doing tonight), or should
    they be in GRB or whatever? 
  */

  get_C_ready(tmp);
  return tmp;
}

//SHOULD THE NEXT FUNCTION TAKE A POINTER OR A REFERENCE?
//function that updates C
void get_C_ready (dspWin *target) {
  int width = target->B->width();
  int height = target->B->height();
  int count = 0;  //counter for loop
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
  delete src->A;
  src->A = src->B->copy();
}

/*Function that deletes a dspWin */
void deleteDspWin(dspWin *target) {
  delete target->A;
  delete target->B;
  delete [] target->C;
}

#endif
