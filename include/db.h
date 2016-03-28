#ifndef DB_H
#define DB_H

#include<stdio.h>
#include<mysql.h>
//#include"visitor.h"
//#include"strbuf.h"
struct _database_
{
MYSQL mysql;
struct file_list* start;
int current_version;
};

struct file_list{
  int _version;
  int number_of_files;
  char *name;
  char *path;
  struct file_list *next;
};

static long long int VERSION = 0;

/* Following function is called once only */

/*Each function will automatically make a MYSQL object */

int initialize_database(struct _database_*,char*username,char * password);
/*Initialize the variables in _database- struct*/

int create_schema(struct _database_*);
/* This function creates the whole schema and initialize the schema with 
 * default values, where-ever necessary .It returns positive value on
 * success , and -1 on failure.
 */
 int update_project_version(struct _database_*,char*);
 int change_context(struct _database_*);
 /*Similar to using USE database_name in SQL Query*/

 int create_new_project(struct _database_*,char * project_name);
 /* Create a new project as specified by the name which is passed as 
  * argument to the function.This function returns non-zero value which
  * is the project-id of the new project .
  */

int add_single_file(struct _database_*,int project_id,char *path,int version);

/* This function adds a file to project defined by project_id . The path
 * of this file to be added is specified by path , and the additon is  
 * to take place in current version , else we may also specify the 
 * version in which the file is to be added .
 */

int add_multiple_file(struct _database_*,struct file_list*,int,int);
/* Add multiple files as referred by structure file_list*/

int find_in_database(struct _database_*,char*,char*);

int find_fname(char*);

 int get_files_from_given_version(int project,int version);

 /* This function creates a new folder and puts all the files which where
  * present in the given version of the project. Returns positive value 
  * on success and -1 on failure.
  */

 int get_files_from_current_version(int project);

 /* This function returns all the files present in current latest 
  * version of the project.
  */

 char * get_project_id(struct _database_*,char *);

 /* This function returns the project_id of the project with the 
  * specified name.
  */

#endif
