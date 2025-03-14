#include <reg52.h>
typedef unsigned char uchar;
//ADC���ֶ���------------------------------------------
sbit START=P3^0;  //STARTΪ0ʱ��ʼת��
sbit EOC=P3^1; //Ϊ1 ʱ��������ת�����
sbit OE=P3^7;  //Ϊ1ʱ������ȡ����

#define ADC0809_Data P0//����˿ڽ���ADC0809����ת�����

//�������ʾ����--------------------------------------- 
sbit DIN=P1^0;          //MAX7219��������       1��
sbit LOAD=P1^1;          //MAX7219Ƭѡ           12��
sbit CLK=P1^2;           //MAX7219����ʱ��        13��

//�Ĵ����궨��
 
#define DECODE_MODE  0x09   //������ƼĴ���
 
#define INTENSITY    0x0A   //���ȿ��ƼĴ���
 
#define SCAN_LIMIT   0x0B   //ɨ����޼Ĵ���
 
#define SHUT_DOWN    0x0C   //�ض�ģʽ�Ĵ���
 
#define DISPLAY_TEST 0x0F   //���Կ��ƼĴ��� 
#define PWM_PERIOD 254	//����PWM����


 //	�������ת-----------------------------------------
sbit shun = P3^5;
sbit ni = P3^6;
sbit shun_control = P1^6;
sbit ni_control = P1^7;			//��ʼΪ�ߵ�ƽ
sbit fast = P3^2;
sbit slow = P3^3;
uchar speed1 = 0;	//���������������ٶ�
uchar speed2 = 0;	//���������ٶ�
uchar speed = 0;	//����������ٶ�

uchar counter;		  //����ֵ��ռ�ձ�
uchar direction;	 //���ת����� 0��˳ʱ�� 1����ʱ��

//led buzzer������ʾ����------------------------------------
sbit led = P1^4;
sbit buzzer = P1^3;
uchar  alam = 0;

//��������--------------------------------------------------
void Timer0_Init();
//�������ʾ������������
 
void Write7219(unsigned char address,unsigned char dat);
void Initial(void);


//��ʱ����---------------------------------------------------

void delay_ms(uchar ms)
{
	uchar i,j;
	for(i=ms;i>0;i--)
	for(j=110;j>0;j--);
}


//adcת�����غ���------------------------------------------
uchar ADC0809_Read()
{
	uchar Temp=0X00;
	OE=0;//�����Ϊ����̬
	START=0;
	START=1;//�ṩ����������ADת��
	START=0;//�ṩ�½���
	while(!EOC);//�ȴ�ת������
	OE=1;//�������������
	Temp=ADC0809_Data;
	OE=0;//�Ͽ�������
	return Temp;//���ؽ��
}


//���������ٶȺ���--------------------------------------------------
uchar Final_Speed(uchar x1, uchar x2)
{
	uchar value = (x1 + x2);
	return value;
}



//��ʾ�ٶ�ת����---------------------------------------------------
void toShow(number)
{
	uchar hundreds =number/100;
	uchar tens = (number/10)%10;
	uchar units = number%10;
	Write7219(2,hundreds) ;
	Write7219(3,tens) ;
	Write7219(4,units) ;
	Write7219(1,direction) ;


}

//�������ʾ������������  ��ַ�����ݷ����ӳ���---------------------
void Write7219(uchar address,uchar dat)
{   
    uchar i;
    LOAD=0;    //����Ƭѡ�ߣ�ѡ������
 
    //���͵�ַ 
    for (i=0;i<8;i++)        //��λѭ��8��              
    {  
       CLK=0;        //����ʱ������ 
       DIN=(bit)(address&0x80); //ÿ��ȡ���ֽ�      
       address<<=1;             //����һλ 
       CLK=1;        //ʱ�������أ����͵�ַ
    }
 
    //�������� 
    for (i=0;i<8;i++)               
    {   
       CLK=0;
       DIN=(bit)(dat&0x80);    
       dat<<=1;  
       CLK=1;        //ʱ�������أ���������
    }
     LOAD=1;    //���ͽ�������������������                      
}

