#ifndef DEALPKT_C
#define DEALPKT_C

#pragma once
#include <string.h>
#include <winsock.h>

#include "tftp12header.h"

/*
+-------+---~~---+---+---~~---+---+---~~---+---+---~~---+---+---------+---+---~~---+---+
|  opc  |filename| 0 |  mode  | 0 |  opt1  | 0 | value1 | 0 |   optN  | 0 | valueN | 0 |
+-------+---~~---+---+---~~---+---+---~~---+---+---~~---+---+---------+---+---~~---+---+
*/

#define DEBUG 1	/*是否开启调试输出,默认为不输出*/
#define PAUSE 0 /*是否执行一个函数暂停一次,默认为不暂停*/



/******************************创建报文**********************************************/
/*创建一个Request报文，返回报文长度*/
INT32 tftp12CreateREQPkt(TFTP12Description *pktDescriptor);

//还未实现
/*创建一个Data报文，返回报文长度*/
//INT32 tftp12CreateDataPkt(TFTP12Description *pktDescriptor);

/*创建一个ACK报文，返回报文长度*/
INT32 tftp12CreateACKPkt(TFTP12Description *pktDescriptor, INT32 blockNum);

/*创建一个OACK报文，返回报文长度*/
INT32 tftp12CreateOACKPkt(TFTP12Description *pktDescriptor);

/*创建一个Error报文，返回报文长度*/
INT32 tftp12CreateERRPkt(TFTP12Description *pktDescriptor, INT16 errorCode, UINT8 *errorMsg);
/****************************************************************************/

/*****************************解析报文***********************************************/
/*解析Request报文，返回报文长度*/
INT32 tftp12ParseREQPkt(TFTP12Description *pktDescriptor);

//还未实现
/*解析一个Data报文，返回报文长度*/
//INT32 tftp12ParseDataPkt(TFTP12Description *pktDescriptor);

/*解析一个ACK报文，返回报文长度*/
INT16 tftp12ParseACKPkt(TFTP12Description *pktDescriptor);

/*解析一个OACK报文，返回报文长度*/
INT32 tftp12ParseOACKPkt(TFTP12Description *pktDescriptor);

/*解析一个Error报文，返回报文长度*/
INT32 tftp12ParseERRPkt(TFTP12Description *pktDescriptor, INT16 errorCode, UINT8 *errorMsg);
/****************************************************************************/




