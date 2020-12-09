#ifndef HIST_H
#define HIST_H
typedef struct Node Node;
typedef struct Bin Bin;
struct Node{
	int color;
	int value;
	Node* next;
};

struct Bin{
    int range[2];
    int value;
    Bin* next;
};
int rgbToGray(int R, int G, int B);
#endif