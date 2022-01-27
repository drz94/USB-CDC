#include "config.h"
#define __inline static inline

#include "usb.h" 
#include "sam3s.h" 
#include <stdio.h>
#include <string.h>
unsigned long cpt;


//размеры аппаратного буфера для разных конечных точек
#define EP_BUFFER_SIZE                  64
#define EP_CFG_BUFFER_SIZE              64


#define USB_UNICODE(a)			(a), 0x00

//-----------------------------------------------------------------------------------------------------------------------------


typedef struct {
	unsigned char bmRequestType;
	/*
	bmRequestType – битовое поле, которое содержит характеристики запроса:

	Бит 7: Направление передачи данных в DATA фазе:
	0 = от хоста к устройству
	1 = от устройства к хосту

	Биты 6...5: Тип запроса
	0 = стандартный запрос USB
	1 = стандартный запрос для определенного класса устройств USB
	2 = пользовательский запрос
	3 = зарезервировано

	Биты 4...0: Кому адресован запрос
	0 = устройству
	1 = интерфейсу
	2 = конечной точке
	3 = другое
	4...31 = зарезервировано
	*/
	unsigned char bRequest; //bRequest – уникальный код запроса (например, 0 = GET_STATUS, 5 = SET_ADDRESS, 6 = GET_DESCRIPTOR и т. д.)
	unsigned short wValue;//для передачи параметров запроса (адрес..)
	unsigned short wIndex;// значение данного поля зависит от типа запроса. Например, для запроса GET_STATUS к конечной точке, данное поле содержит индекс конечной точки, статус которой хост хочет получить.
	unsigned short wLength;
} UsbRequest;


typedef struct {
	unsigned long DTERRate;
	unsigned char CharFormat;
	unsigned char ParityType;
	unsigned char DataBits;
} LineCoding;

//-----------------------------------------------------------------------------------------------------------------------------
volatile static unsigned char CurrentConfiguration = 0;

//-----------------------------------------------------------------------------------------------------------------------------
volatile static LineCoding line = { 115200 /*baudrate*/, 0 /*1 Stop Bit*/, 0 /*None Parity*/, 8 /*8 Data bits*/};
//unsigned char line[7];// = {0x00, 0xC2, 0x01, 0x00, 0x00, 0x00, 0x08};

//-----------------------------------------------------------------------------------------------------------------------------
static const unsigned char devDescriptor[] = {
	/* Device descriptor */
	0x12,   // bLength
	0x01,   // bDescriptorType
	0x00,   // bcdUSBL
	0x02,   //
	0x02,   // bDeviceClass:    CDC class code
	0x00,   // bDeviceSubclass: CDC class sub code
	0x00,   // bDeviceProtocol: CDC Device protocol
	0x40,   // bMaxPacketSize0
	0xEB,   // idVendorL
	0x03,   //
	0x27,   // idProductL
	0x61,   //
	0x10,   // bcdDeviceL
	0x01,   //
	0x01,   // iManufacturer
	0x02,   // iProduct
	0x03,   // SerialNumber
	0x01    // bNumConfigs
};

