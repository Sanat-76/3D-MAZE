#ifndef MAZE_H
#define MAZE_H

#define MAZE_SIZE 20

void initMaze();
void drawMaze(unsigned int shaderProgram);
bool checkCollision(float x, float z, float spacing);

#endif
