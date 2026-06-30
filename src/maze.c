#include <stdio.h>

int maze[100];      // -1 = non visité, 0 = vide, 1 = mur

int main(void)
{
    for (int i = 0; i < 100; i++)
    {
        maze[i] = -1;
    }


    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            printf("%d ", maze[i*j]);
        }
        printf("\n");
    }

    return 0;
}