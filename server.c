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

#include "libtelnet.h"

//#include "../sharedfuncs.h"
#include "server.h"

int running = 0;

displaynode *dnroot = NULL, *dnptr = NULL;

displaynode *adddisplay(int socketfd)
{
  displaynode *adn = (displaynode *) malloc(sizeof(displaynode));
  if (!adn) return NULL;
  memset(adn, 0, sizeof(displaynode));
  adn->socketfd = socketfd;
  adn->next = NULL;
  adn->telnet = telnet_init(telopts, _event_handler, 0, (void *) &adn);
  if (!dnroot)
  {
    dnroot = adn;
  }
  else
  {
    for (dnptr = dnroot; dnptr->next != NULL; dnptr = dnptr->next);
    dnptr->next = adn;
  }
  return adn;
}

displaynode *getdisplay(int socketfd)
{
  for (dnptr = dnroot; dnptr != NULL; dnptr = dnptr->next)
  {
    if (dnptr->socketfd == socketfd) return dnptr;
  }
  return NULL;
}

int freedisplay(int socketfd)
{
  displaynode *adn;
  for (dnptr = dnroot; dnptr->next != NULL; dnptr = dnptr->next);
  {
    if (dnptr->next->socketfd == socketfd)
    {
      adn = dnptr->next;
      dnptr->next = adn->next;
      if (adn->telnet != NULL) telnet_free(adn->telnet);
      if (adn->terminaltype != NULL) free(adn->terminaltype);
      if (adn->metaformat != NULL) free(adn->metaformat);
      if (adn->ipaddress != NULL) free(adn->ipaddress);
      if (adn->currentdata != NULL) free(adn->currentdata);
      if (adn->nextdata != NULL) free(adn->nextdata);
      if (adn->transitiondata != NULL) free(adn->transitiondata);
      free(adn);
      return 1;
    }
  }
  return 0;
}

int isrunning()
{
  return running;
}

void handle_sigint(int sig)
{
  running = 0;
  printf("Terminating.\n");
}

void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) return &(((struct sockaddr_in *) sa)->sin_addr);
  return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

int setupconnection(char *port_str)
{
  //Returns a file descriptor to listen on
  int listener;
  struct sockaddr_storage remoteaddr;
  socklen_t addrlen;
  
  int yes=1;
  
  struct addrinfo hints, *addrinfo, *p;
  
  int rv;
  
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  
  if ((rv = getaddrinfo(NULL, port_str, &hints, &addrinfo)) != 0)
  {
    fprintf(stderr, "Error getting socket: %s\n", gai_strerror(rv));
    return -1;
  }
  
  for (p = addrinfo; p != NULL; p = p->ai_next)
  {
    listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listener < 0) continue;
    
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    
    if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
    {
      close(listener);
      continue;
    }
    
    break;
  }
  
  if (p == NULL)
  {
    fprintf(stderr, "Error: Failed to bind socket\n");
    freeaddrinfo(addrinfo);
    return -2;
  }
  
  freeaddrinfo(addrinfo);
  
  return listener;
}

int closeconnections(int listener)
{
  close(listener);
  //Close all connections too!
  while (dnroot != NULL)
  {
    //Send a close warning! (TODO)
    //Then initiate the close:
    doclose(dnroot->socketfd);
    
    /*//Send a close warning!
    //then close the socket
    close(dnptr->socketfd);
    //then delete it
    dnptr = dnroot->next;
    free(dnroot);
    dnroot = dnptr;*/
  }
  //then
  running = 0;
  return 1;
}

int dolisten(int listener)
{
  if (listen(listener, 10) == -1)
  {
    perror("Error listening");
    return -1;
  }
  
  if (signal(SIGINT, handle_sigint) == SIG_ERR)
  {
    perror("Sigint registration error");
    return -2;
  }
  else running = 1;
  
  return 1;
}

displaynode *doaccept(int listener)
{
  displaynode *adn = NULL;
  int newfd;
  struct sockaddr_storage remoteaddr;
  socklen_t addrlen;
  
  addrlen = sizeof(remoteaddr);
  newfd = accept(listener, (struct sockaddr *) &remoteaddr, &addrlen);
  if (newfd == -1)
  {
    return NULL;
  }
  else
  {
    adn = adddisplay(newfd);
    if (!adn)
    {
      close(newfd);
      return NULL;
    }
    adn->ipaddress = (char *) malloc(sizeof(char)*(1+INET6_ADDRSTRLEN));
    if (adn->ipaddress)
    {
      //If that worked, fill it in.
      inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr *) &remoteaddr), adn->ipaddress, INET6_ADDRSTRLEN);
    }
    //TODO: Do Option Negotiation Here?
  }
  return adn;
}

