#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <sqlite3.h>

#include "libtelnet.h"


fd_set master, read_fds;
int fdmax;


int main(int argc, char *argv[])
{
  int i, nbytes;
  int listener;
  char remoteip[INET6_ADDRSTRLEN];
  char buffer[BUFFERSIZE] = "";
  displaynode *currentdn = NULL;
  
  if ((listener = setupconnection(PORT_STR)) < 0)
  {
    fprintf(stderr, "Error: Could not start server!")
    exit(1);
  }
  
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  
  fdmax = listener;
  FD_SET(listener, &master);
  FD_SET(STDINFD, &master);
  
  if (dolisten(int listener) < 0)
  {
    fprintf(stderr, "Error: Could not listen for connections!")
    closeconnections(listener);
    exit(2);
  }
  
  while (isrunning())
  {
    read_fds = master;
Do_Select_Restart:
    if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
    {
      if (errno == EINTR)
      {
        if (isrunning()) goto Do_Select_Restart;
        break;
      }
      perror("Error Selecting");
      break;
    }
    
    //Find out what was set
    for (i = 0; i<=fdmax; i++)
    {
      if (FD_ISSET(i, &read_fds))
      {
        //This was set
        if (i == STDINFD)
        {
          //TUI Input
          //TODO: Do something in the TUI
        }
        else if (i == listener)
        {
          //New Connection
          currentdn = doaccept(listener);
          if (!currentdn) perror("Accept Error");
          else
          {
            FD_SET(currentdn->socketfd, &master);
            if (currentdn->socketfd > fdmax) fdmax = currentdn->socketfd;
            //Ask for info here...
            /*if (fullsend(currentdn->socketfd, MSG_REQUEST MSG_COLOURTYPE MSG_SEPARATOR, sizeof(char)*strlen(MSG_REQUEST MSG_COLOURTYPE MSG_SEPARATOR), 0) == -1)
            {
              perror("Send Error for requesting colourtype");
            }
            //Be sure to receive this (sending ROK) and then ask for Width and Height...
            */
          }
        }
        else
        {
          //New Data
          if ((nbytes = recv(i, buffer, sizeof(char)*BUFFERSIZE, 0)) <= 0)
          {
            //Error or closed connection
            if (nbytes < 0)
            {
              perror("Receive Error");
            }
            //Close the connection and anything related:
            if (!doclose(i))
            {
              perror("Close Error");
            }
            FD_CLR(i, &master);
          }
          else
          {
            //We have data!
            
            //TODO: Do something with the data...
            currentdn = getdisplay(i);
            telnet_recv(currentdn->telnet, buffer, nbytes);
          }
        }
      }
    }
  }
  
  closeconnections(listener);
}