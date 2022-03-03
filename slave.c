#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>

//struct used to store data in shared memory segment
//response will store the message itself (child number)
//index will hold the value of the next available space in response
struct CLASS {
	int index;
	int response[10];
};

int main(int argc, char *argv[]) {
	printf("Slave begins execution.\n");

	printf("I am child number ");
	printf("%s", argv[1]);
	printf(" received shared memory name ");
	printf("%s", argv[2]);
	printf("\n");

	//shm_open will open our shared memory segment
	//this memory segment however already exists as created in our master program
	//we do not use the O_CREAT flag here but only give read and write access
	int shm_fd = shm_open(argv[2], O_RDWR, 0666);

	//we use mmap system call here to retrieve a pointer to the shared memory object instantiated with shm_open above
	//mmmap returns a void pointer, however in C we do not need to typecast, as a void pointer can be assigned to any other type of pointer
	//therefore we will assign it to a pointer of our struct CLASS
	//the first index will be null
	//the second index will describe the length of the mapping which will be the size of our struct
	//PROT_WRITE will give us access to write to our shared memory segment as that is the purpose of the slave
	//MAP_SHARED indicated that the map is being shared between programs
	//shm_fd is our file descriptor which tells mmap which shared memory segment to pull from
	struct CLASS *shared_memory = mmap(0, sizeof(struct CLASS), PROT_WRITE, MAP_SHARED, shm_fd, 0);

	//our shared memory pointer == MAP_FAILED then we print the mapping failed
	if(shared_memory == MAP_FAILED) {
		printf("MAP FAILED\n");
	}

	//this will make sure that if our child number is one then we set the index to start at 0
	//otherwise we would get an error because our index would be empty
	if(atoi(argv[1]) == 1) {
		shared_memory->index = 0;
	}

	//this stores the child number into our struct CLASS object 
	shared_memory->response[shared_memory->index] = atoi(argv[1]);

	printf("I have written my child number to shared memory.\n");

	//increments the index by 1 for next child process to use
	//otherwise we would overwrite the same index in the array
	shared_memory->index++;

	//munmap will unmap our pointer from the shared memory segment making us unable to read from it anymore
	//if this fails then it returns -1, at which point we print an error message
	if(munmap(shared_memory, sizeof(struct CLASS)) == -1) {
		printf("Unmapping failed: %s\n", strerror(errno));
		exit(1);
	}
	//close will close the shared memory segment
	//returning -1 indicates failure and so we will print it
	if(close(shm_fd) == -1) {
		printf("Close failed: %s\n", strerror(errno));
		exit(1);
	}

	printf("Slave closed access to shared memory and terminates.\n");

	return 0;
}	