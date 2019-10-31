#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "termstyles.h"
#include "termdisp.h"

//stylesblock normalstyle, currentstyle;
int forcedcolourtypespec = 0;
int colourtypespec = 0;

int converttruecolour(colourtype ctype, int r, int g, int b)
{
  int t_r, t_g, t_b, bri;
  switch (ctype)
  {
    case monochrome:
      bri = (r & 255) + (g & 255) + (b & 255);
      if (bri>382) return 1; //765/2=382
      return 0;
    break;
    
    case eight:
      if ((r & 255) > 127) t_r = 4; else t_r = 0;
      if ((g & 255) > 127) t_g = 2; else t_g = 0;
      if ((b & 255) > 127) t_b = 1; else t_b = 0;
      t_r += t_g + t_b;
      return t_r;
    break;
    
    case sixteen:
      bri = (r & 255) + (g & 255) + (b & 255);
      if ((r & 255) > 127) t_r = 4; else t_r = 0;
      if ((g & 255) > 127) t_g = 2; else t_g = 0;
      if ((b & 255) > 127) t_b = 1; else t_b = 0;
      t_r += t_g + t_b;
      if (bri > 382) t_r += 8;
      return t_r;
    break;
    
    case byte;
      t_r = (r & 255) / 42;
      t_g = (g & 255) / 42;
      t_b = (b & 255) / 42;
      bri = (r & 255) + (g & 255) + (b & 255);
      bri = bri >> 5; // should be over 33, but /32 and int should work fine?
      if (t_r == t_g && t_g == t_b)
      {
        return (bri + COLOUR256GREYOFFSET);
      }
      else
      {
        bri = (t_r * COLOUR256R) + (t_g * COLOUR256G) + (t_b * COLOUR256B) + COLOUR256OFFSET;
        return bri;
      }
    break;
    
    case truecolour:
      return -2;
    break;
  }
  return -1;
}

