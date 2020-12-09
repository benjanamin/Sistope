#ifndef MAIN_H
#define MAIN_H
int verify(int* array, int size);
char* nameMaker(char* sucession);
char* getName(int processNumber, char* cadena);
void writeFileMain(char*fileName,char* sucession,int sizeOfCadena);
int distributeProcess(int numberOfLines, int NumberOfProcess);
void controller(int numberOfProcess,int numberOfLines,int sizeCadena,char* fileName,char* sucession, int sizeSuccession, int flag);

#endif