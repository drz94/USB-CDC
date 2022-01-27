 #include "sam.h"

 #include "SSP.h"

#define baudrate 115200
#define mainfrec 18432000
extern unsigned char usbFlag;
extern unsigned char usartFlag;
const unsigned short Crc16Table[256] =
{
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
	0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
	0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
	0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
	0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
	0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
	0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
	0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
	0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
	0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
	0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
	0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
	0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
	0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
	0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
	0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
	0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
	0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
	0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
	0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
	0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
	0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
	0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};
//=======================================================================================================================
#define BUFFER_SIZE 400
#define SELFADDRESS 100
//=======================================================================================================================
unsigned char out_buf_lng = 0;
unsigned char in_buf_ptr = 0;
unsigned char in_buf[BUFFER_SIZE] = {0};
unsigned char out_buf[BUFFER_SIZE] = {0};
const char *const IDN_String = "ROM Emulator";
const char *const ID_String = "ROM ver 1";
unsigned char quarters [9*8*3+6*4];
unsigned int errorMarker10;
unsigned int errorMarker11;
unsigned int errorMarker13;
unsigned int errorMarker12;


unsigned char Update_CRC8(unsigned char in, unsigned char* crc, unsigned char bits) //INITIAL = 0x00
{
	#define POLYNOMIAL 0x07
	
	if ((!bits) || (bits > 8)) return 1;
	in <<=(8-bits);
	
	*crc = *crc ^ in;
	for (unsigned char i = 0; i < bits; i++)
	*crc = (*crc & 0x80)? (*crc << 1) ^ POLYNOMIAL : (*crc << 1);

	return 0;
}


//unsigned char fulldata[256];
//=======================================================================================================================

void writetoflashpage() __attribute__((section(".ramfunc")));
unsigned short table13array[4];
void writetoflashpage(unsigned char data[], unsigned int* PageFlashMemory, int number)
{
	unsigned int* ptrdata = (unsigned int*)data;
	//int number = (256/4);  //++4 bytes
	for (int i = 0; i < number; i++)
	*(PageFlashMemory++) = *(ptrdata++);
	 
	EFC -> EEFC_FCR = 0x5a << 24 | (((unsigned int)PageFlashMemory>>8)&0x3FF)<<8 | 0x03;	
	while(!(EFC -> EEFC_FSR & EEFC_FSR_FRDY));
}

