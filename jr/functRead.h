//Function to read in the user defined functions file (funct.fuk)
//Created on 2/22/05
//Edited:

/*trevor's random thoughts (updated):
 
 *the glut menu function that creates a new menu item glutMenuAddEnty(char, int) the second param is what we have to work with
  so i'm thinking that we WILL have to create a vector of the functions that we loaded into twoc from the func.fuk file
  this will be an array of the data structure that we previously discussed.

  *we need to make this a header file at some point (do we do includes here, or is that not kosher?)
*/

#include <iostream.h>
#include <fstream.h>
#include <string>
#include <list.h>
#include <utility> //for pairs
#include <vector.h> //maybe params should be a vector too?

using namespace std;

//structure def

struct pairBuff10{ //this is here cuz stl pair wont take char*
  char first[10];
  char second[10];
};

struct function{
  char name[128];
  char path[128];
  vector< pairBuff10 > params;
};

//loop thru the file, creating a vector of structs, this array will have to be globally defined
//i think here too, we can just create the menu items as we make the structs
//this prob won't be void either
void createMenu(char *fileName, vector<function> &loadedFunctions){
 
  ifstream fileStream;
  fileStream.open(fileName, ifstream::in);
  
  if(!fileStream.is_open()){
    //add error
    cout << "Error reading function definitions file" << endl;
    exit(1);
  }

  while(fileStream.good()){
    
    function tmpFunction;
   
    //discard comments
    /*char c;
    fileStream.get(c);
    fileStream.unget();
    */

    if(fileStream.peek() == '\n')
      fileStream.ignore();

    while(fileStream.get() == '#'){
      fileStream.ignore(512, '\n'); //throw out the line (512 is arbitrary, we just ignore until we get to newline)
    }
    fileStream.unget(); //the last thing was good so put it back
    
    //at this point we're at line which has a function on it so...
    //do our thang
    
    fileStream.get(tmpFunction.name, 128, ' ');
    fileStream.get(); //kill the space
    fileStream.get(tmpFunction.path, 128, ' ');
    fileStream.get(); //kill the space

    //now read the params, until we get a new line
    /*fileStream.get(c);
    fileStream.unget();
    */
    while(fileStream.get() != '\n' && !fileStream.eof()){
      
      fileStream.unget();

      pairBuff10 tmp;
      
      fileStream.get(tmp.first, 10, ' ');
      fileStream.get(); //kill the space
      fileStream.get(tmp.second, 10, ' ');
      fileStream.get();//kill the space
      tmpFunction.params.push_back(tmp);
      
    }
    
    //at this point we have a nice new function struct
    //here we'd make the glut calls
    loadedFunctions.push_back(tmpFunction);
    
  }
  fileStream.close();
}
