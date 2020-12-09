#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "main.h"


/*
POR FAVOR LEEER

No logre encontrar una manera para hacer que se distribuyeran los procesos asi que siempre se ejecutara 1 por linea,
sin embargo se lee desde la consola cuantos procesos se quieren


*/
/*
Funcion que dice cuantas lineas tiene un archivo
Entrada: nombre del archivo, largo de cada linea
Salida: un entero que indica la cantidad de lineas que tiene el archivo
*/
int verify(int* array, int size){
    for(int i = 0; i < size; i++){
        if(!array[i]){
            return 0;
        }
    }
    return 1;
}
/*
Funcion que genera el nombre del archivo
Entrada: un string con la sucesion
Salida: un string
*/
char* nameMaker(char* sucession){
    char* aux = (char*)malloc(sizeof(char)*100);
    strcpy(aux, "rc_");
    strcat(aux, sucession);
    strcat(aux,".txt"); 
    return aux;
}
/*
Funcion que genera el nombre del archivo de un proceso
Entrada: un int de un id, un string con la sucesion a buscar
Salida: un string
 */
char* getName(int processNumber, char* cadena){
    char* aux = (char*)malloc(sizeof(char)*100);
    char* numberToString = (char*)malloc(sizeof(char));
    sprintf(numberToString,"%d",processNumber);
    strcpy(aux, "rc_");
    strcat(aux,cadena);
    strcat(aux,"_");
    strcat(aux,numberToString);
    strcat(aux, ".txt");
    return aux;
}
/*
Funcion que lee los archivos que generan las cadenas y escribe en el archivo final
Entrada: nombre del archivo, la sucesion, el largo de las cadenas
*/
void writeFileMain(char*fileName,char* sucession,int sizeOfCadena){
    char aux[sizeOfCadena];
    char* name = nameMaker(sucession);
    FILE * reader = fopen(fileName, "r");
    FILE * writer = fopen(name, "a");
    if(!reader || !writer){
        printf("\n Unable to write or read: %s ", fileName);
        return;
    }
    while (fscanf(reader, "%s", aux) != EOF){
        if(strstr(aux,"SI") || strstr(aux,"NO")){
            fprintf(writer, "%s \n",aux);
        }
        else{
            fprintf(writer, "%s ",aux);
        }
    }
    fclose(reader);
    fclose(writer);
}
/*
Funcion que calcula la cantidad de cadenas que cada proceso debe analizar
Entrada: el numero de lineas que tiene el archivo, el numero de procesos que se quiere ejecutar
Salida: un entero
*/
int distributeProcess(int numberOfLines, int NumberOfProcess){
    if(NumberOfProcess > numberOfLines){
        return 1;
    }
    else{
        int aux = numberOfLines / NumberOfProcess;
        return aux;
    }
}


//(sizeCadena,fileName,sucession,sizeSucession,initialChain,endingChain,id)

/*
Funcion que se encarga de controladora
Entrada: el numero de procesos, el numero de lineas que tiene el archivo, el largo de las cadenas, el nombre del archivo
la sucesion a buscar, el largo de la sucesion, un booleano

*/
void controller(int numberOfProcess,int numberOfLines,int sizeCadena,char* fileName,char* sucession, int sizeSuccession, int flag){

   
    //int lines = numberOfLines;
    char* sizeCadenaChar =(char*)malloc(sizeof(char));
    char* sizeSucessionChar =(char*)malloc(sizeof(char));
    char* iChar =(char*)malloc(sizeof(char));
    char* idChar =(char*)malloc(sizeof(char));
    char* flagChar = (char*)malloc(sizeof(char));
    sprintf(sizeCadenaChar,"%d",sizeCadena);
    sprintf(sizeSucessionChar,"%d",sizeSuccession);
    sprintf(flagChar,"%d",flag);

    int fd[numberOfProcess][2];
    char** names = (char**)malloc(sizeof(char*)*numberOfProcess);

    //fd 0 read
    //fd 1 write
    
    for(int i=0;i<numberOfProcess;i++) {   
        
        if(pipe(fd[i]) == -1){
            printf("falla en crear un pine \n");
            return;
        }
        if(fork() == 0) 
        {   
            close(fd[i][0]);
            int number = getpid();
            
            char* name = getName(number, sucession);
            write(fd[i][1], name,sizeof(char)*50);
            close(fd[i][1]);
            sprintf(iChar,"%d",i);
            sprintf(idChar,"%d",number);
            //read(sizeCadena,fileName,sucession,sizeSucession,initialChain,endingChain,id);
            char* args[] = {"./lector",sizeCadenaChar,fileName,sucession,sizeSucessionChar,iChar,iChar,idChar,flagChar,NULL};
            
            execv(args[0],args);
            //ejecutar el exec
            
            exit(1);
        }
        else{
           
            names[i] = (char*)malloc(sizeof(char)*50);
            close(fd[i][1]);
            read(fd[i][0],names[i],sizeof(char)*50);
            close(fd[i][1]);
            wait(NULL);
        } 
    } 
    wait(NULL);

    for (int i = 0; i < numberOfProcess; i++)
    {      
        writeFileMain(names[i],sucession,2);
    }
}

int main(int argc, char* argv[]){
    int opt, processNumber,numberOfLines, sucessionSize;
    int flag = 0;
    char* name;
    char* aux3;
    char* sucession;
    int* aux = (int*)malloc(sizeof(int)*4);
    while((opt = getopt(argc, argv, "i:n:c:p:d")) != -1) { 
        switch(opt){
            case 'i':
                name = optarg;
                aux[0] = 1;
                break;
            case 'n':
                processNumber = strtol(optarg, &aux3, 10);
                aux[1] = 1;
                break;

            case 'c':
                numberOfLines = strtol(optarg, &aux3, 10);
                aux[2] = 1;
                break;
            case 'p':
                sucession = optarg;
                sucessionSize = strlen(sucession);
                aux[3] = 1;
                break;
            case 'd':
                flag = 1;
                break;
            default:
                printf("mal ingresado, por favor ingrese los valores bien \n");
                exit(EXIT_FAILURE);
                break;
        }
	}
    if(!verify(aux,4)){
        printf("mal ingresado, por favor ingrese los valores bien \n");
        exit(EXIT_FAILURE);
    }
    controller(numberOfLines,processNumber,numberOfLines,name,sucession,sucessionSize,flag);
    
    return 0;

}