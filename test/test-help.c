#include<stdio.h>
#include"help.h"
int main()
{
	char str[200];
	printf("Enter the string to search");
	scanf("%s",str);
	display_help(str);
	return 0;
}