void revertcurrenttonormal(displaynode *adn)
{
  char colstr[128] = "";
  int basecol = 0;
  memcpy(adn->currentstyle, adn->normalstyle, sizeof(stylesblock));
  appendtonextdisplaybuffer(adn, STYLESTART PLAINTEXT STYLEEND, sizeof(char)*(N_STYLESTART+N_PLAINTEXT+N_STYLEEND));
  if (adn->currentstyle.isbold) appendtonextdisplaybuffer(adn, STYLESTART BRIGHTTEXTON STYLEEND, sizeof(char)*(N_STYLESTART+N_ON+N_STYLEEND));
  if (adn->currentstyle.isitalics) appendtonextdisplaybuffer(adn, STYLESTART ITALICTEXTON STYLEEND, sizeof(char)*(N_STYLESTART+N_ON+N_STYLEEND));
  if (adn->currentstyle.isunderlined) appendtonextdisplaybuffer(adn, STYLESTART UNDERLINETEXTON STYLEEND, sizeof(char)*(N_STYLESTART+N_ON+N_STYLEEND));
  if (adn->currentstyle.isshadowed)  {}
  //Then deal with colours...
  switch (adn->colours)
  {
    case truecolour:
      sprintf(colstr, STYLESTART FG24BSTART "%d;%d;%d" STYLEEND,
              (adn->currentstyle.textcolour.r & 255), (adn->currentstyle.textcolour.g & 255), (adn->currentstyle.textcolour.b & 255));
      appendtonextdisplaybuffer(adn, colstr, sizeof(char)*(strlen(colstr)));
      sprintf(colstr, STYLESTART BG24BSTART "%d;%d;%d" STYLEEND,
              (adn->currentstyle.backcolour.r & 255), (adn->currentstyle.backcolour.g & 255), (adn->currentstyle.backcolour.b & 255));
      appendtonextdisplaybuffer(adn, colstr, sizeof(char)*(strlen(colstr)));
      
    break;
    
    case byte:
      sprintf(colstr, STYLESTART FG256START "%d" STYLEEND,
              converttruecolour(adn->colours, adn->currentstyle.textcolour.r, adn->currentstyle.textcolour.g, adn->currentstyle.textcolour.b));
      appendtonextdisplaybuffer(adn, colstr, sizeof(char)*(strlen(colstr)));
      sprintf(colstr, STYLESTART BG256START "%d" STYLEEND,
              converttruecolour(adn->colours, adn->currentstyle.backcolour.r, adn->currentstyle.backcolour.g, adn->currentstyle.backcolour.b));
      appendtonextdisplaybuffer(adn, colstr, sizeof(char)*(strlen(colstr)));
      
    break;
    
    case sixteen:
    case eight:
      basecol = converttruecolour(adn->colours, adn->currentstyle.textcolour.r, adn->currentstyle.textcolour.g, adn->currentstyle.textcolour.b);
      switch (basecol)
      {
        case 0:
          appendtonextdisplaybuffer(adn, STYLESTART FGBLACK COMBINATOR, sizeof(char)*(N_STYLESTART+N_FGDARK+N_COMBINATOR));
        break;
        
        case 1:
          appendtonextdisplaybuffer(adn, STYLESTART FGMAROON COMBINATOR, sizeof(char)*(N_STYLESTART+N_FGDARK+N_COMBINATOR));
        break;
        
        case 2:
          appendtonextdisplaybuffer(adn, STYLESTART FGGREEN COMBINATOR, sizeof(char)*(N_STYLESTART+N_FGDARK+N_COMBINATOR));
        break;
        
        case 3:
          appendtonextdisplaybuffer(adn, STYLESTART FGOLIVE COMBINATOR, sizeof(char)*(N_STYLESTART+N_FGDARK+N_COMBINATOR));
        break;
        
        case 4:
          appendtonextdisplaybuffer(adn, STYLESTART FGNAVY COMBINATOR, sizeof(char)*(N_STYLESTART+N_FGDARK+N_COMBINATOR));
        break;
        
        case 5:
          appendtonextdisplaybuffer(adn, STYLESTART FGMAGENTA COMBINATOR, sizeof(char)*(N_STYLESTART+N_FGDARK+N_COMBINATOR));
        break;
        
        case 6:
          appendtonextdisplaybuffer(adn, STYLESTART FGTEAL COMBINATOR, sizeof(char)*(N_STYLESTART+N_FGDARK+N_COMBINATOR));
        break;
        
        case 7:
          appendtonextdisplaybuffer(adn, STYLESTART FGSILVER COMBINATOR, sizeof(char)*(N_STYLESTART+N_FGDARK+N_COMBINATOR));
        break;
        
        case 8:
          appendtonextdisplaybuffer(adn, STYLESTART FGGREY COMBINATOR, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_COMBINATOR));
        break;
        
        case 9:
          appendtonextdisplaybuffer(adn, STYLESTART FGRED COMBINATOR, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_COMBINATOR));
        break;
        
        case 10:
          appendtonextdisplaybuffer(adn, STYLESTART FGLIME COMBINATOR, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_COMBINATOR));
        break;
        
        case 11:
          appendtonextdisplaybuffer(adn, STYLESTART FGYELLOW COMBINATOR, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_COMBINATOR));
        break;
        
        case 12:
          appendtonextdisplaybuffer(adn, STYLESTART FGBLUE COMBINATOR, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_COMBINATOR));
        break;
        
        case 13:
          appendtonextdisplaybuffer(adn, STYLESTART FGFUCHSIA COMBINATOR, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_COMBINATOR));
        break;
        
        case 14:
          appendtonextdisplaybuffer(adn, STYLESTART FGCYAN COMBINATOR, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_COMBINATOR));
        break;
        
        case 15:
          appendtonextdisplaybuffer(adn, STYLESTART FGWHITE COMBINATOR, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_COMBINATOR));
        break;
        
        
      }
      basecol = converttruecolour(adn->colours, adn->currentstyle.backcolour.r, adn->currentstyle.backcolour.g, adn->currentstyle.backcolour.b);
      switch (basecol)
      {
        case 0:
          appendtonextdisplaybuffer(adn, BGBLACK STYLEEND, sizeof(char)*(+N_BGDARK+N_STYLEEND));
        break;
        
      }
      
    break;
    
    case monochrome:
      if (!converttruecolour(adn->colours, adn->currentstyle.textcolour.r, adn->currentstyle.textcolour.g, adn->currentstyle.textcolour.b) ) //&& 
      //    converttruecolour(adn->colours, adn->currentstyle.backcolour.r, adn->currentstyle.backcolour.g, adn->currentstyle.backcolour.b) ) 
        appendtonextdisplaybuffer(adn, STYLESTART REVERSETEXTON STYLEEND, sizeof(char)*(N_STYLESTART+N_ON+N_STYLEEND));
    break;
  }
  
  //THEN
  switch (adn->currentstyle.outlinecolour) 
  {
    case 1:
    break;
    
    case 2:
    break;
    
    case 3:
      appendtonextdisplaybuffer(adn, STYLESTART REVERSETEXTON STYLEEND, sizeof(char)*(N_STYLESTART+N_ON+N_STYLEEND));
    break;
  }
  
}

