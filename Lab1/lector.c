#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lector.h"

//funcion que reinicia un array booleano dejando todos los valores en 0
//Entrada: un arreglo de enteros y su tamaño
//Salida: un arreglo de enteros
int* reset(int* array, int size){
    for(int i = 0; i < size; i++){
        array[i] = 0;
    }
    return array;
}

int verifyLector(int* array, int size){
    for(int i = 0; i < size; i++){
        if(!array[i]){
            return 0;
        }
    }
    return 1;
}

//funcion que dice si un arreglo booleano es verdadero
//Entrada: un arreglo de enteros y su tamano
//Salida: un entero que indica si es verdadero o falso
int isTrue(int* array, int size){
    for(int i = 0; i < size; i++){
        if(!array[i]){
            return 0;
        }
    }
    return 1;
}

//Funcion que imprime un array de enteros
//entrada: un array de enteros y su largo
void print(int* array, int size){
    for(int i = 0; i < size; i++){
        printf("%d ",array[i]);
    }
    printf("\n");
}
/*
Funcion que dice si se encuentra la sucesion en una cadena
Entrada: string con la cadena, el largo del string, string con la sucesion a buscar, largo de la sucesion, posicion inicial
Salida: entero que entrega si se encuentra la cadena en el archivo
*/
int isHere(char* cadena, int sizeCadena, char* succession, int sizeSuccession, int initialPosition){
    int *aux = (int*)malloc(sizeof(int) * sizeSuccession);
    
    if(initialPosition + sizeSuccession > sizeCadena){
        return 0;
    }
    else{
        
        for(int i = initialPosition; i + sizeSuccession <= sizeCadena; i++){
            
            for(int j = 0; j < sizeSuccession; j++){
                if(cadena[i + j] == succession[j]){
                    aux[j] = 1;
                    continue;
                }
                else{
                    aux = reset(aux, sizeSuccession);
                    break;
                }

            }
            
            if(isTrue(aux, sizeSuccession)){
                free(aux);
                return 1;
            }
            
        }
        return 0;
    }
}

//Funcion que escribe un archivo
//Entrada: el texto a escribir, la sucesion a buscar, el numero del proceso, booleano
void writeFile(char* text, char* cadena, char* processNumber, int found,int flag){
    char aux[100];
    
    strcpy(aux, "rc_");
    strcat(aux,cadena);
    strcat(aux,"_");
    strcat(aux,processNumber);
    strcat(aux, ".txt");
    printf("%s",aux);

    FILE * fp = fopen(aux, "a");
    if(!fp){
        printf("\n Unable to write : %s ", aux);
        
        return;
    }
    if(found){
        fprintf(fp, "%s SI\n",text);
        if(flag){
            printf(" %s SI\n",text);
        }
    }
    else{
        fprintf(fp, "%s NO\n",text);
        if(flag){
            printf(" %s NO\n",text);
        }
    }
    fclose(fp);
}
//Funcion que lee un archivo, busca la cadena en una posicion dada y busca una sucesion, escribiendo en un archivo si se encontro
//Entrada: el largo de la cadena, el nombre del archivo, la sucesion a buscar, el largo de la sucesion, el numero de la cadena inicial, hasta que cadena buscar, un id
void read(int sizeCadena,char* fileName,char* succession, int sizeSuccession, int chainNumber,int endingChain, char* id,int flag){
    char cadena[sizeCadena];
    FILE* file = fopen(fileName, "r"); 
    int i = 0;

    if(!file){
        printf("\n Unable to open : %s ", fileName);
        
        return;
    }
    while (fscanf(file, "%s\n", cadena) != EOF && i <= endingChain) {

        if(sizeCadena != sizeof(cadena)){
            printf("Error! numero de cadena mal ingresado \n");
            return;
        }
        else if(i >= chainNumber && i <= endingChain){
            
            if(cadena != "\n" && cadena != NULL ){
                int found = isHere(cadena,sizeCadena,succession,sizeSuccession,0);
                writeFile(cadena, succession, id,found,flag);
            }
        }
       
        i++;
    }

    fclose(file);
}



int main(int argc, char const *argv[])
{   
    

    int sizeCadena = atoi((char*)argv[1]);
    char* fileName = (char*)argv[2];
    char* sucession = (char*)argv[3];
    int sizeSucession = atoi((char*)argv[4]);
    int initialChain = atoi((char*)argv[5]);
    int endingChain = atoi((char*)argv[6]);
    char* id = (char*)argv[7];
    int flag = atoi((char*)argv[8]);
    read(sizeCadena,fileName,sucession,sizeSucession,initialChain,endingChain,id,flag);
    return 0;
}