void readfromflashchar(unsigned char arraytest[], unsigned char* pageFlashMemory, char number) //write here new array
{

	//unsigned char ptrdata;
	//int number = 256;
	for (int i = 0; i < number; i++)
	{
		arraytest[i] = *(pageFlashMemory++);
	}
}
void readfromflashshort(unsigned short arraytest[], unsigned short* pageFlashMemory, char number) //write here new array
{

	//unsigned short ptrdata;
	//int number = 256;
	for (int i = 0; i < number; i++)
	{
		arraytest[i] = *(pageFlashMemory++);
	}
}
unsigned char readfromaddr(unsigned short* PageFlashMemory) //write here new array
{
	unsigned char elem;
	elem = *(PageFlashMemory);
	return(	elem);
}
unsigned char readfromaddrchar (unsigned char* PageFlashMemory) //write here new array
{
	unsigned char elem;
	elem = *(PageFlashMemory);
	return(	elem);
}
void makeTable()
{
	//unsigned char crc = 0xff;
	unsigned short table10array[16];
	unsigned char crc = 0;
	unsigned short table11array[16];
	unsigned short table12array[16];
	for (int i = 0; i<17; i++)
	{
		table10array[i] = 0;
		table11array[i] = 0;
		table12array[i] = 0;
	}
	//unsigned short table13array[4];
	unsigned short* Page10FlashMemory = (unsigned short*)(0x400000 + 0x40000 - 0x400);
	unsigned short* Page11FlashMemory = (unsigned short*)(0x400000 + 0x40000 - 0x300);
	unsigned short* Page12FlashMemory = (unsigned short*)(0x400000 + 0x40000 - 0x200);
	unsigned short* Page13FlashMemory = (unsigned short*)(0x400000 + 0x40000 - 0x100);
	unsigned long long output[8*3+4];//по количеству строк
	unsigned short size1 = readfromaddr (Page10FlashMemory+16);
	unsigned short size2 = readfromaddr (Page11FlashMemory+16);
	unsigned short size3 = readfromaddr (Page12FlashMemory+16);
	readfromflashshort(table10array, Page10FlashMemory, 16);
	readfromflashshort(table11array, Page11FlashMemory, 16);
	readfromflashshort(table12array, Page12FlashMemory, 16);
	readfromflashshort(table13array, Page13FlashMemory, 4);

	for (int i = 0; i<28; i++) output[i] = 0;
	
	unsigned char cnt1 = (size1/2) - 1;
	unsigned char cnt2 = (size2/2) - 1;;
	unsigned char cnt3 = (size3/2) - 1;;
	unsigned char cnt4 = 0;

	size1/=4; size2/=4; size3/=4;
	errorMarker10 &=0xFF;
	errorMarker11 &=0xFF;
	errorMarker12 &=0xFF;
	errorMarker13 &=0xF;
	//unsigned long long temp;
	if (size1 == 0)
	{
		for (int i = 7; i>=0; i--)
		{
			crc = 0xFF;
			output[i] = 0;
			for (int k=6; k >= 0; k--)	Update_CRC8((output[i] >> 4*k) & 0x0F, &crc, 4);
			output[i] = (output[i]+((unsigned long long)crc<<28));
		}
	}
	else
	{
		for (int i = 7; i>=0; i--)
		{
			if (size1>0)
			{
				crc = 0xFF;
				output[i] = table10array[cnt1]& 0x3FFF;
				output[i]<<=14;
				output[i] += table10array[cnt1-1] & 0x3FFF;
				for (int k=6; k >= 0; k--)
				{
					Update_CRC8((output[i] >> 4*k) & 0x0F, &crc, 4);
				}

				output[i] = (output[i]+((unsigned long long)crc<<28));
				if (errorMarker10 & (1<<(7-i))) output[i] = ~output[i]&0xFFFFFFFFF;
				cnt1-=2;
				size1--;
			}
			else
			{
				unsigned short size1 = readfromaddr (Page10FlashMemory+16);
				output[i] = output[8-(size1/4)];
				if (errorMarker10 & (1<<(size1/4-1))) output[i] = ~output[i]&0xFFFFFFFFF;
				if (errorMarker10 & (1<<(7-i))) output[i] = ~output[i]&0xFFFFFFFFF;
			}
		}
	}
	
	if (size2 == 0)
	{
		for (int i = 15; i>=8; i--)
		{
			crc = 0xFF;
			output[i] = 0;
			for (int k=6; k >= 0; k--)	Update_CRC8((output[i] >> 4*k) & 0x0F, &crc, 4);
			output[i] = (output[i]+((unsigned long long)crc<<28));
		}
	}
	else
	{
		for (int i = 15; i>=8; i--)
		{
			if (size2>0)
			{
				crc = 0xFF;
				output[i] = table11array[cnt2]& 0x3FFF;
				output[i]<<=14;
				output[i] += table11array[cnt2-1] & 0x3FFF;
				for (int k=6; k >= 0; k--)
				{
					Update_CRC8((output[i] >> 4*k) & 0x0F, &crc, 4);
				}
				output[i] = (output[i]+((unsigned long long)crc<<28));
				if (errorMarker11 & (1<<(15-i))) output[i] = ~output[i]&0xFFFFFFFFF;
				cnt2-=2;
				size2--;
			}
			else
			{
				unsigned short size2 = readfromaddr (Page11FlashMemory+16);
				output[i] = output[16-(size2/4)];
				if (errorMarker11 & (1<<(size2/4))) output[i] = ~output[i]&0xFFFFFFFFF;
				if (errorMarker11 & (1<<(15-i))) output[i] = ~output[i]&0xFFFFFFFFF;
			}
		}
	}
	
	
	if (size3 == 0)
	{
		for (int i = 23; i>=16; i--)
		{
			crc = 0xFF;
			output[i] = 0;
			for (int k=6; k >= 0; k--)	Update_CRC8((output[i] >> 4*k) & 0x0F, &crc, 4);
			output[i] = (output[i]+((unsigned long long)crc<<28));
		}
	}
	else
	{
		for (int i = 23; i>=16; i--)
		{
			if (size3>0)
			{
				crc = 0xFF;
				output[i] = table12array[cnt3]& 0x3FFF;
				output[i]<<=14;
				output[i] += table12array[cnt3-1] & 0x3FFF;
				for (int k=6; k >= 0; k--)
				{
					Update_CRC8((output[i] >> 4*k) & 0x0F, &crc, 4);
				}
				output[i] = (output[i]+((unsigned long long)crc<<28));
				if (errorMarker12 & (1<<(23-i))) output[i] = ~output[i]&0xFFFFFFFFF;
				cnt3-=2;
				size3--;
			}
			else
			{
				unsigned short size3 = readfromaddr (Page12FlashMemory+16);
				output[i] = output[24-(size3/4)];
				if (errorMarker12 & (1<<(size3/4))) output[i] = ~output[i]&0xFFFFFFFFF;
				if (errorMarker12 & (1<<(23-i))) output[i] = ~output[i]&0xFFFFFFFFF;
			}
		}
	}
	unsigned int crc4;
	for (int i = 24; i < 28; i++)
	{
		crc4 = 0xFF;
		output[i] = table13array[cnt4];
		for (int k=3; k >= 0; k--)
		{
			Update_CRC8((output[i] >> 4*k) & 0x0F,(unsigned char*)&crc4, 4);
		}
		output[i] = (output[i] +(crc4<<16));
		if (errorMarker13 & (1<<(i-24))) output[i] = ~output[i]&0xFFFFFF;
		cnt4++;
	}
	
	//============================================================================
	//form quarters
	unsigned char cntQua1 = 0;
	unsigned char cntQua2 = 0;
	int k = 0;
	//unsigned char tempQua;
	for (int i=0; i<(9*8*3); i++)
	{
		quarters[i]= (output[k] >> cntQua1)&0xF;
		cntQua1+=4;
		if (cntQua1 == (9*4))
		{
			k++;
			cntQua1 = 0;
		} 
	}
	for (int i=(9*8*3); i<(9*8*3)+6*4; i++)
	{
		quarters[i] = (output[k] >> cntQua2)&0xF;
		cntQua2+=4;
		if (cntQua2 == (6*4))
		{
			k++;
			cntQua2 = 0;
		} 
	}
}



