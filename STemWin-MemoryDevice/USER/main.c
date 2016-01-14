#include "stm32f10x.h"
#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "ILI93xx.h"
#include "key.h"
#include "malloc.h" 
#include "usmart.h" 
#include "GUI.h"
#include "touch.h"
#include "includes.h"
#include "math.h"
#include "WM.h"
#include <string.h>
#include "GUI_Private.h"
#include "stdio.h"

//ALIENTEK Mini STM32开发板范例代码27
//内存管理实验  
//技术支持：www.openedv.com
//广州市星翼电子科技有限公司 
extern void _MY_GetTouchPos(void);
	static const GUI_POINT aPoints[] = {   
	{-50,   0},
	{-10, 10},
	{   0, 50},
	{ 10, 10},
	{ 50,   0},
	{ 10,-10},
	{   0,-50},
	{-10,-10}
	};
	typedef struct 
	{
	int XPos_Poly;
	int YPos_Poly;
	int XPos_Text;
	int YPos_Text;
	GUI_POINT aPointsDest[8];
	} tDrawItContext;
 #define SIZE_OF_ARRAY(Array) (sizeof(Array) / sizeof(Array[0]))
/////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务
#define START_TASK_PRIO      		20        //开始任务的优先级设置为最低
#define START_STK_SIZE  				64        //设置任务堆栈大小
OS_STK START_TASK_STK[START_STK_SIZE];    //任务堆栈	
void start_task(void *pdata);	            //任务函数

#define LED_TASK_PRIO      			9        //开始任务的优先级设置为最低
#define LED_STK_SIZE  					64        //设置任务堆栈大小
OS_STK LED_TASK_STK[LED_STK_SIZE];    //任务堆栈	
void led_task(void *pdata);	            //任务函数

#define EMWIN_DEMO_TASK_PRIO    8        //开始任务的优先级设置为最低
#define EMWIN_STK_SIZE  				3096        //设置任务堆栈大小
OS_STK EMWIN_TASK_STK[EMWIN_STK_SIZE];    //任务堆栈	
void emwin_demo_task(void *pdata);	            //任务函数

#define TOUCH_TASK_PRIO      		10        //开始任务的优先级设置为最低
#define TOUCH_STK_SIZE  				64        //设置任务堆栈大小
OS_STK TOUCH_TASK_STK[TOUCH_STK_SIZE];    //任务堆栈	
void touch_task(void *pdata);	            //任务函数

void BSP_Init(void)
{
	NVIC_Configuration();	 
	delay_init();	    			 //延时函数初始化	  
	uart_init(115200);	 		//串口初始化为9600
	LED_Init();		  				//初始化与LED连接的硬件接口
	TFTLCD_Init();			   	//初始化LCD		 
	tp_dev.init();
//	tp_dev.adjust();
	mem_init();				//初始化内存池

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC,ENABLE);
  GUI_Init();
	
}
void main_ui(void)
{
#if 0
	 _MY_GetTouchPos();
#endif
	GUI_SetBkColor(GUI_BLACK);
	GUI_SetColor(GUI_WHITE);
	GUI_DispStringAt("Hello World!", 30, 200);
	GUI_DispStringAt("Hello emWin!", 30, 216);
	GUI_DrawRoundedRect(0,0,200,200,5);
	GUI_DrawRoundedFrame(2,2,180,20,5,2);
}	

void _DrawIt(void * pData) 
{
	tDrawItContext * pDrawItContext = (tDrawItContext *)pData;
	GUI_Clear();
	GUI_SetFont(&GUI_Font8x8);
	GUI_SetTextMode(GUI_TM_TRANS);

	GUI_SetColor(GUI_GREEN);
	GUI_FillRect(pDrawItContext->XPos_Text, 
	pDrawItContext->YPos_Text - 25,
	pDrawItContext->XPos_Text + 100,
	pDrawItContext->YPos_Text - 5);

	GUI_SetColor(GUI_BLUE);
	GUI_FillPolygon(pDrawItContext->aPointsDest, SIZE_OF_ARRAY(aPoints), 120, 160);

	GUI_SetColor(GUI_RED);
	GUI_FillRect(140 - pDrawItContext->XPos_Text,  pDrawItContext->YPos_Text + 105,140 - pDrawItContext->XPos_Text + 100,pDrawItContext->YPos_Text + 125);

}	
typedef void (*p) (void *p);
p p1;
void memdisplay(void)
{
	tDrawItContext DrawItContext;
	int i, swap=0;
	
	void (*pp)(void *p);
	
	GUI_RECT Rect = {0, 70, 240,320};
	pp = _DrawIt;
	p1 = _DrawIt;
	GUI_SetBkColor(GUI_BLACK);
	GUI_Clear();
	GUI_SetColor(GUI_YELLOW);
	GUI_SetFont(&GUI_Font24_ASCII);
	GUI_DispStringHCenterAt("MEMDEV_Banding", 120, 5);
	GUI_SetFont(&GUI_Font16_ASCII);
	GUI_DispStringHCenterAt("Banding memory device\nwithout flickering", 120, 40);
	DrawItContext.XPos_Poly = 120;
	DrawItContext.YPos_Poly = 160;
	DrawItContext.YPos_Text = 110;

	while (1) 
	{
		swap = ~swap;
		for (i = 0; i < 20; i++) 
		{
			float angle = i * 3.1415926 / 25;
			DrawItContext.XPos_Text = (swap) ? i*7 : 140 - i*7;

			GUI_RotatePolygon(DrawItContext.aPointsDest, aPoints, SIZE_OF_ARRAY(aPoints), (swap)?-angle:angle);
			
			GUI_MEMDEV_Draw(&Rect,pp,&DrawItContext,0,0);
		}
	}
}

int main(void)
{
	BSP_Init();
//	main_ui();
memdisplay();

	OSInit();
	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO);//创建起始任务
	OSStart();
}
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr = 0;

//	GUI_Delay(1000);
	OS_ENTER_CRITICAL();
	OSTaskCreate(emwin_demo_task,(void *)0,&EMWIN_TASK_STK[EMWIN_STK_SIZE-1],EMWIN_DEMO_TASK_PRIO);
	OSTaskCreate(touch_task,(void *)0,&TOUCH_TASK_STK[TOUCH_STK_SIZE-1],TOUCH_TASK_PRIO);
	OSTaskCreate(led_task,(void *)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);
	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();
}
void led_task(void *pdata)
{
	
	while(1)
	{
//		memdisplay();
		LED0 = !LED0;
		OSTimeDlyHMSM(0,0,0,500);
	}
}
void touch_task(void *pdata)
{
	while(1)
	{
		GUI_TOUCH_Exec();
		OSTimeDlyHMSM(0,0,0,10);
	}
}
void emwin_demo_task(void *pdata)
{
	GUI_DispStringAt("mem use:",0,100);
		GUI_DispDecAt((u8)mallco_dev.perused,100,100,3);
	while(1)
	{
//		GUIDEMO_Main();
		
		GUI_DispDecAt((u8)mallco_dev.perused,100,100,3);
		
		OSTimeDlyHMSM(0,0,0,10);
	}
}