//������ƺ���-----------------------------------------------------------
void motor_control(uchar speedx)
{
	if	(ni_control == 1 && shun_control == 0)
	{
		 direction = 0;		 //��ʾ�����ת��˳ʱ��
	}
	else if(shun_control == 1 && ni_control == 0 )	
	{
	 	 direction = 1;		//��ʾ�����ת����ʱ��
	}
	else
	{
		direction = 9;		//��Чλ
	}

	switch(direction)
	{
	 	case 0:			//����˳ʱ��ת��
			{		  
				  if (counter < speedx )
				  {
				  	  	shun = 1;
						ni = 0;
				  }
				  else
				  {
				  	 	shun = 0;
						ni = 0;
				  }
			}
			break;
		case 1:					 //������ʱ��ת��
			 {
				  if (counter < speedx )
				  {
				  	  	shun = 0;
						ni = 1;
				  }
				  else
				  {
				  	 	shun = 0;
						ni = 0;
				  }
			 }
			 break;	
		case 9:					 //��Ч���أ�ʹ��ֹͣ
			 {
				  	 	shun = 0;
						ni = 0;				  
			 }
			 break;		  
	}

}

void Alaming()		//�������б�������-----------------
{
	if (speed >= 245 && speed < 252)	  //�������о�ʾ
	{
		led = 1;		//led���� ����
		buzzer = 1;
	}
	else if(speed >= 252)
	{
		led = 1;
		delay_ms(10); //��˸����
		led = 0;
		buzzer = 0;
		alam ++;		
	}
	else
	{alam = 0;
	 led = 0;
	 buzzer = 1;
	}
	if (alam > 100)
	   {
		  speed = 0;
		  shun = 1;
		  ni = 1;
		  shun_control = 0;
		  ni_control = 0;
	   }
}

//��ʼ������-----------------------------------------------------------
void interrurput_init()		//��ʼ���ⲿ�ж�
{
	EA = 1;
    EX0 = 1;
    IT0 = 1;
	EX1 = 1;
	IT1 = 1;
	PX1 = 1;
	PX0 = 1;

}


void Timer0_Init()	   //��ʼ����ʱ��0
{
	TMOD = 0X01;
	TH0 =0xff;	//12mhz����ʱ1ms
	TL0 =0xfc;
	ET0 = 1;
	TR0 = 1;
}

//MAX7219��ʼ��������MAX7219�ڲ��Ŀ��ƼĴ���
void Initial(void)            
 
{
 
    Write7219(SHUT_DOWN,0x01);         //������������ģʽ��0xX1��
 
    Write7219(DISPLAY_TEST,0x00);      //ѡ����ģʽ��0xX0��
 
    Write7219(DECODE_MODE,0xff);       //ѡ��ȫ����ģʽ
 
    Write7219(SCAN_LIMIT,0x03);        //3ֻLEDȫ��
 
    Write7219(INTENSITY,0x04);          //���ó�ʼ����     
 
}
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^




//������----------------------------------------------------------
int main()
{	
	/* ��ʼ���ж� */
	interrurput_init();
	Timer0_Init();		//��ʼ����ʱ��0
	Initial();	//��ʼ�������



    /* ���ٶȱ�����P2�����,ͨ��DAC0832ת����ģ���ѹ */
    while(1)
    {  	
		 		
		 speed1 = ADC0809_Read()/2;
		 speed =  Final_Speed(speed1,speed2);
		 motor_control(speed);
		 Alaming();
		 toShow(speed);
    			       
    }
}


//�жϺ�����--------------------------------------------------------------
//�����Ӽ��ٺ���--------------------
void FAST()interrupt 0
{	
	if	(direction != 9)   //������Ч���ܼӼ�
		{
			if (speed2 != 128) //speed����ٶ�������255
        		speed2 += 8;
		}	  	
}


void SLOW()interrupt 2
{	
	
		if (speed2 != 0) //speed����ٶ�������128
        speed2 -= 2;			 		   
}



void Timer0_ISR()interrupt 1
{
	   TR0=0;		//����ֵʱ���رն�ʱ��
       TH0=0xff;	//(65536-5)/256;//����ֵ��ʱ
       TL0=0xfb;	//(65536-5)%256;//0.01ms
	   counter++;

	if (counter >= PWM_PERIOD) //һ���������������
	{
		counter = 0;
	}
	TR0 = 1;
}