static const unsigned char cfgDescriptor[] = {
	/* ============== CONFIGURATION 1 =========== */
	/* Configuration 1 descriptor */
	0x09,   // CbLength
	0x02,   // CbDescriptorType
	0x43,   // CwTotalLength 2 EP + Control
	0x00,
	0x02,   // CbNumInterfaces
	0x01,   // CbConfigurationValue
	0x00,   // CiConfiguration
	0xC0,   // CbmAttributes 0xA0
	0x00,   // CMaxPower

	/* Communication Class Interface Descriptor Requirement */
	0x09, // bLength
	0x04, // bDescriptorType
	0x00, // bInterfaceNumber
	0x00, // bAlternateSetting
	0x01, // bNumEndpoints
	0x02, // bInterfaceClass
	0x02, // bInterfaceSubclass
	0x00, // bInterfaceProtocol
	0x00, // iInterface

	/* Header Functional Descriptor */
	0x05, // bFunction Length
	0x24, // bDescriptor type: CS_INTERFACE
	0x00, // bDescriptor subtype: Header Func Desc
	0x10, // bcdCDC:1.1
	0x01,

	/* ACM Functional Descriptor */
	0x04, // bFunctionLength
	0x24, // bDescriptor Type: CS_INTERFACE
	0x02, // bDescriptor Subtype: ACM Func Desc
	0x00, // bmCapabilities

	/* Union Functional Descriptor */
	0x05, // bFunctionLength
	0x24, // bDescriptorType: CS_INTERFACE
	0x06, // bDescriptor Subtype: Union Func Desc
	0x00, // bMasterInterface: Communication Class Interface
	0x01, // bSlaveInterface0: Data Class Interface

	/* Call Management Functional Descriptor */
	0x05, // bFunctionLength
	0x24, // bDescriptor Type: CS_INTERFACE
	0x01, // bDescriptor Subtype: Call Management Func Desc
	0x00, // bmCapabilities: D1 + D0
	0x01, // bDataInterface: Data Class Interface 1

	/* Endpoint 1 descriptor */
	0x07,   // bLength
	0x05,   // bDescriptorType
	0x83,   // bEndpointAddress, Endpoint 03 - IN
	0x03,   // bmAttributes      INT
	0x08,   // wMaxPacketSize
	0x00,
	0xFF,   // bInterval

	/* Data Class Interface Descriptor Requirement */
	0x09, // bLength
	0x04, // bDescriptorType
	0x01, // bInterfaceNumber
	0x00, // bAlternateSetting
	0x02, // bNumEndpoints
	0x0A, // bInterfaceClass
	0x00, // bInterfaceSubclass
	0x00, // bInterfaceProtocol
	0x00, // iInterface

	/* First alternate setting */
	/* Endpoint 1 descriptor */
	0x07,   // bLength
	0x05,   // bDescriptorType
	0x01,   // bEndpointAddress, Endpoint 01 - OUT
	0x02,   // bmAttributes      BULK
	EP_BUFFER_SIZE,   // wMaxPacketSize
	0x00,
	0x00,   // bInterval

	/* Endpoint 2 descriptor */
	0x07,   // bLength
	0x05,   // bDescriptorType
	0x82,   // bEndpointAddress, Endpoint 02 - IN
	0x02,   // bmAttributes      BULK
	EP_BUFFER_SIZE,   // wMaxPacketSize
	0x00,
	0x00    // bInterval
};

static const unsigned char strLanguage[] = {
	0x04,
	0x03,
	0x04,
	0x09
};



static const unsigned char  strManufacturer[] = {
    0x12,                                                   /*bLength must match size of stringImanufacturerDescriptor array (here 18 bytes - hex 0x12) */
    0x03,                                      /*bDescriptorType 0x03*/
    'o',0,'p',0,'t',0,'o',0,'l',0,'i',0,'n',0,'k',0,        /*bString iManufacturer - mbed.org*/
    };


static const unsigned char strProduct[] = {
	0x02,
	0x03,
};
static const unsigned char strSerial[] = {
	0x03,
	0x03,
	0x30,
};

//-----------------------------------------------------------------------------------------------------------------------------
/* USB standard request code */
#define STD_GET_STATUS_ZERO           0x0080
#define STD_GET_STATUS_INTERFACE      0x0081
#define STD_GET_STATUS_ENDPOINT       0x0082

#define STD_CLEAR_FEATURE_ZERO        0x0100
#define STD_CLEAR_FEATURE_INTERFACE   0x0101
#define STD_CLEAR_FEATURE_ENDPOINT    0x0102

#define STD_SET_FEATURE_ZERO          0x0300
#define STD_SET_FEATURE_INTERFACE     0x0301
#define STD_SET_FEATURE_ENDPOINT      0x0302

#define STD_SET_ADDRESS               0x0500
#define STD_GET_DESCRIPTOR            0x0680
#define STD_SET_DESCRIPTOR            0x0700
#define STD_GET_CONFIGURATION         0x0880
#define STD_SET_CONFIGURATION         0x0900
#define STD_GET_INTERFACE             0x0A81
#define STD_SET_INTERFACE             0x0B01
#define STD_SYNCH_FRAME               0x0C82

/* CDC Class Specific Request Code */
#define GET_LINE_CODING               0x21A1
#define SET_LINE_CODING               0x2021
#define SET_CONTROL_LINE_STATE        0x2221


//-----------------------------------------------------------------------------------------------------------------------------
static void USB_Enumerate(UsbRequest * Request);



