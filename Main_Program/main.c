#include <sys/ipc.h>    
#include <sys/msg.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>      /// The main locks: used for synchronization purposes
#include <sys/mman.h>       /// for nmap() : used in semaphore to map it to a shared memory between the children and the parent processes
#include <unistd.h>         
#include <pthread.h>        /// for mutex handling
#include <sys/wait.h>       /// for signals and process functionality
#include "userinterface.h"  /// has the menu interface and functionaly from option 0 to 4



/* Global variables area - used in the handle_user_signal() */
sem_t *mutex;                   /// Semaphore for synchronization insurance
//pthread_mutex_t lock;            /// global lock for insuring the reading/writing synchronization from buffers: Not used anymore
const char * g_filename = NULL; /// file name of the database
int counter = 0;                /// number of people printed
int file_descriptors[2];        /// the pipe that going to be used for communication between process children
int vaccinated_people[2];       /// child to parent pipe


/*_______________________________  Funciton definitions area ___________________________ */
/*
*  Count the none vaccinated people and print them to the screen...
*  Returns: the count of the none vaccinated people
*/
int countNoneVaccinated(const char* dataFileName){
    FILE* pDatabase = fopen(dataFileName, "rb");

    if(!pDatabase){
        fclose(pDatabase);
        puts("Error with opening the file. Permission denied!");
        return -1;
    }

    int count = 0;          ///count of all elements in the file
    ApplicantData user;     ///temporary holder
    for(int i = 0 ; !feof(pDatabase) ; i++ ){
        if(fread(&user, sizeof(ApplicantData), 1, pDatabase) == 1 && user.vaccinated == 0){
            count++;
            printf("%d: (%s, %d,%s,%d,%d (No))\n", count, user.name, user.birthdate, user.phoneNumber, user.choice, user.vaccinated );
        }
    }

    fclose(pDatabase);
    return count;
}





/*
* Vaccination bus process. Child process, 
* Its job is to vaccinate 5 people with a chance of 90% each. then send back data to parent
*/
void vaccinationBus(){
    kill(getppid(), SIGUSR1); // sending signal to print the five names to be passed in this bus

    /*int val;
    if(sem_getvalue(mutex, &val) == -1) { printf("error in child"); exit(0);}
    printf("%d: is the sem value in child before waiting.\n", val );*/
    sem_wait(mutex);
    /*sem_getvalue(mutex, &val);
    printf("%d: is the sem value in child after waiting.\n", val ); */

    printf("\nprinting in the child...! \n");
    ApplicantData user;
    int count = 0;
    while(count < 5 && read(file_descriptors[0], &user, sizeof(ApplicantData)) > 0 ){
        printf("%d (%s, %d,%s,%d,%d (No) )\n", count, user.name, user.birthdate, user.phoneNumber, user.choice, user.vaccinated);
        srand(time(NULL)); //vaccinating people and appending to the pipe of the final vaccinated people
        int chance = rand() % 100; 
        if(chance < 90) user.vaccinated = 1; // vaccinate the user
        write(vaccinated_people[1], &user, sizeof(ApplicantData)); // write pass it back to the parent.
        count++;
    }
    printf("done printing \n\n");
    
    exit(1);
}





/*
*   Parameters: Filename-string 
*   Returns the total applicants data in the given file.. 
*/
int countList(const char* dataFileName){
    FILE* pDatabase = fopen(dataFileName, "rb");
    if(!pDatabase){
        fclose(pDatabase);
        puts("Error with opening the file. Permission denied!");
        return -1;
    }
    int count = 0;          ///count of all elements in the file
    ApplicantData user;     ///temporary holder
    while(fread(&user, sizeof(ApplicantData), 1, pDatabase) == 1){
        count++;
    }
    fclose(pDatabase);
    return count;
}







