#include <reg52.h>
typedef unsigned char uchar;
//ADC部分定义------------------------------------------
sbit START=P3^0;  //START为0时开始转换
sbit EOC=P3^1; //为1 时表明数据转换完成
sbit OE=P3^7;  //为1时读可以取数据

#define ADC0809_Data P0//定义端口接收ADC0809数据转换结果

//数码管显示部分--------------------------------------- 
sbit DIN=P1^0;          //MAX7219串行数据       1脚
sbit LOAD=P1^1;          //MAX7219片选           12脚
sbit CLK=P1^2;           //MAX7219串行时钟        13脚

//寄存器宏定义
 
#define DECODE_MODE  0x09   //译码控制寄存器
 
#define INTENSITY    0x0A   //亮度控制寄存器
 
#define SCAN_LIMIT   0x0B   //扫描界限寄存器
 
#define SHUT_DOWN    0x0C   //关断模式寄存器
 
#define DISPLAY_TEST 0x0F   //测试控制寄存器 
#define PWM_PERIOD 254	//定义PWM周期


 //	电机正反转-----------------------------------------
sbit shun = P3^5;
sbit ni = P3^6;
sbit shun_control = P1^6;
sbit ni_control = P1^7;			//初始为高电平
sbit fast = P3^2;
sbit slow = P3^3;
uchar speed1 = 0;	//滑动变阻器控制速度
uchar speed2 = 0;	//按键控制速度
uchar speed = 0;	//电机控制总速度

uchar counter;		  //计数值作占空比
uchar direction;	 //电机转向变量 0：顺时针 1：逆时针

//led buzzer报警显示功能------------------------------------
sbit led = P1^4;
sbit buzzer = P1^3;
uchar  alam = 0;

//函数声明--------------------------------------------------
void Timer0_Init();
//数码管显示函数函数声明
 
void Write7219(unsigned char address,unsigned char dat);
void Initial(void);


//延时函数---------------------------------------------------

void delay_ms(uchar ms)
{
	uchar i,j;
	for(i=ms;i>0;i--)
	for(j=110;j>0;j--);
}


//adc转换返回函数------------------------------------------
uchar ADC0809_Read()
{
	uchar Temp=0X00;
	OE=0;//输出设为高阻态
	START=0;
	START=1;//提供上升沿启动AD转换
	START=0;//提供下降沿
	while(!EOC);//等待转换结束
	OE=1;//连接数据线输出
	Temp=ADC0809_Data;
	OE=0;//断开数据线
	return Temp;//返回结果
}


//返回最终速度函数--------------------------------------------------
uchar Final_Speed(uchar x1, uchar x2)
{
	uchar value = (x1 + x2);
	return value;
}



//显示速度转向函数---------------------------------------------------
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

//数码管显示函数函数定义  地址、数据发送子程序---------------------
void Write7219(uchar address,uchar dat)
{   
    uchar i;
    LOAD=0;    //拉低片选线，选中器件
 
    //发送地址 
    for (i=0;i<8;i++)        //移位循环8次              
    {  
       CLK=0;        //清零时钟总线 
       DIN=(bit)(address&0x80); //每次取高字节      
       address<<=1;             //左移一位 
       CLK=1;        //时钟上升沿，发送地址
    }
 
    //发送数据 
    for (i=0;i<8;i++)               
    {   
       CLK=0;
       DIN=(bit)(dat&0x80);    
       dat<<=1;  
       CLK=1;        //时钟上升沿，发送数据
    }
     LOAD=1;    //发送结束，上升沿锁存数据                      
}

//电机控制函数-----------------------------------------------------------
void motor_control(uchar speedx)
{
	if	(ni_control == 1 && shun_control == 0)
	{
		 direction = 0;		 //表示电机正转即顺时针
	}
	else if(shun_control == 1 && ni_control == 0 )	
	{
	 	 direction = 1;		//表示电机反转即逆时针
	}
	else
	{
		direction = 9;		//无效位
	}

	switch(direction)
	{
	 	case 0:			//控制顺时针转速
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
		case 1:					 //控制逆时针转速
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
		case 9:					 //无效开关，使其停止
			 {
				  	 	shun = 0;
						ni = 0;				  
			 }
			 break;		  
	}

}

void Alaming()		//高速运行报警功能-----------------
{
	if (speed >= 245 && speed < 252)	  //高速运行警示
	{
		led = 1;		//led点亮 常亮
		buzzer = 1;
	}
	else if(speed >= 252)
	{
		led = 1;
		delay_ms(10); //闪烁报警
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

//初始化函数-----------------------------------------------------------
void interrurput_init()		//初始化外部中断
{
	EA = 1;
    EX0 = 1;
    IT0 = 1;
	EX1 = 1;
	IT1 = 1;
	PX1 = 1;
	PX0 = 1;

}


void Timer0_Init()	   //初始化定时器0
{
	TMOD = 0X01;
	TH0 =0xff;	//12mhz晶振定时1ms
	TL0 =0xfc;
	ET0 = 1;
	TR0 = 1;
}

//MAX7219初始化，设置MAX7219内部的控制寄存器
void Initial(void)            
 
{
 
    Write7219(SHUT_DOWN,0x01);         //开启正常工作模式（0xX1）
 
    Write7219(DISPLAY_TEST,0x00);      //选择工作模式（0xX0）
 
    Write7219(DECODE_MODE,0xff);       //选用全译码模式
 
    Write7219(SCAN_LIMIT,0x03);        //3只LED全用
 
    Write7219(INTENSITY,0x04);          //设置初始亮度     
 
}
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^




//主程序----------------------------------------------------------
int main()
{	
	/* 初始化中断 */
	interrurput_init();
	Timer0_Init();		//初始化定时器0
	Initial();	//初始化数码管



    /* 将速度变量从P2口输出,通过DAC0832转换成模拟电压 */
    while(1)
    {  	
		 		
		 speed1 = ADC0809_Read()/2;
		 speed =  Final_Speed(speed1,speed2);
		 motor_control(speed);
		 Alaming();
		 toShow(speed);
    			       
    }
}


//中断函数区--------------------------------------------------------------
//按键加减速函数--------------------
void FAST()interrupt 0
{	
	if	(direction != 9)   //开关有效才能加减
		{
			if (speed2 != 128) //speed最大速度限制在255
        		speed2 += 8;
		}	  	
}


void SLOW()interrupt 2
{	
	
		if (speed2 != 0) //speed最大速度限制在128
        speed2 -= 2;			 		   
}



void Timer0_ISR()interrupt 1
{
	   TR0=0;		//赋初值时，关闭定时器
       TH0=0xff;	//(65536-5)/256;//赋初值定时
       TL0=0xfb;	//(65536-5)%256;//0.01ms
	   counter++;

	if (counter >= PWM_PERIOD) //一个控制周期完更新
	{
		counter = 0;
	}
	TR0 = 1;
}


