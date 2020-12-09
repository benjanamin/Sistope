#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <pthread.h>
#include <unistd.h>
#include "libbmp.h"
#include "hist.h"

/*
 libbmp.c
 https://github.com/marc-q/libbmp
Libreria

*/
void
bmp_header_init_df (bmp_header *header,
                    const int   width,
                    const int   height)
{
	header->bfSize = (sizeof (bmp_pixel) * width + BMP_GET_PADDING (width))
	                  * abs (height);
	header->bfReserved = 0;
	header->bfOffBits = 54;
	header->biSize = 40;
	header->biWidth = width;
	header->biHeight = height;
	header->biPlanes = 1;
	header->biBitCount = 24;
	header->biCompression = 0;
	header->biSizeImage = 0;
	header->biXPelsPerMeter = 0;
	header->biYPelsPerMeter = 0;
	header->biClrUsed = 0;
	header->biClrImportant = 0;
}

enum bmp_error
bmp_header_write (const bmp_header *header,
                  FILE             *img_file)
{
	if (header == NULL)
	{
		return BMP_HEADER_NOT_INITIALIZED; 
	}
	else if (img_file == NULL)
	{
		return BMP_FILE_NOT_OPENED;
	}
	
	// Since an adress must be passed to fwrite, create a variable!
	const unsigned short magic = BMP_MAGIC;
	fwrite (&magic, sizeof (magic), 1, img_file);
	
	// Use the type instead of the variable because its a pointer!
	fwrite (header, sizeof (bmp_header), 1, img_file);
	return BMP_OK;
}

enum bmp_error
bmp_header_read (bmp_header *header,
                 FILE       *img_file)
{
	if (img_file == NULL)
	{
		return BMP_FILE_NOT_OPENED;
	}
	
	// Since an adress must be passed to fread, create a variable!
	unsigned short magic;
	
	// Check if its an bmp file by comparing the magic nbr:
	if (fread (&magic, sizeof (magic), 1, img_file) != 1 ||
	    magic != BMP_MAGIC)
	{
		return BMP_INVALID_FILE;
	}
	
	if (fread (header, sizeof (bmp_header), 1, img_file) != 1)
	{
		return BMP_ERROR;
	}

	return BMP_OK;
}

// BMP_PIXEL

void
bmp_pixel_init (bmp_pixel           *pxl,
                const unsigned char  red,
                const unsigned char  green,
                const unsigned char  blue)
{
	pxl->red = red;
	pxl->green = green;
	pxl->blue = blue;
}

// BMP_IMG

void
bmp_img_alloc (bmp_img *img)
{
	const size_t h = abs (img->img_header.biHeight);
	
	// Allocate the required memory for the pixels:
	img->img_pixels = malloc (sizeof (bmp_pixel*) * h);
	
	for (size_t y = 0; y < h; y++)
	{
		img->img_pixels[y] = malloc (sizeof (bmp_pixel) * img->img_header.biWidth);
	}
}

void
bmp_img_init_df (bmp_img   *img,
                 const int  width,
                 const int  height)
{
	// INIT the header with default values:
	bmp_header_init_df (&img->img_header, width, height);
	bmp_img_alloc (img);
}

void
bmp_img_free (bmp_img *img)
{
	const size_t h = abs (img->img_header.biHeight);
	
	for (size_t y = 0; y < h; y++)
	{
		free (img->img_pixels[y]);
	}
	free (img->img_pixels);
}

