#ifndef DB_H
#define DB_H

#include<stdio.h>
#include<mysql.h>
//#include"visitor.h"
//#include"strbuf.h"

static long long int VERSION = 0;

/* Following function is called once only */

/*Each function will automatically make a MYSQL object */

int create_schema(char *username, char *password);
/* This function creates the whole schema and initialize the schema with 
 * default values, where-ever necessary .It returns positive value on
 * success , and -1 on failure.
 */

extern void initialize_database(char *username,char * password);
/* This function helps to connect to mysql-server i.e. the local host on
 * the client machine.
 */

 int create_new_project(char * project_name);
 /* Create a new project as specified by the name which is passed as 
  * argument to the function.This function returns non-zero value which
  * is the project-id of the new project .
  */

int add_new_file(int project_id,char *path,int version);

/* This function adds a file to project defined by project_id . The path
 * of this file to be added is specified by path , and the additon is  
 * to take place in current version , else we may also specify the 
 * version in which the file is to be added .
 */

 int get_files_from_given_version(int project,int version);

 /* This function creates a new folder and puts all the files which where
  * present in the given version of the project. Returns positive value 
  * on success and -1 on failure.
  */

 int get_files_from_current_version(int project);

 /* This function returns all the files present in current latest 
  * version of the project.
  */

 int get_project_id(char *name);

 /* This function returns the project_id of the project with the 
  * specified name.
  */

#endif
