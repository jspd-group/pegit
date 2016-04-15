#include"help.h"
#include<string.h>
#include<stdio.h>

#include<windows.h>

void display_help(char *str)
{
	if(strcmp(str,"insert")==0)
	{
		printf("Opening the insert help page ....\n");
		ShellExecute(NULL,"open",".\\help-pages\\peg_insert.html",NULL,NULL,SW_SHOWNORMAL);
		
	}
	else if(strcmp(str,"create")==0)
	{
		printf("Opening the peg-create help page ....\n");
		ShellExecute(NULL,"open",".\\help-pages\\create_project.html",NULL,NULL,SW_SHOWNORMAL);

	}
	else if(strcmp(str,"commit")==0)
	{
		printf("Opening the peg-commit help page ....\n");
		ShellExecute(NULL,"open",".\\help-pages\\peg_commit.html",NULL,NULL,SW_SHOWNORMAL);

	}
	else if(strcmp(str,"about")==0)
	{
		printf("Opening the peg-about help page ....\n");
		ShellExecute(NULL,"open",".\\help-pages\\about.html",NULL,NULL,SW_SHOWNORMAL);

	}
	else 
	{
		printf("Entered string is %s [404 : No such keyword] \nEnter peg help all to list all commands",str);
		//ShellExecute(NULL,"open",".\\help-pages\\about.html",NULL,NULL,SW_SHOWNORMAL);	
	}
}