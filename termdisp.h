#ifndef TERMDISP_INCLUDED
#define TERMDISP_INCLUDED 1


struct rgbcolour
{
  int r;
  int g;
  int b;
}

union colours_union
{
  struct rgbcolour truecolour;
  int colour16;
};

typedef struct stylesblock_struct
{
  int isbold : 1;
  int isitalics : 1;
  int isundelined : 1;
  int isshadowed : 1;
  int outlinecolour : 2; //0=None, 1=black, 2=white, 3=inverse
  int istruetextcolour : 1;
  int istruebackcolour : 1;
  //union colours_union
  struct rgbcolour textcolour;
  //union colours_union 
  struct rgbcolour backcolour;
  //Size is ignored
} stylesblock;

#include "server.h"


void revertcurrenttonormal(displaynode *adn);
//void setforcedcolourtypespec(int colourtype);
//int runexternal(char *outputstr, int outputstr_size, char *program, char *arg1);
//int updatecolourtypespec();
int writestylefromtag(displaynode *adn, char *atag);


#endif