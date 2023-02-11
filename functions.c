#include <stdio.h>
#include <stdlib.h>
#include <string.h>



void connect_system()
{

}

void numtoText(int num)
{
    char *convertedNum;
    convertedNum =(char*)malloc(sizeof(char)*100);
    sprintf(convertedNum, %d, num);

    printf("Converting number to text: %s\n", convertedNum);

    return 0;
}

void texttoNum(char text[])
{
    int size= sizeof(text);
    int numConverted = atoi(text);

    printf("Converting text to number:  %d\n", numConverted);

    return 0;
}

void store()
{

}
void recall()
{

}

