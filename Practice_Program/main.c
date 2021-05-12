//===================
// Compile with -pthread
//===================

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/msg.h> 


// The report Structure (Used when child process report to parent)
typedef struct {
	int incomingPatients;
	int vaccinatedPatients;
} Report;

// Message queue structure
struct messg { 
    long mtype;			//this is a free value e.g for the address of the message
    Report mrep; 		//this is the message itself
}; 
typedef struct messg Message;


/*Main program code*/
int msgid; 						/// the message ID of the communication
int file_descriptors[2];        /// the pipe that going to be used for communication between process children
int file_descriptors1[2];		/// pipe for the second child  
								/*Note: 2 pipes needed since 1 child will take the even processed data,
										while the other will take the odd ones*/



/*
* SIGUSR1 signal handler
*/
void handle_signal(){
	printf("1 child process begin\n");
	return;
}


/**
 * A full process incapsulated in a funciton. It reads data from a pipe (filled in the parent). 
 * And process the data. Finally return the processed data to the parent through a message queue
 */
void firstChild(){
	kill(getppid(), SIGUSR1);

	Report rep = {0,0}; // no incomiing and no vaccinated
	/*recieving people data from the parent*/

	/*closing first child discreptors*/
	close(file_descriptors1[0]);
	close(file_descriptors1[1]);


	/*recieving people data from the parent*/
	close(file_descriptors[1]);
	int vaccine_shot;
	while(read(file_descriptors[0], &vaccine_shot,sizeof(int)) > 0){
		printf("recieved in Ursulu someone who needs: %d vaccine shots\n", vaccine_shot);
		rep.incomingPatients++;
		int chance = rand() % 100 + 1; // 1-100%
		if(chance <= 50){ //sickness chance
			rep.vaccinatedPatients++; // increasing the count of the vaccinated people
			printf("Ursulu gave to patient all shots!\n");
		} else{
			printf("Ursulu DIDNOT vaccinate the patient!\n");
		}
	}
	close(file_descriptors[0]);

	printf("\n*** Ursulu vaccinated %d people!! ***\n", rep.vaccinatedPatients);


	/*reporting data to teh parent*/
	const Message m = { 5, {rep.incomingPatients, rep.vaccinatedPatients}}; 
    int status = msgsnd( msgid, &m, sizeof(Message) , 0 ); 
    if ( status < 0 ) perror("msgsnd error"); 

	exit(0);
}




/**
 * A full process incapsulated in a funciton. It reads data from a pipe (filled in the parent). 
 * And process the data. Finally return the processed data to the parent through a message queue
 */
void secondChild(){
	sleep(1); // avoiding signal dropping
	kill(getppid(), SIGUSR1);

	Report rep = {0,0}; // no incomiing and no vaccinated

	/*recieving people data from the parent*/
	close(file_descriptors[0]); // closing first child discreptors
	close(file_descriptors[1]);


	close(file_descriptors1[1]);
	int vaccine_shot;
	while(read(file_descriptors1[0], &vaccine_shot,sizeof(int)) > 0){
		printf("recieved in Beakmaster someone who needs: %d vaccine shots\n", vaccine_shot);
		rep.incomingPatients++;
		int chance = rand() % 100 + 1; // 1-100%
		if(chance <= 50){
			rep.vaccinatedPatients++; // increasing the count of the vaccinated people
			printf("Beakmaster gave to patient all shots!\n");
		} else{
			printf("Beakmaster DIDNOT vaccinate the patient!\n");
		}
	}
	close(file_descriptors1[0]);

	printf("\n*** Beakmaster vaccinated %d people!! *** \n", rep.vaccinatedPatients);
	/*reporting data to teh parent*/
	const Message m = { 5, {rep.incomingPatients, rep.vaccinatedPatients}}; 
    int status = msgsnd( msgid, &m, sizeof(Message) , 0 ); 
    if ( status < 0 ) perror("msgsnd error"); 


	exit(0);
}