int doclose(int socketfd)
{
  if (close(socketfd))
  {
    //perror("Close Error");
    return 0;
  }
  if (!freedisplay(socketfd)) return 0;
  return 1;
}


int fullsend(int socketfd, const char *senddata, int datalen, int flags)
{
  int n, total = 0;
  int remaining = datalen;
  
  while (total < datalen)
  {
    n = send(socketfd, senddata+(sizeof(char)*total), remaining, flags);
    if (n == -1) break;
    total += n;
    remaining -= n;
  }
  
  if (n == -1) return n;
  return total;
}

static void _send(int socketfd, const char *buffer, size_t size, int flags)
{
  if (socketfd < 0) return;
  if (fullsend(socketfd, buffer, (int) size, flags) < 1)
  {
    //Error!  Display in the TUI somehow!
  }
}

/*int sendslidetext(int socketfd, char *slidedata)
{
  int sdlen = strlen(slidedata);
  char sdlenstr[30];
  char buffer[BUFFERSIZE];
  sprintf(sdlenstr, "%d", sdlen);
  strcpy(buffer, MSG_SLIDEHEADER_START);
  strcat(buffer, sdlenstr);
  strcat(buffer, MSG_SLIDEHEADER_END);
  
  if (fullsend(socketfd, buffer, strlen(buffer)*sizeof(char), 0) == -1)
  {
    return 0;
  }
  if (fullsend(socketfd, slidedata, sdlen*sizeof(char), 0) == -1)
  {
    return 0;
  }
  if (send(socketfd, MSG_SLIDEFOOTER MSG_SEPARATOR, strlen(MSG_SLIDEFOOTER MSG_SEPARATOR)*sizeof(char), 0) == -1)
  {
    return 0;
  }
  return 1;
}
*/

static void _input(displaynode *adn, const char *buffer, size_t size)
{
  if ((adn->flags & DISPLAYFLAG_ECHO) != 0)
  {
    telnet_send_text(adn->telnet, buffer, size); //Might need to do telnet_send here instead if this outputs with double line breaks!
  }
  adn->ibuff.buffer = realloc(adn->ibuff.buffer, adn->ibuff.size + size);
  if (adn->ibuff.buffer)
  {
    memcpy(adn->ibuff.buffer + adn->ibuff.size, buffer, size);
    adn->ibuff.size += size;
    if ((adn->flags & DISPLAYFLAG_QRYCOLOURS) != 0)
    {
      char *posst = memmem(adn->ibuff.buffer,adn->ibuff.size, TERM_ST, N_TERM_ST);
      char *posdcs = posst;
      if (posdcs)
      {
        while (posdcs >= adn->ibuff.buffer)
        {
          if (memcmp(posdcs, TERM_DCS, N_TERM_DCS)==0) break;
          posdcs-=sizeof(char);
        }
        if (posdcs >= adn->ibuff.buffer)
        {
          if ((posdcs == posst + 1 - strlen(TERM_DECREPLYSS_START BG24BSTART "1;2;3" STYLEEND TERM_DECREPLYSS_END)) 
              && (memcmp(posdcs, TERM_DECREPLYSS_START BG24BSTART "1;2;3" STYLEEND TERM_DECREPLYSS_END, 
                         strlen(TERM_DECREPLYSS_START BG24BSTART "1;2;3" STYLEEND TERM_DECREPLYSS_END))))
          {
            adn->colours = truecolour;
          }
          else if ((posdcs == posst + 1 - strlen(TERM_DECREPLYSS_START BG256START "67" STYLEEND TERM_DECREPLYSS_END)) 
                     && (memcmp(posdcs, TERM_DECREPLYSS_START BG256START "67" STYLEEND TERM_DECREPLYSS_END, 
                                       strlen(TERM_DECREPLYSS_START BG256START "67" STYLEEND TERM_DECREPLYSS_END))))
          {
            adn->colours = byte;
          }
          else if ((posdcs == posst + 1 - strlen(TERM_DECREPLYSS_START BGCYAN STYLEEND TERM_DECREPLYSS_END)) 
                     && (memcmp(posdcs, TERM_DECREPLYSS_START BGCYAN STYLEEND TERM_DECREPLYSS_END, 
                                       strlen(TERM_DECREPLYSS_START BGCYAN STYLEEND TERM_DECREPLYSS_END))))
          {
            adn->colours = sixteen;
          }
          else if ((posdcs == posst + 1 - strlen(TERM_DECREPLYSS_START BGMAGENTA STYLEEND TERM_DECREPLYSS_END)) 
                     && (memcmp(posdcs, TERM_DECREPLYSS_START BGMAGENTA STYLEEND TERM_DECREPLYSS_END, 
                                       strlen(TERM_DECREPLYSS_START BGMAGENTA STYLEEND TERM_DECREPLYSS_END))))
          {
            adn->colours = eight;
          }
          //Do other responses (if needed) here too...
          
          //Now remove that section so it doesn't confuse us
          char aptr,bptr;
          for (aptr = posdcs, bptr=posst+sizeof(char); bptr<(adn->ibuff.buffer + adn->ibuff.size); aptr+=sizeof(char),bptr+=sizeof(char))
          {
            aptr[0] = bptr[0];
          }
          size_t newsize = (size_t) (aptr - adn->ibuff.buffer);
          adn->ibuff.buffer = realloc(adn->ibuff.buffer, newsize);
          if (adn->ibuff.buffer)
          {
            adn->ibuff.size = newsize;
          }
          else
          {
            //Memory Error!  TODO: Send Error to TUI somehow!
          }
        }
      }
    }
  }
  else
  {
    //Out of Memory!  TODO: Send Error to TUI somehow!
  }
}