//--- Сброс конечной точки (КТ)
static void EndpointReset()
{
	//Аппаратно "передергиваем" КТ
	UDP->UDP_RST_EP |= UDP_RST_EP_EP0;
	UDP->UDP_RST_EP &= ~UDP_RST_EP_EP0;
	UDP->UDP_RST_EP |= UDP_RST_EP_EP1;
	UDP->UDP_RST_EP &= ~UDP_RST_EP_EP1;
	UDP->UDP_RST_EP |= UDP_RST_EP_EP2;
	UDP->UDP_RST_EP &= ~UDP_RST_EP_EP2;
}


unsigned char in_buf_usb[400] = {0};
unsigned char in_bank = 0; 
//--- Обработка аппаратного прерывания
void UDP_Handler(void)
 {
	unsigned int isr = 0;
	isr = UDP->UDP_ISR;
	//обнаружен "Сброс" на шине
	if (isr & UDP_ISR_ENDBUSRES)
	{
			EndpointReset();
			UDP -> UDP_CSR[0] = (UDP_CSR_EPEDS | UDP_CSR_EPTYPE_CTRL);
			//Сбрасываем флаг прерывания
			UDP->UDP_ICR = UDP_ICR_ENDBUSRES;
     	    //Разрешаем прерывания для "нулевой конечной точки"
			UDP->UDP_IER |= UDP_IER_EP0INT;
			UDP->UDP_FADDR |= UDP_FADDR_FEN;
			UDP->UDP_FADDR |= UDP_FADDR_FADD(0);
			//Разрешаем передачу
			UDP -> UDP_TXVC &= ~UDP_TXVC_TXVDIS;
			//Сбрасываем номер текущей конфигурации
			CurrentConfiguration = 0;
		}

	//Прерывание на конечной точке 0 - вызываем обработчик и сбрасываем флаг
	if (isr & UDP_ISR_EP0INT)
	{
 		//EndpointHandler(&Endpoint[0]);
   		if (UDP->UDP_CSR[0] & UDP_CSR_RX_DATA_BK0)
  		{
   			clrflagCSR(0, UDP_CSR_RX_DATA_BK0); 		
  		}
		//SETUP recv
		if(UDP->UDP_CSR[0] & UDP_CSR_RXSETUP) {
			UsbRequest Request;
			for(unsigned char i = 0; i < 8; i++) ((unsigned char *) &Request)[i] = UDP->UDP_FDR[0];
			if(Request.bmRequestType & 0x80) setFlagCSR(0, UDP_CSR_DIR );
			clrflagCSR(0, UDP_CSR_RXSETUP);
			USB_Enumerate(&Request);
			UDP->UDP_IER |= UDP_IER_EP1INT;
			UDP->UDP_IER |= UDP_IER_EP2INT;
		}
		//STALL handshake
		if(UDP->UDP_CSR[0]& UDP_CSR_STALLSENT)
		{
			clrflagCSR(0, UDP_CSR_STALLSENT);
			clrflagCSR(0, UDP_CSR_FORCESTALL);
		}
	}

	//Прерывание на конечной точке 1 - вызываем обработчик и сбрасываем флаг
	if (isr  & UDP_ISR_EP1INT)
	{		
		if((UDP->UDP_CSR[1] & UDP_CSR_RX_DATA_BK0) || (UDP->UDP_CSR[1] & UDP_CSR_RX_DATA_BK1)  )
		{	
			cpt = UDP->UDP_CSR[1] >> 16;
			for (int i = 0; i<cpt; i++)
			{
				in_buf_usb[i] = UDP->UDP_FDR[1];
			}
			if(UDP->UDP_CSR[1] & UDP_CSR_RX_DATA_BK1) clrflagCSR(1,UDP_CSR_RX_DATA_BK1);
			if(UDP->UDP_CSR[1] & UDP_CSR_RX_DATA_BK0) clrflagCSR(1,UDP_CSR_RX_DATA_BK0);
			ep1_Handler(in_buf_usb, cpt);
		}

		if(UDP -> UDP_CSR[1] & UDP_CSR_STALLSENT)
		{
			clrflagCSR(1, UDP_CSR_STALLSENT);
			clrflagCSR(1, UDP_CSR_FORCESTALL);
		}

	}


	//Заглушка для событий Sleep Wakeup и т. д. - просто сбрасываем флаг прерывания
	if(isr  && UDP_ISR_RXSUSP | UDP_ISR_RXRSM | UDP_ISR_EXTRSM | UDP_ISR_SOFINT |UDP_ISR_WAKEUP) {
		UDP->UDP_ICR = UDP_ICR_RXSUSP | UDP_ICR_RXRSM | UDP_ICR_EXTRSM | UDP_ICR_SOFINT | UDP_ICR_WAKEUP;
	}
	
}


