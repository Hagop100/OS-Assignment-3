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

//commandline arguments for master will be the executable, number of children, and name of shared memory segment
int main(int argc, char *argv[]) {
	printf("Master has begun execution.\n");

	//shm_open will be used to create a shared memory segment
	//it returns an integer called a file descriptor which can be used for ftruncate and mmap
	//O_CREAT and O_RDWR are flags that tell shm_open what we can do to the shared memory segment
	//O_CREAT will create a new share memory object if none exist and give it read and write access through O_RDWR
	int shm_fd = shm_open(argv[2], O_CREAT | O_RDWR, 0666);

	printf("Master created a shared memory segment named: ");
	printf("%s\n", argv[2]);

	//ftruncate will size the share memory segment according to our needs
	//we will make it the size of our struct because that is what we are using to store our data
	ftruncate(shm_fd, sizeof(struct CLASS));

	printf("Master created ");
	printf("%s", argv[1]);
	printf(" child processes to execute slave.\n");

	//our for loop here will execute the slave program the amount of children we have specificed in the commandline arguments
	for(int i = 1; i <= atoi(argv[1]); i++) {
		char childNum[256]; //this will be used to store i (child number) as a char arry for our exec call below
		sprintf(childNum, "%d", i); //sprintf will take i and store it as a char array
		pid_t pid = fork(); //we call fork here and a child process begins after this line

		//this char* array is used as an argument in our exec call below
		//the first index will always be the name of the executable
		//second index is the childNum created above with sprintf
		//third index is argv[2] which is the name given to our shared memory object which each child must have access to
		char* childArgs[] = {(char*)"./slave", childNum, argv[2], NULL}; 

		if(pid == 0) {
			//if pid == 0 then we are in child process and will call execvp()
			//we pass in childArgs[0] which is ./slave
			//second argument is the entire array which include the childNum and the name of shared memory object
			execvp(childArgs[0], childArgs);
		}
		else if(pid > 0) {
			//parent will wait for each child to complete before moving on to the next iteration of the for loop
			wait(NULL);
		}
	}
	//Special note that in my program, the master does not wait for all child processes to terminate in the end
	//Instead it will execute one child process at a time and will wait for that one to end before moving on
	//We could have called the wait system call outside of the for loop, however this will make the output on the console unordered
	//To avoid confusion and to enhance readability, I call wait within the for loop
	printf("Master waits for all child process to terminate.\n");
	printf("Master received termination signals from all ");
	printf("%s", argv[1]);
	printf(" child processes.\n");

	//we use mmap system call here to retrieve a pointer to the shared memory object instantiated with shm_open above
	//mmmap returns a void pointer, however in C we do not need to typecast, as a void pointer can be assigned to any other type of pointer
	//therefore we will assign it to a pointer of our struct CLASS
	//the first index will be null
	//the second index will describe the length of the mapping which will be the size of our struct indicated above in ftruncate
	//PROT_READ tells us that we can only read from the data we've been given access to through the mapping
	//MAP_SHARED indicated that the map is being shared between programs
	//shm_fd is our file descriptor which tells mmap which shared memory segment to pull from
	struct CLASS* shm = mmap(0, sizeof(struct CLASS), PROT_READ, MAP_SHARED, shm_fd, 0);

	printf("Content of shared memory segment filled by child processes:\n");
	printf("---content of shared memory---\n");

	//we simply print the data in our shared memory segment
	for(int i = 0; i < atoi(argv[1]); i++) {
		printf("%d\n", shm->response[i]);
	}

	//munmap will unmap our pointer from the shared memory segment making us unable to read from it anymore
	//if this fails then it returns -1, at which point we print an error message
	if(munmap(shm, sizeof(struct CLASS)) == -1) {
		printf("Unmapping failed: %s\n", strerror(errno));
		exit(1);
	}
	//close will close the shared memory segment
	//returning -1 indicates failure and so we will print it
	if(close(shm_fd) == -1) {
		printf("Close failed: %s\n", strerror(errno));
		exit(1);
	}

	//shm_unlink will deallocate and destory the contents of the shared memory segment that was created from shm_open
	shm_unlink(argv[2]);
	printf("Master removed shared memory segment, and is exiting.\n");

	return 0;
}