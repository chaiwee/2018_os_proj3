/** this is a simple file explorer  */
/*
    my implementations
 * filename is max 16-char long
 * directory-filemust be NULL(entry)-terminated
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"

// method signatures
unsigned int open(struct inode *directory_inode, char *file_name);
struct super_block *get_sb(FILE *storage);
void print_inode(struct inode *inode);
struct inode *get_inode(int inode_num);
FILE *open_image();
void read_input();
// controll methods
void fix_sb(struct super_block *sb, FILE *storage);
void print_dir_entrs(FILE *storage, struct super_block *sb);
void print_sb();
void print_all_inode();

FILE *storage;
struct super_block *sb;

int main(int argc, char *argv[])
{
    //variables values
    storage = open_image();
    sb = get_sb(storage);
    printf("geting inode for '/'\n");
    struct inode *cwd_inode = get_inode(2); // set current working dirctory to "/"

    print_inode(cwd_inode);

    unsigned int inode_num = open(cwd_inode, "file_15");
    printf("the inode# of 'file_15' is %u\n", inode_num);

    //print_dir_entrs(storage, sb);
    //print_sb(sb);
    //print_all_inode(storage, sb);
    //read_input();
    fclose(storage);
}

/** this method is never the same, it is used to fix the image*/
void fix_sb(struct super_block *sb, FILE *storage)
{
    printf("fixing superblock:\n");
    sb->first_data_block = sb->first_data_block + 1;
    sb->first_inode_block = sb->first_inode_block + 1;

    printf("writing superblock:\n");
    fseek(storage, 0, SEEK_SET);
    int size = sizeof(struct super_block) - sizeof(((struct super_block *)0)->padding) + 3; // calculating size without padding
    //overide wrong values with modified ones
    fwrite(sb, size, 1, storage);
    printf("writed\n");
    return;
}

void print_sb()
{
    //printf("printing superblock:\n"); // control
    printf("partition_type: %u \n", sb->partition_type);
    printf("block_size: %u \n", sb->block_size);
    printf("inode_size: %u \n", sb->inode_size);
    printf("first_inode_block: %u \n", sb->first_inode_block);

    printf("num_inodes_tot: %u \n", sb->num_inodes_tot);
    printf("num_inode_blocks: %u \n", sb->num_inode_blocks);
    printf("num_free_inodes: %u \n", sb->num_free_inodes);
    printf("num_blocks: %u \n", sb->num_blocks);

    printf("num_free_blocks: %u \n", sb->num_free_blocks);
    printf("first_data_block: %u \n", sb->first_data_block);
    printf("volume_name: %s \n", sb->volume_name);
}

struct inode *get_inode(int inode_num)
{
    struct inode *inode_p = malloc(sizeof(struct inode));

    long int offset = (sb->block_size * sb->first_inode_block) + (sb->inode_size * inode_num);
    fseek(storage, offset, SEEK_SET);
    fread(inode_p, sb->inode_size, 1, storage);

    return inode_p;
}

struct super_block *get_sb(FILE *storage) //read super block
{
    int size = sizeof(struct super_block) - sizeof(((struct super_block *)0)->padding); // calculating size without padding

    struct super_block *sblk = malloc(size);

    //printf("reading superblock\n"); // control
    fseek(storage, 0, SEEK_SET);
    fread(sblk, size, 1, storage);

    return sblk;
}

void print_inode(struct inode *inode)
{
    printf("mode: %X\n", inode->mode); // reg. file, directory, dev., permissions
    //printf("locked: %u \n", inode->locked); // opened for write
    //printf("date: %u \n", inode->date);
    printf("size: %u\n", inode->size);
    //printf("indrct_blk: %d \n", ind->indirect_block); // N.B. -1 for NULL
    printf("blocks: %hd %hd %hd %hd %hd %hd\n\n", inode->blocks[0], inode->blocks[1], inode->blocks[2], inode->blocks[3], inode->blocks[4], inode->blocks[5]);
}

