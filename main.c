#include <stdio.h>
#include <ti/ndk/inc/netmain.h>
#include <ti/ndk/inc/_stack.h>
#include <ti/ndk/inc/tools/console.h>
#include <ti/ndk/inc/tools/servers.h>
#include <ti/sysbios/knl/Task.h>

/* BIOS6 include */
#include <ti/sysbios/BIOS.h>


#include <xdc/runtime/Error.h>
#include <ti/bios/include/std.h>
#include <ti/sysbios/family/c64p/Hwi.h>

#include <cslr_gpio.h>


/* Platform utilities include */
#include "ti/platform/platform.h"

/* Resource manager for QMSS, PA, CPPI */
#include "ti/platform/resource_mgr.h"

#include "main.h"
#include "Driver_Init.h"
#include "Driver_Interrupt.h"
#include "Driver_Common.h"
#include "Driver_Timer.h"
#include "Driver_Srio.h"
#include "c6678_gpio.h"
/////////////////////////////////////
#include"Communication.h"
#include"struct.h"
#include"Detect.h"
#include "track.h"
#include "associate.h"
////////////////////////////////////

/* Platform Information - we will read it form the Platform Library */
extern unsigned int  g_timerISRCounter;
extern ErrInfo gTestInfo;

static HANDLE hTcpTask;
SOCKET g_UDPSockserver;
struct sockaddr_in g_UDPaddrServer,g_UDPaddrClient;
//unsigned char *IplImage;
extern unsigned char              *pSrc1  ;

PROCWIN PWin;    //   待处理窗口区域

SOCKET gsockConn;
extern unsigned int g_dw422IntFlag0;
extern unsigned int g_Resultbuf[28];
extern unsigned int  g_IntDHflag;
extern unsigned int  g_IntCXflag;
extern unsigned char			*RS422_R0 ;  	//
extern unsigned char			*RS422_R1 ;  	//
extern unsigned char			*RS422_R2 ;  	//
extern unsigned char			*RS422_R3 ;  	//

extern unsigned short			*RS422_T0 ;  	//
extern unsigned short			*RS422_T1 ;  	//
extern unsigned short			*RS422_T2 ;  	//
extern unsigned short			*RS422_T3 ;  	//

#define KICK0  0x02620038
#define KICK1  0x0262003C
#define DEVICE_REG32_W(x,y) *(volatile unsigned long int *)(x) = (y)

//unsigned short	*PC_EMIF_COMM		= (unsigned short*)0x7C200002;  	//发送目标信息至转台缓存地址

platform_info	gPlatformInfo;
extern unsigned int g_fifthMScount;
unsigned int count = 0;
char Flag_Image = 0;
extern volatile unsigned int gSRIORecvDataFlag;
extern volatile unsigned int gEMIFRecvDataFlag;

/////////////////////////////////////////////////////////////////////////////////
extern volatile unsigned char  FovLabel,StartTrackLabel,SiFuTrackOpen,inipro,FirstDetectLabel;
extern unsigned char 	         Seg_Code_Over;
extern unsigned short          FPGA_Frame_Num,Pro_Frame_Num;
extern unsigned char           SiFuSearchOpen;
extern unsigned char           StartSearchLabel;
extern unsigned char           StartJumpLabel;
extern unsigned char           SiFuJumpOpen;
extern unsigned int            Jump_Num;
extern int                     Jump_Start_fuyangjiao;
extern int          			g_fuyangjiao ;
extern int						g_ReceiveCommand;
extern  PROCWIN PWin;
extern volatile int g_iTrackObjCount;
extern int		        center_y,lostnum;
extern volatile unsigned char  FovLabel,StartTrackLabel,SiFuTrackOpen,inipro,FirstDetectLabel;
extern short					LastRow,LastCol,LastRow_DSP4,LastCol_DSP4;
extern volatile int g_iTrackObjCount;
extern float                   XFov,YFov;                 // 默认为大视场
extern volatile int        	XiuZhengZhi_X,XiuZhengZhi_Y,m_GuideState,StoreFrame;
extern unsigned char  	     g_bSearchInTrackState;
extern volatile int			m_PossTarNum,m_CandTarNum,RealTarNum;
extern unsigned char           ExitOpen;
extern int				g_bFirstMatch ;
extern volatile int g_iFrameCount ;

extern char			        *pImageInDSP;

