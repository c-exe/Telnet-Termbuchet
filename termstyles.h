#ifndef TERMSTYLES_INCLUDED
#define TERMSTYLES_INCLUDED 1

//Styles
#define PLAINTEXT "0"
#define BRIGHTTEXTON "1"    /* Also used as Bold */
#define BRIGHTTEXTOFF "21"  /* Also used as Bold */
#define DIMTEXTON "2"
#define DIMTEXTOFF "22"
#define ITALICTEXTON "3"
#define ITALICTEXTOFF "23"
#define UNDERLINETEXTON "4"
#define UNDERLINETEXTOFF "24"
#define BLINKTEXTON "5"
#define BLINKTEXTOFF "25"
#define FASTBLINKON "6"
#define FASTBLINKOFF "26"
#define REVERSETEXTON "7"
#define REVERSETEXTOFF "27"
#define INVISIBLETEXTON "8"
#define INVISIBLETEXTOFF "28"
#define CROSSEDOUTTEXTON "9"
#define CROSSEDOUTTEXTOFF "29"

#define N_PLAINTEXT 1
#define N_ON 1
#define N_OFF 2

//16 Colours
#define FGBLACK "30"
#define FGMAROON "31"
#define FGGREEN "32"
#define FGOLIVE "33"
#define FGNAVY "34"
#define FGMAGENTA "35"
#define FGTEAL "36"
#define FGSILVER "37"
#define FGGREY "90"
#define FGRED "91"
#define FGLIME "92"
#define FGYELLOW "93"
#define FGBLUE "94"
#define FGFUCHSIA "95"
#define FGCYAN "96"
#define FGWHITE "97"
#define BGBLACK "40"
#define BGMAROON "41"
#define BGGREEN "42"
#define BGOLIVE "43"
#define BGNAVY "44"
#define BGMAGENTA "45"
#define BGTEAL "46"
#define BGSILVER "47"
#define BGGREY "100"
#define BGRED "101"
#define BGLIME "102"
#define BGYELLOW "103"
#define BGBLUE "104"
#define BGFUCHSIA "105"
#define BGCYAN "106"
#define BGWHITE "107"

#define N_FGDARK 2
#define N_FGLIGHT 2
#define N_BGDARK 2
#define N_BGLIGHT 3

//256 colours
#define FG256START "38;5;"
#define BG256START "48;5;"

#define N_FG256 5
#define N_BG256 5

//24bit Colours
#define FG24BSTART "38;2;"  /* Officially it should be 38;2;0; but in practise this is fine */
#define BG24BSTART "48;2;"  /* Officially it should be 48;2;0; but in practise this is fine */

#define N_FG24B 5           /* 7? */
#define N_BG24B 5           /* 7? */

//Style Formatters
#define COMBINATOR ";"
#define STYLESTART "\033["
#define STYLEEND "m"

#define N_COMBINATOR 1
#define N_STYLESTART 2
#define N_STYLEEND 1

//Misc
#define HIDECURSOR "\033[?25l"
#define SHOWCURSOR "\033[?25h"
#define ALTBUFFER  "\033[?1049h"
#define MAINBUFFER "\033[?1049l"
#define CLS        "\033[2J"

#define N_HIDECURSOR 6
#define N_SHOWCURSOR 6
#define N_ALTBUFFER  8
#define N_MAINBUFFER 8
#define N_CLS        4

//256 colour calculator
#define COLOUR256OFFSET 16
#define COLOUR256R 36
#define COLOUR256G 6
#define COLOUR256B 1
#define COLOUR256GREYOFFSET 232
#define COLOUR256MAX 255

//Querying
#define TERM_DCS "\033P"
#define TERM_ST "\033\\"
#define N_TERM_ST 2
#define N_TERM_DCS 2
#define TERM_DECRQSS_START TERM_DCS "$q"
#define TERM_DECRQSS_END TERM_ST
#define TERM_DECRQSS_OPT_SGR "m"
#define QRY_COLOURS TERM_DECRQSS_START TERM_DECRQSS_OPT_SGR TERM_DECRQSS_END
#define TERM_DECREPLYSS_START TERM_DCS "1$r"
#define TERM_DECERRORSS_START TERM_DCS "0$r"
#define TERM_DECREPLYSS_END TERM_ST
#define TERM_DECERRORSS_END TERM_ST
/*
 * Basically, if we can't get the colour support any other way, test with this...
 * For TrueColour test, send: STYLESTART PLAINTEXT STYLEEND STYLESTART BG24BSTART "1;2;3" STYLEEND QRY_COLOURS
 *  if the response is not TERM_DECREPLYSS_START BG24BSTART "1;2;3" STYLEEND TERM_DECREPLYSS_END then it doesn't have TrueColour support
 * For 256 colour test, send: STYLESTART PLAINTEXT STYLEEND STYLESTART BG256START "67" STYLEEND QRY_COLOURS
 *  if the response is not TERM_DECREPLYSS_START BG256START "67" STYLEEND TERM_DECREPLYSS_END then it doesn't have 256 colour support
 * For 16 colour test, send STYLESTART PLAINTEXT STYLEEND STYLESTART BGCYAN STYLEEND QRY_COLOURS
 *  if the response is not TERM_DECREPLYSS_START BGCYAN STYLEEND TERM_DECREPLYSS_END then it doesn't have 16 colour support
 * For 8 colour test, send STYLESTART PLAINTEXT STYLEEND STYLESTART BGMAGENTA STYLEEND QRY_COLOURS
 *  if the response is not TERM_DECREPLYSS_START BGMAGENTA STYLEEND TERM_DECREPLYSS_END then it doesn't have 8 colour support
 * Otherwise, assume Monochrome!
 * (This assumes the Telnet connection is properly transparent and that the terminal supports DEC420 escape sequences...)
*/

#endif