void print_all_inode()
{
    unsigned int starting_point = sb->block_size * (sb->first_inode_block - 1);
    //printf("[print_all_inode] starting_point: %x\n", starting_point); //control

    struct inode *ind = malloc(sizeof(struct inode));

    int limit = sb->num_inodes_tot - sb->num_free_inodes;
    fseek(storage, starting_point, SEEK_SET);
    for (int i = 0; i < limit; i++)
    {
        if (fread(ind, 32, 1, storage) != 1)
        {
            printf("[print_all_inode] error at read. %d\n", i);
            continue;
        }

        printf("id: %d\n", i); // jut a counter
        print_inode(ind);
    }
    free(ind);
}

unsigned int open(struct inode *directory_inode, char *file_name)
{
    struct dentry *dir_entr = malloc(sizeof(struct dentry));
    unsigned int inodes_per_block = sb->block_size / sb->inode_size;
    printf("inodes_per_block: %u\n", inodes_per_block);

    for (char i = 0; i < 6; i++) //for the 6 blocks
    {
        unsigned int starting_point = (sb->first_data_block + directory_inode->blocks[i]) * sb->block_size;
        fseek(storage, starting_point, SEEK_SET);
        for (int j = 0; j < inodes_per_block; i++) // read as many inodes as posible
        {
            int ret = fread(dir_entr, sizeof(struct dentry), 1, storage);
            if (ret != 1){
                printf("[open] error reading dir_entry\n");
                exit(1);
            }

            if (dir_entr->inode == 0) break;

            if (strcmp(dir_entr->name, file_name) == 0)
            {
                printf("found dir_entry\n");
                printf("inode: %u\n", dir_entr->inode);
                //printf("dir_length: %u\n", dir_entr->dir_length);
                //printf("name_len: %u\n", dir_entr->name_len);
                printf("file_type: %u\n", dir_entr->file_type);
                printf("name: <%s>\n", dir_entr->name);
                return dir_entr->inode;
            }
        }
    }
}

void print_dir_entrs(FILE *storage, struct super_block *sb)
{
    unsigned int starting_point = 0x2000;
    //printf("[print_dir_entrs]\n"); //control

    struct dentry *dir_entr = malloc(sizeof(struct dentry));

    int limit = 103;
    fseek(storage, starting_point, SEEK_SET);
    printf("[print_dir_entrs] size of d_entry: %lu\n", sizeof(struct dentry));
    for (int i = 1; i < limit; i++)
    {
        if (fread(dir_entr, sizeof(struct dentry), 1, storage) != 1)
            printf("[print_dir_entrs] error at read\n");

        printf("--------\n");
        printf("id: %d\n", i);                            // jut a counter
        printf("inode: %u\n", dir_entr->inode);           // reg. file, directory, dev., permissions
        printf("dir_length: %u\n", dir_entr->dir_length); // reg. file, directory, dev., permissions
        printf("name_len: %u\n", dir_entr->name_len);     // reg. file, directory, dev., permissions
        printf("file_type: %u\n", dir_entr->file_type);   // reg. file, directory, dev., permissions
        printf("name: <%s>\n", dir_entr->name);           // reg. file, directory, dev., permissions
    }
    free(dir_entr);
}

void read_input()
{
    /*
    posible inputs:
     * mkdir <dir name>
     * mkfile <file name>
     * rmfile <file name>
     * ls
     * cd <directory name>      //only options within current directory ( .  ..  / other-directoryes )
     * oppen <filename>
    */
    char str[20];
    printf("input a string: ");         //instruction to user
    fgets(str, sizeof(str), stdin);     //read 20-char-max string
    printf("string says :<%s>\n", str); //show string
}

FILE *open_image()
{
    FILE *temp = fopen("disk.img", "r+");
    if (!temp)
    {
        printf("file_exp] error opening disk.img\n");
        exit(1);
    }
    return temp;
}