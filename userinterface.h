#ifndef userinterface_h
#define userinterface_h

/*The structure for minimpulating the data (the database relation) */
typedef struct ApplicantData
{
    char name[20];                   //e.g: Vaci Nation
    int birthdate;                   // e.g: 1953
    char phoneNumber[12];            // e.g: 36301234567
    int choice;                      // e.g: yes/no (boolean)
    int vaccinated;
} ApplicantData ;



/*
* Prints the menu and returns the chosen option.
*/
int loadMenu(){

    printf("\n\n");
    printf("_______________________________________________________Insert An Option_____________________________________________________\n");
    puts("1: To add an applicant");
    puts("2: To modify an applicant");
    puts("3: To delete an applicant");
    puts("4: To print all applicants data");
    puts("5: To start a new committee day");
    puts("0: To Exit");
    puts("________________________________________________");
    fflush(NULL);
    char option = getchar();

    while( option == '\n'){option = getchar();}
    

    fflush(NULL);                   /// The next line stays in the buffer, so we flush the buffer once we are done with it .. It only gets first value.

    switch (option)
    {
        case '0':
            return 0;
            break;
        case '1':
            return 1;
            break;
        case '2':
            return 2;
            break;
        case '3':
            return 3;
            break;
         case '4':
            return 4;
            break;
        case '5':
            return 5;
            break;
        default:
            return -1;
            break;
    }
}







/**
 * Add applicants to the given file name.
 * If no file is found, it creates oneo
 * */
void addApplicant(const char* dataFileName){
    FILE* pDatabase = fopen(dataFileName, "ab");

    if(!pDatabase){
        fclose(pDatabase);
        puts("Error with opening the file. Permission denied!");
        return;
    }

    ///inserting data!.. file data would look like (name,birthdate,phonenumber,choice)
    ApplicantData user;

    user.vaccinated = 0;    // by default the person is not vaccinated

    fflush(stdin);
    printf("Please Insert your name: ");

    char option = getchar(); while( option == '\n'){option = getchar();} //clearing newspaces
    user.name[0] = option; // returning 
    fgets(&user.name[1],19, stdin);
    user.name[strlen(user.name) - 1] = '\0';

    printf("Please Insert your birthdate: ");
    scanf("%d", &(user.birthdate));     fflush(stdin);

    printf("Please Insert your phone number: ");
    scanf("%s", &(user.phoneNumber));     fflush(stdin);

    //option=getchar(); while( option == '\n'){option = getchar();} //clearing newspaces

    printf("Do you want to pay for it? (yes = 1 / no = 0) only answers ");
    scanf("%d", &(user.choice));     fflush(stdin);

    fwrite( &user , sizeof(ApplicantData), 1 , pDatabase);
    
    fclose(pDatabase);
    return;
}
    



/**
 * Lists all the data in the given file.
 * returns: The count of the entries withen that file.
 * */
int listApplicants(const char* dataFileName){
    FILE* pDatabase = fopen(dataFileName, "rb");

    if(!pDatabase){
        fclose(pDatabase);
        puts("Error with opening the file. Permission denied!");
        return -1;
    }

    int count = 0;          ///count of all elements in the file
    ApplicantData user;     ///temporary holder
    while(fread(&user, sizeof(ApplicantData), 1, pDatabase) == 1){
        printf("%d: (%s, %d,%s,%d,%d)\n",count, user.name, user.birthdate, user.phoneNumber, user.choice, user.vaccinated );
        count++;
    }

    fclose(pDatabase);
    return count;
}







/**
 * A function that is used to read data from a Binary file to the given data structure.
 * It reads data until the giving reading lines.
 * */
void readData(ApplicantData* pData, const char * dataFileName, int readingLines){
    FILE* pDatabase = fopen(dataFileName, "rb");

    if(!pDatabase){
        fclose(pDatabase);
        return;
    }

    fread(pData, sizeof(ApplicantData), readingLines, pDatabase);

    fclose(pDatabase);
    return;
}







/**
 * A helper function that is used to write the data structure into the Binary file with the given name, 
 * with ignoring 1 index of the given datastructure. ignoredLine can be put to -1 if you want everything to obe written with out 
 * ignoring specific value.
 * */
void chopWriting(ApplicantData* pData, const char * dataFileName, int ignoredLine, int totalLines){
    FILE* pDatabase = fopen(dataFileName, "wb");

    if(!pDatabase){
        fclose(pDatabase);
        return;
    }

    for(int i = 0 ; i < totalLines; i++){
        if(i != ignoredLine){
             fwrite( &pData[i] , sizeof(ApplicantData), 1 , pDatabase);
        }
    }

    fclose(pDatabase);
    return;
}






/**
 * shows all the data and asks which one you want to delete.
 * */
void deleteApplicant(const char* dataFileName){

    printf("Please choose which file you want to delete from the giving list.\n");
    int totalLines = listApplicants(dataFileName);      /// Lists all the users and gives back their count.

    int lineNumber = -1;
    printf("your selection is: ");
    scanf("%d", &lineNumber);           fflush(stdin);

    if(lineNumber >= totalLines){
        printf("Error: The given number is larger than the file lines count!\n");
        return; 
    }

    ApplicantData* pData = (ApplicantData*)malloc(sizeof(ApplicantData) * totalLines);              /// Temporarily buffer to store the data
    readData(pData , dataFileName, totalLines);                                                      ///reading all data from file to the structure buffer
    chopWriting(pData, dataFileName, lineNumber, totalLines);                                         ///Writing the buffer data to a file except for piece of data.
    free(pData);
}







/**
 * Shows all the data and asks you which one you want to modify. Then you will be asked to enter the new data.
 * */
void modifyApplicant(const char* dataFileName){

    printf("Please choose which entry you want to modify from the giving list.\n");
    int totalLines = listApplicants(dataFileName);      /// Lists all the users and gives back their count.

    int lineNumber = -1;
    printf("your selection is: ");
    scanf("%d", &lineNumber);           fflush(stdin);

    if(lineNumber >= totalLines){
        printf("Error: The given number is larger than the file lines count!\n");
        return; 
    }

    ApplicantData* pData = (ApplicantData*)malloc(sizeof(ApplicantData) * totalLines);              /// Temporarily buffer to store the data
    readData(pData , dataFileName, totalLines);  

    printf("\n_______Modifying:\n");
    printf("Please Insert the name: ");
    scanf("%s", &(pData[lineNumber].name));                fflush(stdin);
    
    printf("Please Insert the birthdate: ");
    scanf("%d", &(pData[lineNumber].birthdate));    fflush(stdin);

    printf("Please Insert the phone number: ");
    scanf("%s", &(pData[lineNumber].phoneNumber));    fflush(stdin);

    printf("Do you want to pay for it? (yes = 1 / no = 0) only answers ");
    scanf("%d", &(pData[lineNumber].choice));    fflush(stdin);

    chopWriting(pData, dataFileName, -1, totalLines);    /// -1 to not ignore any line while writing to the file!
    free(pData);
}



#endif