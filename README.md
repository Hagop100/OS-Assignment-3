# OS-Assignment-3

Program Description

The purpose of this assignment is to create a shared memory segment that two
programs will use. The master will create this shared memory segment and will then execute a
number of child processes indicated in the command line arguments. These child processes will
then write to the shared memory segment their child number. After executing the child
processes, the master will display the contents of the shared memory segment, demonstrating
communication between the two programs via shared memory.
The command line arguments passed into the master program will be the executable,
the number of child processes we want to create, and the name of our shared memory object.
The master program begins by creating a new shared memory segment using the shm_open
system call. The name given to this shared memory object will be the name defined in the
command line parameters. The file descriptor integer that this function returns will be used
throughout the program to access this specific shared memory segment created. Next we use
ftruncate to define the size of our shared memory segment. In this case it will be the size of our
struct CLASS because that is what will be holding the data. We then begin our for loop, where
the loop will run as many times as there are child processes defined in the command line
parameters. We will fork within the for loop and call execvp if the pid is equal to 0. It is important
to note here that the arguments passed in each time into execvp will change with each iteration
of the for loop as we must pass in the appropriate child number each time. This is accounted for
in the beginning of each for loop. Additionally, if we are in the parent process, then we will call
wait(NULL) which will wait for each child to finish processing before executing a new one. This
will also keep our output in the console ordered properly. We could have called wait outside of
the for loop, but this would make our child processes interweave and create a less organized
output. After the child processes are done, we use mmap to gain reference to our shared
memory object and we assign the void pointer it returns to a pointer of our struct CLASS. This
gives us access to the contents that were written into the shared memory object in the child
processes. We then print the data from the shared memory segment and unmap the pointer to
the shared memory segment and close the shared memory segment. Finally, we use
shm_unlink to deallocate and destroy the contents of the shared memory segment.

In the slave process, we are simply writing data into the shared memory segment for the
master process to read. This data will be the child number since each execution of the slave
program is a child process. We begin by calling shm_open again to gain access to the shared
memory segment. We then use mmap to gain a mapping to the shared memory segment so that
we can write data to it. Again, we store the void pointer it returns into a pointer of type struct
CLASS. We can now begin to write data into the shared memory segment. If we are in the first
child process, then we will set our index used to access our array to 0. We will then store the
child number into the response array in our struct CLASS object. We then must increment the
index variable so that the next child process will know which index to store the next piece of
data in. Finally, we unmap the poiunter and close the shared memory segment and return 0.