/*
Name:tftp12CreateREQPkt
function:create a request packet
input: pktDescriptor
output: packet length
packet format:
    2       n      1     n      1     n      1      n     1      n      1      n     1
+-------+---~~---+---+---~~---+---+---~~---+---+---~~---+---+---------+---+---~~---+---+
|  opc  |filename| 0 |  mode  | 0 |  opt1  | 0 | value1 | 0 |   optN  | 0 | valueN | 0 |
+-------+---	printf("length = %d\n", strlen(currentString));~~---+---+---~~---+---+---~~---+---+---~~---+---+---------+---+---~~---+---+
1-Read/2-Write   netascii/octet/mail  blockSize               timeout                  tsize
*/
INT32 tftp12CreateREQPkt(TFTP12Description *pktDescriptor)
{
    UINT8 preStringLength = 0;  /*在填充一个字段之前已填好的字段长度*/
printf("create opCode = %d\n", pktDescriptor->opCode);
    /*填充opcode:1/2*/
    *((UINT8 *)pktDescriptor->buffer) = (pktDescriptor->opCode >> 8) & 0xFF;
    *((UINT8 *)pktDescriptor->buffer+1) = pktDescriptor->opCode & 0xFF;
    preStringLength = 2;
    printf("create opCode = %d\n", pktDescriptor->opCode);
    /*填充文件名*/
    if (strlen(pktDescriptor->filename) > 256)
        return -1;  /*文件名错误*/
    strncpy((UINT8 *)pktDescriptor->buffer+preStringLength, pktDescriptor->filename, strlen(pktDescriptor->filename));   /*填充文件名*/
    *((UINT8 *)pktDescriptor->buffer+2+strlen(pktDescriptor->filename)) = '\0';
	preStringLength = preStringLength + strlen(pktDescriptor->filename) + 1;	/*更新字段长度*/

	#if DEBUG
	printf("tftp12CreateREQPkt填充文件名后preStringLength = %d\n", preStringLength);
	#endif

    /*填充模式, netascii/octet/mail*/
    if (strlen(pktDescriptor->mode) > 8)
        return -1;  /*模式错误*/
    strncpy((UINT8 *)pktDescriptor->buffer+preStringLength, pktDescriptor->mode, strlen(pktDescriptor->mode));   /*填充文件名*/
    *((UINT8 *)pktDescriptor->buffer+preStringLength+strlen(pktDescriptor->mode)) = '\0';
	preStringLength = preStringLength + strlen(pktDescriptor->mode) + 1;

	#if DEBUG
	printf("tftp12CreateREQPkt填充模式后preStringLength = %d\n", preStringLength);
	#endif

    /*填充blockSize，当选项值为0时考虑不填充还是填0，此处采用的方式为不填充*/
    if (pktDescriptor->option.blockSize > 0)
    {
        strncpy((UINT8 *)pktDescriptor->buffer+preStringLength, "blkSize", strlen("blkSize"));   /*填充文件名*/
        *((UINT8 *)pktDescriptor->buffer+preStringLength+strlen("blkSize")) = '\0';
		preStringLength = preStringLength + strlen("blkSize") + 1;

		UINT8 value[10] = {0};
		itoa(pktDescriptor->option.blockSize, value, 10);
		strncpy((UINT8 *)pktDescriptor->buffer+preStringLength, value, strlen(value));   /*填充文件名*/
        *((UINT8 *)pktDescriptor->buffer+preStringLength+strlen(value)) = '\0';
		preStringLength = preStringLength + strlen(value) + 1;

		#if DEBUG
		printf("tftp12CreateREQPkt填充blocksize后preStringLength = %d\n", preStringLength);
		#endif
    }

	/*填充timeout，当选项值为0时考虑不填充还是填0，此处采用的方式为不填充*/
    if (pktDescriptor->option.timeout > 0)
    {
        strncpy((UINT8 *)pktDescriptor->buffer+preStringLength, "timeout", strlen("timeout"));   /*填充文件名*/
        *((UINT8 *)pktDescriptor->buffer+preStringLength+strlen("timeout")) = '\0';
		preStringLength = preStringLength + strlen("timeout") + 1;

		UINT8 value[10] = {0};
		itoa(pktDescriptor->option.timeout, value, 10);
		strncpy((UINT8 *)pktDescriptor->buffer+preStringLength, value, strlen(value));   /*填充文件名*/
        *((UINT8 *)pktDescriptor->buffer+preStringLength+strlen(value)) = '\0';
		preStringLength = preStringLength + strlen(value) + 1;

		#if DEBUG
		printf("tftp12CreateREQPkt填充timeout后preStringLength = %d\n", preStringLength);
		#endif
    }

	/*填充tSize，当选项值为0时考虑不填充还是填0，此处采用的方式为不填充*/
    //if (pktDescriptor->option.tsize > 0)
    {
        strncpy((UINT8 *)pktDescriptor->buffer+preStringLength, "tSize", strlen("tSize"));   /*填充文件名*/
        *((UINT8 *)pktDescriptor->buffer+preStringLength+strlen("tSize")) = '\0';
		preStringLength = preStringLength + strlen("tSize") + 1;

		UINT8 value[10] = {0};
		itoa(pktDescriptor->option.tsize, value, 10);
		strncpy((UINT8 *)pktDescriptor->buffer+preStringLength, value, strlen(value));   /*填充文件名*/
        *((UINT8 *)pktDescriptor->buffer+preStringLength+strlen(value)) = '\0';
		preStringLength = preStringLength + strlen(value) + 1;

		#if DEBUG
		printf("tftp12CreateREQPkt填充tsize后preStringLength = %d\n", preStringLength);
		#endif
    }

    return preStringLength;
}

/*
Name:tftp12CreateACKPkt
function:create an ACK packet
input: pktDescriptor, blockNum
output: packet length
packet format:
    2       2
+-------+---~~---+---+---~~---+---+---~~---+---+---~~---+---+---------+---+---~~---+---+
|  opc  |blockNum|                             NULL                                    |
+-------+---~~---+---+---~~---+---+---~~---+---+---~~---+---+---------+---+---~~---+---+
 4-ACK
*/
INT32 tftp12CreateACKPkt(TFTP12Description *pktDescriptor, INT32 blockNumber)
{
    UINT8 preStringLength = 0;  /*在填充一个字段之前已填好的字段长度*/

	/*填充opcode:4*/
    *((UINT8 *)pktDescriptor->buffer) = 0x00;
    *((UINT8 *)pktDescriptor->buffer+1) = 0x04;
    preStringLength = 2;

    /*有问题128转换错误*/
    blockNumber = htons(blockNumber);
	*((UINT8 *)pktDescriptor->buffer+preStringLength) = blockNumber & 0xFF;
    *((UINT8 *)pktDescriptor->buffer+preStringLength+1) = (blockNumber >> 8)&0xFF;
    preStringLength = 4;

	return preStringLength;
}