enum bmp_error
bmp_img_write (const bmp_img *img,
               const char    *filename)
{
	FILE *img_file = fopen (filename, "wb");
	
	if (img_file == NULL)
	{
		return BMP_FILE_NOT_OPENED;
	}
	
	// NOTE: This way the correct error code could be returned.
	const enum bmp_error err = bmp_header_write (&img->img_header, img_file);
	
	if (err != BMP_OK)
	{
		// ERROR: Could'nt write the header!
		fclose (img_file);
		return err;
	}
	
	// Select the mode (bottom-up or top-down):
	const size_t h = abs (img->img_header.biHeight);
	const size_t offset = (img->img_header.biHeight > 0 ? h - 1 : 0);
	
	// Create the padding:
	const unsigned char padding[3] = {'\0', '\0', '\0'};
	
	// Write the content:
	for (size_t y = 0; y < h; y++)
	{
		// Write a whole row of pixels to the file:
		fwrite (img->img_pixels[abs (offset - y)], sizeof (bmp_pixel), img->img_header.biWidth, img_file);
		
		// Write the padding for the row!
		fwrite (padding, sizeof (unsigned char), BMP_GET_PADDING (img->img_header.biWidth), img_file);
	}
	
	// NOTE: All good!
	fclose (img_file);
	return BMP_OK;
}

enum bmp_error
bmp_img_read (bmp_img    *img,
              const char *filename)
{
	FILE *img_file = fopen (filename, "rb");
	
	if (img_file == NULL)
	{
        exit(1);
		return BMP_FILE_NOT_OPENED;
	}
	
	// NOTE: This way the correct error code can be returned.
	const enum bmp_error err = bmp_header_read (&img->img_header, img_file);
	
	if (err != BMP_OK)
	{
		// ERROR: Could'nt read the image header!
		fclose (img_file);
		return err;
	}
	
	bmp_img_alloc (img);
	
	// Select the mode (bottom-up or top-down):
	const size_t h = abs (img->img_header.biHeight);
	const size_t offset = (img->img_header.biHeight > 0 ? h - 1 : 0);
	const size_t padding = BMP_GET_PADDING (img->img_header.biWidth);
	
	// Needed to compare the return value of fread
	const size_t items = img->img_header.biWidth;
	
	// Read the content:
	for (size_t y = 0; y < h; y++)
	{
		// Read a whole row of pixels from the file:
		if (fread (img->img_pixels[abs (offset - y)], sizeof (bmp_pixel), items, img_file) != items)
		{
			fclose (img_file);
			return BMP_ERROR;
		}
		
		// Skip the padding:
		fseek (img_file, padding, SEEK_CUR);
	}
	
	// NOTE: All good!
	fclose (img_file);
	return BMP_OK;
}

/*

Funciones numericas

*/
//Funcion que ve si un numero es potencia de 2
//Entrada: un numero entero
//Salida: un booleano
int checkPower(int number){
    int n = 2;
    while(n < number){
        n = n*2;
    }
    if(n == number || number == 1){
        return 1;
    }
    else{
        return 0;
    }
}
//Funcion que ve si un numero es valido como bin
//Entrada: un numero entero
//Salida: un booleano
int checkBins(int bins){
    if(bins < 1 || bins > 256){
        return 0;
    }
    else{
        return checkPower(bins);
    }
}
//Funcion que ve si un numero es valido como level
//Entrada: un numero entero
//Salida: un booleano
int isLevelCorrect(int level){
    if(level < 0 || level > 8){
        return 0;
    }
    else{
        if(level == 0){
            return 1;
        }
        else{
            return checkPower(level);
        }
    }

}


/*

trabajo de Nodos

lista = queue
queue = lista

*/


//funcion que crea un nodo
//entrada: un entero que representa el codigo del color, un entero que representa cuantas veces aparece
//salida: un nodo
Node* createNode(int color, int value){
    Node* aux= (Node*)malloc(sizeof(Node));
    aux->color = color;
    aux->value = value;
    aux->next = NULL;
    return aux;
}

//Funcion que libera la memoria de una lista
//entrada: un nodo cabezal
void freeQueue(Node* node){
    while(node != NULL){
        Node* aux = node;
        node = node->next;
        free(aux);
    }
}