static void _event_handler(telnet_t *telnet, telnet_event_t *ev, void *user_data)
{
  const char envsendb[] = {TELNET_ENVIRON_SEND, TELNET_ENVIRON_USERVAR, 'T', 'E', 'R', 'M'} 
  const char envisb[] = {TELNET_ENVIRON_IS, TELNET_ENVIRON_USERVAR, 'T', 'E', 'R', 'M', TERM_ENVIRON_VAL} 
  const char envinfob[] = {TELNET_ENVIRON_INFO, TELNET_ENVIRON_USERVAR, 'T', 'E', 'R', 'M', TERM_ENVIRON_VAL} 
  displaynode *adn = (displaynode *) user_data;
  int i;
  long n;
  
  switch (ev->type)
  {
    /* data received */
    case TELNET_EV_DATA:
      _input(adn, ev->data.buffer, ev->data.size);
      /* telnet_negotiate(telnet, TELNET_WONT, TELNET_TELOPT_ECHO);
      telnet_negotiate(telnet, TELNET_WILL, TELNET_TELOPT_ECHO); */
    break;
    /* data must be sent */
    case TELNET_EV_SEND:
      _send(adn->socketfd, ev->data.buffer, ev->data.size, 0);
    break;
    /* enable options if accepted by client */
    case TELNET_EV_DO:
      if (ev->neg.telopt == TELNET_TELOPT_COMPRESS2)
        telnet_begin_compress2(telnet);
      else if (ev->neg.telopt == TELNET_TELOPT_ECHO)
      {
        adn->flags |= DISPLAYFLAG_ECHO;
      }
    break;
    /* Negotiating options: WILL */
    case TELNET_EV_WILL:
      if (ev->neg.telopt == TELNET_TELOPT_ENVIRON)
      {
        if ((adn->flags & DISPLAYFLAG_SENTDOENVIRON) == 0)
        {
          //Client jumped the gun, but OK...
          telnet_negotiate(telnet, TELNET_DO, TELNET_TELOPT_ENVIRON);
          adn->flags |= DISPLAYFLAG_SENTDOENVIRON;
        }
        //SEND USERVAR "TERM": TELNET_ENVIRON_SEND TELNET_ENVIRON_USERVAR "TERM"
        
        telnet_subnegotiation(telnet, TELNET_TELOPT_ENVIRON, envsendb, 6); /*buffer, buffsize*/
        //Need to do this for COLOURS/COLORS too at some point, but where?
      }
      else if (ev->neg.telopt == TELNET_TELOPT_NAWS)
      {
        if ((adn->flags & DISPLAYFLAG_SENTDONAWS) == 0)
        {
          //Client jumped the gun, but OK...
          telnet_negotiate(telnet, TELNET_DO, TELNET_TELOPT_NAWS);
          adn->flags |= DISPLAYFLAG_SENTDONAWS;
        }
        
      }
    break;
    /* Actual Subnegotiation */
    case TELNET_EV_SUBNEGOTIATION:
      if (ev->sub.telopt == TELNET_TELOPT_ENVIRON)
      {
        //IS USERVAR "TERM" VALUE "...": TELNET_ENVIRON_IS TELNET_ENVIRON_USERVAR "TERM" TELNET_ENVIRON_VALUE "..."
        
        if (ev->sub.size >= 7 && (memcmp(ev->sub.buffer, envisb, 7) == 0 || memcmp(ev->sub.buffer, envinfob, 7) == 0))
        {
          adn->terminaltype = realloc((adn->terminaltype), sizeof(char)*(ev->sub.size - 6));
          if (adn->terminaltype)
          {
            memcpy(adn->terminaltype, ev->sub.buffer + (sizeof(char)*7), sizeof(char)*(ev->sub.size - 7));
            adn->terminaltype[(ev->sub.size - 7)] = 0;
          }
        }
        //TODO: Process the terminal type!
      }
      else if (ev->sub.telopt == TELNET_TELOPT_NAWS)
      {
        //Window Size!
        if (ev->sub.size == 4 && memcmp(ev->sub.buffer, "\x00\x00\x00\x00", 4) != 0) //Should be!
        {
          adn->width = (((unsigned long)ev->sub.buffer[0]) << 8) | ((unsigned long)ev->sub.buffer[1]);
          adn->height = (((unsigned long)ev->sub.buffer[2]) << 8) | ((unsigned long)ev->sub.buffer[3]);
        }
      }
    break;
    /* Environment Subnegotiation if not done above? */
    case TELNET_EV_ENVIRON:
      if (ev->environ.cmd == TELNET_ENVIRON_IS || ev->environ.cmd == TELNET_ENVIRON_INFO)
      {
        for (i=0;i<ev->environ.count;i++)
        {
          if (ev->environ.values[i].type == TELNET_ENVIRON_USERVAR && strcmp(ev->environ.values[i].var,"TERM")==0)
          {
            adn->terminaltype = realloc((adn->terminaltype), sizeof(char)*(1+strlen(ev->environ.values[i].value)));
            if (adn->terminaltype) strcpy(adn->terminaltype, ev->environ.values[i].value);
            //TODO: Process the terminal type!
          }
          else if (ev->environ.values[i].type == TELNET_ENVIRON_USERVAR && (strcmp(ev->environ.values[i].var,"COLOURS")==0 || strcmp(ev->environ.values[i].var,"COLORS")==0 ))
          {
            n = strtol(ev->environ.values[i].value);
            if (n<=0)
              adn->colours = unknowncolour;
            else if (n < 8)
              adn->colours = monochrome;
            else if (n < 16)
              adn->colours = eight;
            else if (n < 256)
              adn->colours = sixteen;
            else if (n < 0xFFFFFF)
              adn->colours = byte;
            else
              adn->colours = truecolour;
          }
        }
      }
    break;
    /* Terminal Type if needed */
    case TELNET_EV_TTYPE:
      if (ev->ttype.cmd == TELNET_TTYPE_IS || ev->ttype.cmd == TELNET_TTYPE_INFO)
      {
        if (ev->ttype.name[0] != 0)
        {
          adn->terminaltype = realloc((adn->terminaltype), sizeof(char)*(1+strlen(ev->ttype.name)));
          if (adn->terminaltype) strcpy(adn->terminaltype, ev->ttype.name);
          //TODO: Process the terminal type!
        }
      }
    break;
    /* error */
    case TELNET_EV_ERROR:
      /*close(user->sock);
      user->sock = -1;
      if (user->name != 0)
      {
        _message(user->name, "** HAS HAD AN ERROR **");
        free(user->name);
        user->name = 0;
      }
      telnet_free(user->telnet);*/
      
      //Report error to TUI somehow
      
    break;
    default:
      /* ignore */
    break;
  }
}