/*
Name:tftp12CreateOACKPkt
function:create an OACK packet
input: pktDescriptor
output: packet length
packet format:
    2       n      1     n      1                                n      1      n     1
+-------+---~~---+---+---~~---+---+---~~---+---+---~~---+---+---------+---+---~~---+---+
|  opc  |  opt1  | 0 | value1 | 0 |          ......             optN  | 0 | valueN | 0 |
+-------+---~~---+---+---~~---+---+---~~---+---+---~~---+---+---------+---+---~~---+---+
 6-OACK
*/
INT32 tftp12CreateOACKPkt(TFTP12Description *pktDescriptor)
{
    UINT8 preStringLength = 0;  /*在填充一个字段之前已填好的字段长度*/

	/*填充opcode:6*/
    *((UINT8 *)pktDescriptor->buffer) = 0x00;
    *((UINT8 *)pktDescriptor->buffer+1) = 0x06;
    preStringLength = 2;

    /*填充blockSize，当选项值为0时考虑不填充还是填0，此处采用的方式为不填充*/
    if (pktDescriptor->option.blockSize > 0)
    {
        strncpy((UINT8 *)pktDescriptor->buffer+preStringLength, "blockSize", strlen("blockSize"));   /*填充文件名*/
        *((UINT8 *)pktDescriptor->buffer+preStringLength+strlen("blockSize")) = '\0';
		preStringLength = preStringLength + strlen("blockSize") + 1;

		UINT8 value[10] = {0};
		itoa(pktDescriptor->option.blockSize, value, 10);
		strncpy((UINT8 *)pktDescriptor->buffer+preStringLength, value, strlen(value));   /*填充文件名*/
        *((UINT8 *)pktDescriptor->buffer+preStringLength+strlen(value)) = '\0';
		preStringLength = preStringLength + strlen(value) + 1;

		#if DEBUG
		printf("tftp12CreateOACKPkt填充blocksize后preStringLength = %d\n", preStringLength);
		#endif
    }

	/*填充timeout，当选项值为0时考虑不填充还是填0，此处采用的方式为不填充*/
    if (pktDescriptor->option.timeout > 0)
    {
        strncpy((UINT8 *)pktDescriptor->buffer+preStringLength, "timeout", strlen("timeout"));   /*填充文件名*/
        *((UINT8 *)pktDescriptor->buffer+preStringLength+strlen("timeout")) = '\0';
		preStringLength = preStringLength + strlen("timeout") + 1;

		UINT8 value[10] = {0};
		itoa(pktDescriptor->option.timeout, value, 10);
		strncpy((UINT8 *)pktDescriptor->buffer+preStringLength, value, strlen(value));   /*填充文件名*/
        *((UINT8 *)pktDescriptor->buffer+preStringLength+strlen(value)) = '\0';
		preStringLength = preStringLength + strlen(value) + 1;

		#if DEBUG
		printf("tftp12CreateOACKPkt填充timeout后preStringLength = %d\n", preStringLength);
		#endif
    }

	/*填充tSize，当选项值为0时考虑不填充还是填0，此处采用的方式为不填充*/
    if (pktDescriptor->option.tsize > 0)
    {
        strncpy((UINT8 *)pktDescriptor->buffer+preStringLength, "tSize", strlen("tSize"));   /*填充文件名*/
        *((UINT8 *)pktDescriptor->buffer+preStringLength+strlen("tSize")) = '\0';
		preStringLength = preStringLength + strlen("tSize") + 1;

		UINT8 value[10] = {0};
		itoa(pktDescriptor->option.tsize, value, 10);
		strncpy((UINT8 *)pktDescriptor->buffer+preStringLength, value, strlen(value));   /*填充文件名*/
        *((UINT8 *)pktDescriptor->buffer+preStringLength+strlen(value)) = '\0';
		preStringLength = preStringLength + strlen(value) + 1;

		#if DEBUG
		printf("tftp12CreateOACKPkt填充tsize后preStringLength = %d\n", preStringLength);
		#endif
    }


	return preStringLength;
}

