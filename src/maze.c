#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int maze[100];      // 0 = mur, 1 = chemin, 2 chemin(backtracking)

void print_maze(void)
{
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            printf("%d ", maze[i*10 + j]);
        }
        printf("\n");
    }
    printf("\n");
}

int cell_available(int cell)
{
    if (maze[cell] != 0) return 0;  // ← déjà visitée, pas disponible

    int count = 0;
    if (0 <= cell-10 && maze[cell-10] != 0)       // ← nord
        count += 1;
    if (cell % 10 != 9 && maze[cell+1] != 0)      // ← est
        count += 1;
    if (cell+10 <= 99 && maze[cell+10] != 0)      // ← sud
        count += 1;
    if (cell % 10 != 0 && maze[cell-1] != 0)      // ← ouest
        count += 1;

    return (count == 1) ? 1 : 0;
}

int get_nex_cell(int cell)
{
    int count = 0;
    int available[4];

    if (0 <= cell-10 && cell_available(cell-10))  // ← nord
    {
        available[count] = cell-10;
        count += 1;
    }
    if (cell % 10 != 9 && cell_available(cell+1)) // ← est
    {
        available[count] = cell+1;
        count += 1;
    }
    if (cell+10 <= 99 && cell_available(cell+10)) // ← sud
    {
        available[count] = cell+10;
        count += 1;
    }
    if (cell % 10 != 0 && cell_available(cell-1)) // ← ouest
    {
        available[count] = cell-1;
        count += 1;
    }

    if (count >= 1)
    {
        int rng = rand() % count;
        maze[cell] = 1;
        int next = available[rng];
        maze[next] = 1;  // ← marquer la destination
        return next;
    }
    else
    {
        if (0 <= cell-10 && maze[cell-10] == 1)       // ← nord
        {
            maze[cell] = 2;
            return cell-10;
        }else if (cell % 10 != 9 && maze[cell+1] == 1)      // ← est
        {
            maze[cell] = 2;
            return cell+1;
        }else if (cell+10 <= 99 && maze[cell+10] == 1)      // ← sud
        {
            maze[cell] = 2;
            return cell+10;
        }else if (cell % 10 != 0 && maze[cell-1] == 1)      // ← ouest
        {
            maze[cell] = 2;
            return cell-1;
        }else
        {
            return 0;
        }
    }
}

int main(void)
{
    srand(time(NULL));
    for (int i = 0; i < 100; i++)
        maze[i] = 0;

    int current_cell = 0;
    maze[current_cell] = 1;

    do
    {
        current_cell = get_nex_cell(current_cell);
        // print_maze();
    } while (current_cell != 0);

    print_maze();
    return 0;
}