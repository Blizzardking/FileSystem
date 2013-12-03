#ifndef __UTIL_FILESYSTEM_RK__
#define __UTIL_FILESYSTEM_RK__
/*
dependent on:
  void list_current_directory(char *entry_names[],int* entry_number);
  int find_file_in_current_directory(char *file_name);//non_directory file;
  int find_directory_in_current_directory(char *direcotry_name);
  int write_file(char *file_name, char* buffer, int count);//if file no exist, create one. otherwise create a file named by file_name; Write the buffer into this file, return the number of bytes written; suppoting multiple writing.
  int read_file(char *file_name, char* buffer, int count);//if file no exist, return -1; Read into the buffer  from the file, return the number of bytes read; suppoting multiple reading.
  int mkdir(uint inode_number)//make a directory using the inode_number; set its entries to be . and ..;
  void mkdir(char * dir_name) //make a directory entry in curr_dir; recurisily call mkdir(inode_number);
*/
int curr_fd;
int initfs(int argc, char** argv);
int cpin(int argc, char** argv);
int cpout(int argc, char** argv);
int mkdir1(int argc, char** argv);
int ls(int argc, char** argv);
int cd(int argc, char** argv);
#endif
