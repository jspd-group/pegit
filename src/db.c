#include "db.h"
#include "util.h"

int die_with_error(MYSQL mysql_obj) {
  printf("Program's database terminated because %s", mysql_error(&mysql_obj));
  exit(1);
}

int initialize_database(struct _database_*db, char *username, char *password) {
  FILE *fp;
  fp = fopen("credentials.txt", "w");
  fseek(fp, 0, SEEK_SET);
  fprintf(fp, "%s %s", username, password);
  fclose(fp);
  if (mysql_init(&db->mysql) == NULL) {
    printf("Failed to create MYSQL object\n");
    die_with_error(db->mysql);
  }
  if (!mysql_real_connect(&db->mysql, NULL, username, password, NULL, 0, NULL,
                          0)) {
    printf("Failed to connect to LocalHost: Error: %s\n",
           mysql_error(&db->mysql));
    die_with_error(db->mysql);
  } else {
    printf("Logged on to database sucessfully : %s\n", mysql_error(&db->mysql));
  }
  return 1;
}

int change_context(struct _database_ *db) {
  if (mysql_query(&db->mysql, "USE pegit") == 0) {
    printf("Connected to database successfully\n");
  } else {
    printf("Failed to connect to database : %s\n", mysql_error(&db->mysql));
    die_with_error(db->mysql);
  }
}

int create_schema(struct _database_ *db) {

  if (mysql_query(&db->mysql, "CREATE DATABASE pegit") == 0 && printf("CREATE DATABASE pegit")) {
    printf(GREEN".......DONE\n");
  } else {
    printf("Failed to create database : %s\n", mysql_error(&db->mysql));
    die_with_error(db->mysql);
  }
  change_context(db);
  printf("CREATE TABLE project( project_id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, project_name VARCHAR(30) , version_id INT NOT NULL);");
  if (mysql_query(&db->mysql, "CREATE TABLE project( project_id INT NOT NULL "
                              "AUTO_INCREMENT PRIMARY KEY, project_name "
                              "VARCHAR(30) , version_id INT NOT NULL);") == 0) {
    printf(GREEN"........  [DONE]\n");
  } else {
    printf("Failed to create project table : %s\n", mysql_error(&db->mysql));
    die_with_error(db->mysql);
  }

  if (mysql_query(&db->mysql, "CREATE TABLE main_table(project_id INT ,"
                              "version_num INT NOT NULL , blob_id "
                              "INT NOT NULL , path VARCHAR(100) ,filename "
                              "VARCHAR(100),FOREIGN KEY(project_id) "
                              "REFERENCES project(project_id));") == 0) {
    printf("main_table table created \n");
  } else {
    printf("Failed to create main_table : %s\n", mysql_error(&db->mysql));
    die_with_error(db->mysql);
  }

  if (mysql_query(&db->mysql,
                  "CREATE TABLE storage(blob_index INT NOT NULL "
                  "AUTO_INCREMENT PRIMARY KEY , sha VARCHAR(20) , data "
                  "MEDIUMBLOB ,filename VARCHAR(30));") == 0) {
    printf("Project table created \n");
  } else {
    printf("Failed to create project table : %s\n", mysql_error(&db->mysql));
    die_with_error(db->mysql);
  }
}

int create_new_project(struct _database_ *db, char *project_name) {
  int i;
  char query[200] = {NULL};
  char temp[100] = {NULL};
  char q[200] = {NULL};

  change_context(db);

  strcat(query, "INSERT INTO project(project_name,version_id) VALUES ('");
  strcat(query, project_name);
  strcat(query, " ',1);");
  if (mysql_query(&db->mysql, query) == 0) {
    printf("VALUE INSERTED IN PROJECT TABLE\n");
  } else {
    printf("Failed to insert values in project table : %s\n",
           mysql_error(&db->mysql));
    die_with_error(db->mysql);
  }
  int id = mysql_insert_id(&db->mysql);
  return id;
}