//-----------------------------------------------------------------------------------------------------------------------------
static void USB_CfgSendStall(void)
{
	setFlagCSR(0, UDP_CSR_FORCESTALL);
}

void USB_CfgSendZlp1 (unsigned char ep)
{
	while(UDP->UDP_CSR[ep] & UDP_CSR_TXPKTRDY);
	UDP -> UDP_FDR[ep] = 0;
	UDP->UDP_CSR[ep] |= UDP_CSR_TXPKTRDY;
	while ((UDP->UDP_CSR[ep] & UDP_CSR_TXPKTRDY)==0);
	clrflagCSR(ep,UDP_CSR_TXCOMP);
}
unsigned const char GET = 0x30;
unsigned const char SET = 0x31;
//-----------------------------------------------------------------------------------------------------------------------------
static void USB_Enumerate(UsbRequest * Request) {

	unsigned short wStatus;

	switch ((Request->bRequest << 8) | Request->bmRequestType) {
		//Запрос дескриптора
		case STD_GET_DESCRIPTOR:
		switch(Request->wValue) {
			case 0x100:
			//Запрос дескриптора устройства
			sendArray(0, devDescriptor, 18);
			break;
			case 0x200:
			//Запрос дескриптора конфигурации
			//USB_CfgSend(cfgDescriptor, min(sizeof(cfgDescriptor), Request->wLength), NULL, NULL);
			sendArray(0, cfgDescriptor, min(sizeof(cfgDescriptor), Request->wLength));
			break;
			case 0x300:
			//Запрос строкового дескриптора
			//USB_CfgSend(strLanguage, min(sizeof(strLanguage), Request->wLength), NULL, NULL);
			sendArray(0, strLanguage, sizeof(strLanguage));
			break;
			case 0x301:
			//Запрос строкового дескриптора
			//USB_CfgSend(strManufacturer, min(sizeof(strManufacturer), Request->wLength), NULL, NULL);
			sendArray(0, strManufacturer, sizeof(strManufacturer));
							if (USART0 -> US_CSR & US_CSR_TXEMPTY)
							USART0 -> US_THR = 0x33;

			break;
			case 0x302:
			//Запрос строкового дескриптора
			//USB_CfgSend(strProduct, min(sizeof(strProduct), Request->wLength), NULL, NULL);
			sendArray(0, strProduct, sizeof(strProduct));
							if (USART0 -> US_CSR & US_CSR_TXEMPTY)
							USART0 -> US_THR = 0x32;

			break;
			case 0x303:
			//Запрос строкового дескриптора
			//USB_CfgSend(strSerial,min(sizeof(strSerial), Request->wLength), NULL, NULL);
			sendArray(0, strSerial, sizeof(strSerial));
							if (USART0 -> US_CSR & US_CSR_TXEMPTY)
							USART0 -> US_THR = 0x31;
			break;
			default:
			//Неизвестный тип дескриптора
			USB_CfgSendStall();
			break;
		}
		break;
		case STD_SET_ADDRESS:
		{

			unsigned long Value = (unsigned long) Request->wValue;
			//ждем готовности хоста принять данные
			while(UDP->UDP_CSR[0] & UDP_CSR_TXPKTRDY); //1 = The data is waiting to be sent upon reception of token IN.
			//write nothing
			//"data in buffer ready"
			UDP->UDP_CSR[0] |= UDP_CSR_TXPKTRDY;
			while ((UDP->UDP_CSR[0] & UDP_CSR_TXPKTRDY)==0);
			//wait host to read data
			while (!(UDP->UDP_CSR[0] & UDP_CSR_TXCOMP));
			UDP->UDP_FADDR = (UDP_FADDR_FEN | (Value & UDP_FADDR_FADD_Msk));
			UDP_SetState(UDP_GLB_STAT_FADDEN);
			UDP->UDP_CSR[0] &= ~(UDP_CSR_TXCOMP);
			while((UDP->UDP_CSR[0] & UDP_CSR_TXCOMP) != 0);
		}
		break;
		case STD_SET_CONFIGURATION:
		//Установка конфигурации устройства
		//USB_CfgSendZlp(USB_SetConfig, (void *) (unsigned long) Request->wValue);
		{
			//посылаем пустой пакет, чтобы подтвердить получение адреса
			unsigned long Value = (unsigned long) Request->wValue;
			//ждем готовности хоста принять данные
			while(UDP->UDP_CSR[0] & UDP_CSR_TXPKTRDY); //1 = The data is waiting to be sent upon reception of token IN.
			//write nothing
			//"data in buffer ready"
			UDP->UDP_CSR[0] |= UDP_CSR_TXPKTRDY;
			while ((UDP->UDP_CSR[0] & UDP_CSR_TXPKTRDY)==0);
			//wait host to read data
			while (!(UDP->UDP_CSR[0] & UDP_CSR_TXCOMP));
			UDP->UDP_GLB_STAT  |= UDP_GLB_STAT_CONFG;
			UDP->UDP_CSR[1] = Value ? (UDP_CSR_EPEDS | UDP_CSR_EPTYPE_BULK_OUT) : 0;
			UDP->UDP_CSR[2] = Value ? (UDP_CSR_EPEDS | UDP_CSR_EPTYPE_BULK_IN)  : 0;
			UDP->UDP_CSR[3] = Value ? (UDP_CSR_EPEDS | UDP_CSR_EPTYPE_INT_IN)   : 0;
			 CurrentConfiguration = Value;
			UDP->UDP_CSR[0] &= ~(UDP_CSR_TXCOMP);
			while((UDP->UDP_CSR[0] & UDP_CSR_TXCOMP) != 0);
		}
		break;
		case STD_GET_CONFIGURATION:
		//Запрос номера конфигурации устройства
		//USB_CfgSend((unsigned char*) &(CurrentConfiguration), sizeof(CurrentConfiguration), NULL, NULL);
		sendArray(0, (unsigned char*) &(CurrentConfiguration), sizeof(CurrentConfiguration));
		break;
		case STD_GET_STATUS_ZERO:
		//Запрос статуса устройства
		wStatus = 0;
		//USB_CfgSend((unsigned char*) &wStatus, sizeof(wStatus), NULL, NULL);
		sendArray(0, (unsigned char *) &wStatus, sizeof(wStatus));
		break;
		case STD_GET_STATUS_INTERFACE:
		//Запрос статуса интерфейса
		wStatus = 0;
		//USB_CfgSend((unsigned char *) &wStatus, sizeof(wStatus), NULL, NULL);
		sendArray(0, (unsigned char *) &wStatus, sizeof(wStatus));
		break;
		case STD_GET_STATUS_ENDPOINT:
		//Запрос статуса конечной точки
		wStatus = 0;
		Request->wIndex &= 0x0F;
		if ((UDP->UDP_GLB_STAT & UDP_GLB_STAT_CONFG) && (Request->wIndex <= 3)) {
			wStatus = (UDP->UDP_CSR[Request->wIndex] & UDP_CSR_EPEDS) ? 0 : 1;
			sendArray(0, (unsigned char *) &wStatus, sizeof(wStatus));
		}
		else if ((UDP->UDP_GLB_STAT & UDP_FADDR_FEN) && (Request->wIndex == 0)) {
			wStatus = (UDP->UDP_CSR[Request->wIndex] & UDP_CSR_EPEDS) ? 0 : 1;
			sendArray(0, (unsigned char *) &wStatus, sizeof(wStatus));
		}
		else
		USB_CfgSendStall();
		break;
		case STD_SET_FEATURE_ZERO:
		USB_CfgSendStall();
		break;
		case STD_SET_FEATURE_INTERFACE:
		USB_CfgSendZlp1 (0);
		break;
		case STD_SET_FEATURE_ENDPOINT:
		Request->wIndex &= 0x0F;
		if ((Request->wValue == 0) && Request->wIndex && (Request->wIndex <= 3)) {
			UDP->UDP_CSR[Request->wIndex] = 0;
			USB_CfgSendZlp1 (0);
		}
		else
		USB_CfgSendStall();
		break;
		case STD_CLEAR_FEATURE_ZERO:
		USB_CfgSendStall();
		break;
		case STD_CLEAR_FEATURE_INTERFACE:
		USB_CfgSendZlp1(0);
		break;
		case STD_CLEAR_FEATURE_ENDPOINT:
		Request->wIndex &= 0x0F;
		if ((Request->wValue == 0) && Request->wIndex && (Request->wIndex <= 3)) {
			if (Request->wIndex == 1)
			UDP->UDP_CSR[1] = (UDP_CSR_EPEDS | UDP_CSR_EPTYPE_BULK_OUT);
			else if (Request->wIndex == 2)
			UDP->UDP_CSR[2] = (UDP_CSR_EPEDS | UDP_CSR_EPTYPE_BULK_IN);
			else if (Request->wIndex == 3)
			UDP->UDP_CSR[3] = (UDP_CSR_EPEDS | UDP_CSR_EPTYPE_INT_IN);
			USB_CfgSendZlp1 (0);
		}
		else
		USB_CfgSendStall();
		break;

		//Запросы специфические для CDC устройств
		case SET_LINE_CODING:
			while (!(UDP->UDP_CSR[0] & UDP_CSR_RX_DATA_BK0) );
			{
				cpt = UDP->UDP_CSR[0] >> 16;
				for (int i=0; i<cpt; i++) (*(((unsigned char*)&line) + i)) = UDP -> UDP_FDR[0];
			//for (int i=0; i<sizeof(line); i++) line[i] = UDP -> UDP_FDR[0];
				clrflagCSR(0,UDP_CSR_RX_DATA_BK0);
			}
			USB_CfgSendZlp1(0);
		break;

		case GET_LINE_CODING:
		//USB_CfgSend((unsigned char*) &line, min(sizeof(line), Request->wLength), NULL, NULL);
		sendArray(0, (unsigned char*) &line, sizeof(line));
		break;

		case SET_CONTROL_LINE_STATE:
		USB_CfgSendZlp1 (0);
		break;

		default:
		//_DBG("[USB ENUM REQUEST %04X]\n", (Request->bRequest << 8) | Request->bmRequestType);
		USB_CfgSendStall();
		break;
	}
}

