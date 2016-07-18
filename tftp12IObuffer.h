#ifndef _TFTP12IOBUFFER_H
#define _TFTP12IOBUFFER_H

#define TFTP12_IO_MEMEORY_MAX		(10*1024*1024)

#define TFTP12_IO_BUFFERSIZE(x)		((TFTP12_IO_MEMEORY_MAX/x)*x)

#include "windows.h"




char *tftp12IOBufferInit(INT32 id, INT32 blocksize, FILE *file, INT32 fileSize, enum TFTP12_ReadOrWrite rwFlag);
char *tftp12ReadNextBlock(INT32 id, INT32 *size);
char *tftp12WriteNextBlock(INT32 id, char *buf, INT32 writeSize);
INT32 tftp12IOBufferFree(INT32 id);
void tftp12WaitIOFinishById(INT32 id);


#endif
