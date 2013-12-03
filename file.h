#ifndef __FILES__FILESYSTEM_JUN__
#define __FILES__FILESYSTEM_JUN__

#include "common.h"
#include "inode.h"
#include "block.h"

#define FILENAME_LENGTH (32 - sizeof(uint))

struct V6_file {
	uint inumber;
	byte filename[FILENAME_LENGTH];
};


//typedef struct V6_file ;
#define file_entry V6_file


#define FILE_ENTRY_SIZE (sizeof(struct V6_file))

int open_file(char *filename, struct V6_file *spec_dir, struct V6_file *opened_file);
ssize_t read_file(struct V6_file *opened_file, void *buf, size_t count);
ssize_t write_file(struct V6_file *opened_file, void *buf, size_t count);
int create_file(char *filename, struct V6_file *spec_dir);
//size_t read_file(char *filename, void *buf, size_t count);
//size_t write_file(char *filename, void *buf, size_t count);
//int read_file_by_inode(struct inode *file_inode, void* buf, size_t count);
//int write_file_by_inode(struct inode *file_inode, void* buf, size_t count);
int read_directory(struct inode *dir_inode, struct file_entry **entries, int *entry_num);

int find_file_in_directory(const char *filename, struct V6_file *spec_dir);
//uint find_file_in_current_directory(const char *filename);
int find_directory_in_directory(const char *filename, struct V6_file *spec_dir);
int make_directory_in_directory(char *filename, struct V6_file *spec_dir);
int create_directory(char *filename, struct V6_file *spec_dir);
inline int add_directory_to_inode(struct file_entry *dir_entry, struct inode *parent_dir_inode);
int add_directory_to_directory(struct file_entry *dir_entry, struct file_entry *parent_dir_entry);
int add_entry_to_inode(struct file_entry *entry, struct inode *dir_inode);
inline int read_inode_data(struct inode *file_inode, struct inode_data *data);
int ensure_enough_blocks(struct inode * file_inode, size_t total_size);

int list_directory(char ***all_filename, struct V6_file *spec_dir);
int make_root_directory(struct V6_file *root);
int get_file_size(struct V6_file *);

inline int is_directory_inode(struct inode *file_inode);
int current_directory(char* filename, struct V6_file *spec_dir);
int is_this_file(struct file_entry *entry, const char* filename);

//int get_root_directory(struct V6_file *root);

//int current_directory(const char *filename);
//extern int read_directory(struct inode *dir_inode, struct file_entry *entries, int *entry_num);

#endif