unsigned short TableCRC16(unsigned char * pcBlock, unsigned short len)
{
	unsigned short crc = 0xFFFF;
	
	while (len--)
	crc = (crc << 8) ^ Crc16Table[(crc >> 8) ^ *pcBlock++]; // ref to the first element of array and so on
	
	return crc;
}
//=======================================================================================================================
void StartTx() // function to transmit nac/ack
{
	static unsigned char out_buf_tx[BUFFER_SIZE] = {0};
	unsigned char out_buf_ptr_tx = 1;

	unsigned short crc = TableCRC16(out_buf, out_buf_lng);
	out_buf[out_buf_lng++] = crc & 0xFF;
	out_buf[out_buf_lng++] = (crc >>8) & 0xFF;

	out_buf_tx[0] = _END;
	for ( int i = 0; i < out_buf_lng; i++)
	{
		if (out_buf[i] == _END)
		{
			out_buf_tx[out_buf_ptr_tx++] = _ESC;
			out_buf_tx[out_buf_ptr_tx++] = _ESC_END;
		}
		else if (out_buf[i] == _ESC)
		{
			out_buf_tx[out_buf_ptr_tx++] = _ESC;
			out_buf_tx[out_buf_ptr_tx++] = _ESC_ESC;
		}	
		else
		out_buf_tx[out_buf_ptr_tx++] = out_buf[i];
	
	}

	out_buf_tx[out_buf_ptr_tx++] = _END;
	if (usbFlag)
	{
		if (!(UDP->UDP_CSR[2] & UDP_CSR_TXPKTRDY))
		{
			for (int i = 0; i < out_buf_ptr_tx; i++)
			{
				if ( !((i+1) % 64) || (i == (out_buf_ptr_tx - 1)) )
				{
					UDP -> UDP_FDR[2] = out_buf_tx[i];
					UDP->UDP_CSR[2] |= UDP_CSR_TXPKTRDY;
					while(!(UDP->UDP_CSR[2] & UDP_CSR_TXPKTRDY));
					//wait host to read data
					while (!(UDP->UDP_CSR[2] & UDP_CSR_TXCOMP));
					UDP->UDP_CSR[2] &= ~(UDP_CSR_TXCOMP);
					while((UDP->UDP_CSR[2] & UDP_CSR_TXCOMP) != 0);
				}
				else
				{
					UDP -> UDP_FDR[2] = out_buf_tx[i];
				}
			}
		usbFlag = 0;
		}
		if (UDP -> UDP_CSR[2] & UDP_CSR_TXCOMP) clrflagCSR(2,UDP_CSR_TXCOMP);
	}
	if (usartFlag)
	{
		if (!(USART0->US_CSR & US_IER_TXRDY))return; //if (!(AT91C_BASE_US1->US_CSR & US_CSR_TXRDY))return;

		USART0->US_PTCR = US_PTCR_TXTDIS; //pdc transfer request?
		USART0->US_TCR = (unsigned int)(out_buf_ptr_tx);
		USART0->US_TPR = (unsigned int)(out_buf_tx); //The pointer registers (US_TPR and US_RPR) are used to store the address of the buffers.
		USART0->US_PTCR = US_PTCR_TXTEN; //PDC_TXTEN trans control reg
		usartFlag = 0;
	}
	
}	
void DispIn()
{
	unsigned short get_put_address;
	unsigned long  read_write_address;
	unsigned short crc;
	MEMMOVE_2(&crc, in_buf + (in_buf_ptr-2));
	if (crc != TableCRC16(in_buf, in_buf_ptr-2)) return;
	out_buf[0] = in_buf[1];
  	out_buf[1] = SELFADDRESS; //Settings.SelfAddress;
  	out_buf_lng = 3;
  	in_buf_ptr -= 2;

	switch (in_buf[2] & 0x3F)
	{
		case _PING:
		{
			out_buf[2] = _ACK | 0x40;
			break;
		}

		case _INIT:
		{
			out_buf[2] = _ACK;
			break;
		}

		case _ID:
		{
			out_buf[2] = _ACK;
			for(int k=0 ; ; k++) 
			{
				out_buf[k+3] = ID_String[k]; 
				if (ID_String[k] == '\0')
				{
					out_buf_lng = k+4;
					break;
				}
			}
		break;
		}

    	case _IDN:
		{
      		out_buf[2] = _ACK;
 			for(int k=0 ; ; k++)
 			{
	 			out_buf[k+3] = IDN_String[k];
	 			if (ID_String[k] == '\0')
	 			{
		 			out_buf_lng = k+4;
		 			break;
	 			}
 			}
      		out_buf_lng = 28;
      			break;
		}

    	case _WRITE:
		{

		unsigned char indata [32] = {0};		
		int size; 	
		out_buf[2] = _ACK;
		out_buf_lng = 3;
		MEMMOVE_4(&read_write_address, &in_buf[3]);  //memmove(&read_write_address, &in_buf[3], 4);
		void* newdata = &in_buf[7];	
		size = in_buf_ptr-7; //number of bytes we got
		unsigned int* PageFlashMemory = 0;
		
		switch (read_write_address) // first byte of data 
			{
			case 0:
				out_buf[2] = _ACK; 
               	out_buf_lng = 3;
               	out_buf[1] = SELFADDRESS; //Settings.SelfAddress;
               	break;

			case 10: // First table 32 bytes
				{
					crc = 0xFF;
					unsigned char fulldata[40];
					if ((size > 32) || (size % 4)) break;
					for (int i = 0; i < size; i++) // 0 - 32+0
					*((unsigned char*)indata + i) = *((unsigned char*)newdata + i);				
					for (int i = 0; i < 32; i++)
					{
						if ( i<(size) ) fulldata[i] = indata[i]; //num of lines
						else fulldata[i] = 0;
					}
					((unsigned int*)fulldata)[8] = size;
					for (int i = 0; i < 32; i++)  Update_CRC8((fulldata[i]) & 0xFF,(unsigned char*)&crc, 8);
					((unsigned int*)fulldata)[9] = crc;
					PageFlashMemory = (unsigned int*)(0x400000 + 0x40000 - 0x400);
					writetoflashpage(fulldata, PageFlashMemory,10);//32bytes max / 4 
				}				
				break;
			case 11: // Second table 32 bytes
				{
					crc = 0xFF;
					unsigned char fulldata[40];
					if ((size > 32) || (size % 4)) break;
					for (int i = 0; i < size; i++) // 0 - 32+0
					*((unsigned char*)indata + i) = *((unsigned char*)newdata + i);
					for (int i = 0; i < 32; i++)
					{
						if ( i<(size) ) fulldata[i] = indata[i]; //num of lines
						else fulldata[i] = 0;
					}
					((unsigned int*)fulldata)[8] = size;
					for (int i = 0; i < 32; i++)  Update_CRC8((fulldata[i]) & 0xFF,(unsigned char*)&crc, 8);
					((unsigned int*)fulldata)[9] = crc;
					PageFlashMemory = (unsigned int*)(0x400000 + 0x40000 - 0x300);
					writetoflashpage(fulldata, PageFlashMemory,10);//32bytes max / 4
				}
				break;		
								
			case 12: // Third table 32  bytes
				{
					crc = 0xFF;
					unsigned char fulldata[40];
					if ((size > 32) || (size % 4)) break;
					for (int i = 0; i < size; i++) // 0 - 32+0
					*((unsigned char*)indata + i) = *((unsigned char*)newdata + i);
					for (int i = 0; i < 32; i++)
					{
						if ( i<(size) ) fulldata[i] = indata[i]; //num of lines
						else fulldata[i] = 0;
					}
					((unsigned int*)fulldata)[8] = size;
					for (int i = 0; i < 32; i++)  Update_CRC8((fulldata[i]) & 0xFF,(unsigned char*)&crc, 8);
					((unsigned int*)fulldata)[9] = crc;
					PageFlashMemory = (unsigned int*)(0x400000 + 0x40000 - 0x200);
					writetoflashpage(fulldata, PageFlashMemory,10);//32bytes max / 4
				}
				break;
				

				
			default:
				out_buf[2] = _NAK;
				out_buf_lng = 3;
				break;
			}
		makeTable();
		break;
		
		}
		
		case _GET:
		{
			out_buf[2] = _ACK;
			int i = 3; 
			int j = 3;


			while (in_buf_ptr > i)
			{
				get_put_address = in_buf[i+1]; get_put_address <<= 8; get_put_address |= in_buf[i];
				//if (get_put_address == 4)
				if (get_put_address > 7) {out_buf[2] = _NAK; out_buf_lng = 3; StartTx(); return;} //four numbers + errmaker
				if (get_put_address < 2)
				{
					*((int*)&out_buf[j]) = (short)table13array[get_put_address];
				}
				else *((unsigned int*)&out_buf[j]) = (unsigned short)table13array[get_put_address];
				if (get_put_address == 4)  *((unsigned char*)&out_buf[j]) =  (unsigned char)errorMarker10;
				if (get_put_address == 5)  *((unsigned char*)&out_buf[j]) =  (unsigned char)errorMarker11;
				if (get_put_address == 6)  *((unsigned char*)&out_buf[j]) =  (unsigned char)errorMarker12;
				if (get_put_address == 7)  *((unsigned char*)&out_buf[j]) =  (unsigned char)errorMarker13;
				//if (Variables[get_put_address].ReadAddress > -1) readfromflashshort(Variables[get_put_address].ReadAddress);
				//MEMMOVE_2(&out_buf[j],&table13array[get_put_address]);
				//out_buf[j+2] = 0;
				//out_buf[j+3] = 0;

				i += 2;
				j += 4;


			out_buf_lng +=4;
			}
			if (out_buf_lng == 3) out_buf[2] = _NAK;
		break;
		}
		
	case _PUT:
	{
		unsigned int* Page13FlashMemory = (unsigned int*)(0x400000 + 0x40000 - 0x100);
		out_buf[2] = _ACK;
	    int i = 3; 

	    while (in_buf_ptr > i) 
		{
		    get_put_address = in_buf[i+1]; get_put_address <<= 8; get_put_address |= in_buf[i];
		    if (get_put_address == 4) MEMMOVE_4(&errorMarker10,&in_buf[i + 2]);
		    if (get_put_address == 5) MEMMOVE_4(&errorMarker11,&in_buf[i + 2]);
		    if (get_put_address == 6) MEMMOVE_4(&errorMarker12,&in_buf[i + 2]);
		    if (get_put_address == 7) MEMMOVE_4(&errorMarker13,&in_buf[i + 2]);
			if (get_put_address > 7) {out_buf[2] = _NAK; out_buf_lng = 3; StartTx(); return;}
			MEMMOVE_2(&table13array[get_put_address],&in_buf[i + 2]);
			i += 6; // 2 байта для адреса 4 для значения
		}
        writetoflashpage((unsigned char*)table13array, Page13FlashMemory, 2);
        makeTable();
		break;
	}	
	  		
		case _READ: //send what we wrote in _write case
		{
			unsigned char readarray [32] = {0};
			out_buf[2] = _ACK;	
			void* outdata = &out_buf[3];//data to transmit
			unsigned char* PageFlashMemory;
			MEMMOVE_4(&read_write_address, &in_buf[3]);  //memmove(&read_write_address, &in_buf[3], 4);
			switch (read_write_address) 
			{
	       		case 10://answer is I table
				{
					crc = 0xFF;				
					PageFlashMemory = (unsigned char*)(0x400000 + 0x40000 - 0x400);
					unsigned char size1 = readfromaddrchar (PageFlashMemory+32);
					unsigned char currentCRC1 = readfromaddrchar (PageFlashMemory+36);			
					readfromflashchar(readarray, PageFlashMemory,size1+1);
 					for (int i = 0; i < 32; i++)  Update_CRC8((readarray[i]) & 0xFF,(unsigned char*)&crc, 8);
					if ( (currentCRC1 & 0xFF) == (crc & 0xFF) )
					{
						out_buf_lng = size1+2+1;//+addr +ACK
						for (int i = 0; i < size1; i++) *((unsigned char *)outdata+i) = *((unsigned char*)readarray+i);
					}
					else out_buf_lng = 3;
					break;
				}
				case 11://answer is II table
				{
					crc = 0xFF;				
					PageFlashMemory = (unsigned char*)(0x400000 + 0x40000 - 0x300);
					unsigned char size2 = readfromaddrchar (PageFlashMemory+32);
					unsigned char currentCRC2 = readfromaddrchar (PageFlashMemory+36);			
					readfromflashchar(readarray, PageFlashMemory,size2+1);
 					for (int i = 0; i < 32; i++)  Update_CRC8((readarray[i]) & 0xFF,(unsigned char*)&crc, 8);
					if ( (currentCRC2 & 0xFF) == (crc & 0xFF) )
					{
						out_buf_lng = size2+2+1;//+addr +ACK
						for (int i = 0; i < size2; i++) *((unsigned char *)outdata+i) = *((unsigned char*)readarray+i);
					}
					else out_buf_lng = 3;
					break;				
				}
				case 12://answer is III table
				{
					crc = 0xFF;				
					PageFlashMemory = (unsigned char*)(0x400000 + 0x40000 - 0x200);
					unsigned char size3 = readfromaddrchar (PageFlashMemory+32);
					unsigned char currentCRC3 = readfromaddrchar (PageFlashMemory+36);			
					readfromflashchar(readarray, PageFlashMemory,size3+1);
 					for (int i = 0; i < 32; i++)  Update_CRC8((readarray[i]) & 0xFF,(unsigned char*)&crc, 8);
					if ( (currentCRC3 & 0xFF) == (crc & 0xFF) )
					{
						out_buf_lng = size3+2+1;//+addr +ACK
						for (int i = 0; i < size3; i++) *((unsigned char *)outdata+i) = *((unsigned char*)readarray+i);
					}
					else out_buf_lng = 3;
					break;
				}
				case 13://answer is IV table
				{
					
					makeTable();
					//unsigned char size4 = readfromaddrchar (PageFlashMemory+8);
					out_buf_lng = 240+3;
					for (int i = 0; i < 240; i++) *((unsigned char *)outdata+i) = *((unsigned char*)quarters+i);
					break;
				}		

        		default:
				{	
					out_buf[2] = _NAK;
            		out_buf_lng = 3;
        		}
            	break; 
			}
		}	
	}	
StartTx();
}
//=======================================================================================================================