/*
void setforcedcolourtypespec(int colourtype)
{
  forcedcolourtypespec = colourtype;
}

int runexternal(char *outputstr, int outputstr_size, char *program, char *arg1)
{
  //A quick bodge for running external programs with one argument
  int outpipe[2];
  pid_t apid
  char buffer[MAXBUFFERSIZE];
  int rdn = MAXBUFFERSIZE-1;
  int n, xn = outputstr_size, xon = 0;
  if (rdn>outputstr_size-1) rdn = outputstrsize-1;
  pipe(outpipe);
  apid = fork();
  switch (apid)
  {
    case -1:
      return -1;
    break;
    
    case 0:
      //child
      close(outpipe[0]);
      dup2(outpipe[1],SCREENFD);
      execlp(program, program, arg1);
      //If we got here, it failed
      write(SCREENFD,"\004",1); //Send an EOF
      exit(1);
      _exit(1);
    break;
    
    default:
      //parent
      close(outpipe[1]);
      waitpid(apid, NULL, 0); //I should really have a timer to kill it if it goes on too long
      do
      {
        n = read(outpipe[0], buffer, rdn);
        if (n>0 && buffer[0]==4) break;
        buffer[rdn] = 0;
        if (n>0) memcpy(outputstr + (sizeof(char)*xon), buffer, rdn);
        xn -= n;
        xon += n;
        if (xn <= 0) break;
        rdn = MAXBUFFERSIZE-1;
        if (rdn > xn) rdn = xn;
      } while(n>0);
      close(outputpipe[0]);
    break;
    if (xon>0 && outputstr[0]==4) return -2;
  }
  return 1;
}

int updatecolourtypespec()
{
  //Returns: 1=Mono, 3=8 colours, 4=16 colours, 8=256 colours, 24=truecolour, 0=unknown
  
  //Check if colour type has been forced to a value and return if so
  if (forcedcolourtypespec)
  {
    colourtypespec = forcedcolourtypespec;
    return colourtypespec;
  }
  //Otherwise check if it's a tty and if not say mono
  if (!isatty(SCREENFD))
  {
    colourtypespec = 1;
    return 1;
  }
  //Otherwise, we could look up the TERM in TERMINFO or TERMCAP (tricky, bloated and it turns out it's not definitive anyway!), or...
  const char *tt = getenv("TERM");
  char theoutput[512]="";
  int numcols = 0;
  int rv = runexternal(theoutput, 512, "tput","colors"); //risky
  if (rv == 1) //It worked
  {
    sscanf(theoutput,"%d", &numcols);
    if (numcols == 8) colourtypespec = 3;
    else if (numcols == 16) colourtypespec = 4;
    else if (numcols == 256) colourtypespec = 8;
    else if (numcols > 256) colourtypespec = 24; //probably
    else if (numcols < 0) colourtypespec = 1; //usually
    if (colourtypespec) return colourtypespec;
  }
  rv = runexternal(theoutput, 512, "infocmp",tt); //riskier
  if (rv == 1) //It worked
  {
    char *colout = strstr("colors",theoutput);
    if (colout)
    {
      if (colout[6] == '@') colourtypespec = 1; //usually
      else if (colout[6] == '#')
      {
        char *comma = strstr(",",colout);
        if (comma)
        {
          comma[0] = 0;
          sscanf(theoutput,"colors#%d", &numcols);
          if (numcols == 8) colourtypespec = 3;
          else if (numcols == 16) colourtypespec = 4;
          else if (numcols == 256) colourtypespec = 8;
          else if (numcols > 256) colourtypespec = 24; //probably
        }
      }
    }
    if (colourtypespec) return colourtypespec;
  }
  
  //Use this if we still don't have an answer...
  if (strstr("-mono", tt) != NULL || strstr("-MONO", tt) != NULL)
  {
    //Convention says this is mono.
    colourtypespec = 1;
    return 1;
  }
  if (strstr("-256", tt) != NULL)
  {
    //Convention says this is 256 colours.
    colourtypespec = 8;
    return 8;
  }
  //I dunno!
  return 0;
}
*/

