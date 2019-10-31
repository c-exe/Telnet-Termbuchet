#ifndef SERVER_INCLUDED
#define SERVER_INCLUDED 1

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "libtelnet.h"
#include "termdisp.h"
#include "termstyles.h"


#define PORT_STR "9034"
#define PORT_N 9034

#define BUFFERSIZE 512

#define DISPLAYFLAG_ECHO 1
#define DISPLAYFLAG_SENTDOENVIRON 2
#define DISPLAYFLAG_SENTDONAWS 4
#define DISPLAYFLAG_QRYCOLOURS 8 /* Currently querying colours */

typedef enum colourtype_enum
{
  unknowncolour = 0, //None
  monochrome,    //1 bit = 2 colour
  eight,         //3 bit = 8 colour
  sixteen,       //4 bit = 16 colour
  byte,          //8 bit = 256 colour
  truecolour     //24 bit
} colourtype;

typedef struct databuffer
{
  size_t size;
  char *buffer;
} databuff;

typedef struct displaynode_struct
{
  int socketfd;
  telnet_t *telnet;
  int flags; //b0=Echo, b1=SentDoEnviron, b2=SentDoNAWS, b3=QryColours
  colourtype colours;
  unsigned long width;
  unsigned long height;
  char *terminaltype;
  char *metaformat;  //Format of the song/bible passage metadata
  char *ipaddress;
  databuff currentdata; //Current Slide
  databuff nextdata; //Next Slide
  databuff transitiondata; //Transitioning slide (only used if the terminal supports a fade)
  short transitionval; //How far through the transition are we?
  
  databuff ibuff; //Input buffer: unlikely to be used but we need it just in case.
  
  stylesblock normalstyle;
  stylesblock currentstyle;
  
  struct displaynode_struct *next;
} displaynode;

displaynode *adddisplay(int socketfd);
displaynode *getdisplay(int socketfd);
int freedisplay(int socketfd);
int isrunning();
void handle_sigint(int sig);
void *get_in_addr(struct sockaddr *sa);
int setupconnection(char *port_str); //Returns listener fd
int closeconnections(int listener);
int dolisten(int listener);
displaynode *doaccept(int listener);
int doclose(int socketfd);
int fullsend(int socketfd, char *senddata, int datalen, int flags);
//int sendslidetext(int socketfd, char *slidedata);

int appendtonextdisplaybuffer(displaynode *adn, char *data, size_t size);
void querycolours(displaynode *adn);


#endif