/**
 * Parent Process: It generates data and send them to the children through a pipe. 
 * After that it saves the reported back data from the children in a file
 */
void parent(int pid1, int pid2){

	/*waiting for children process*/
	pause();
	pause();

	/*Parent main job*/
	int waitingPatients;
	printf("\nHow many patients are there? : ");
	scanf("%d", &waitingPatients);
	printf("\n", waitingPatients);

	/*writing data to a pipe*/
	close(file_descriptors[0]);
	close(file_descriptors1[0]);
	for(int i = 0 ; i < waitingPatients; i++){
		int vaccine_shot = 1; 	// 1 shot or 2 shots
		if(i % 2 == 0) // even goes to ursulo
			write(file_descriptors[1], &vaccine_shot, sizeof(int));
		else// odd goes to beakmaster
			write(file_descriptors1[1], &vaccine_shot, sizeof(int));
	}
	close(file_descriptors[1]);
	close(file_descriptors1[1]);

	/*terminating children*/
	int returnStatus;
	if(pid1 != -1) waitpid(pid1, &returnStatus, 0); /// waiting for the children processess to finish
	if(pid2 != -1) waitpid(pid2, &returnStatus, 0); /// waiting for the children processess to finish
	printf("\nChildren are dead!\n\n");

	/*reading reports from message queue*/
	Report finalrep = {0,0};
	Message m;	
    int status = msgrcv(msgid, &m, sizeof(Message) , 5, 0 ); 
    if ( status < 0 ) perror("msgsnd error"); 
    else
	{	
		finalrep.incomingPatients +=  m.mrep.incomingPatients;
		finalrep.vaccinatedPatients +=  m.mrep.vaccinatedPatients;
		printf("1 child report is:\n 1) total patients: %d\n 2) vaccinated patients: %d\n", m.mrep.incomingPatients, m.mrep.vaccinatedPatients);
	}
	status = msgrcv(msgid, &m, sizeof(Message) , 5, 0 ); 
    if ( status < 0 ) perror("msgsnd error"); 
    else
	{	
		finalrep.incomingPatients +=  m.mrep.incomingPatients;
		finalrep.vaccinatedPatients +=  m.mrep.vaccinatedPatients;
		printf("1 child report is:\n 1) total patients: %d\n 2) vaccinated patients: %d\n", m.mrep.incomingPatients, m.mrep.vaccinatedPatients);
	}
	

	/*reporting to a file*/
	char* dataFileName = "data.bin";
	FILE* pDatabase = fopen(dataFileName, "wb");
	if(!pDatabase){
        fclose(pDatabase);
        puts("Error with opening the file. Permission denied!");
        return;
    }
	printf("Report with: %d total patients, and %d vaccinated patients, have been logged to data.bin file\n", finalrep.incomingPatients,finalrep.vaccinatedPatients);
	fwrite( &finalrep , sizeof(Report), 1 , pDatabase);		// writing the data to the file
    fclose(pDatabase);										// closing the reporting file
	printf("\n\nDoctor closes the clinic!\n\n");			// closing the doctor

	return;
}




//------------------------------ Main funciton -------------------------------

int main(int argc, char* argv[]){
	// initializing area
	srand(time(NULL)); // for how many shots of vaccine per patient
	signal(SIGUSR1, handle_signal);
	pipe(file_descriptors); 	// first child pipe
	pipe(file_descriptors1);	// second child pipe
    key_t key = ftok(argv[0],1); 
	msgid = msgget( key, 0600 | IPC_CREAT );
	if ( msgid < 0 ) { perror("msgget error"); return 1; } 

	int c1 = fork();
	if(c1 == 0)	firstChild();
	else{
		int c2 = fork();
		if(c2 == 0) secondChild();
		else {
			parent(c1,c2);
			if ( msgctl( msgid, IPC_RMID, NULL ) < 0 ) perror("msgctl error");   // destroying the message queue
          	return 0; 
		}
	}
	return 0;
}