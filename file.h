#ifndef FILE_H
#define FILE_H

void FilePackInit();
void *FileRead(const char *name);
void FileFree(void *ptr);

#endif