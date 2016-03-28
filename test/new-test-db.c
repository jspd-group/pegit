#include<stdio.h>
#include"db.h"
int main()
{

	struct _database_ db;
	printf("Done check 0\n");
	initialize_database(&db,"jatinder","dhawan");
	printf("Done check 1\n");
	create_schema(&db);
	printf("Done check 2\n");
	int id=create_new_project(&db,"git_pro");
	printf("id = %d SUCCESS\n",id);
	add_single_file(&db,1,"C:\\Users\\Jatinder Dhawan\\Desktop\\git\\abc.png",3);
	printf("=======================\n");	
	find_in_database(&db,"C:\\Users\\Jatinder Dhawan\\Desktop\\git\\abc.png","C:\\Users\\Jatinder Dhawan\\Desktop\\git\\abc.png");
	printf("=======================\n");
	get_project_id(&db,"git_pro");

	struct file_list* start=(struct file_list*)malloc(sizeof(struct file_list));
	struct file_list*make=start;
	start->_version =5;
	start->number_of_files=3;
	start->name="sjvndfjv";
	start->path="C:\\Users\\Jatinder Dhawan\\Desktop\\database\\credentials.txt";
	start->next=(struct file_list*)malloc(sizeof(struct file_list));
	start=start->next;
	start->_version =5;
	start->number_of_files=3;
	start->name="sjvndfjv";
	start->path="C:\\Users\\Jatinder Dhawan\\Desktop\\database\\tempo.txt";
	start->next=NULL;
	add_multiple_file(&db,make,1,3);

	update_project_version(&db,"git_pro");
	return 0 ;	
}