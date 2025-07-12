#ifndef MAZE_H
#define MAZE_H

#define MAZE_SIZE 20

extern int maze[MAZE_SIZE][MAZE_SIZE];

void drawCube(float x, float y, float z);
void drawMaze();
void drawBasePlatform(float totalSize);

#endif
