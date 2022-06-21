#include<reg51.h>
#include<iic.h>
#include<onewire.h>

#define uchar unsigned char
#define uint unsigned int
	

sbit DQ = P1^4; 

//总线引脚定义
sbit SDA = P2^1;  /* 数据线 */
sbit SCL = P2^0;  /* 时钟线 */

sbit R1=P3^0;//第一排初始化（如果跳线帽模式是BTN，则只有最左侧一列能用）
sbit R2=P3^1;//第二排初始化
sbit R3=P3^2;//第三排初始化
sbit R4=P3^3;//第四排初始化

sbit C1=P4^4;//第一列初始化
sbit C2=P4^2;//第二列初始化
sbit C3=P3^5;//第三列初始化
sbit C4=P3^4;//第四列初始化

//////////////////////////// 0    1     2    3    4    5    6   7    8    9    -   全灭   .
unsigned char code shuzi[]={0XC0,0XF9,0XA4,0XB0,0X99,0X92,0X82,0XF8,0X80,0X90,0xbf,0xff,0x7f};
/////////////////////////// A     b   C     d    E    F    H   L     P    U   n
unsigned char code zimu[]={0x88,0x83,0xc6,0xa1,0x86,0x8e,0x89,0xc7,0x8c,0xc1,0xc8};
////////////////////////////  0     1     2    3    4    5    6    7   
unsigned char code weizhi[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x00,0x00,0x00,0x00,0x00,0x00};
//////////////////////////   1    2   3    4    5    6    7    8    灭
unsigned char ledweizhi[]={0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f,0xff};

//温度变量定义
int temp;
uchar shi;
uchar ge;
uchar shi_p;
uchar ge_p;
int temp2;
//参数设置
uchar temp_cs=25;
//s5模式切换数值
uchar mode_dac=0;
//dac的输出数值
int voltage;
int dat;


void Scan_S8();
void Scan_S9();
void Scan_S5();
void judge_dac();
//////延时1ms
void delay_ms(int ms)
{
 int q,w;
 for(q = 0;q<ms;q++){
  for(w=845;w>0;w--);
 }
}
///数码管数字显示
void shumaguan_shuzi(uchar a,uchar b)
{
	delay_ms(1);
	P2=(P2&0X1f)|0xC0;P0=weizhi[a];
	P2=(P2&0X1f)|0xE0;P0=shuzi[b];
	delay_ms(1);
	P2 = (P2 & 0x1f) | 0xc0;
	P0 = 0x00;
	P2 = P2 & 0x1f;
}
//数码管字母显示
void shumaguan_zimu(uchar a,uchar b)
{
	delay_ms(1);
	P2=(P2&0X1f)|0xC0;P0=weizhi[a];
	P2=(P2&0X1f)|0xE0;P0=zimu[b];
	delay_ms(1);
	P2 = (P2 & 0x1f) | 0xc0;
	P0 = 0x00;
	P2 = P2 & 0x1f;
}
void ledlight(uchar x)
{
	P2=(P2&0X1f)|0x80;
	P0=ledweizhi[x];
}
void Init_Keys()
{
	R1=R2=R3=R4=1;
	C1=C2=C3=C4=1;
}

//DS18B20的配置
void temperature_get()
{
	uchar high,low;
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);

	
	
  init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	low=Read_DS18B20();
	high=Read_DS18B20();
	
	
	temp=((high<<4)|(low>>4));//不带小数点，注掉后面一行就行
	temp2=low&0x0f*100/16;//取小数点两位,注掉上面一行就行
	shi=temp%100/10;
	ge=temp%10;
	shi_p=temp2/10;
	ge_p=temp%10;
}
void pcf8591_dac(char dat)//dac读取数值
{
 IIC_Start(); 
 IIC_SendByte(0x90);
 IIC_WaitAck();

 IIC_SendByte(0x40);
 IIC_WaitAck();

 IIC_SendByte(dat);
 IIC_WaitAck();
 IIC_Stop();
	
}

void display_temp()
{
	shumaguan_zimu(0,2);
	
	shumaguan_shuzi(4,shi);
	shumaguan_shuzi(5,ge);
	shumaguan_shuzi(6,shi_p);
	shumaguan_shuzi(5,12);
	shumaguan_shuzi(7,ge_p);
	ledlight(1);
}
void display_cs()
{
	ledlight(2);
	shumaguan_zimu(0,8);
	
	shumaguan_shuzi(6,temp_cs/10);
	shumaguan_shuzi(7,temp_cs%10);
	Scan_S9();
	Scan_S8();
}
void display_dacnum()
{
	judge_dac();
	if(mode_dac==0)
	{
	shumaguan_zimu(0,0);
	shumaguan_shuzi(5,voltage);
	shumaguan_shuzi(5,12);
	shumaguan_shuzi(6,0);
	shumaguan_shuzi(7,0);
	P2=0x80;
	P0=0xf6;
	}
	if(mode_dac==1)
	{
	shumaguan_zimu(0,0);
	shumaguan_shuzi(5,voltage/100);
	shumaguan_shuzi(5,12);
	shumaguan_shuzi(6,voltage%100/10);
	shumaguan_shuzi(7,voltage%10);
	ledlight(3);
	}

	
}
void judge_dac()
{
	if(mode_dac==0)
	{
		temperature_get();
		if(temp>temp_cs)
		{
			voltage=5;
			pcf8591_dac(0xff);
		}
		if(temp<temp_cs)
		{
			voltage=0;
			pcf8591_dac(0x00);
		}
	}
	if(mode_dac==1)
	{
		temperature_get();
		if(temp<=20)
		{
			voltage=1;
		}
		if(temp>20&&temp<40)
		{
			voltage=0.15*temp*100;
			if(voltage>=400) voltage=400;
			if(voltage<=200) voltage=200;
		}
		if(temp>=40)
		{
			voltage=4;
		}
	}
}
void Scan_S4()
{
	R4=0;R3=1;
	if(C1==0)
	{
		
		while(C1==0)display_temp();
		while(1)
		{
			judge_dac();
			display_cs();
			R4=0;R3=1;
			if(C1==0)
			{
				while(C1==0)display_dacnum();
				while(1)
				{
					
						Scan_S5();
						R4=0;R3=1;
					if(C1==0)
					{
						while(C1==0) display_temp();
						while(1)
						{
						ledlight(3);
						temperature_get();
						judge_dac();
						display_temp();
						Scan_S4();
						}
					}
				}
			}
		}
	}
	
}

void Scan_S8()
{
	R4=0;R3=1;
	if(C2==0&&R4==0)
	{
		while(C2==0&&R4==0);
		temp_cs--;
	}
}
void Scan_S9()
{
	R3=0;R4=1;
	if(C2==0&&R3==0)
	{
		while(C2==0&&R3==0);
		temp_cs++;
	}
}
void Scan_S5()
{
	uchar temp=4;
	
	R3=0;R4=1;
	if(C1==0&&R3==0)
	{
			while(C1==0&&R3==0);
			temp++;
	}
	if(temp%2==0)
	{
		mode_dac=0;
		judge_dac();
		display_dacnum();
	}
	if(temp%2==1)
	{
		while(1)
		{
			mode_dac=1;
			judge_dac();
			display_dacnum();
			if(C1==0&&R3==0)
			{
					while(C1==0&&R3==0);
					break;
			}
		}
	}
}

void main()
{
	while(1)
	{
		Init_Keys();
		Scan_S4();
		judge_dac();
		temperature_get();
		display_temp();

	}
}