//Funcion que añade un nodo a una lista ordenada
//Entrada: nodo cabezilla, nodo a añadir
//Salida: un nodo cabezilla
Node* addNode(Node* first, Node* add){
    if(first== NULL){
        return add;
    }
    else{
        Node* aux = first;
        while(aux->next != NULL){
            if(add->color < first->color){
                add->next = first;
                return add;
            }
            else{
                if(aux->next->color > add->color){
                    add->next = aux->next;
                    aux->next = add;
                    return first;
                }
                else{
                    aux = aux->next;
                }
            }
        }
        if(first->color > add->color){
            add->next = first;
            return add;
        }
        aux->next = add;
        return first;
    }
}

//Funcion que ve si un color esta en la lista
//Entrada: nodo cabezilla, el valor del color
//Salida: entero que representa si se encontro o no
int isInQueue(Node* first, int color){
    Node* aux = first;
    while(aux != NULL){
        if(color == aux->color){
            return 1;
        }
        else{
            aux = aux->next;
        }
    }
    return 0;
}

//Funcion que añade el valor de un nodo a una lista
//Entrada: nodo cabezal, numero del color, valor del nodo
//Salida: nodo cabezal
Node* addValue(Node* head,int color ,int value){
    Node* aux = head;
    while(aux != NULL){
        //si encuentra el nodo (cosa que deberia pasar)
        if(aux->color == color){
            aux->value = aux->value + value;
            return head;
        }
        aux= aux->next;
    }
    return head;

}


//Funcion que junta los histogramas
//Entrada: 2 listas de nodos
//salida: una lista de nodos
Node* joinHistograms(Node* head1, Node* head2){
    Node* aux = head2;
    if(head1==NULL && head2 == NULL){
        printf("\n TERRIBLE DE NULL \n");
        return NULL;
    }
    if(head1 == NULL){
        printf("\n head 1 null \n");
        return head2;
    }
    if(head2 == NULL){
        printf("\n head 2 null \n");
        return head1;
    }
    while(aux != NULL){
        //si esta el color en la queue, se agrega el valor
        if(isInQueue(head1,aux->color)){
            head1 = addValue(head1,aux->color,aux->value);
            aux = aux->next;
            
        }
        //si no esta en la queue se agrega el nodo
        else{
            Node* aux2 = createNode(aux->color,aux->value);
            head1 = addNode(head1,aux2);
            Node* aux3 = aux;
            aux = aux->next;
            free(aux3);
        }
    }

    return head1;
}
void printHistogram(Node* node){
    Node* aux = node;
    while(aux != NULL){
        printf("color : %d cantidad %d \n",aux->color, aux->value);
        aux = aux->next;
    }
}
//Funcion que crea un histograma
//Entrada: matriz rgb, posicion i inicial, altura de cobertura, posicion j inicial, largo de cobertura
//Saluda: un nodo
Node* makeHistogram(int*** matrix,int iMin, int height, int jMin, int width){
    Node* head = NULL;
    //recorre la matriz 
    for(int i = iMin; i < iMin + height; i++){
        for(int j = jMin; j < jMin + width; j++){            
            int color = rgbToGray(matrix[i][j][0], matrix[i][j][1], matrix[i][j][2]);
            if(head == NULL){
                head = createNode(color, 1);
            }
            else{
                if(!isInQueue(head,color)){
                    
                    Node* aux = createNode(color,1);
                    head = addNode(head,aux);
                }
                else{
                    head = addValue(head,color,1);
                    
                }
            }
        }
    }
    return head;
}

/*
Trabajo de bins

*/

Bin* createBin(int i, int iMax){
    Bin* aux = (Bin*)malloc(sizeof(Bin));
    aux->next= NULL;
    aux->range[0] = i;
    aux->range[1] = iMax;
    aux->value = 0;
    return aux;
}

Bin* addBin(Bin* head,Bin* add){
    Bin* aux = head;
    while(aux->next != NULL){
        aux = aux->next;
    }
    aux->next = add;
    return head;
}

