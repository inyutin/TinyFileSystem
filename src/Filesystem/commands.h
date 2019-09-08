#pragma once
#include "../define.h"

/* List directory contents */
void ls_t (int currentDirId);

/* Changes the current working directory. */
int cd_t (int currentDirId, char name[MAX_LENGTH_FILE_NAME]);

/* Make directory */
void mkdir_t(int currentDirId, char name[MAX_LENGTH_FILE_NAME]);

/* Create file */
void touch_t(int currentDirId, char name[MAX_LENGTH_FILE_NAME]);

/* Print contents of file */
void cat_t (int currentDirId, char name[MAX_LENGTH_FILE_NAME]);

/* Write text to the end of file */
void echo_t (int currentDirId, char name[MAX_LENGTH_FILE_NAME], char text[MAX_LENGTH_TEXT]);