/*
Name:tftp12CreateERRPkt
function:create an Error packet
input: pktDescriptor, errorMsg
output: packet length
packet format:
    2       2        n      1
+-------+---~~---+---~~---+---+---~~---+---+---~~---+---+---~~---+---+---------+---+---~~---+---+
|  opc  |errCode |errorMsg| 0 |                          NULL                                   |
+-------+---~~---+---~~---+---+---~~---+---+---~~---+---+---~~---+---+---------+---+---~~---+---+
 5-ERR
*/
INT32 tftp12CreateERRPkt(TFTP12Description *pktDescriptor, INT16 errorCode, UINT8 *errorMsg)
{
    UINT32 preStringLength = 0;  /*在填充一个字段之前已填好的字段长度*/

	/*填充opcode:5*/
    *((UINT8 *)pktDescriptor->buffer) = 0x00;
    *((UINT8 *)pktDescriptor->buffer+1) = 0x05;
    preStringLength = 2;

    /*填充errCode:0-7*/
    {
		*((UINT8 *)pktDescriptor->buffer+preStringLength) = (errorCode >> 8) & 0xFF;
		*((UINT8 *)pktDescriptor->buffer+preStringLength+1) = errorCode & 0xFF;
		preStringLength = 4;

		#if DEBUG
		printf("tftp12CreateERRPkt填充errorCode后preStringLength = %d\n", preStringLength);
		#endif
    }

	/*填充errorMsg*/
    {
        strncpy((UINT8 *)pktDescriptor->buffer+preStringLength, errorMsg, strlen(errorMsg));   /*填充文件名*/
        *((UINT8 *)pktDescriptor->buffer+preStringLength+strlen(errorMsg)) = '\0';
		preStringLength = preStringLength + strlen(errorMsg) + 1;

		#if DEBUG
		printf("tftp12CreateOACKPkt填充errorMsg后preStringLength = %d\n", preStringLength);
		#endif
    }

	return preStringLength;
}





/************************************************************************/

void getNextSegment(UINT8 **currentString, INT32 *preStringLength)
{
    *preStringLength += strlen(*currentString) + 1;
    *currentString = (UINT8 *)(*currentString) + strlen(*currentString) + 1; /*此处注意，不要弄错了*/

    /*有问题，如果已经到达末尾，则会一直读下去*/
    #if 0
    while(**currentString == '\0')
    {
        *preStringLength += 1;
        *currentString = (UINT8 *)(currentString) + 1;
    }
    #endif
}