Bin* makeQueue(int binSize){
    int add = 256/binSize;
    Bin* head = NULL;
    for (int i = 0; i < 256;){
        if(head == NULL){
            head = createBin(i,i+add -1);
            i += add;
        }
        else{
            
            Bin* aux = createBin(i,i + add - 1);
            head = addBin(head,aux);
            i+=add;
            
        }
        /* code */
    }
    return head;
}

Bin* addValueBin(Bin* head,int position,int value){
    Bin* aux = head;
    while(aux != NULL ){
        if(aux->range[0] <= position && position <= aux->range[1]){
            aux->value += value;
            return head;
        }
        else{
            aux = aux->next;
        }
    }
    return head;

}
void printBin(Bin* head){
    Bin* aux = head;
    while(aux != NULL){
        printf("[%d %d] %d\n",aux->range[0],aux->range[1],aux->value);
        aux = aux->next;
    }
}


Bin* histogramToBin(int binSize, Node* histogram){
    Bin* bin = makeQueue(binSize);
    Node* aux = histogram;
    while(aux != NULL){
        Node* aux2 = aux;
        bin = addValueBin(bin,aux->color,aux->value);
        aux = aux->next;
        free(aux2);
    }
    return bin;
}
/*

Trabajo de archivos

*/
void writeFile(Node* histogram,int binSize, char* name){

    FILE * fp = fopen(name, "w");
    Bin* aux = histogramToBin(binSize,histogram);
    if(!fp){
        printf("\n Unable to write : %s ", name);
        
        return;
    }
    else{
        while(aux != NULL){
            fprintf(fp,"[%d %d] %d\n",aux->range[0],aux->range[1],aux->value);
            aux = aux->next;
        }
        
    }
    
    fclose(fp);
}
/*

    Trabajo de colores y matrices

*/
int rgbToGray(int R, int G, int B){
    return R * 0.3 + G * 0.59 + B * 0.11;
}