//////////////////////////////////////////////////////////////////////////////////
volatile int            MaxObjNum=0;
//---------------------------------------------------------------------
// Main Entry `
//---------------------------------------------------------------------
CSL_GpioRegs   *gpGPIO_regsflash = (CSL_GpioRegs *)(0x2320000);
#define NAND_FLASH_TEST 0

int TestRapidIO(void);

void My_Delay(volatile unsigned int n)
{
	unsigned int i;
	for(i = n; i > 0; i--);
}


//主要的处理函数
void MainProcess()
{
  switch(g_ReceiveCommand)
   {
	case SearchAlarm:                  //  全帧搜索
		g_bSearchInTrackState=1;
	    SelectInitialTrackRegion();
	    AutoTarDetect(PWin.m_StartRow,PWin.m_EndRow,PWin.m_StartCol,PWin.m_EndCol);

      if (m_PossTarNum>0)
		{

		   SendAlarmTarInfoToPC();

		 }
      ClearSearchWindow();
		 break;

	  case SearchTrackInGate:	                //  跟踪
		if(inipro==0)
	     {
			SelectInitialTrackRegion();
			inipro=1;
		 }
		else
		 {
			ProcWinAdapt();
		 }
			SmallSearchBaseImage(PWin.m_StartRow,PWin.m_EndRow,PWin.m_StartCol,PWin.m_EndCol);
			AutoAriSmallTarTrack();
	   if(g_iTrackObjCount == 0)
		 {
			lostnum++; //  目标连续丢失帧数
			SiFuTrackOpen=1;
			StartTrackLabel=0;
		  if(lostnum>9)
		    {
			 SiFuTrackStop();
			 }
		  }
	    else if(g_iTrackObjCount == 1)
		  {
			 lostnum=0;
			 SendTargetInfoToSiFu(LastRow+XiuZhengZhi_Y,LastCol+XiuZhengZhi_X,YFov,XFov);
		  }
			 SendTargetInfoToPC();
			 DrawTrackWindow();
			  break;

		case ExitTrack:
			 SUANFAFUWEI();
			 ClearTrackWindow();
			 DrawSearchWindow();
			 if(ExitOpen==1)
			  {
			     SiFuTrackStop();
			     SiFuSearchStop();
			     ExitOpen=0;
			  }
			 break;

		default:
			 break;
	 }
}

void SelectInitialTrackRegion()
{
   if(g_ReceiveCommand==SearchTrackInGate)
   {
 	   PWin.m_StartRow   = 256-64;  //512行中心
       PWin.m_EndRow     = 256+64;
       PWin.m_StartCol   = 320-64;  //640列中心
       PWin.m_EndCol     = 320+64;
   }
   else
   {
        PWin.m_StartRow = 20;
        PWin.m_EndRow = 492;
        PWin.m_StartCol = 30;
        PWin.m_EndCol = 610;
   }
}

int main()
{
	Task_Handle nettask;
	Task_Params net_task_paras;
	int status = 0;
	int i,j;
	unsigned int dwAddr;
	BYTE ucResult[10];
	unsigned int inenable;
	unsigned long long delay,preTSC,tempTSC;
	unsigned int temp;
	unsigned char buf[512];

	ClearBuffer(ucResult, 10);
	TSC_init();
//初始化EMIF和SRIO**********************************************************************
	DriverSysConfig();
//中断中对GPIO管脚进行了初始化***************************************************************
	InterruptInit();
//初始化GPIO部分管脚**********************************************************************
	c6678_gpio_init();				//GPIO设置
	DDR_Init();

	BIOS_start();
}


//测试RapidIO的NWIRTE与NREAD*************************************************************
int TestRapidIO(void)
{
	int nRetCode = 0;
	unsigned int i = 0;
    unsigned int *gtxbuf1 = (unsigned int *)0x11800000;
//    unsigned int *grxbuf1 = (unsigned int *)0x11803200;
	unsigned short* DDR31_place = (unsigned short *)0x81000000;
	unsigned short* DDR32_place = (unsigned short *)0x82000000;
//RapidIO发送数据初始化**************************************************
	for(i=0; i<256*4; i++)
	{
		gtxbuf1[i] = (0xaa55aa55 + i +count);
	}
//RapidIO发送数据**************************************************

	SRIOSendForChannel(0, (unsigned int *)gtxbuf1, 8, 0, 0XFF);
//循环接收RapidIO数据***************************************
	i = 0;
	while(1)
	{
      if( i < 10000 )
      {
			while(0 == gSRIORecvDataFlag);
			{
			   gSRIORecvDataFlag = 0;
			}

//			SRIORecvForChannel(0, (unsigned int *)grxbuf1, 256*4*320, 0, 0xFF);
			SRIORecvForChannel(0, (unsigned int *)pImageInDSP, 256*4*320, 0, 0xFF);

//			printf("Test times :%d Test_srio OK\n", i);
		    i++;
       }
      else
      {
    	  printf("Test times :%d Test_srio OK\n", i);

      }


	}
	return nRetCode;
}

