#include "stdio.h"
#include "stdlib.h"
#include "tftp12header.h"
#include "tftp12IObuffer.h"
#include "tftp12FormatConvert.h"

#include "windows.h"


typedef struct _iobuffnode
{
	INT32 id;
	INT32 blockSize;
	INT32 endOfFile;				/*是否到达文件末尾的标志*/
	INT32 fileSize;					/*文件大小*/
	FILE *targetFile;
	INT8 temChar;					/*netascii的临时字符*/
	enum TFTP12_TRANS_MODE mode;	/*读写模式，octet*/
	enum TFTP12_ReadOrWrite rwFlag;
	INT32 firstRun;
	INT32 IOtaskIsRunning;
	INT32 bufferSize;
	char *nextPosition;
	char *currentReadBuffer;
	char *currentWriteBuffer;
	char *fileEndPosition;			/*在缓冲区中，指向文件结束的位置*/
	char *pFree;					/*Free的时候使用这个指针*/
	char temPktHead[4];				/*用来保存写入的时候被破坏的四个字节*/

	struct _iobuffnode *next;
}TFTP12IOBufferNode_t;

TFTP12IOBufferNode_t *head = NULL;


static void tftp12IOBufferRequest(INT32 id);

static void tftp12IOListInsert(TFTP12IOBufferNode_t *node)
{
	TFTP12IOBufferNode_t *pWalk = head;
	if (head == NULL)
	{
		head = node;
		return;
	}
	while (pWalk->next != NULL)
	{
		pWalk = pWalk->next;
	}
	pWalk->next = node;
}

static INT32 tftp12IOListDelete(TFTP12IOBufferNode_t *node)
{
	TFTP12IOBufferNode_t *pWalk = head;
	if (head == node)
	{
		head = head->next;
		return TRUE;
	}
	while (pWalk->next != NULL)
	{
		if (pWalk->next == node)
		{
			pWalk->next = pWalk->next->next;
			return TRUE;
		}
		pWalk = pWalk->next;
	}
	return FALSE;
}


static TFTP12IOBufferNode_t *tftp12FindNodeByid(INT32 id)
{
	TFTP12IOBufferNode_t *pWalk = head;
	while (pWalk != NULL)
	{
		if (pWalk->id == id)
		{
			return pWalk;
		}
		pWalk = pWalk->next;
	}
	return NULL;
}

static void tftp12WaitIOFinish(TFTP12IOBufferNode_t *node)
{	/*sleep太弱智，需要换*/
	while (node->IOtaskIsRunning == TRUE)
	{
		Sleep(5);
	}
}

void tftp12WaitIOFinishById(INT32 id)
{
	TFTP12IOBufferNode_t *node = tftp12FindNodeByid(id);
	if (node == NULL)
	{
		return;
	}
	tftp12WaitIOFinish(node);
}


/*返回data块的指针(指针往前推4个字节可以修改)*/
char *tftp12ReadNextBlock(INT32 id, INT32 *size)
{
	INT32 diff = 0;
	TFTP12IOBufferNode_t *node = tftp12FindNodeByid(id);
	char *ret = NULL;

	if (node == NULL)
	{
		*size = 0;
		return NULL;
	}

	if (node->mode == TFTP12_NETASCII)
	{
		*size=tftp12FileToAscii(node->targetFile, node->nextPosition, node->blockSize, (&node->temChar));
		return node->nextPosition;
	}

	if (node->firstRun == TRUE)
	{
		/*如果是第一次运行需要将另外一个buffer填充满*/
		node->firstRun = FALSE;
		node->IOtaskIsRunning = TRUE;
		tftp12IOBufferRequest(node->id);
	}
	diff = node->fileEndPosition - node->currentReadBuffer;

	/*这种情况属于文件末尾不在buffer的开头*/
	if (node->nextPosition == node->fileEndPosition)
	{
		/*说明到文件末尾了*/
		*size = 0;
		return node->nextPosition;
	}
	else if ((node->endOfFile == TRUE) && (diff >= 0) && (diff < node->bufferSize) && \
		((node->fileEndPosition - node->nextPosition) <= node->blockSize))
	{
		/*说明文件结束在本缓冲区内,且小于等于一个blocksize*/
		*size = node->fileEndPosition - node->nextPosition;
		ret = node->nextPosition;
		node->nextPosition += *size;
		return ret;
	}
	else
	{
		/*只要不是在文件末尾的地方，size都是这么大*/
		*size = node->blockSize;

		/*如果指针加了以后等于了buff结束*/
		if (node->nextPosition == (node->currentReadBuffer + node->bufferSize))
		{
			tftp12WaitIOFinish(node);
			node->IOtaskIsRunning = TRUE;
			/*交换缓冲区*/
			char *tem = node->currentReadBuffer;
			node->currentReadBuffer = node->currentWriteBuffer;
			node->currentWriteBuffer = tem;
			node->nextPosition = node->currentReadBuffer;
			tftp12IOBufferRequest(node->id);
		}

		/*有可能文件末尾在另外一个缓冲区的开头，如果是的话，就返回0*/
		if (node->nextPosition == node->fileEndPosition)
		{
			/*说明到文件末尾了*/
			*size = 0;
			return node->nextPosition;
		}
		else
		{
			ret = node->nextPosition;
			node->nextPosition += *size;
		}
	}
	return ret;
}


