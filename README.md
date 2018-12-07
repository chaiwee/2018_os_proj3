# os_proj3 - File system

## Introduction

In the previous projects, we simulated virtual-memory in multi-process-execution environment. Based upon that, we'll add storing large volume of permanent user data.
Most operating systems have file systems to store an amount of data in permanent storage. A file-system provides a concept of "file" to users, which is the unit of storing independent information in a computer system.

Non-volatile (durable even if the power-off) user data are stored on a permanent storage device that consists of multiple blocks. A file system manages the location of stored data in the storage device providing a human-friendly interface to the users.

files:
* have different lengths.
* can be dynamically created/deleted.
* can be edited, length can be changed.
Thus, the physical location in the storage device is dynamically changed.

There are numerous files in a system, and the block locations are changing. Therefore, a file system is needed.
FS remembers the physical location of data in the storage.

File system maps logical file"s locations. That is, the logical file offset is not always the same as the physical device offset, and OS remembers the physical data block that holds the corresponding logical data block.
When you read a file, the physical disk block is found based on the mapping information in the file system.

meta-data(concept):
is (additional/descriptive) data for the data (content).
For example, let"s assume you have a file named "file_1". Inside "file_1", you have the text value 1234. String "file_1" is not data that is directly related to its content "1234".
To search a file that has content "1234", you should know the name of it, the physical blocks (locations of the file), access permission, the current offsets, etc.

Inside OS, files are represented with some number, the number is used as an index of inode (index node) data structure, that holds meta-data for a file. (Similarly, PCB represents a process in OS.).
i-node has rich information about the file, like disk data blocks" location, file size, access permission, etc. 
When you request to open a file:
1. OS looks up the corresponding i-node in the file system.
2. invoking reading/writing operation on the disk block that is given by the i-node.

### "i-node" blocks
OS has to store i-node in permanent storage device. because when you add/remove/edit a file, your file information is also (permanently) changed. so the device has to update bouth i-node and data itself.
Storage defines special "i-node" blocks, which stores only i-nodes, as an array.

### "superblock"
storage also defines a special block, called "superblock" which stores overall file system information.
the superblock defines i-node size, the number of free i-node blocks, actual usage of i-nodes, the first data block number, the volume name, etc. 

Thus, storage partition usually looks like this:
[Superblock]-[i-node table blocks]-[data blocks]

Note that, the i-node table needs to be loaded into memory from storage.
When OS boots up the system, root-file-system(the top-namespace) is mounted. Usually, root directory fileis the "/" file.
When OS mounts the root file system, OS reads the superblock from the disk, and it populates files in the system by reading i-nodes from disk into memory.

### hierarchy
A directory is a special type of file that contains the list of a group of files.
In a directory-file, all files in the same sub-directory are stored.

In addition, a directory file provides a convenient interface to the user.
To aid the human, directory-file stores alphabetical name and its inode number, so that a human can easily remember and distinguish files.

Each element that represents a file in the directory-file is called as "directory entry." Each directory-entry includes i-node, and file names for the files in the sub-directory. Although it provides a convenient, human-readable interface, it makes some indirection.

examples:

to open a file "/file1":
1. mount the root filesystem when you boot your OS. (load i-node table)
2. look in the / directory-file to identify "file1"'s i-node 
    1. i-node of "/" is previously determined.
    2. In the "/" i-node, find out data blocks in the disk.
    3. read data blocks into memory, and interpret data as directory-entry
        1. parse all the data in the directory-file
        2. identify files in "/" directory
        3. compare the filename in directory-entry against "file1"
    4. find the i-node number for "file1"
        1. the directory-entry has i-node number
3. mark i-node for "/file1" as opened
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
    * Support the following file operations: mount, open, read, close
    * You should be able to read at least one (normal) file (open, read, close), rather than directory files.
* You can assume the following (or your own) data structure for superblock, i-nodes, and data blocks.
    * You can use pre-built filesystem image (disk.img), based on the following
    structure definitions:
    // look at code
* Kernel mounts the root file-system at system start:
    * At the booting time, the root-file-system (rootfs) has to be mounted.
    * Mounting rootfs populates all the files in the root-directory, and load i-nodes from the disk-image. Note that the capacity of a disk is so large, so all the contents of a disk cannot be loaded into the memory.
    * When booting the system, kernel mounts the root file system, and prints the files in the root-directory. (similar to ls -al)
* Single user file operations: open, read, close.
    * Assume a single child process.
    * The child process conducts only file operations. (open, read, close)
    * To access a file, we need to open the file before use it.
    * When a user opens a file, it requires two parameters: pathname and open mode(O_RD), and returns with the file descriptor
        * The kernel has to read the directory file and look up the i-node number for the corresponding file.
        * PCB should have a data structure for the open file. (file structure)
        * file structure should have a pointer to the corresponding i-node.
    * When a user reads a file, the user should provide three parameters: file descriptor, a buffer pointer, read data size.
        * File structure should have offset value, where the previous file operation has been stopped.
        * The kernel has to calculate the logical block numbers to obtain raw data blocks.
        * The kernel reads the disk"s physical data blocks.
        iv. Copy it to the user buffer
        v. return with actual read bytes.
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
* directory entry cache/hash
    * Directory files are one of the most frequently used files because all the files operations require directory access. Thus, you can cache the directory structure inside the memory, as well as disk.
    * To quickly access the correct directory entry, we can make an in-memory hash function for the directory structure. For example, we can make a hash function that takes keys from the file name, and generate value with the i-node pointer.
    * Because hash keys are smaller than the actual file names (space), you should check once again for exact file name match.
* write operation
    * You can simulate implement file write operation. To write to a file, you should provide data, along with the file pointer. If you are writing data to a file, the data is written from the beginning of the file.
    * You may need to consider data block allocation if the writing data size is over the data block boundary. In the case, you have to allocate a new data block from a free data block pool, and write it to i-node. Then, you should write data on the block.
    * After writing, you should be able to read data after the proper close operation.
    * Indirection blocks access to large files can also be considered.
* Buffer cache
    * To read-in data from disk, OS should prepare free memory region (free page)
    * You can link the disk operation with virtual memory free page.
    * When you read-in data, prepare free page frame that can store disk storage block.
    * The page frame can hold data in the storage, serving as a disk cache.
    * Then, you can read data from the buffer cache, instead of disk.
    * Because buffer cache is a cache to a disk, disk content would be different from the buffer cache. You should have a proper synchronization mechanism.
* Make disk image creation tool
    * A disk image file contains the raw image of a disk partition: superblock, inode table, data block.
        * you can use the following definitions, or you can define your own structure
    * Files population in a disk image.
        * You can give input file names or directory.
        * You can randomly generate filenames, contents.
    * Your kernel should be able to mount the disk image from your tool.
    * Output disk image is stored on file "disk.img."
    * Your file system would populate files named with file_[n] so that you can easily generate random file names.
* work with multiple users
    * When there are multiple users, file access should be properly managed by the kernel. Because our previous simulator allows multiple user processes, you can take control with it.
    * Each PCB should hold open file descriptor structure.
    * When a user tries to open a file, that is being used by another process; you have two choices.
        * Return failure for open syscall.
        * Make the process wait until the previous user closes the file, and the file is available again.

### Evaluation
Different credits are given for implementations and demos.