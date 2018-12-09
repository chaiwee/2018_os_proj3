# os_proj3 - File system

## Introduction

In the previous projects, we simulated virtual-memory in multi-process environment. Based on P2, we'll add storing large volume of permanent user data.

Most operating systems have file systems to store an amount of data in permanent storage. A file-system provides a concept of "file" to users, which is the unit of storing independent information in a computer system.

Non-volatile (permanent) data is stored on a storage-device, consiststed of blocks. A file-system manages the location data in the device, providing a human-friendly interface to the users.

files:
* There are a lot of them.
* have different lengths.
* can be dynamically created/deleted.
* content/length can be changed.
Thus, the physical-location(block) is dynamically changed.
Therefore, a FS is needed.

FS remembers the physical-location(block) of data in the storage.

File-system maps logical file locations. That is, the logical-file-offset is not always the same as the physical-device-offset, and OS remembers the physical-data-block that holds the corresponding logical-data-block.
When you read a file, the physical-disk-block is found based on the mapping information in the FS.

meta-data(concept):
is (additional/descriptive) data for the data (content).
For example
    Let"s assume you have a file named "file_1".
    Inside "file_1", you have the text value 1234.
    String "file_1" is not directly-related to the content, "1234"
To search a file with content "1234", you should know the name of it, the physical-blocks (locations of the file), access permission, the current offsets, etc.

Inside OS, files are represented with some number, the number is used as an index of inode (i-node) data structure, that holds meta-data for a file.
i-node has rich information about the file (like disk data-block locations, file-size, access-permission, etc)
When you request to open a file:
1. OS looks up the corresponding i-node in the file system.
2. invoking reading/writing operation on the disk block that is given by the i-node.

### "i-node" blocks
OS has to store i-node in permanent storage device. because when you add/remove/edit a file, your file information is also (permanently) changed. so the device has to update bouth i-node and data itself.
Storage defines special "i-node" blocks, which stores only i-nodes, as an array.

### "superblock"
storage also defines a special block, called "superblock" which stores overall file-system-information.
the superblock defines i-node size, the number of free i-node blocks, actual usage of i-nodes, the first data block number, the volume name, etc. 

Thus, storage partition usually looks like this:
[Superblock]-[i-node-table blocks]-[data blocks]

Note that, the i-node table needs to be loaded into memory from storage.
When OS boots up the system
    root-file-system(the top-most-namespace) is mounted. (Usually is "/")
        OS reads the superblock from the disk
        and it populates files in the system
            by reading i-nodes, from disk into memory.

### hierarchy (directorys/folders)
A directory is a special type of file that contains the list of all files in the same sub-directory.
In addition, a directory-file provides a convenient interface to the user.

Each element that represents a file in the sub-directory is called "directory entry".
Each directory-entry includes i-node, and file name for the file.

Although it provides a convenient, human-readable interface, it makes some indirection.

examples:

at boot-up of the system:
1. mount the root filesystem (load i-node table)

to open the file "/file1":
1. look in the "/" directory-file to identify "file1"'s i-node 
    1. i-node of "/" is previously determined.
    2. In the "/" i-node, find out data blocks in the disk.
    3. read data blocks into memory, and interpret data as directory-entry
        1. parse all the data in the directory-file
        2. identify files in "/" directory
        3. compare the filename in directory-entry against "file1"
    4. find the i-node number for "file1"
        1. the directory-entry has i-node number
2. mark i-node for "/file1" as opened
    1. additional data is initialized (file offset is set to zero, for supporting data stream)

to read data on "/file1":
1. open the file before you use it.
    1. you should have a proper i-node pointer
2. identify the blocks to read
    1. check the current read position (file offset)
    2. calculate the starting byte (in a file) to read
    3. calculate the starting logical block number (in a file)
    4. look up i-node to get the physical block number
        1. i-node has blocks array that maps logical blocks to physical blocks (data blocknumber)
    5. read in data blocks
    6. copy it to the user buffer
        1. calculate the first byte of data blocks
        2. copy of it

## Explanation

In this project, you will build a simple filesystem. You have to implement above-mentioned simple file-system with the proper assumptions.

This program assignment could be one of your major take-outs from this course, so please work hard to complete the job/semester. If you need help, please ask for the help. (I am here for that specific purpose.) I, of course, welcome any questions on the subject. Note for the one strict rule that you should not copy the code from any others. Deep discussion on the subject is okay (and encouraged), but the same code (or semantics) will result in a sad ending.