/*
Name:tftp12ParseREQPkt
function:Parse a request packet
input: pktDescriptor
output: packet length
packet format:
    2       n      1     n      1     n      1      n     1      n      1      n     1
+-------+---~~---+---+---~~---+---+---~~---+---+---~~---+---+---------+---+---~~---+---+
|  opc  |filename| 0 |  mode  | 0 |  opt1  | 0 | value1 | 0 |   optN  | 0 | valueN | 0 |
+-------+---~~---+---+---~~---+---+---~~---+---+---~~---+---+---------+---+---~~---+---+
1-Read/2-Write   netascii/octet/mail  blockSize               timeout                  tsize
*/
INT32 tftp12ParseREQPkt(TFTP12Description *pktDescriptor)
{
	UINT8 *currentString = 0;
	INT32 preStringLength = 2;

    /*提取opcode,服务器根据此值取反,R->W,W->R*/
    pktDescriptor->opCode = *((UINT8 *)pktDescriptor->buffer);
    pktDescriptor->opCode = (pktDescriptor->opCode << 8) ^ *((UINT8 *)pktDescriptor->buffer+1);

	#if DEBUG
	printf("tftp12ParseREQPkt提取到的opCode=%d\n", pktDescriptor->opCode);
	#endif

	/*提取fileName,考虑到操作系统对大小写敏感性,文件名未进行大小写转换*/
	currentString = (UINT8 *)pktDescriptor->buffer + preStringLength;

    #if DEBUG
	printf("tftp12ParseREQPkt提取到的currentString=%s\n", currentString);
	#endif
	printf("length = %d\n", strlen(currentString));

	pktDescriptor->filename = (UINT8 *)malloc(strlen(currentString) + 1);
	memset(pktDescriptor->filename, 0, strlen(currentString) + 1);			/*清空*/
	strncpy(pktDescriptor->filename, currentString, strlen(currentString));		/*注意filename存储空间的申请和释放*/

	#if DEBUG
	printf("tftp12ParseREQPkt提取到的filename=%s\n", pktDescriptor->filename);
	#endif

	/*提取mode*/

    getNextSegment(&currentString, &preStringLength);       /*指向下一个字段*/
	pktDescriptor->mode = (UINT8 *)malloc(currentString + 1);
	memset(pktDescriptor->mode, 0, strlen(currentString) + 1);
	strncpy(pktDescriptor->mode, currentString, strlen(currentString));
	strlwr(pktDescriptor->mode);	/*转化为小写字符*/

	#if DEBUG
	printf("tftp12ParseREQPkt提取到的mode=%s\n", pktDescriptor->mode);
	#endif

    /*循环提取option选项*/
    getNextSegment(&currentString, &preStringLength);     /*指向下一个字段,字段之间只能一个\0分割，如果有多个会出现解析错误*/
	while (preStringLength < 511)   /*限制位置*/
	{
		strlwr(currentString);

		#if DEBUG
		if (strlen(currentString) >0)
            printf("tftp12ParseREQPkt提取到的option=%s\n", currentString);
		#endif

		if (strnicmp(currentString, "blocksize", strlen("blocksize")) == 0)
		{
		    /*
			preStringLength += strlen(currentString) + 1;
			currentString = (UINT8 *)pktDescriptor->buffer + preStringLength;
			*/
            getNextSegment(&currentString, &preStringLength);   /*使用getNextSegment来代替了上述操作*/
			pktDescriptor->option.blockSize = atoi(currentString);	/*最大为INT32,有问题，如果发过来为空、负数和0怎么处理*/


			#if DEBUG
			printf("tftp12ParseREQPkt提取到的blocksize=%s\n", currentString);
			printf("tftp12ParseREQPkt提取到的pktDescriptor->option.blocksize=%d\n", pktDescriptor->option.blockSize);
			#endif
		}
		else if (strnicmp(currentString, "timeout", strlen("timeout")) == 0)
		{
			getNextSegment(&currentString, &preStringLength);

			pktDescriptor->option.timeout = atoi(currentString);	/*最大为INT32*/

			#if DEBUG
			printf("tftp12ParseREQPkt提取到的timeout=%s\n", currentString);
			printf("tftp12ParseREQPkt提取到的pktDescriptor->option.timeout=%d\n", pktDescriptor->option.timeout);
			#endif
		}
		else if (strnicmp(currentString, "tsize", strlen("tsize")) == 0)
		{
			getNextSegment(&currentString, &preStringLength);

			pktDescriptor->option.tsize = atoi(currentString);	/*最大为INT32*/

			#if DEBUG
			printf("tftp12ParseREQPkt提取到的tsize=%s\n", currentString);
			printf("tftp12ParseREQPkt提取到的pktDescriptor->option.tsize=%d\n", pktDescriptor->option.tsize);
			#endif
		}
		else
			;	/*不明字符串未做处理*/
        getNextSegment(&currentString, &preStringLength);
	}

    return 0;	/*暂时未定义返回值*/
}

/*
Name:tftp12ParseACKPkt
function:Parse an ACK packet
input: pktDescriptor
output: packet blockNumber
packet format:
    2       2
+-------+---~~---+---+---~~---+---+---~~---+---+---~~---+---+---------+---+---~~---+---+
|  opc  |blockNum|                             NULL                                    |
+-------+---~~---+---+---~~---+---+---~~---+---+---~~---+---+---------+---+---~~---+---+
 4-ACK
*/
INT16 tftp12ParseACKPkt(TFTP12Description *pktDescriptor)
{
    UINT16 blockNumber = 0;  /*在填充一个字段之前已填好的字段长度*/

	/*提取opcode*/
    blockNumber = *((UINT8 *)pktDescriptor->buffer+2);
    blockNumber = (blockNumber << 8) & 0x00FF00 | *((UINT8 *)pktDescriptor->buffer+3);
    //blockNumber = blockNumber | *((UINT8 *)pktDescriptor->buffer+2)

	#if DEBUG
	printf("tftp12ParseACKPkt收到的大端十六进制blockNumber=%s\n", (UINT8 *)pktDescriptor->buffer+2);
	#endif

	return blockNumber;
}

