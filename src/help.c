#include "help.h"

#include "util.h"
#include <string.h>
#include <stdio.h>
#include <dos.h>

#include <windows.h>

void display_help(char *str)
{
	if(strcmp(str, "insert")==0)
	{
		printf("Opening the peg-insert help page ....\n");
		ShellExecute(NULL, "open", ".\\help-pages\\peg_insert.html", NULL, NULL,
            SW_SHOWNORMAL);

	}
	else if(strcmp(str,"create")==0)
	{
		printf("Opening the peg-create help page ....\n");
		
		ShellExecute(NULL, "open", ".\\help-pages\\create_project.html", NULL, NULL,
            SW_SHOWNORMAL);

	}
	else if(strcmp(str,"commit")==0)
	{
		printf("Opening the peg-commit help page ....\n");
		ShellExecute(NULL, "open", ".\\help-pages\\peg_commit.html", NULL, NULL,
            SW_SHOWNORMAL);

	}
	else if(strcmp(str,"about")==0)
	{
		printf("Opening the peg-about help page ....\n");
		ShellExecute(NULL, "open", ".\\help-pages\\about.html", NULL, NULL,
            SW_SHOWNORMAL);

	}
	else if(strcmp(str,"all")==0)
	{
		printf("Opening the peg-all help page ....\n");
		ShellExecute(NULL, "open", ".\\help-pages\\all_help.html", NULL, NULL,
            SW_SHOWNORMAL);

	}
	else
	{
		die("Entered string is %s [404: No such keyword]\n"
            "Enter 'peg --help' to list all commands\n", str);
		//ShellExecute(NULL,"open",".\\help-pages\\about.html",NULL,NULL,SW_SHOWNORMAL);
	}
}