/*  Signal hanlder (Fight up signal) Note: this function uses theh global variables defined aboce and their values are defined in the
*   StartDay() funcion
*/
void handle_user_signal(){
    
    printf("\nSignal processing...!\n");
    
    if(g_filename == NULL) {printf("No global file name exists in the handler!\n");exit(-1);}

    /*Loop through all people and get the none vaccinated ones and delete htem from the main file at the end... */
    int count = 0;                                                   /// count of all elements in the file
    int indeces[5];                                                  /// indeces of the people
    FILE* pDatabase = fopen(g_filename, "rb");
    if(!pDatabase) {fclose(pDatabase);puts("Error with opening the file. Permission denied!");exit(-1);}
    ApplicantData user;         /// temporary holder
    for(int i = 0 ; !feof(pDatabase) && counter < 10; i++ ){
        if(fread(&user, sizeof(ApplicantData), 1, pDatabase) == 1 && user.vaccinated == 0){
            if(count >= 5) break;
            printf("%d: (%s, %d,%s,%d,%d (No) )\n", count, user.name, user.birthdate, user.phoneNumber, user.choice, user.vaccinated);
            write(file_descriptors[1], &user, sizeof(ApplicantData));
            indeces[count - counter] = i;
            count++;
        }
    }
    counter += count;  /// counter keeps track of the total counts we did so far
    fclose(pDatabase);

    /*deleting the chosen people from the file*/
    for(int i = 0 ; i < count ; i++){
        int totalLines = countList(g_filename);
        ApplicantData* pData = (ApplicantData*)malloc(sizeof(ApplicantData) * totalLines);               /// Temporarily buffer to store the data
        readData(pData , g_filename, totalLines);                                                        /// reading all data from file to the structure buffer
        chopWriting(pData, g_filename, indeces[count - i - 1], totalLines);                              /// Writing the buffer data to a file except for piece of data. Starting from bottom up since it is monotone increasing
        free(pData);
    }
    printf("Signal handled!!\n\n");

    printf("\nall applicants\n");
    listApplicants(g_filename);
    printf("all applicants\n");

    sem_post(mutex); // increasing the semaphore by 1 ( 1 child can start processing the data )
    return;
}







/*
* Start a new day for the committee to get data and process it..
*/
void startDay(const char* dataFileName){

    printf("Operating committe started!, we list all the people in the file!\n");

    /*initializing global variables*/
    counter = 0;
    pipe(file_descriptors);                                                                         /// for parent to child
    pipe(vaccinated_people);                                                                        /// child to parent
    g_filename = dataFileName;                                                                      /// having a global pointer so the parent signal handler can read the file name.
    mutex = (sem_t*)mmap(0, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, 0, 0 );  /// mapping the semaphore to the shared memory
    if ((void*)mutex == MAP_FAILED) { printf("mmap");  return; }                                    /// check if mapping successed
    if(sem_init(mutex, 1, 0) == -1){ printf("EROOOR");return; }                                     /// initialization of the shared semaphore between children


    /*Counting non vaccinated people*/
    int n_vaxcount = countNoneVaccinated(dataFileName);      /// list all none vaccinated people and return their count
    if(n_vaxcount < 5){
        printf("Number of none vaccinated people is not enough\n");
        return;
    }

    /* Task solution: initializing 2 child processes */
    int pid1 = -1;
    int pid2 = -1;
    if(n_vaxcount >= 5){ // create one bus
        pid1 = fork();
        if(pid1 == 0){ // child1
            printf("first bus started!!\n");
            vaccinationBus();
        }// end child1
        else{ // parent
            signal(SIGUSR1, handle_user_signal); // handling parent recieved signals

            if(n_vaxcount >= 10){ // starting the second bus if needed
                pid2 = fork();
                if(pid2 == 0){ // child2
                    printf("second bus started!!\n");
                    vaccinationBus();
                } //end child2
            }
            ///.... parent code!
            /*waiting for child process to terminate*/
            int returnStatus;
            if(pid1 != -1) waitpid(pid1, &returnStatus, 0); /// waiting for the children processess to finish
            if(pid2 != -1) waitpid(pid2, &returnStatus, 0); /// waiting for the children processess to finish
            printf("Children are dead!\n");

            /* Add the people that are vaccinated and terminate the day*/
            printf("\nFinal vaccinated people of the day..\n");
            FILE* pDatabase = fopen(dataFileName, "ab"); 
            if(!pDatabase){fclose(pDatabase); puts("Error with opening the file. Permission denied!"); return;}
            ApplicantData user;
            int i = 0;
            while( i < counter && read(vaccinated_people[0], &user, sizeof(ApplicantData)) > 0){
                printf("%d: (%s, %d,%s,%d,%d) : yes=1, no=0\n", i+1, user.name, user.birthdate, user.phoneNumber, user.choice, user.vaccinated);
                fwrite( &user , sizeof(ApplicantData), 1 , pDatabase);
                i++;
            }
            fclose(pDatabase);
            printf("finished day\n");

            sem_destroy(mutex); // destroying the semaphore

        } // end parent
    }
    return;
}




/* ______________________________ Main function area _______________________________________ */
int main(){

    const char* filename = "Data.bin";
    int menu_choice = loadMenu();

    while(menu_choice != 0){
        printf("Your choice is: %d\n", menu_choice);

        switch (menu_choice)
        {
            case 1:
                addApplicant(filename);
                break;
            case 2:
                modifyApplicant(filename);
                break;
            case 3:
                deleteApplicant(filename);
                break;
            case 4:
                listApplicants(filename);
                break;
            case 5:
                startDay(filename);
                break;
            default:
                break;
        }
	fflush(stdin);
        menu_choice = loadMenu();
    }


    return 0;
}