/*
Name:tftp12ParseOACKPkt
function:Parse an OACK packet
input: pktDescriptor
output: option number
packet format:
    2       n      1     n      1                                n      1      n     1
+-------+---~~---+---+---~~---+---+---~~---+---+---~~---+---+---------+---+---~~---+---+
|  opc  |  opt1  | 0 | value1 | 0 |          ......             optN  | 0 | valueN | 0 |
+-------+---~~---+---+---~~---+---+---~~---+---+---~~---+---+---------+---+---~~---+---+
 6-OACK
*/
INT32 tftp12ParseOACKPkt(TFTP12Description *pktDescriptor)
{
	/*解析得到的option值会更新原始值，如果对端不支持option则清空相应字段的值*/
	INT32 preStringLength = 2;
    UINT8 argNum = 0;  /*在解析一个字段之前已解析的字段长度*/
	UINT8 *currentString = 0;	/*指向当前被分割出的字符串*/

	/*提取opcode:6,没必要再进行此操作,进此函数代表已知道opcode*/
	#if 0
    *((UINT8 *)pktDescriptor->buffer);
    *((UINT8 *)pktDescriptor->buffer+1);
	#endif

	currentString = (UINT8 *)pktDescriptor->recvBuffer;
	currentString = currentString + 2;	/*跳过opcode*/

	/*协商option过程中,收到对方的OACK数据包后更新值,更新前先清0，此处有问题*/
//	pktDescriptor->option.blockSize = 0;
//	pktDescriptor->option.timeout = 0;
//	pktDescriptor->option.tsize = 0;

	while (*(currentString) != '\0')   /*更改结束条件*/
	{
		argNum++;
		strlwr(currentString);		/*转化为小写*/

		#if DEBUG
		printf("tftp12ParseOACKPkt解析到的option为:%s\n", currentString);
		printf("strnicmp(currentString, \"blkSize\", strlen(\"blkSize\") = %d\n", strnicmp(currentString, "blkSize", strlen("blkSize")));
		#endif

		if (strnicmp(currentString, "blkSize", strlen("blkSize")) == 0)
		{
		    printf("匹配到blkSize!\n");
			getNextSegment(&currentString, &preStringLength);   /*第二个参数暂未使用*/
			pktDescriptor->option.blockSize = atoi(currentString);	/*最大为INT32,有问题，如果发过来为空、负数和0怎么处理*/

			#if DEBUG
			printf("tftp12ParseOACKPkt解析到的blocksize=%s\n", currentString);
			printf("tftp12ParseOACKPkt解析到的pktDescriptor->option.blocksize=%d\n", pktDescriptor->option.blockSize);
			#endif
		}
		else if (strnicmp(currentString, "timeout", strlen("timeout")) == 0)
		{
			getNextSegment(&currentString, &preStringLength);
			pktDescriptor->option.timeout = atoi(currentString);	/*最大为INT32*/

			#if DEBUG
			printf("tftp12ParseOACKPkt解析到的timeout=%s\n", currentString);
			printf("tftp12ParseOACKPkt解析到的pktDescriptor->option.timeout=%d\n", pktDescriptor->option.timeout);
			#endif
		}
		else if (strnicmp(currentString, "tSize", strlen("tSize")) == 0)
		{
			getNextSegment(&currentString, &preStringLength);
			pktDescriptor->option.tsize = atoi(currentString);	/*最大为INT32*/

			#if DEBUG
			printf("pktDescriptor->option.tsize的地址为：%x\n", &pktDescriptor->option.tsize);
			printf("tftp12ParseOACKPkt解析到的tsize=%s\n", currentString);
			printf("tftp12ParseOACKPkt解析到的pktDescriptor->option.tsize=%d\n", pktDescriptor->option.tsize);
			#endif
		}
		else
			;	/*不明字符串未做处理*/

		getNextSegment(&currentString, &preStringLength);
	}
	//getchar();

	return argNum;
}

