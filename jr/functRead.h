//Function to read in the user defined functions file (funct.fuk)
//Created on 2/22/05


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
void createMenu(char *fileName, vector<function> &loadedFunctions){
 
  ifstream fileStream;
  fileStream.open(fileName, ifstream::in);
  
  if(!fileStream.is_open()){
    cout << "Error reading function definitions file" << endl;
    exit(1);
  }

  while(fileStream.good()){
    
    function tmpFunction;
   
    //discard comments
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
