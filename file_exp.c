/** this is a simple file explorer  */
#include <stdio.h>
#include <stdlib.h>
#include "fs.h"

// method signatures
void mount_sb(struct super_block *spr_blk);
void print_sb(struct super_block *spr_blk);
void fix_sb(struct super_block *sb, FILE *storage);
void read_input();
void check_all_inode(unsigned int starting_point, unsigned int inode_size, unsigned int ending_point);
FILE *open_image();

//variables values
FILE *storage;
char wd[100];      //working directory
char wd_inode = 0; //working directory i-node

int main(int argc, char *argv[])
{
    struct super_block *spr_blk = malloc(sizeof(struct super_block));

    printf("welcome to the file explrer\n");
    storage = open_image();
    mount_sb(spr_blk);
    print_sb(spr_blk);
    unsigned int starting_point = spr_blk->block_size * spr_blk->first_inode_block;
    unsigned int ending_point = spr_blk->block_size * spr_blk->first_data_block;
    check_all_inode(starting_point, spr_blk->inode_size, ending_point);
    //read_input();
    fclose(storage);
}

/** this method is never the same, it is used to fix the image*/
void fix_sb(struct super_block *sb, FILE *storage)
{
    printf("fixing superblock:\n");
    // modify values

    printf("writing superblock:\n");
    fseek(storage, 0, SEEK_SET);
    int size = sizeof(struct super_block) - sizeof(((struct super_block *)0)->padding) + 3; // calculating size without padding
    //overide wrong values with modified ones
    fwrite(sb, size, 1, storage);
    printf("writed\n");
    return;
}

void print_sb(struct super_block *sb)
{
    printf("printing superblock:\n");
    printf("\tvolume_name: %s \n", sb->volume_name);
    printf("block_size: %u \n", sb->block_size);
    printf("num_blocks: %u \n", sb->num_blocks);
    printf("first_inode block: %u \n", sb->first_inode_block);
    printf("first_data_block: %u \n", sb->first_data_block);
    printf("num_free_blocks: %u \n", sb->num_free_blocks);
    printf("\n");
    printf("inode_size: %u \n", sb->inode_size);
    printf("num_tot_inodes: %u \n", sb->num_inodes);
    printf("num_free_inodes: %u \n", sb->num_free_inodes);
}

void mount_sb(struct super_block *spr_blk) //read super block
{
    printf("reading superblock\n");
    fseek(storage, 0, SEEK_SET);
    int size = sizeof(struct super_block) - sizeof(((struct super_block *)0)->padding); // calculating size without padding
    fread(spr_blk, size, 1, storage);
    return;
}

void check_all_inode(unsigned int starting_point, unsigned int inode_size, unsigned int ending_point)
{
    struct inode *ind;
    int i = 1;

    ending_point = starting_point + (32 * 20); // controll

    fseek(storage, starting_point, SEEK_SET);
    while (starting_point < ending_point)
    {
        fread(ind, inode_size, 1, storage);

        printf("------------\n");
        printf("#: %d \n", i);                // jut a counter
        printf("mode: %x \n", ind->mode);     // reg. file, directory, dev., permissions
        printf("locked: %u \n", ind->locked); // opened for write
        printf("date: %u \n", ind->date);
        printf("size: %u \n", ind->size);
        printf("indrct_blk: %d \n", ind->indirect_block); // N.B. -1 for NULL
        printf("blocks: not implemented \n");               // unsigned short blocks[0x6];

        starting_point += inode_size;
        i++;
    }
}

void read_input()
{
    char str[50];
    printf("input a string: ");
    fgets(str, sizeof(str), stdin);
    printf("string says :<%s>\n", str);
    return;
}

FILE *open_image()
{
    FILE *temp = fopen("disk.img", "r+"); // to read and write use "r+"
    if (!temp)
    {
        printf("file_exp] error opening disk.img\n");
        exit(1);
    }
    return temp;
}