/*
Name:tftp12ParseERRPkt
function:Parse an Error packet
input: pktDescriptor,errorCode,errorMsg buffer
output: packet length
packet format:
    2       2        n      1
+-------+---~~---+---~~---+---+---~~---+---+---~~---+---+---~~---+---+---------+---+---~~---+---+
|  opc  |errCode |errorMsg| 0 |                          NULL                                   |
+-------+---~~---+---~~---+---+---~~---+---+---~~---+---+---~~---+---+---------+---+---~~---+---+
 5-ERR
*/
INT32 tftp12ParseERRPkt(TFTP12Description *pktDescriptor, INT16 errorCode, UINT8 *errorMsg)
{
	/*提取errCode*/
    errorCode = *((UINT8 *)pktDescriptor->buffer+2);
    errorCode = (errorCode << 8) ^ *((UINT8 *)pktDescriptor->buffer+3);

	/*提取errorMsg*/
    {
        strncpy((UINT8 *)errorMsg, (UINT8 *)pktDescriptor->buffer+4, strlen((UINT8 *)pktDescriptor->buffer+4));   /*这里可以不用拷贝的？*/
        *(errorMsg+strlen((UINT8 *)pktDescriptor->buffer+4)) = '\0';

		#if DEBUG
		printf("tftp12ParseERRPkt提取的(UINT8 *)pktDescriptor->buffer+4为：%s\n", (UINT8 *)pktDescriptor->buffer+4);
		printf("tftp12ParseERRPkt提取的errorMsg为：%s\n", errorMsg);
		#endif
    }

	return errorCode;
}

char *tftp12CreateDataPkt(char *buf, INT16 blockNum)
{
	char *pktHead = buf - 4;
	pktHead = 0;
	pktHead++;
	pktHead = TFTP12_OPCODE_DATA;
	pktHead++;
	*(INT16 *)pktHead = htonl(blockNum);
	pktHead++;
	return pktHead;
}

INT16 tftp12ParseDataPkt(char *buf, INT32 blockSize)
{
	INT16 blockNum = 0;
	char *tem = buf;
	tem++;
	if (*tem!=TFTP12_OPCODE_DATA)
	{
		//换宏
		return -1;
	}
	tem++;
	blockNum = *(INT16*)tem;
	blockNum = htonl(blockNum);
	return blockNum;
}


/*
typedef struct
{
	INT16 opCode;
	UINT8 *filename;	//目的文件名
	INT32 *openFile;	//打开的文件
	UINT8 *mode;	//netascii/octet/mail
	TFTP12Option option;
	INT32 localPort;	//本地出端口
	INT32 sock;
	INT32 transmitBytes;	//已经接收/发送的字节数
	struct sockaddr_in peerAddr;		//对端地址
	void *buffer;	//接收和发送共用缓冲区
}TFTP12Description;

*/

#if 0
int main()
{
    TFTP12Description newSession;
    TFTP12Description session;
    newSession.buffer = malloc(600);
    session.buffer = malloc(1000);
    memset(session.buffer, 0, 256);
    newSession.filename = (UINT8 *)malloc(256);
    memset(newSession.filename, 0, 256);
    strcpy(newSession.filename, "test_parse.a");
    newSession.opCode = 2;
    newSession.option.blockSize = 255;
    newSession.option.timeout = 10;
    newSession.option.tsize = 512;
    newSession.mode = "octet";

    INT32 count = tftp12CreateREQPkt(&newSession);

    printf("count=%d\n", count);
    memcpy(session.buffer, newSession.buffer, 600);
    //session.buffer = newSession.buffer;

    printf("session.buffer=%s\n", (UINT8 *)session.buffer+2);
    tftp12ParseREQPkt(&session);
    printf("session.opcode=%d\n", session.opCode);
    printf("session.filename=%s\n", session.filename);
        printf("session.mode=%s\n", session.mode);
            printf("session.blockSize=%d\n", session.option.blockSize);
                printf("session.timeout=%d\n", session.option.timeout);
                    printf("session.tsize=%d\n", session.option.tsize);

    printf("***************ACK***************************8\n");

    tftp12CreateACKPkt(&newSession, 511);
    printf("blockNumber = %d\n", tftp12ParseACKPkt(&newSession));
    printf("***************OACK***************************8\n");
    printf("OACK length = %d\n", tftp12CreateOACKPkt(&newSession));
    printf("***************errorMsg***************************8\n");
    tftp12CreateERRPkt(&newSession, 32, "This is an error message!");
    UINT8 *error = (UINT8 *)malloc(256);
    memset(error, 0, 256);
    INT16 code;
    printf("构建消息完成!\n");
    printf("errorCode = %d\n", tftp12ParseERRPkt(&newSession, code, error));
    return 0;
}
#endif

#endif
