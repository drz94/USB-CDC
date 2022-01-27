
#ifndef __SSP__
#define __SSP__

#define _PING    0    //
#define _INIT    1    //
#define _ACK     2    //
#define _NAK     3    //
#define _GET     4    //
#define _PUT     5    //
#define _READ    6
#define _WRITE   7    //
#define _ID      8    //
#define _FASTGET 10
#define _IDN     20

#define MAX_SSP_ADDRESS 4
#define _END     0xC0	/* indicates end of packet */
#define _ESC     0xDB	/* indicates byte stuffing */
#define _ESC_END 0xDC	/* ESC ESC_END means END data byte */
#define _ESC_ESC 0xDD	/* ESC ESC_ESC means ESC data byte */
#define MEMMOVE_4(dest,srce) {*(unsigned char*)(dest) = *(unsigned char*)(srce); *((unsigned char*)(dest)+1) = *((unsigned char*)(srce)+1); *((unsigned char*)(dest)+2) = *((unsigned char*)(srce)+2); *((unsigned char*)(dest)+3) = *((unsigned char*)(srce)+3);}
#define MEMMOVE_3(dest,srce) {*(unsigned char*)(dest) = *(unsigned char*)(srce); *((unsigned char*)(dest)+1) = *((unsigned char*)(srce)+1); *((unsigned char*)(dest)+2) = *((unsigned char*)(srce)+2);}
#define MEMMOVE_2(dest,srce) {*(unsigned char*)(dest) = *(unsigned char*)(srce); *((unsigned char*)(dest)+1) = *((unsigned char*)(srce)+1);}

#define WRITABLE     0x01
#define PROTECTED    0x02
#define FLASH        0x04


unsigned short TableCRC16(unsigned char * pcBlock, unsigned short len);



#endif