void ProcWinAdapt()
{
	PWin.m_StartRow = LastRow-64;
    PWin.m_EndRow   = LastRow+64;;
    if(PWin.m_StartRow<TOPBORDER)
    {
       PWin.m_StartRow=TOPBORDER;
       PWin.m_EndRow  =BOTTOMBORDER-12;//12

    }
    if(PWin.m_EndRow>BOTTOMBORDER)
    {
       PWin.m_EndRow  =BOTTOMBORDER-12;//12
       PWin.m_StartRow=TOPBORDER;
    }

    PWin.m_StartCol = LastCol-64;
    PWin.m_EndCol   = LastCol+64;
    if(PWin.m_StartCol<LEFTBORDER)
    {
       PWin.m_StartCol=LEFTBORDER;
       PWin.m_EndCol  =RIGHTBORDER-10;//20

    }
    if(PWin.m_EndCol>RIGHTBORDER)
    {
       PWin.m_EndCol  =RIGHTBORDER-10;  //20
       PWin.m_StartCol=LEFTBORDER;
    }

}

/*工作模式设置：
1、全帧搜索跟踪模式及告警模式时：不显示搜索框，
有目标则画出目标框，无目标则将跟踪框画无限大，相当于去除跟踪框；
2、波门搜索跟踪，首先在视场中心画出搜索框，一旦跟踪到目标则去除搜索框，显示跟踪框；
若目标消失，则去除跟踪框，显示搜索框；*/


//==========================================================================================================
/*函数作用：使算法回到检测前状态，当接受到检测开始命令后，程序重新进行目标检测
*/

void SUANFAFUWEI()
{
  InitTargetInChain();
  g_iFrameCount = 0;
  MaxObjNum = 0;
  g_bSearchInTrackState = 1;
  g_ReceiveCommand = 0;
  g_bFirstMatch = 1;
  g_iTrackObjCount = 0;
  inipro=0;                      // 对于波门跟踪模式，一旦没有检测到目标，就重新初始化，选图像中心128*128窗口为搜索检测区域
  lostnum=0;
  SiFuTrackOpen=1;               // 一旦收到退出跟踪指令，就重新初始化，准备下一时刻给伺服发开始跟踪指令
  FirstDetectLabel=0;	         // 进入任意一种跟踪模式后，第一次检测到目标的标识符
  SiFuSearchOpen=1;
  SiFuJumpOpen=1;
  Jump_Num=0;
}


void FPGA_Config()
{
	int i;
 	unsigned char FPGAInfor[10]={0};
 	unsigned short StartInfor[2]={0};
 	unsigned short	*Config_FPGA  = (unsigned short*)0x78000000;
/*
 	FPGAInfor[0] = 0xFFFF;   //DSP配置预处理参数使能
 	FPGAInfor[1] = 0xAAAA;   //边缘提取阈值（任意）
 	FPGAInfor[2] = 0x0020;   // DSP预处理上边界
 	FPGAInfor[3] = 0x0020;   // DSP预处理左边界
 	FPGAInfor[4] = 0x0082;   //二值化阈值取130
 	FPGAInfor[5] = 0x0020;   // DSP预处理左边界
 	FPGAInfor[6] = 0x0020;   // DSP预处理右边界
 	FPGAInfor[7] = 0x0200;   // DSP预处理行数 512
 	FPGAInfor[8] = 0x0280;   // DSP预处理列数 640

 */
 	    FPGAInfor[0] = 0x00;   //视频源选择，0x00 原始图像
 	 	FPGAInfor[2] = 0x20;   // 上下边界去除行数
 	 	FPGAInfor[1] = 0x20;   // 左右边界去除行数
 	 	FPGAInfor[3] = 0x82;   //二值化阈值取1

 	 	StartInfor[0] = 0x2000;
 	 	StartInfor[1] = 0x0120;

	for(i=0;i<2;i++)
	{
		*(Config_FPGA +i ) = StartInfor[i];
	}
}