char* get_project_id(struct _database_ *db, char *p_name) {
  char qu[200] = {NULL};
  change_context(db);
  strcat(qu, "SELECT project_id FROM project WHERE project_name = '");
  strcat(qu, p_name);
  strcat(qu, "';");
  if (mysql_query(&db->mysql, qu) == 0) {
    printf("query successfull\n");
  } else {
    printf(";-(");
    die_with_error(db->mysql);
  }
  MYSQL_RES *result = mysql_store_result(&db->mysql);
  if (result == NULL) {
    printf("query unsuccessfull \n");
    die_with_error(db->mysql);
  }
  int num_fields = mysql_num_fields(result);
  printf("Num fields %d\n",num_fields);
  MYSQL_ROW row = mysql_fetch_row(result);
  unsigned long *lengths = mysql_fetch_lengths(result);
  printf("%.*s ", (int) lengths[0], row[0] ? row[0] : "NULL");
  if (lengths == NULL) {
    die_with_error(db->mysql);
  }
  mysql_free_result(result);
}

/////testing required------------------------------------

int add_multiple_file(struct _database_ *db, struct file_list *listadd,
                      int project_id, int version) {
  change_context(db);
  while (listadd != NULL) {
    char file_path[100];
    char file_name[100];
    strcpy( file_path , listadd->path);
    strcpy(file_name , listadd->name);
    // find file from database with this name and path and compare it with
    // the given file.
    FILE *file = fopen(listadd->path, "rb");
    if (file == NULL) {
      printf("Invalid filename : %s ", listadd->path);
      die_with_error(db->mysql);
    }
    //int abc = find_in_database(db, listadd->path, listadd->name);
    int abc=0;
    if (abc == 0) {
      add_single_file(db, project_id, listadd->path, version);
    } else {
      // int check= compare_file_function
      int check=0;
      if (check != 1) {
        add_single_file(db, project_id, listadd->path, listadd->_version);
      }
    }
    listadd = listadd->next;
    printf("Yup added\n");
  }
  return 1;
}

int update_project_version(struct _database_* db, char * project_name)
{
  change_context(db);
  char str[100]={ NULL};
  strcat(str,"UPDATE project SET version_id = version_id+1 WHERE project_name = '");
  strcat(str,project_name);
  strcat(str,"';");
  if (mysql_query(&db->mysql, str) == 0) {
    printf("query done %s \n",str);
  } else {
    printf("query not exec: %s\n", mysql_error(&db->mysql));
    die_with_error(db->mysql);
  }
return 0;

}

int find_in_database(struct _database_ *db, char *_path, char *_name) {
  change_context(db);
  // select data from storage where blob_index = (SELECT blob_index from
  // main_table where path = _path and name=_name
  char q[400] = { '\0' };
  strcat(q, "SELECT blob_id FROM main_table WHERE path = '");
  strcat(q, _path);
  strcat(q, "' AND filename = '");
  strcat(q, _name);
  strcat(q, "';");

  if (mysql_query(&db->mysql, q) == 0) {
    printf("query done %s \n",q);
  } else {
    printf("query not exec: %s\n", mysql_error(&db->mysql));
    die_with_error(db->mysql);
  }


unsigned int num_fields,i;
MYSQL_ROW row;
//MYSQL_RES *results = mysql_store_result(&db->mysql);
//unsigned long *lengths = mysql_fetch_lengths(results);
/*
if (results) // there are rows
{

num_fields = mysql_num_fields(results);

row = mysql_fetch_row(results);
printf("fdvsfdvdf\n");

printf("fdvsfdvdf\n");
fwrite(row[0],lengths[0],1,fp);
printf("fdvsfdvdf\n");
mysql_free_result(results);
}
*/

printf("vdvsd\n");
  MYSQL_RES *results = mysql_store_result(&db->mysql);
   if (results == NULL) {
    printf("NULL obtained\n");
    return 0;
  } 
  printf("Rows : %d \n",mysql_num_rows(results));
  while((row = mysql_fetch_row(results))!=NULL){
    printf("id : %s , \n",(row[0]?row[0]:"NULL"));

  }
  printf("Done\n");
  num_fields=mysql_num_fields(results);
    printf("fvsdfvd\n");
  row=mysql_fetch_row(results);
  unsigned long*lengths = mysql_fetch_lengths(results);
  printf("skfvsdfjb\n");
  //int k=0;
  //k=atoi(row[1]);
  printf("vafvafd");
  printf("-----------[%.*s]------------",(int) lengths[0],row[0] ? row[0] : "NULL" );
  //printf("row = %d %d\n",k,sizeof(row[0]));
  if(lengths == NULL)
  {
    die_with_error(db->mysql);
  }
    printf("fvsdfvd\n");
 
}
/*
  void find_fname(char *fname)
  {
    char buff[250]=path.buf;
    int lent = path.len;
    int i,j;

    for(i=lent-1;i>0;i--)
    {
      if (buff[i]=='\\')
      {
        for(j=i;j<=lent;j++)
         {
           fname[j-i]=buff[j];
         }
    
       break;
      }
    }
  }
  */

