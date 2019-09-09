#pragma once
#include "../define.h"

/* List directory contents */
void ls_t (int currentDirId, int sock);

/* Changes the current working directory. */
int cd_t (int currentDirId, char name[MAX_LENGTH_FILE_NAME], int sock);

/* Make directory */
void mkdir_t(int currentDirId, char name[MAX_LENGTH_FILE_NAME], int sock);

/* Create file */
void touch_t(int currentDirId, char name[MAX_LENGTH_FILE_NAME], int sock);

/* Print contents of file */
void cat_t (int currentDirId, char name[MAX_LENGTH_FILE_NAME], int sock);

/* Write text to the end of file */
void echo_t (int currentDirId, char name[MAX_LENGTH_FILE_NAME], char text[MAX_LENGTH_TEXT], int sock);