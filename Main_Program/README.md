# Description:
A Full program that has an interface. 
It is mainly used for inputting data to a file, process these data, modify,delete them as well.
Finally, it has a multiprocessing (IPC) simple program for processing some task (starting a day to vaccinate the people).

# Notes: 
1) This is a linux exclusive program that runs on linux based systems.
2) Data.bin is the normal database file that the program uses.. you can: add, modify, delete or list all the data in the file. Plus, you can vaccinate the people in the file.
3) Data2.bin is a backup of the original given data for testing. Data.bin contains the data after a certien procedure (Vaccinating procedure). So to start from the begining overwrite the Data.bin with the backed up file ( Data2.bin).
4) userinterface.c contains the functionality of the first 4 options in the menu, while the "vaccinate people - start day" option is implemented in the main.c
5) main.c is responsible for multi-processing part implementation. while userinterface.c is the responsible of handling file manipulation.