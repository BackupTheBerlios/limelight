/*This is the function that takes in information from the GUI and adds functions to
  a file.  If the file doesn't exist, it makes it.
  Information this expects from GUI:
  - Function name
  - Function path
  - Type for each parameter
  - Name for each parameter
  but type and name with be pairs, made into a list of params

  GUI should take name, path, and number of functions on one window, then
  generate a second window based on number of functions entered, so that
  it has the right number of text boxes
*/

/*
  Should this function be templatized?  That is, we could have the GUI instantiate a
  class, which would automatically print a file when it was created
*/

/*Thought - should this function be made to not require a paramlist, so that if
  there isn't one, then it just doesn't output any params?  Or, is it better on both
  sides if the functions just read a blank list? */


// If this is going to use struct, what header should "parameter" be in?


#include <list>
#include <utility>
#include <string>
#include <fstream>

//for tests
#include <iostream>
//WHAT ELSE??

#ifndef PARAM_S
#define PARAM_S
struct parameter {
  string name;
  string type;
};
#endif

void newFunct(string name, string path, list<parameter> &paramList) {
  
  /*Should I make vector of functions? */

  ofstream outFile("/funct.fuk", ios::app);  //open file that will hold function list

  outFile << '\n' << '#' << name << ' ' << path;  //insert name and path

  //output list of params
  list<parameter>::iterator it = paramList.begin();
  list<parameter>::iterator end = paramList.end();
  while (it!=end) {
    outFile << it->name << ' ' << it->type << ' ';
    it++;
  }

  //close file
  outFile.close();
  return;
}
