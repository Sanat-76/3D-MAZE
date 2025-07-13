#ifndef MAZE_H
#define MAZE_H

#define MAZE_SIZE 20

extern int maze[MAZE_SIZE][MAZE_SIZE];

void drawMaze();
bool checkCollision(float x, float z, float spacing);

#endif