void printMatrix(int** matrix, int m, int n){
    for(int i = 0; i < m; i++){
        for(int j = 0; j < n; j++){
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}
void printMatrixByParameters(int** matrix,int iMin, int height, int jMin, int width){
    for(int i = iMin; i < iMin + height; i++){
        for(int j = jMin; j < jMin + height; j++){
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}


//Funcion que determina el maximo nivel de hebras que se puede tener
//entrada: un entero que representa el largo y ancho de la matriz N * N
//Salida: un entero que representa la cantidad de niveles maximos que se pueden hacer
int maxLevel(int n){
    int aux = 0;
    while(n != 1){
        n /= 2;
        aux++;
    }
    return aux;
}

void freeMatrix(int*** matrix, int height, int width){
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            free(matrix[i][j]);
        }
        free(matrix[i]);
    }
    free(matrix);
}

Node* childs(int*** matrix,int iMin, int height, int jMin, int width,int level){
    if(level <= 0){
        if(iMin == iMin+height || jMin == jMin+width ){
            return NULL;
        }
        Node* node = makeHistogram(matrix,iMin,height,jMin,width);
        return node;
    }
    else{
        Node* aux;
        Node* aux2;
        Node* aux3;
        Node* aux4;
        aux = childs(matrix,iMin, height/2,jMin, width/2,level - 1);
        aux2 = childs(matrix,iMin + height/2, height/2,jMin, width/2,level - 1);
        aux3 = childs(matrix,iMin, height/2,jMin + width/2, width/2,level - 1);
        aux4 = childs(matrix,iMin + height/2, height/2,jMin + width/2, width/2,level - 1);
        
        // esperar que terminen
      
        aux = joinHistograms(aux,aux2);
        aux3 = joinHistograms(aux3,aux4);
        aux = joinHistograms(aux, aux3);
        
      

         return aux;
        //
    }
}

void distribute(int level, int binSize,char* inName, char* ouName){
    Node* head;
    bmp_img img;
    int height,width;
    if(level == 0){
        return;
    }
	bmp_img_read(&img,inName);
    int*** matrix = (int***)malloc(sizeof(int**)*img.img_header.biHeight);
    for(int i = 0; i < img.img_header.biHeight;i++){
        matrix[i] = (int**)malloc(sizeof(int*)*img.img_header.biWidth);
        for(int j = 0; j < img.img_header.biWidth; j++){
            matrix[i][j] = (int*) malloc(sizeof(int)* 3);
            matrix[i][j][0] = img.img_pixels[i][j].red;
            matrix[i][j][1] = img.img_pixels[i][j].green;
            matrix[i][j][2] = img.img_pixels[i][j].blue;
        }
    }
    height = img.img_header.biHeight;
    width = img.img_header.biWidth;
    //crear thread
    head = childs(matrix,0,height,0,width,level);
    writeFile(head, binSize,ouName);
    //guardar resultado de los threads

    freeMatrix(matrix,img.img_header.biHeight,img.img_header.biWidth);
    
    
    
    
}
/*
void* thread(void* arg){
    int* aux = (int*) arg;
    int level = *aux;
    level -= 1;
    
    
    if(level > 0){
        pthread_t* threadArray = (pthread_t*)malloc(sizeof(pthread_t)* 4);
        for(int i = 0; i < 4; i++){
            pthread_create(&threadArray[i],NULL,thread,&level);
        }
        printf("Nivel esperando: %d \n",level);
        for(int i = 0; i < 4; i++){
            pthread_join(threadArray[i],NULL);
        }

    }
    else{
        printf("Nivel final: %d \n",level);
    }
    
}

void prueba3(int level, int **matrix){
    if(level == 0){
        printf("level 0");
    }
    else{
        pthread_t* threadArray = (pthread_t*)malloc(sizeof(pthread_t)* 4);
        for(int i = 0; i < 4; i++){
            pthread_create(&threadArray[i],NULL,thread,&level);
        }
        for(int i = 0; i < 4; i++){
            pthread_join(threadArray[i],NULL);
        }
    }
}*/


int main(int argc, char* argv[]){

    printf("Inicio \n");
    int opt;
    int bin;
    int level;
    char* inName;
    char* ouName;
    char* aux2;
    char* aux3;
    int* aux = (int*)malloc(sizeof(int)*4);
    while((opt = getopt(argc, argv, "i:o:L:B:")) != -1) { 
        switch(opt){
            case 'i':
                
                inName = optarg;
                aux[0] = 1;
                break;
            
            case 'o':
                
                ouName = optarg;
                aux[1] = 1;
                break;
            
            case 'L':
                level = strtol(optarg, &aux2, 10);
                if(isLevelCorrect(level)){
                    aux[2] = 1;
                }
                else{
                    aux[2] = 0;
                }
                
                break;
            
            case 'B':
                bin = strtol(optarg, &aux3, 10);
                if(checkBins(bin)){
                    aux[3] = 1;
                }
                else{
                    aux[3] = 0;
                }
                
                break;
            
            default:
                printf("mal ingresado, por favor ingrese los valores bien \n");
                exit(EXIT_FAILURE);
                break;
        }
	}
    if(aux[0] == 0){
        printf("Nombre no ingresado o mal ingresado \n");
    }
    if(aux[1] == 0){
        printf("Nombre no ingresado o mal ingresado \n");
    }
    if(aux[2] == 0){
        printf("Nivel no ingresado o mal ingresado \n");
    }
    if(aux[3] == 0){
        printf("Bin no ingresado o mal ingresado \n");
    }
    for(int i = 0; i < 4; i++){
        if(!aux[i]){
            exit(1);
        }
    }
    
    distribute(level,bin,inName,ouName);
	printf("Fin \n");
	
	return 0;
    
    /*
    prueba();
    
    */
    
    return 0;
}