void ep1_Handler(unsigned char in_array[], unsigned char size) //nvic sam visivaet
{
	usbFlag = 1;
	static unsigned char PrevByte = 0;
	in_buf_ptr = 0;// -2 end

	for (int i = 0; i<size; i++)
	{
		if (in_array[i] == _END)
		{
			if ( in_buf_ptr > 4 )
			{
				if( (in_buf[0] == SELFADDRESS) || ( (in_buf[0] == 0) && (in_buf[2] == _WRITE) ) )
				{
					if((in_buf[2] != _ACK) && (in_buf[2] != _NAK)) DispIn();
				}
				// max size of mess is 4 bytes. If we got any data mess...
			}
			in_buf_ptr = 0;
		}
		else
		{
			if (in_array[i] == _ESC) PrevByte = _ESC;
			else
			{
				if (PrevByte == _ESC)
				{
					if (in_array[i] == _ESC_END) in_array[i] = _END;
					if (in_array[i] == _ESC_ESC) in_array[i] = _ESC;
				}
				PrevByte = 0;
				
				if (in_buf_ptr < BUFFER_SIZE)
				{
					in_buf[in_buf_ptr] = in_array[i];
					in_buf_ptr++;
				}
				else in_buf_ptr = 0;
			}
		}
	}		
}	