Extra implementation/analysis is highly encouraged.
For example, you can implement user process that requests of reading for some specific file, instead of doing simple I/O.
You can implement file with write operation that requires storage block allocation for additional data, requiring i-node change.
You may want to load directory-entry into memory because every file-access implies addition directory-file-access; and it will be redundant if you read data from storage at every directory-file-access, which would be one of slowest operations in a computer system.
You may want to efficiently search directory entries because a directory may include many files. So, you would use a hash-map to easily get the inode from the requested file name.

There are various topics related to file-systems, including I/O, scheduling, and virtual-memory. Do as much as you can, and learn from it. (if you need a specific guideline, I can provide some.)

Lastly, some advice: begin as early as possible. Ask for help as early as possible. Try as early as possible. They are for your happy ending.

### requirements:
* Objective: understand file systems structure and organization.
    * The simulator has to work correctly. (should not break down, should obtain exact data that you requested)
    * Support the following file operations:
        * mount
        * open
        * read
        * close
    * You should be able to read bouth normal-files and directory-files.
* You can assume the following (or your own) data structure
    * /* code example: structures for superblock, i-nodes, and data blocks */
    * filesystem image (disk.img) is based on that.
* root-file-system(rootfs) mount
    * at system start/booting time rootfs has to be mounted.
    * populates all the files in the root-directory, and loads i-nodes from the disk-image.
        * Note: the capacity of a disk is only so large, so all the contents of a disk can't be loaded into memory.
    * When booting the system
        * kernel mounts the root-file system
        * prints the files in the root-directory. (similar to ls -al)
* file operations: open, read, close.
    * Assume a single child process.
    * To access a file, we need to open the file before use it.
    * on open:
        * requires two parameters (pathname and open-mode(O_RD)), and returns with the file descriptor
        * kernel reads the directory-file and looks up the i-node number, corresponding to the file.
        * PCB should have a data structure for the open-file. (file structure)
        * file-structure should have a pointer to the corresponding i-node.
    * on read:
        * requires three parameters (file descriptor, a buffer pointer, read data size)
        * File structure should have offset value, where the previous file operation has been stopped.
        * The kernel has to calculate the logical block numbers to obtain raw data blocks.
        * The kernel reads the disk"s physical data blocks.
        * Copy it to the user buffer
        * return with actual read bytes.
    * When the read is completed, the user closes the file.
    * You may assume one user process for this project, and read text data from randomly chosen ten files.
        * Look at the directory file for the file names.


* Output:
    * disk image tool (mk_simplefs disk.img …) :
        * prints out all the files information,
        * prints out all the superblock, i-nodes table
    * You can assume single user process or multiple user processes:
        * A user process makes open/read/close file operations pair for 10 random files
        * A user process prints out each file operations, and file contents on the screen
    * When you added write operation,
        * A user process makes open/[read|write]/close file operations for random files
        * Kernel prints out all file operations (pid, file op, result with content)

### Extra implementation
* directory-entry cache/hash
    * Directory-files frequently used because all the file-operations require directory access.
        * Thus, you can cache the directory-structure inside the memory, as well as disk.
    * To quickly access the correct directory-entry, we can make an in-memory hash function for the directory-structure.
        * For example, we can make a hash function that takes keys from the file-name, and generate a value with the i-node pointer.
        * Because hash keys are smaller than the actual file names (space), you should check once again for exact file name match.
* write operation (to a file)
    * you should provide data and file pointer.
    * the data is written from the beginning of the file.
    * You may need to consider data-block allocation if the new data overflows the block boundary.
        * In that case:
            1. allocate a new data-block from a free-data-block-pool
            2. write data-block to i-node.
            3. write data on the block.
    * After writing, you should be able to read data after the proper close operation.
    * Indirection blocks access to large files can also be considered.
* Buffer cache
    * To read-in data from disk, OS should prepare free-memory-region (free page)
    * You can link the disk operation with virtual-memory free page.
    * When you read-in data, prepare free-page frame that can store disk storage block.
    * The page frame can hold data in the storage, serving as a disk cache.
    * Then, you can read data from the buffer cache, instead of disk.
    * Because buffer cache is a cache to a disk, disk content would be different from the buffer cache. You should have a proper synchronization mechanism.
* disk-image creation tool
    * A disk image file contains the raw image of a disk partition: superblock, inode-table, data block.
        * you can use the following definitions, or you can define your own structure
    * Files population in a disk image.
        * You can give input file names or directory.
        * You can randomly generate filenames, contents.
    * Your kernel should be able to mount the disk image from your tool.
    * Output disk-image is stored on file "disk.img."
    * would populate files named with file_[n] so that you can generate random file names.
* work with multiple users
    * file access should be managed by kernel.
    * Each PCB should hold open-file descriptor structure.
    * When a user tries to open a file that is being used, you have two choices:
        * Return failure for open syscall.
        * Make the process wait until the previous user closes the file.

### Evaluation
Different credits are given for implementations and demos.