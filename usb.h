#ifndef _USB_H
#define _USB_H
#include "sam.h"
#define TRANSFER_STATUS_SUCCESS	0
#define TRANSFER_STATUS_ABORTED	1

typedef void (*TrasferCallback)(unsigned char * Buffer, unsigned short Size, void * Param, unsigned char Status);

void USB_Init(void);
unsigned char USB_IsConfigured(void);
// это внешняя функция, она читает данные из первой КТ (стандартом CDC эта КТ выделена под передачу данных от ПК к устройству).
unsigned long USB_Read(unsigned char * pData, unsigned long length, TrasferCallback callback, void * param);
unsigned long USB_Write(const unsigned char * pData, unsigned long length, TrasferCallback callback, void * param);
__inline void UDP_SetAddress (  // \arg pointer to a UDP controller
unsigned char address)   // \arg new UDP address
{
	UDP->UDP_FADDR = (UDP_FADDR_FEN | address);
}
__inline void UDP_SetState (
unsigned int flag)   // \arg new UDP address
{
	//UDP->UDP_GLB_STAT  &= ~(UDP_GLB_STAT_FADDEN| UDP_GLB_STAT_CONFG);
	UDP->UDP_GLB_STAT  |= flag;
}

#if defined ( __ICCARM__ )
#define nop() (__no_operation())
#elif defined ( __GNUC__ )
#define nop() __asm__ __volatile__ ( "nop" )
#endif
/// Bitmap for all status bits in CSR that are not effected by a value 1.
#define REG_NO_EFFECT_1_ALL UDP_CSR_RX_DATA_BK0\
							| UDP_CSR_RX_DATA_BK1\
							| UDP_CSR_STALLSENT\
							| UDP_CSR_RXSETUP\
							| UDP_CSR_TXCOMP
/// Sets the specified bit(s) in the UDP_CSR register.
/// \param endpoint The endpoint number of the CSR to process.
/// \param flags The bitmap to set to 1.

#endif