void USART0_Handler() //nvic sam visivaet
{
	usartFlag = 1;
	//если это прерывание при приеме символа  USART0, то обработать его
	unsigned char ReceiveByte;
	static unsigned char PrevByte = 0;
	
	if (USART0 -> US_CSR & US_CSR_RXRDY) //if uart feels smth
	{

		ReceiveByte = USART0 -> US_RHR & 0xFF; //write this to char
		switch (ReceiveByte)
		{
			case _END:

			if ( in_buf_ptr > 4 )
			{
				if( (in_buf[0] == SELFADDRESS) || ( (in_buf[0] == 0) && (in_buf[2] == _WRITE) ) )
				{
					if((in_buf[2] != _ACK) && (in_buf[2] != _NAK)) DispIn();
				}
				// max size of mess is 4 bytes. If we got any data mess...
			}
			in_buf_ptr = 0;
			break;
			
			case _ESC: PrevByte = _ESC; //remember esc
			break;
			
			default:
			if (PrevByte == _ESC)
			{
				switch (ReceiveByte)
				{
					case _ESC_END:{ReceiveByte = _END; break;}
					case _ESC_ESC:{ReceiveByte = _ESC; break;}
				}
			}
			
			PrevByte = 0;
			
			if (in_buf_ptr < BUFFER_SIZE)
			{
				in_buf[in_buf_ptr] = ReceiveByte;
				in_buf_ptr++;
			}
			else in_buf_ptr = 0;
			break;
			
		}
	}
}