int appendtonextdisplaybuffer(displaynode *adn, char *data, size_t size)
{
  adn->nextdata.buffer = realloc(adn->nextdata.buffer, adn->nextdata.size + size);
  if (!adn->nextdata.buffer)
  {
    //Out of memory Error!
    return 0;
  }
  memcpy(adn->nextdata.buffer + adn->nextdata.size, data, size);
  adn->nextdata.size += size;
  return 1;
}

void querycolours(displaynode *adn)
{
  //Do them in increasing order so they overwrite each other if supported!
  appendtonextdisplaybuffer(adn, STYLESTART PLAINTEXT STYLEEND STYLESTART BGMAGENTA STYLEEND QRY_COLOURS,
                                 strlen(STYLESTART PLAINTEXT STYLEEND STYLESTART BGMAGENTA STYLEEND QRY_COLOURS));
  appendtonextdisplaybuffer(adn, STYLESTART PLAINTEXT STYLEEND STYLESTART BGCYAN STYLEEND QRY_COLOURS,
                                 strlen(STYLESTART PLAINTEXT STYLEEND STYLESTART BGCYAN STYLEEND QRY_COLOURS));
  appendtonextdisplaybuffer(adn, STYLESTART PLAINTEXT STYLEEND STYLESTART BG256START "67" STYLEEND QRY_COLOURS,
                                 strlen(STYLESTART PLAINTEXT STYLEEND STYLESTART BG256START "67" STYLEEND QRY_COLOURS));
  appendtonextdisplaybuffer(adn, STYLESTART PLAINTEXT STYLEEND STYLESTART BG24BSTART "1;2;3" STYLEEND QRY_COLOURS,
                                 strlen(STYLESTART PLAINTEXT STYLEEND STYLESTART BG24BSTART "1;2;3" STYLEEND QRY_COLOURS));
}