int writestylefromtag(displaynode *adn, char *atag)
{
  /* Writes a style to buffer based on the tag provided */
  if (strsame(atag,"[[") || strsame(atag,"[]")
  {
    //Escaped tag character
    appendtonextdisplaybuffer(adn, atag+sizeof(char), sizeof(char));
  }
  else if (strisame(atag,"[=N]"))
  {
    //Normal - TODO: need to get this from settings
    //For the moment, we'll assume Plain Text
    appendtonextdisplaybuffer(adn, STYLESTART PLAINTEXT STYLEEND, sizeof(char)*(N_STYLESTART+N_PLAINTEXT+N_STYLEEND));
  }
  else if (strisame(atag, "[=B=0]"))
  {
    //Bold Off (using bright)
    appendtonextdisplaybuffer(adn, STYLESTART BRIGHTTEXTOFF STYLEEND, sizeof(char)*(N_STYLESTART+N_BRIGHTTEXTOFF+N_STYLEEND));
    adn->currentstyle.isbold = 0;
  }
  else if (strisame(atag, "[=B=1]"))
  {
    //Bold On (using bright)
    appendtonextdisplaybuffer(adn, STYLESTART BRIGHTTEXTON STYLEEND, sizeof(char)*(N_STYLESTART+N_BRIGHTTEXTON+N_STYLEEND));
    adn->currentstyle.isbold = 1;
  }
  else if (strisame(atag, "[=B=N]"))
  {
    //Bold Normal (using bright) using normalstyle
    if (adn->normalstyle.isbold && !adn->currentstyle.isbold) 
    {
      appendtonextdisplaybuffer(adn, STYLESTART BRIGHTTEXTON STYLEEND, sizeof(char)*(N_STYLESTART+N_BRIGHTTEXTON+N_STYLEEND));
    }
    else if (!adn->normalstyle.isbold && adn->currentstyle.isbold) 
    {
      appendtonextdisplaybuffer(adn, STYLESTART BRIGHTTEXTOFF STYLEEND, sizeof(char)*(N_STYLESTART+N_BRIGHTTEXTOFF+N_STYLEEND));
    }
    adn->currentstyle.isbold = adn->normalstyle.isbold;
  }
  else if (strisame(atag, "[=I=0]"))
  {
    //Italics Off
    appendtonextdisplaybuffer(adn, STYLESTART ITALICTEXTOFF STYLEEND, sizeof(char)*(N_STYLESTART+N_ITALICTEXTOFF+N_STYLEEND));
    adn->currentstyle.isitalics = 0;
  }
  else if (strisame(atag, "[=I=1]"))
  {
    //Italics On
    appendtonextdisplaybuffer(adn, STYLESTART ITALICTEXTON STYLEEND, sizeof(char)*(N_STYLESTART+N_ITALICTEXTON+N_STYLEEND));
    adn->currentstyle.isitalics = 1;
  }
  else if (strisame(atag, "[=I=N]"))
  {
    //Italics Normal using normalstyle
    if (adn->normalstyle.isitalics && !adn->currentstyle.isitalics) 
    {
      appendtonextdisplaybuffer(adn, STYLESTART ITALICTEXTON STYLEEND, sizeof(char)*(N_STYLESTART+N_ITALICTEXTON+N_STYLEEND));
    }
    else if (!adn->normalstyle.isitalics && adn->currentstyle.isitalics) 
    {
      appendtonextdisplaybuffer(adn, STYLESTART ITALICTEXTOFF STYLEEND, sizeof(char)*(N_STYLESTART+N_ITALICTEXTOFF+N_STYLEEND));
    }
    adn->currentstyle.isitalics = adn->normalstyle.isitalics;
  }
  else if (strisame(atag, "[=U=0]"))
  {
    //Underline Off
    appendtonextdisplaybuffer(adn, STYLESTART UNDERLINETEXTOFF STYLEEND, sizeof(char)*(N_STYLESTART+N_UNDERLINETEXTOFF+N_STYLEEND));
    adn->currentstyle.isunderlined = 0;
  }
  else if (strisame(atag, "[=U=1]"))
  {
    //Underline On
    appendtonextdisplaybuffer(adn, STYLESTART UNDERLINETEXTON STYLEEND, sizeof(char)*(N_STYLESTART+N_UNDERLINETEXTON+N_STYLEEND));
    adn->currentstyle.isunderlined = 1;
  }
  else if (strisame(atag, "[=U=N]"))
  {
    //Underline Normal using normalstyle
    if (adn->normalstyle.isunderlined && !adn->currentstyle.isunderlined) 
    {
      appendtonextdisplaybuffer(adn, STYLESTART UNDERLINETEXTON STYLEEND, sizeof(char)*(N_STYLESTART+N_UNDERLINETEXTON+N_STYLEEND));
    }
    else if (!adn->normalstyle.isunderlined && adn->currentstyle.isunderlined) 
    {
      appendtonextdisplaybuffer(adn, STYLESTART UNDERLINETEXTOFF STYLEEND, sizeof(char)*(N_STYLESTART+N_UNDERLINETEXTOFF+N_STYLEEND));
    }
    adn->currentstyle.isunderlined = adn->normalstyle.isunderlined;
  }
  else if (strisame(atag,"[=SH=0]") || strisame(atag,"[=SH=1]") || strisame(atag,"[=SH=N]"))
  {
    //Ignore shadow for now
  }
  else if (strisame(atag,"[=OB=0]") || strisame(atag,"[=OB=1]") || strisame(atag,"[=OB=N]"))
  {
    //Ignore Black Border for now
  }
  else if (strisame(atag,"[=OW=0]") || strisame(atag,"[=OW=1]") || strisame(atag,"[=OW=N]"))
  {
    //Ignore White Border for now
  }
  else if (strisame(atag,"[=OI=0]") || strisame(atag,"[=OI=1]") || strisame(atag,"[=OI=N]"))
  {
    //Ignore Inverse Border for now
  }
  else if (strisame(atag, "[=C=Red]"))
  {
    //Red Text
    //if (adn->colours == unknowncolour) updatecolourtypespec();
    if (adn->colours >= sixteen) appendtonextdisplaybuffer(adn, STYLESTART FGRED STYLEEND, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_STYLEEND));
    else if (adn->colours >= eight) appendtonextdisplaybuffer(adn, STYLESTART FGMAROON STYLEEND, sizeof(char)*(N_STYLESTART+N_FGDARK+N_STYLEEND));
  }
  else if (strisame(atag, "[=C=Yellow]"))
  {
    //Yellow Text
    //if (!colourtypespec) updatecolourtypespec();
    if (adn->colours >= sixteen) appendtonextdisplaybuffer(adn, STYLESTART FGYELLOW STYLEEND, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_STYLEEND));
    else if (adn->colours >= eight) appendtonextdisplaybuffer(adn, STYLESTART FGOLIVE STYLEEND, sizeof(char)*(N_STYLESTART+N_FGDARK+N_STYLEEND));
  }
  else if (strisame(atag, "[=C=Green]"))
  {
    //Green Text
    //if (!colourtypespec) updatecolourtypespec();
    if (adn->colours >= sixteen) appendtonextdisplaybuffer(adn, STYLESTART FGLIME STYLEEND, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_STYLEEND));
    else if (adn->colours >= eight) appendtonextdisplaybuffer(adn, STYLESTART FGGREEN STYLEEND, sizeof(char)*(N_STYLESTART+N_FGDARK+N_STYLEEND));
  }
  else if (strisame(atag, "[=C=Blue]"))
  {
    //Blue Text
    //if (!colourtypespec) updatecolourtypespec();
    if (adn->colours >= sixteen) appendtonextdisplaybuffer(adn, STYLESTART FGBLUE STYLEEND, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_STYLEEND));
    else if (adn->colours >= eight) appendtonextdisplaybuffer(adn, STYLESTART FGNAVY STYLEEND, sizeof(char)*(N_STYLESTART+N_FGDARK+N_STYLEEND));
  }
  else if (strisame(atag, "[=C=Cyan]"))
  {
    //Cyan Text
    //if (!colourtypespec) updatecolourtypespec();
    if (adn->colours >= sixteen) appendtonextdisplaybuffer(adn, STYLESTART FGCYAN STYLEEND, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_STYLEEND));
    else if (adn->colours >= eight) appendtonextdisplaybuffer(adn, STYLESTART FGTEAL STYLEEND, sizeof(char)*(N_STYLESTART+N_FGDARK+N_STYLEEND));
  }
  else if (strisame(atag, "[=C=Magenta]"))
  {
    //Magenta Text
    //if (!colourtypespec) updatecolourtypespec();
    if (adn->colours >= sixteen) appendtonextdisplaybuffer(adn, STYLESTART FGFUCHSIA STYLEEND, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_STYLEEND));
    else if (adn->colours >= eight) appendtonextdisplaybuffer(adn, STYLESTART FGMAGENTA STYLEEND, sizeof(char)*(N_STYLESTART+N_FGDARK+N_STYLEEND));
  }
  else if (strisame(atag, "[=C=Black]"))
  {
    //Black Text
    //if (!colourtypespec) updatecolourtypespec();
    if (adn->colours >= eight) appendtonextdisplaybuffer(adn, STYLESTART FGBLACK STYLEEND, sizeof(char)*(N_STYLESTART+N_FGDARK+N_STYLEEND));
  }
  else if (strisame(atag, "[=C=White]"))
  {
    //White Text
    //if (!colourtypespec) updatecolourtypespec();
    if (adn->colours >= sixteen) appendtonextdisplaybuffer(adn, STYLESTART FGWHITE STYLEEND, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_STYLEEND));
    else if (adn->colours >= eight) appendtonextdisplaybuffer(adn, STYLESTART FGSILVER STYLEEND, sizeof(char)*(N_STYLESTART+N_FGDARK+N_STYLEEND));
  }
  else if (strisame(atag, "[=C=Silver]"))
  {
    //Silver Text
    //if (!colourtypespec) updatecolourtypespec();
    if (adn->colours >= eight) appendtonextdisplaybuffer(adn, STYLESTART FGSILVER STYLEEND, sizeof(char)*(N_STYLESTART+N_FGDARK+N_STYLEEND));
  }
  else if (strisame(atag, "[=C=Grey]") || strisame(atag, "[=C=Gray]"))
  {
    //Grey Text
    //if (!colourtypespec) updatecolourtypespec();
    if (adn->colours >= sixteen) appendtonextdisplaybuffer(adn, STYLESTART FGGREY STYLEEND, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_STYLEEND));
    else if (adn->colours >= eight) appendtonextdisplaybuffer(adn, STYLESTART FGBLACK STYLEEND, sizeof(char)*(N_STYLESTART+N_FGDARK+N_STYLEEND));
  }
  else if (stristartsas(atag, "[=C=#"))
  {
    //Arbitrary colour text - TODO: need to find out what the terminal can cope with and implement the closest!
    //...
  }
  else if (strisame(atag, "[=C=N]"))
  {
    //Colour Normal - need to get this from settings
  }
  else if (stristartsas(atag, "[=S="))
  {
    //Ignore text size changes - we can't implement this in a terminal!
  }
  else if (strisame(atag, "[Intro]") || stristartsas(atag, "[Intro ") ||
           strisame(atag, "[Ending]") || stristartsas(atag, "[Ending ") ||
           strisame(atag, "[Outro]") || stristartsas(atag, "[Outro ") ||
           strisame(atag, "[Verse]") || stristartsas(atag, "[Verse ") ||
           strisame(atag, "[Chorus]") || stristartsas(atag, "[Chorus ") ||
           strisame(atag, "[Bridge]") || stristartsas(atag, "[Bridge ") ||
           strisame(atag, "[MiddleEight]") || stristartsas(atag, "[MiddleEight ") ||
           strisame(atag, "[Tag]") || stristartsas(atag, "[Tag ") ||
           strisame(atag, "[Slide]") || stristartsas(atag, "[Slide "))
  {
    //Ignore slide definitions
  }
  else
  {
    fprintf("Warning: Unknown tag \"%s\"!\n",atag);
    return 0;
  }
  return 1;
}