int add_single_file(struct _database_ *db, int project_id, char *path,
                    int version)
{
  change_context(db);
  FILE *fp = fopen(path, "rb");
  if (fp == NULL) {
    printf("Not a valid commit\n");
    die_with_error(db->mysql);
  }
  fseek(fp, 0, SEEK_END);
  if (ferror(fp)) {
    printf("fseek() failed\n");
    int r = fclose(fp);
    if (r == EOF) {
      printf("cannot close file handler\n");
    }
    exit(1);
  }
  long int flen = ftell(fp);
  if (flen == -1) {
    perror("error occurred");
    int r = fclose(fp);
    if (r == EOF) {
      printf("cannot close file handler\n");
    }
    exit(1);
  }
  fseek(fp, 0, SEEK_SET);
  if (ferror(fp)) {
    printf("fseek() failed\n");
    int r = fclose(fp);
    if (r == EOF) {
      printf("cannot close file handler\n");
    }
    exit(1);
  }
  printf("Find  %d \n", flen);
  char data[flen + 1];
  int size = fread(data, 1, flen, fp);
  if (ferror(fp)) {
    printf("fread() failed\n");
    int r = fclose(fp);
    if (r == EOF) {
      printf("cannot close file handler\n");
    }
    exit(1);
  }
  int r = fclose(fp);
  if (r == EOF) {
    printf("cannot close file handler\n");
  }
  char chunk[2 * size + 1];
  mysql_real_escape_string(&db->mysql, chunk, data, size);
  char *st = "INSERT INTO storage(data,sha) VALUES('%s',NULL)";
  size_t st_len = strlen(st);
  char query[st_len + 2 * size + 1];
  int len = snprintf(query, st_len + 2 * size + 1, st, chunk);
  if (mysql_real_query(&db->mysql, query, len)) {
    die_with_error(db->mysql);
  }
  int id = mysql_insert_id(&db->mysql);
  char quer[1000] = {NULL};
  char temp[50];
  strcat(quer, "INSERT INTO main_table VALUES ( ");
  itoa(project_id, temp, 10);
  strcat(quer, temp);
  strcat(quer, " , ");
  strcat(quer, " (SELECT version_id FROM project WHERE project_id = ");
  strcat(quer, temp);
  strcat(quer, "),");
  itoa(id, temp, 10);
  strcat(quer, temp);
  strcat(quer, ",'");
  strcat(quer, path);
  strcat(quer, "','");
  //char fname[100];
  //find_fname(path,fname);
  strcat(quer, path);
  strcat(quer, "');");


  if (mysql_query(&db->mysql, quer)) {
    printf("Not executed %s\n %s ", quer, mysql_error(&db->mysql));
  }
}

int retreive_file(int project_id) {}
/*
  if(mysql_query(&db->mysql,"SELECT data FROM storage WHERE blob_index = 1 "))
  {
    die_with_error(db->mysql);
  }
  fp=fopen("new.png","wb");
  MYSQL_RES *result = mysql_store_result(&db->mysql);
  if(result==NULL)
  {
    printf("NULL obtained\n");
    die_with_error(db->mysql);
  }

  MYSQL_ROW row = mysql_fetch_row(result);
unsigned long *lengths = mysql_fetch_lengths(result);
if (lengths == NULL) {
die_with_error(db->mysql);
}
fwrite(row[0], lengths[0], 1, fp);
if (ferror(fp))
{
printf("fwrite() failed\n");
mysql_free_result(result);
mysql_close(&db->mysql);
exit(1);
}
int rk = fclose(fp);
if (rk == EOF) {
printf("cannot close file handler\n");
}
mysql_free_result(result);
*/
int loop_back_one(struct _database_*db,int project_id)
{
  change_context(db);

}