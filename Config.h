/*
 * IncFile1.h
 *
 * Created: 08.02.2018 16:31:23
 *  Author: Darya
 */ 


#ifndef Config_h
#define Config_h

#define EXT_OC          18432000   // Exetrnal ocilator MAINCK
#define MCK             47923200   // MCK (PLLRC div by 2)
#define MCK_MHZ		48

#define DEBUG_USART_BR	115200

#define TRUE	1
#define FALSE	0


typedef signed short error_t;

typedef void *  HANDLE;

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#define UDP_STALLSENT   (0x1 <<  3) // (UDP) Stall sent (Control, bulk, interrupt endpoints)

#endif