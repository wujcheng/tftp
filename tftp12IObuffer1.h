#ifndef _TFTP12IOBUFFER_H
#define _TFTP12IOBUFFER_H


#include "windows.h"




INT32 tftp12IOBufferInit(INT32 id, INT32 blocksize, FILE *file, INT32 fileSize, enum TFTP12_ReadOrWrite rwFlag);
char *tftp12ReadNextBlock(INT32 id, INT32 *size);
INT32 tftp12WriteNextBlock(INT32 id, char *buf, INT32 writeSize);
INT32 tftp12IOBufferFree(INT32 id);



#endif
