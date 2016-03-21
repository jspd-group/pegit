***********************************************************
				PEGIT-VERSION CONTROL SYSTEM

***********************************************************


/* READ ME - BEFORE USING DATABASE PART */


/* The database of the project is implemented in mysql ... 
* For this , you need to install mysql c api or mysql c connector 
* (header files) + lib files. The link to download this is:
* 
*     https://www.mysql.com/products/connector/
*
* Once intalled you need to include the path of INCLUDE folder to include the
* the header files .
* An example compilation using gcc is :
*
* gcc -I "path of the header files " -l " path of lib folder in mysql c api "
*      file_name.c -L mysql
*
* Here file_name.c is an example c file :')
*
*  Also make changes in MAKEFILE accordingly to change the path 
*  in the macro definitions.
* 
* IMPORTANT NOTE:
* The storage occurs on local host . So , to run this project you need to 
* install localhost server on your PC . Either you may install WAMP /LAMP
* server or run localhost program . You need to know username and password to
* connect to this local host . As these are passed as arguments for initializing
* MYSQL connection.
* /
