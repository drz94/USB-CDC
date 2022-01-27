/*
 * usb_cdc.c
 *
 * Created: 08.02.2018 15:37:53
 * Author : Darya
 */ 


#include "sam.h"

#include "config.h"
#define __inline static inline

#include "usb.h"
extern unsigned char quarters [9*8*3+6*4];
unsigned char usbFlag = 0;
unsigned char usartFlag = 0;
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define BOARD_OSCOUNT (CKGR_MOR_MOSCXTST(0x8))
#define CLOCK_TIMEOUT 0xFFFFFFFF
uint32_t timeout = 0;
/* Clock settings (64MHz) */
#define SYS_BOARD_OSCOUNT   (CKGR_MOR_MOSCXTST(0x8UL))
#define SYS_BOARD_PLLAR     (CKGR_PLLAR_ONE \
| CKGR_PLLAR_MULA(0x49UL) \
| CKGR_PLLAR_PLLACOUNT(0x3fUL) \
| CKGR_PLLAR_DIVA(0xeUL))
#define SYS_BOARD_MCKR      (PMC_MCKR_PRES_CLK_2 | PMC_MCKR_CSS_PLLA_CLK)

#define SYS_CKGR_MOR_KEY_VALUE	CKGR_MOR_KEY(0x37UL) /* Key to unlock MOR register */

 void USART_Init(void)
 {
	 PMC	-> PMC_PCER0 = (1 << ID_USART0);
	 USART0->US_IDR	 = 0xFFFFFFFF; // dis interrupts
	 NVIC -> ISER[0]  = (1 << USART0_IRQn); //nvic on


	 USART0 -> US_IER = US_IER_RXRDY;

	 USART0 -> US_CR	 = US_CR_RSTRX | US_CR_RSTTX | US_CR_RXDIS | US_CR_TXDIS;

	 //UART pins conf
	 PIOA    -> PIO_PDR = PIO_PA5A_RXD0 | PIO_PA6A_TXD0 | PIO_PA7A_RTS0;
	 PIOA	-> PIO_ABCDSR[0] &= (~( PIO_PA5A_RXD0 | PIO_PA6A_TXD0 | PIO_PA7A_RTS0 ));
	 PIOA	-> PIO_ABCDSR[1] &= (~( PIO_PA5A_RXD0 | PIO_PA6A_TXD0 | PIO_PA7A_RTS0 ));
	 //mode conf
	 USART0 -> US_MR = US_MR_USART_MODE_RS485 | US_MR_CHRL_8_BIT | US_MR_PAR_NO | US_MR_NBSTOP_1_BIT;
	 //USART0 -> US_BRGR = 0x0015;
	 USART0 -> US_BRGR = 0x001A;//for approx 48MHz
	 USART0 -> US_CR = US_CR_TXEN | US_CR_RXEN;
 }

void ClockInit(void)
{
	WDT->WDT_MR |= WDT_MR_WDDIS;
	EFC->EEFC_FMR = EEFC_FMR_FWS(2); // FWS = 2 = 3 такта ожидания при доступе к FLASH при 48 МГц
	PMC->PMC_WPMR = PMC_WPMR_WPKEY_PASSWD; // разрешаем запись в PMC

	PMC->PMC_SCDR = 0x780; // выключаем тактирование всей периферии
	PMC->PMC_PCDR0 = 0xFFFFFF00;
	PMC->PMC_PCDR1 = 0x00000007;

	//Switch to slow RC
	PMC -> PMC_MCKR = PMC_MCKR_CSS_SLOW_CLK;


	PMC->CKGR_MOR = (PMC->CKGR_MOR & ~CKGR_MOR_MOSCXTBY) |
	CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCXTEN |
	CKGR_MOR_MOSCXTST(255);
	/* Wait the Xtal to stabilize */
	while (!(PMC->PMC_SR & PMC_SR_MOSCXTS));

	PMC->CKGR_MOR |= CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCSEL;
	PMC->CKGR_PLLAR = CKGR_PLLAR_MULA(0); // выключаем PLL



	PMC->CKGR_PLLAR = (CKGR_PLLAR_ONE | CKGR_PLLAR_PLLACOUNT(28) | CKGR_PLLAR_MULA(25) | CKGR_PLLAR_DIVA(5));
	PMC->CKGR_PLLBR = ( CKGR_PLLBR_PLLBCOUNT(28) | CKGR_PLLBR_MULB(25) | CKGR_PLLBR_DIVB(5));

	// Wait the startup time
	while(!(PMC->PMC_SR & (PMC_SR_LOCKA | PMC_SR_LOCKB)));
	while(!(PMC->PMC_SR & PMC_SR_MCKRDY));


	PMC->PMC_MCKR = PMC_MCKR_CSS_PLLA_CLK | PMC_MCKR_PLLADIV2;
	while(!(PMC->PMC_SR & PMC_SR_MCKRDY));

	PMC->PMC_PCER0 = 1<<ID_PIOA; 
	PIOA -> PIO_PER = PIO_PER_P16; //let PIO controls Vbus pin
	PIOA->PIO_IER = PIO_PA16;

	
	NVIC -> ISER[0]  &=~ (1 << (PIOA_IRQn));
	
	PIOA -> PIO_PUDR = 0xFFFFFFFF;
	PIOB -> PIO_OER = 0xF;
	PIOB->PIO_OWER = 0x0000000F;
	
}

void PIOA_Handler()
{
	if (PIOA ->PIO_ISR & PIO_ISR_P16)
	{
		if (PIOA->PIO_PDSR & PIO_PDSR_P16) USB_Init();
		else 
		USB_Stop();	
	}

}
int main( void ) 
{
	ClockInit();	
	USART_Init();
	makeTable();
	unsigned int pinsStatus;
    while (1) 
    {

		pinsStatus = PIOA->PIO_PDSR;//читаем
		pinsStatus = (pinsStatus>>8)&0xFF; 
		if (pinsStatus < 240) PIOB->PIO_ODSR = (quarters[pinsStatus]);
		else PIOB->PIO_ODSR = 0x0;
		PIOA_Handler();
		
    }
}

