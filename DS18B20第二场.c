#include <STC15F2K60S2.H>
#include "iic.h"
#include "onewire.h"


unsigned char duanma[18] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x80,0xc6,0xc0,0x86,0x8e,0xbf,0x7f};
unsigned Temp;
unsigned char F_SMG=1;
unsigned char hl=0;
unsigned char count=0;
unsigned char K5=0;
unsigned char T_min=20;
unsigned char T_max=30;
unsigned int datt;
unsigned char stat_Led=0xff;
unsigned char v;


void Init138(unsigned char n,unsigned char dat)
{
	   P0=dat;
	   P2=(P2&0x1f)|0x00;
	   switch(n)
	   {
	   		case 4:P2=(P2&0x1f)|0x80; break;
			case 5:P2=(P2&0x1f)|0xa0; break;
			case 6:P2=(P2&0x1f)|0xc0; break;
			case 7:P2=(P2&0x1f)|0xe0; break;
	   }
	   P2=(P2&0x1f)|0x00;
}

void DelaySMG(unsigned int i)
{
	while(i--);
}

void InitSMG(unsigned char wei,unsigned char duan)
{
	Init138(6,0x01<<wei);
	Init138(7,duan);
	DelaySMG(500);
	Init138(6,0x01<<wei);
	Init138(7,0xff);
}

void ALLSMG()
{
	Init138(6,0xff);
	Init138(7,0xff);
}

void Display()
{
	switch(F_SMG)
	{
		case 1:
			   InitSMG(7,duanma[Temp%10]);
			   InitSMG(6,duanma[Temp/10]);
			   InitSMG(0,0xc6);
			   ALLSMG();
		break;
		case 2:
			if(hl==0&K5==0|K5==1)
			{
			   InitSMG(7,duanma[T_min%10]);
			   InitSMG(6,duanma[T_min/10]);
			 }
			 if(hl==0&K5==1|K5==0)
			 {  
			   InitSMG(4,duanma[T_max%10]);
			   InitSMG(3,duanma[T_max/10]);
			   }
			  InitSMG(0,0x8c);
			   ALLSMG();
		break;
		case 3:
			   InitSMG(7,duanma[(datt*5)/255]);
			   InitSMG(6,duanma[(datt*5)/255]);
			   InitSMG(0,0xc1);
		break;	
	}
}

void InitTime0()
{
	TH0=(65535-10000)%256;
	TL0=(65535-10000)/256;
	ET0=1;
	EA=1;
	TR0=1;
}
void SerTime0()interrupt 1
{
	TH0=(65535-10000)%256;
	TL0=(65535-10000)/256;
	   count++;
	 if(count==100)
	 {
	 count=0;
	 	if(hl==0)
		{
			hl=1;
		}
		else if(hl==1)
		{
			hl=0;
		}
	 }

}


void Read_Ds18()
{
	unsigned H,L;
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	DelaySMG(1500);

	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	L=Read_DS18B20();
	H=Read_DS18B20();
	H=H<<8|L;
	if((H&0x8000)==0x0000)
	{
		Temp=H>>4;
	 }
}


void Writ_PCF(unsigned char dat)
{
	 IIC_Start();
	 IIC_SendByte(0x90);
	 IIC_WaitAck();
	 IIC_SendByte(0x40);
	 IIC_WaitAck();
	 IIC_SendByte(dat);
	 IIC_WaitAck();
	 IIC_SendAck(0);
	 IIC_Stop();
}

void Read_PCF()
{
	  IIC_Start();
	 IIC_SendByte(0x90);
	 IIC_WaitAck();
	 IIC_SendByte(0x43);
	 IIC_WaitAck();
	 IIC_Stop();

	 Display();

	 IIC_Start();
	 IIC_SendByte(0x91);
	 IIC_WaitAck();
	 datt=IIC_RecByte();
	 IIC_WaitAck();
	 IIC_SendAck(0);
	 IIC_Stop();
}

void Led()
{
	 if(Temp>T_max)
	 {
	 	  stat_Led=0xff;
	 	   stat_Led&=~0x01;
		   v=4;
		   Writ_PCF(4*51);
		  Init138(4,stat_Led);
	 }
	 else if((T_min<=Temp)&(Temp<=T_max))
	 {
	 	stat_Led=0xff;
		v=3;
		Writ_PCF(3*51);
		stat_Led&=~0x02;
		Init138(4,stat_Led);		
	 }
	 else if(Temp<T_min)
	 {
	 	 stat_Led=0xff;stat_Led&=~0x04;
		 v=2;
		 Writ_PCF(2*51);
		 
		 Init138(4,stat_Led);
	 }
	 else 
	 {
	 	 stat_Led=0xff;
		 stat_Led&=~0x80;
		 Init138(4,stat_Led);
	 }
}

void KEY()
{
	 if(P33==0)
	 {
	 	DelaySMG(200);
		if(P33==0)
		{
			if(F_SMG==1)
			{
				F_SMG=2;
			}
			else if(F_SMG==2)
			{
				F_SMG=3;
			}
			else
			{
				F_SMG=1;
			}
		while(P33==0)
		{

		}
		}
	 }

	 if(P32==0)
	 {
	 	DelaySMG(200);
		if(P32==0)
		{
			if(K5==0)
			{
				K5=1;
			}
			else if(K5==1)
			{
				K5=0;
			}
			while(P32==0)
			{
	
			}
		}
	 }

	 if(P31==0)
	 {
	 	DelaySMG(200);
		if(P31==0)
		{
			if(K5==1)
			{
				 T_max++;
				 if(T_max>99)
				 {
				 	T_max=30;
				 }
			}
			else if(K5==0)
			{
				T_min++;
				if(T_max<T_min)
				{
					T_min=20 ;
				}
			}
			while(P31==0)
			{
		
			}
		}
	 }


	 if(P30==0)
	 {
	 	DelaySMG(200);
		if(P30==0)
		{
			if(K5==1)
			{
				 T_max--;
				 if(T_max<T_min)
				 {
				 	T_max=30;
				 }
			}
			else if(K5==0)
			{
				T_min--;
				if(T_min<0)
				{
					T_min=20 ;
				}
			}
			while(P30==0)
			{
		
			}
		}
	 }
}


void main()
{
ALLSMG();
InitTime0();
	while(1)
	{
		Read_Ds18();
		Led();
		Read_PCF();
		
		Display();
		KEY();
	}
}					