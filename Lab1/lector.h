#ifndef LECTOR_H
#define LECTOR_H
int* reset(int* array, int size);
int isTrue(int* array, int size);
void print(int* array, int size);
int isHere(char* cadena, int sizeCadena, char* succession, int sizeSuccession, int initialPosition);
void writeFile(char* text, char* cadena, char* processNumber, int found,int flag);
void read(int sizeCadena,char* fileName,char* succession, int sizeSuccession, int chainNumber,int endingChain, char* id, int flag);

#endif