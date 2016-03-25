#include<stdio.h>
#include<dirent.h>
#include<sys/stat.h>
#include "util.h"
#include<string.h>
#include"commit.h"
#include<malloc.h>


int file_cmp(struct dirent* , struct strbuf ,struct strbuf);
struct node * createnode();
void intialise_node(struct node ** , struct dirent * , int ,struct node * );
void insert_node(struct node **,struct node **, struct node **);
struct d_node *pop();
void push(struct dirent* , char*);
char * path(struct d_node *);
void print_status(struct node*);


struct node
{
  struct dirent *file_info ;
  int status ;           // status of the file
  struct node * next;
};


struct d_node{
  struct dirent *directory;
  char * parent_dir;
  struct d_node *next;
  struct d_node *previous;
};
/* 
 * To implement stack by linked list of struct d_node s pionters to 
 * current node , head of linked list and previous node are needed
 */
struct d_node *d_root = NULL , *d_sptr = NULL , *d_ptr = NULL;  


int status()
{
  DIR *directory;//  create  a stream to the directory
  char *name ;
  struct dirent * d ;  // dirent stands for directory entry
  struct stat status ;
  FILE *file;
  struct strbuf buffer1,buffer2;
  char c;
  struct node *root = NULL ,*ptr = NULL , *sptr = NULL;
  int response , file_exists_db;
  struct d_node *d_tptr = NULL;
  char *parent_dir = ".";
  back :
  directory = opendir(parent_dir); // open the current working directory
  if(directory == NULL)
    die("Could not open the directory ");
  while((d = readdir(directory))) 
  {  //redir() is analogus to fget()
    
   name = d->d_name;
   stat(name,&status);
   printf("%s: %s\n", name, S_ISDIR(status.st_mode) ? " directory" : "file");
 
   /* if the object returned by readdir is a regular file then 
    * we will create a FILE stream to the file and read the contents of the file
    * if the stream is not created due to any reason it will be reported
    * if the stream is created we will store the content of the file in string buffer1
    * now we will retrieve a file named same as the name of current file and store it in string buffer2
    * we will compare these files using the function strcmp()
    * if the files are same then it means that there is no modification
    * if there is no file by that name then it will mean that the file is newly created
    * if the file by that name exists but contents are different then it will mean modification*/
 
    if(S_ISREG(status.st_mode))
    {
      file = fopen(name,"r"); 

      if(file == NULL)  // unable to create stream to this file
      {
        die("Permission denied");
      }
      strbuf_init(&buffer1,0); // initialisng two string buffers with length 0
      strbuf_init(&buffer2,0);
      while((c=fgetc(file))!=EOF)  // storing the content of file in string buffer1
        strbuf_addch(&buffer1,c);
      
      file_exists_db = find_file_from_head_commit(name,&buffer2); // reading file named name(means same as the name of current file) from database and storing it in string buffer2
      if(file_exists_db)
        response = strcmp(buffer1.buf,buffer2.buf);  // xyz.buf has string data type
                                                   // data type  of buffer1 and 2  is strbuf 
      
     /* Now we need to make a linked list of all the file names and their status 
      * where by status  we mean 
      * 1 for modified
      * 2 for new
      * 3 for not yet commited
      * 4 for no modification
     */
     if(file_exists_db)
     {
        if (response == 0)
        {
         ptr = createnode();
         intialise_node(& ptr , d , 4 , NULL);
         insert_node(&root , &ptr , &sptr);
       }
       else 
       {
         ptr = createnode();
         intialise_node(&ptr , d , 1 , NULL);
         insert_node(&root , &ptr , &sptr);            
       }
     }
     else
     {
        ptr = createnode();
      intialise_node(& ptr , d , 2 , NULL);
      insert_node(&root , &ptr , &sptr);
     }
    }
    /*
    * if the current object being pointed to by dirent is a directory then 
    * we will store this pointer in a stack ,so that its file can be compared 
    * when we are done comparing the files in the current directory */
    else
    {
      if(S_ISDIR(status.st_mode))
      {
        push(d, parent_dir);
      }
    }
  }
  close(directory); 
  if(d_sptr != d_root)
  d_tptr = pop();
  parent_dir = path(d_tptr);
  goto back;
  print_status(root);
}
 
void main(){
 int a;
 a = status() ; 
}

struct node * createnode()
{
  return ((struct node*)malloc(sizeof(struct node)));
}

void intialise_node(struct node ** node , struct dirent * d , int status ,struct node * next)
{
  (*node)->file_info = d;
  (*node)->status = status;
  (*node)->next = NULL;
}

void insert_node(struct node **root,struct node **current_node , struct node **previous_node)
{
        if (*root == NULL)
      {
        *root = *current_node;
        *previous_node = *current_node;
      }
      else
      {
        (*previous_node) -> next = *current_node;
        *previous_node = *current_node;
      }
}


void push(struct dirent * d, char *parent_dir)
{
  d_ptr = ((struct d_node*)malloc(sizeof(struct d_node)));
  d_ptr -> directory = d;
  d_ptr ->parent_dir = parent_dir; 
  d_ptr -> next = NULL;

  if (d_root == NULL)
    {
      d_root = d_ptr;
      d_sptr = d_ptr;
      d_ptr -> previous = NULL;     
    }
    else
    {
      d_sptr -> next = d_ptr;
      d_ptr -> previous = d_sptr;
      d_sptr = d_ptr;
    }
}

struct d_node * pop()
{ 
  struct d_node *tptr;
  tptr = d_sptr;
  d_sptr = d_sptr -> previous;
  d_sptr -> next = NULL ;
  return tptr;
}

char * path(struct d_node * tptr)
{
  struct strbuf parent_dir ;
  strbuf_init(&parent_dir , 0);
  strbuf_addstr(&parent_dir , tptr -> parent_dir);
  strbuf_addch(&parent_dir , '/');
  strbuf_addstr(&parent_dir , (tptr->directory)->d_name);
  return (parent_dir.buf);
}

void print_status(struct node * root)
{
  struct node *tptr = NULL;
  tptr = root;
  
  while(tptr->next != NULL)
  { 
     if (tptr->status == 1)
     {
       printf("modified :\t %s",(tptr->file_info)->d_name);
     }
       tptr = tptr->next ;
    }

    tptr = root;
  
  while(tptr->next != NULL)
  { 
     if (tptr->status == 2)
     {
       printf("new :\t %s",(tptr->file_info)->d_name);
     }
       tptr = tptr->next ;
    }
    tptr = root;
  
  while(tptr->next != NULL)
  { 
     if (tptr->status == 4)
     {
       printf("old :\t %s",(tptr->file_info)->d_name);
     }
       tptr = tptr->next ;
    }
}
