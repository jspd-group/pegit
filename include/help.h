#ifndef HELP_H
#define HELP_H

#include<windows.h>

void display_help(char *);
/* This function will help in opening the correct help file as specified by the
 * user . This function will work only in windows . Corresponding function  for Linux
 * may be used .
 */

void display_error();
/* This function displays required message in case the help keyword is not found .
 */

 #endif
