#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <math.h>
#include <stdlib.h>

#include "readablesize.hpp"

readableSize::readableSize()
{
  this->mysize=0;
}

readableSize::readableSize(std::string &size)
{
  humantosize(size,this->mysize);
}


readableSize::readableSize(size_t size)
{
  this->mysize=size;
}


void readableSize::setSize(std::string &size)
{
  humantosize(size, mysize);
}


void readableSize::setSize(size_t size)
{
  mysize=size;
}

void readableSize::getSize(std::string &size)
{
  sizetohuman(mysize, size);
}


void readableSize::getSize(size_t &size)
{
  size=this->mysize;
}


/*
 * buf must be a pre-allocated buffer with size BUFSIZE+1
 * @Arguments
 * size in bytes
 * @Return
 * returns the char * to the converted string.
 */
void readableSize::sizetohuman(size_t size, std::string &out)
{
  int i = 0;
  double tempsize;
  std::stringstream sizebuf;

  // Sizes to choose from
  static const char* units[]    = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};

  tempsize = size;

  while (tempsize / 1024 > 1) {
    tempsize /= 1024;
    i++;
  }

  sizebuf << std::setprecision(1) << std::fixed << tempsize << units[i];
  sizebuf >> out;
  return;
}


/*
 * buf must be a pre-allocated buffer with size BUFSIZE+1
 * returns 0 and -1 on error
 * size in bytes is returned in argument size
 */
int readableSize::humantosize(std::string &buf, size_t &size)
{
  char    *upcasenum;
  char    *unit=NULL;
  int     i=0;
  long double  tempsize=0.0;
  int     power=0;

  // Conversion table
  static const char* unitmatch  = "BKMGTPEZY";

  // Initialize stuff
  upcasenum = strdup(buf.c_str());

  // transform the human readable string to a power of 1024
  for( i = 0; upcasenum[ i ]; i++)
  {
    upcasenum[ i ] = toupper( upcasenum[ i ] );
  }
  // returns a pointer to the first occurrence in string s1 of any character from string s2, or a null pointer if no character from s2 exists in s1
  unit = strpbrk(upcasenum, unitmatch);
  tempsize = atof(upcasenum);

  // when no unit is found use a power of 1024^0
  // Otherwise calculate number of bytes
  if(unit != NULL){
    // Calculate the number of bytes out.
    while(*(unitmatch + power) != '\0'){
      if(*unit == *(unitmatch + power)) {
        break;
      }
      power++;
    }
  }

  // Calculate response
  size = (double) tempsize * pow(1024, power);

  free(upcasenum);
  return 0;
}