/*返回的指针用作下一次的recvBuffer，buf参数是指向数据块的，要跳过4个字节*/
//desc->recvBuffer = tftp12WriteNextBlock(desc->localPort, desc->recvBuffer + 4, recvBytes - 4);
char *tftp12WriteNextBlock(INT32 id, char *buf, INT32 writeSize)
{
	TFTP12IOBufferNode_t *node = tftp12FindNodeByid(id);
	if (node == NULL)
	{
		return NULL;
	}

	/*如果是netascii模式，单独处理*/
	if (node->mode == TFTP12_NETASCII)
	{
		tftp12AsciiToFile(node->targetFile, buf, writeSize, &(node->temChar), writeSize<node->blockSize);
	}
	else
	{
		/*用保存的数据填充被tftp报文头破坏的上一片的4个字节*/
		*(buf - 4) = node->temPktHead[0];
		*(buf - 3) = node->temPktHead[1];
		*(buf - 2) = node->temPktHead[2];
		*(buf - 1) = node->temPktHead[3];

		if (writeSize > node->blockSize)
		{
			/*正常情况下不可能出现这个*/
			//system("pause");
			return NULL;
		}

		/*如果小于块大小，说明包括了文件结尾*/
		if (writeSize < node->blockSize)
		{
			printf("\nrecevie final pkt:%d", writeSize);
			node->endOfFile = TRUE;
			node->fileEndPosition = node->nextPosition + writeSize;
		}
		node->nextPosition += writeSize;

		if ((node->nextPosition == (node->currentWriteBuffer + node->bufferSize)) || (node->endOfFile == TRUE))
		{

			tftp12WaitIOFinish(node);
			node->IOtaskIsRunning = TRUE;
			char *tem = node->currentReadBuffer;
			node->currentReadBuffer = node->currentWriteBuffer;
			node->currentWriteBuffer = tem;
			node->nextPosition = node->currentWriteBuffer;
			tftp12IOBufferRequest(id);
		}
		else if (node->nextPosition > (node->currentWriteBuffer + node->bufferSize))
		{
			/*下个位置大于了buffer的末尾，正常情况下不可能出现这种情况*/
			//system("pause");
		}

		/*保存上一片数据的后四个字节，给下一个数据报文头部留出空间*/
		node->temPktHead[0] = *(node->nextPosition - 4);
		node->temPktHead[1] = *(node->nextPosition - 3);
		node->temPktHead[2] = *(node->nextPosition - 2);
		node->temPktHead[3] = *(node->nextPosition - 1);
	}

	/* 因为read和writebuffer都在申请内存的时候多留出来了4个字节 *
	 * 用netascii的时候，nextPosition不移动，没有必要做缓冲	 */
	return (node->nextPosition - 4);
}

INT32 testNum = 0, testNum2 = 0;;

