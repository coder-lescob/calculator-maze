#include "maze.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define WALL            (0x01)
#define PATH_IN_SEARCH  (0xff)
#define PATH_DONE       (0x00)

bool cell_visited(Maze maze, int cell) {
    return maze.tiles[cell] != WALL;
}

bool cell_available(Maze maze, int cell)
{
    // cell already visited (air or backtracking trace there)
    if (cell_visited(maze, cell)) return false;

    uint8_t count = 0;
    if ((cell - maze.width) >= 0            && cell_visited(maze, cell - maze.width)) count ++;
    if (cell % maze.width != maze.width - 1 && cell_visited(maze, cell + 1         )) count ++;
    if (cell < maze.width * maze.height     && cell_visited(maze, cell + maze.width)) count ++;
    if (cell % maze.width != 0              && cell_visited(maze, cell - 1         )) count ++;

    return count == 1;
}

/**
 * available MUST have a length of 4
 */
int get_available_cells(Maze maze, int current_cell, int *available) {
    
    int count = 0;

    if ((current_cell - maze.width) >= 0 && cell_available(maze, current_cell - maze.width))  
    {
        available[count] = current_cell - maze.width;
        count ++;
    }
    if (current_cell % maze.width != maze.width - 1 && cell_available(maze, current_cell + 1))
    {
        available[count] = current_cell + 1;
        count ++;
    }
    if (current_cell < maze.width * maze.height && cell_available(maze, current_cell + maze.width))
    {
        available[count] = current_cell + maze.width;
        count ++;
    }
    if (current_cell % maze.width != 0 && cell_available(maze, current_cell - 1))
    {
        available[count] = current_cell - 1;
        count ++;
    }

    return count;
}

int get_next_cell(Maze maze, int cell)
{
    int available[4];
    int num_available = get_available_cells(maze, cell, available);

    if (num_available >= 1)
    {
        int random_cell = eadk_random() % num_available;

        maze.tiles[cell] = PATH_IN_SEARCH;
        int next = available[random_cell];
        maze.tiles[next] = PATH_IN_SEARCH;

        return next;
    }

    // backtraces to before because no next cells are available
    // mark as no go dead ends

    if (0 <= (cell - maze.width)  && maze.tiles[cell - maze.width] == PATH_IN_SEARCH)
    {
        maze.tiles[cell] = PATH_DONE;
        return cell - maze.width;
    }
    if (cell % maze.width < maze.width - 1 && maze.tiles[cell + 1] == PATH_IN_SEARCH)
    {
        maze.tiles[cell] = PATH_DONE;
        return cell + 1;
    }
    if (cell + 10 < maze.width * maze.height && maze.tiles[cell + maze.width] == PATH_IN_SEARCH)
    {
        maze.tiles[cell] = PATH_DONE;
        return cell + maze.width;
    }
    if (cell % maze.width != 0 && maze.tiles[cell - 1] == PATH_IN_SEARCH)
    {
        maze.tiles[cell] = PATH_DONE;
        return cell - 1;
    }
    
    return 0;
}

Maze generate_maze(uint16_t width, uint16_t height)
{
    Maze maze = { .width = width, .height = height, .tiles = calloc(sizeof(uint8_t), width * height)};

    for (int i = 0; i < maze.width * maze.height; i++)
        maze.tiles[i] = WALL;

    int current_cell = 0;
    maze.tiles[current_cell] = PATH_IN_SEARCH;

    do
    {
        current_cell = get_next_cell(maze, current_cell);
    } 
    while (current_cell != 0);

    return maze;
}

void free_maze(Maze *maze) {
    if (maze == NULL) return;

    free(maze->tiles);
}

void print_maze(Maze *maze) {
    for (int y = 0; y < maze->height; y++) {
        for (int x = 0; x < maze->width; x++) {
            if (maze->tiles[x + y * maze->width] > 0) {
                printf("\x1b[1;47m  ");
            }
            else {
                printf("\x1b[1;40m  ");
            }
        }
        printf("\x1b[0m\n");
    }
    printf("\n");
}