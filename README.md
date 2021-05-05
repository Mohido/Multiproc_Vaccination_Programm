# Multiproc_Vaccination_Programm
A simple terminal program for linux systems. It was made to implement the basics of multi-processes , files data manipulation with C.

# Notes: 
1) This is a linux exclusive program that runs on linux based systems.
2) Data.bin is the normal database file that the program uses.. you can: add, modify, delete or list all the data in the file. Plus, you can vaccinate the people in the file.
3) Data2.bin is a backup of the original given data for testing. Data.bin contains the data after a certien procedure (Vaccinating procedure). So to start from the begining overwrite the Data.bin with the backed up file ( Data2.bin).
4) userinterface.c contains the functionality of the first 4 options in the menu, while the "vaccinate people - start day" option is implemented in the main.c
5) main.c is responsible for multi-processing part implementation. while userinterface.c is the responsible of handling file manipulation.

# How to use:
1) Make sure that you are on a linux based system.
2) Open a terminal and navigate to the program working directory.
3) write the following command: <code> gcc -pthread main.c </code>
4) the program will compile succesfully.
5) Run the program by typing: <code> ./a.out </code> or any specific name that you chose when compiling it.