static  void *  WINAPI tftp12IObufferHandleTask(void *arg)
{
	INT32 writeSize = 0;
	INT32 realWrite = 0;
	INT32 realRead = 0;
	INT32 diff = 0;

	TFTP12IOBufferNode_t *node = (TFTP12IOBufferNode_t*)arg;
	if ((node == NULL) || (node->targetFile == NULL)\
		|| (node->endOfFile == TRUE&&node->rwFlag == TFTP12_READ))
	{
		printf("\nio e");
		node->IOtaskIsRunning = FALSE;
		return NULL;
	}


	if (node->rwFlag == TFTP12_READ)
	{
		realRead = fread(node->currentWriteBuffer, 1, node->bufferSize, node->targetFile);
		if (realRead < node->bufferSize)
		{
			node->endOfFile = TRUE;
			node->fileEndPosition = node->currentWriteBuffer + realRead;
		}
	}
	else if (node->rwFlag == TFTP12_WRITE)
	{
		diff = node->fileEndPosition - node->currentReadBuffer;

		/*如果文件到了末尾，且在本次写的缓冲内,写入的数量等于相差的字节数*/
		if ((node->endOfFile == TRUE) && (diff > 0) && (diff < node->bufferSize))
		{
			writeSize = diff;
		}
		else
		{
			writeSize = node->bufferSize;
		}
		realWrite = fwrite(node->currentReadBuffer, 1, writeSize, node->targetFile);
		if (realWrite < writeSize)
		{
			printf("\n写入错误");
		}
	}
	else
	{
		return NULL;
	}
	node->IOtaskIsRunning = FALSE;
	return NULL;
}


static void tftp12IOBufferRequest(INT32 id)
{

	TFTP12IOBufferNode_t *node = tftp12FindNodeByid(id);

	/*在已经读取到文件末尾的时候禁止再次读取*/
	if (node->endOfFile == TRUE&&node->rwFlag == TFTP12_READ)
	{
		node->IOtaskIsRunning = FALSE;
		return;
	}

	if (node->rwFlag == TFTP12_READ)
	{
	}

	HANDLE useless = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tftp12IObufferHandleTask, node, 0, NULL);
	if (useless == NULL)
	{
		printf("\ncreate io buffer handle task failed");
		node->IOtaskIsRunning = FALSE;
	}
	CloseHandle(useless);
}

/*读的时候返回数据块指针，写的时候返回recvBuffer*/
char *tftp12IOBufferInit(
	INT32 id,
	INT32 blocksize,
	FILE *file, UNUSED(INT32 fileSize),
	enum TFTP12_TRANS_MODE mode,
	enum TFTP12_ReadOrWrite rwFlag)
{
	/*应根据剩余内存大小和文件大小分配内存空间*/
	//测试时用1M
	/*两块缓冲区*/
	//INT32 bufSize = ((1*1024*1024) / blocksize)*blocksize + 4;
	char *tem = NULL;
	char *buf = NULL;
	INT32 bufSize = TFTP12_IO_BUFFERSIZE(blocksize);
	TFTP12IOBufferNode_t *node = NULL;
	buf = (char *)malloc(bufSize * 2 + 8);

	if (buf == NULL)
	{
		return NULL;
	}

	node = (TFTP12IOBufferNode_t*)malloc(sizeof(TFTP12IOBufferNode_t));
	if (node == NULL)
	{
		return NULL;
	}
	memset(node, 0, sizeof(TFTP12IOBufferNode_t));
	node->blockSize = blocksize;
	node->fileSize = fileSize;
	node->id = id;
	node->bufferSize = bufSize;
	node->rwFlag = rwFlag;
	node->pFree = buf;
	node->targetFile = file;
	node->currentReadBuffer = buf + 4;
	node->currentWriteBuffer = buf + bufSize + 8;
	node->mode = mode;
	node->temChar = '\0';
	tftp12IOListInsert(node);
	node->firstRun = TRUE;
	if (rwFlag == TFTP12_READ)
	{
		if (mode == TFTP12_OCTET)
		{
			node->IOtaskIsRunning = TRUE;
			tftp12IOBufferRequest(id);
			while (node->IOtaskIsRunning == TRUE)
			{
				Sleep(5);
			}
			/*交换缓冲区,因为第一次读取的数据是放在writeBuffer里面*/
			tem = node->currentReadBuffer;
			node->currentReadBuffer = node->currentWriteBuffer;
			node->currentWriteBuffer = tem;
		}
		node->nextPosition = node->currentReadBuffer;
		return node->nextPosition;
	}
	else
	{
		node->nextPosition = node->currentWriteBuffer;

		/*如果是写磁盘，则返回writebuffer-4*/
		return node->nextPosition - 4;
	}
}

INT32 tftp12IOBufferFree(INT32 id)
{
	TFTP12IOBufferNode_t *node = tftp12FindNodeByid(id);
	if (node == NULL)
	{
		return FALSE;
	}
	FREE_Z(node->pFree);
	tftp12IOListDelete(node);
	FREE_Z(node);
	return TRUE;
}