void setFlagCSR(unsigned char endpoint, unsigned int flags)
{
	unsigned char nop_count;
	volatile unsigned int reg;
	reg = UDP->UDP_CSR[endpoint] ;
	reg |= REG_NO_EFFECT_1_ALL;
	reg |= (flags);
	UDP->UDP_CSR[endpoint] = reg;
	for( nop_count=0; nop_count<15; nop_count++ )
	{
		nop();
	}
}
void clrflagCSR(unsigned char endpoint, unsigned int flags)
{
	unsigned char nop_count;
	volatile unsigned int reg;
	reg = UDP->UDP_CSR[endpoint];
	reg |= REG_NO_EFFECT_1_ALL;
	reg &= ~(flags);
	UDP->UDP_CSR[endpoint] = reg;
	for( nop_count=0; nop_count<15; nop_count++ )
	{
		nop();
	}
}
void sendArray (unsigned char ep, const unsigned char array[], char size)
{
	unsigned char maxSize = 64;
	//ждем готовности хоста принять данные
	while(UDP->UDP_CSR[0] & UDP_CSR_TXPKTRDY);

	for (int i = 0; i < size; i++)
	{
		if ( !((i+1) % maxSize) || (i == (size - 1)) )
		{
			UDP -> UDP_FDR[ep] = array[i];
			UDP->UDP_CSR[0] |= UDP_CSR_TXPKTRDY;
			while ((UDP->UDP_CSR[ep] & UDP_CSR_TXPKTRDY)==0);
			//wait host to read data
			while (!(UDP->UDP_CSR[ep] & UDP_CSR_TXCOMP));
			UDP->UDP_CSR[ep] &= ~(UDP_CSR_TXCOMP);
			while((UDP->UDP_CSR[ep] & UDP_CSR_TXCOMP) != 0);
		}
		else
		{
			UDP -> UDP_FDR[ep] = array[i];
		}
	}
}
//-----------------------------------------------------------------------------------------------------------------------------
void USB_Init(void) 
{
	PMC->PMC_USB = PMC_USB_USBS | PMC_USB_USBDIV(1); //PLLB /2
	PMC->PMC_PCER1 = PMC_PCER1_PID34; // тактирование USB
	PMC->PMC_SCER = PMC_SCER_UDP;
	UDP->UDP_IDR = 0xFFFFFFFF;
	UDP->UDP_TXVC |= UDP_TXVC_PUON;  //подтягиваем
	
	CurrentConfiguration = 0;
	EndpointReset();

	PMC	-> PMC_PCER1 = (1 << (ID_UDP-32));

	NVIC -> ISER[1]  = (1 << (UDP_IRQn-32)); //nvic on
	while(!CurrentConfiguration);
}
void USB_Stop()
{
	PMC	-> PMC_PCER1 &=~ (1 << (ID_UDP-32));

	NVIC -> ISER[1]  &=~ (1 << (UDP_IRQn-32)); //nvic on
}