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

PROCWIN PWin;    //   ������������

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

//unsigned short	*PC_EMIF_COMM		= (unsigned short*)0x7C200002;  	//����Ŀ����Ϣ��ת̨�����ַ

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
extern float                   XFov,YFov;                 // Ĭ��Ϊ���ӳ�
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


//��Ҫ�Ĵ�����
void MainProcess()
{
  switch(g_ReceiveCommand)
   {
	case SearchAlarm:                  //  ȫ֡����
		g_bSearchInTrackState=1;
	    SelectInitialTrackRegion();
	    AutoTarDetect(PWin.m_StartRow,PWin.m_EndRow,PWin.m_StartCol,PWin.m_EndCol);

      if (m_PossTarNum>0)
		{

		   SendAlarmTarInfoToPC();

		 }
      ClearSearchWindow();
		 break;

	  case SearchTrackInGate:	                //  ����
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
			lostnum++; //  Ŀ��������ʧ֡��
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
 	   PWin.m_StartRow   = 256-64;  //512������
       PWin.m_EndRow     = 256+64;
       PWin.m_StartCol   = 320-64;  //640������
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
//��ʼ��EMIF��SRIO**********************************************************************
	DriverSysConfig();
//�ж��ж�GPIO�ܽŽ����˳�ʼ��***************************************************************
	InterruptInit();
//��ʼ��GPIO���ֹܽ�**********************************************************************
	c6678_gpio_init();				//GPIO����
	DDR_Init();

	BIOS_start();
}


//����RapidIO��NWIRTE��NREAD*************************************************************
int TestRapidIO(void)
{
	int nRetCode = 0;
	unsigned int i = 0;
    unsigned int *gtxbuf1 = (unsigned int *)0x11800000;
//    unsigned int *grxbuf1 = (unsigned int *)0x11803200;
	unsigned short* DDR31_place = (unsigned short *)0x81000000;
	unsigned short* DDR32_place = (unsigned short *)0x82000000;
//RapidIO�������ݳ�ʼ��**************************************************
	for(i=0; i<256*4; i++)
	{
		gtxbuf1[i] = (0xaa55aa55 + i +count);
	}
//RapidIO��������**************************************************

	SRIOSendForChannel(0, (unsigned int *)gtxbuf1, 8, 0, 0XFF);
//ѭ������RapidIO����***************************************
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

/*����ģʽ���ã�
1��ȫ֡��������ģʽ���澯ģʽʱ������ʾ������
��Ŀ���򻭳�Ŀ�����Ŀ���򽫸��ٿ����޴��൱��ȥ�����ٿ�
2�������������٣��������ӳ����Ļ���������һ�����ٵ�Ŀ����ȥ����������ʾ���ٿ�
��Ŀ����ʧ����ȥ�����ٿ���ʾ������*/


//==========================================================================================================
/*�������ã�ʹ�㷨�ص����ǰ״̬�������ܵ���⿪ʼ����󣬳������½���Ŀ����
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
  inipro=0;                      // ���ڲ��Ÿ���ģʽ��һ��û�м�⵽Ŀ�꣬�����³�ʼ����ѡͼ������128*128����Ϊ�����������
  lostnum=0;
  SiFuTrackOpen=1;               // һ���յ��˳�����ָ������³�ʼ����׼����һʱ�̸��ŷ�����ʼ����ָ��
  FirstDetectLabel=0;	         // ��������һ�ָ���ģʽ�󣬵�һ�μ�⵽Ŀ��ı�ʶ��
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
 	FPGAInfor[0] = 0xFFFF;   //DSP����Ԥ�������ʹ��
 	FPGAInfor[1] = 0xAAAA;   //��Ե��ȡ��ֵ�����⣩
 	FPGAInfor[2] = 0x0020;   // DSPԤ�����ϱ߽�
 	FPGAInfor[3] = 0x0020;   // DSPԤ������߽�
 	FPGAInfor[4] = 0x0082;   //��ֵ����ֵȡ130
 	FPGAInfor[5] = 0x0020;   // DSPԤ������߽�
 	FPGAInfor[6] = 0x0020;   // DSPԤ�����ұ߽�
 	FPGAInfor[7] = 0x0200;   // DSPԤ�������� 512
 	FPGAInfor[8] = 0x0280;   // DSPԤ�������� 640

 */
 	    FPGAInfor[0] = 0x00;   //��ƵԴѡ��0x00 ԭʼͼ��
 	 	FPGAInfor[2] = 0x20;   // ���±߽�ȥ������
 	 	FPGAInfor[1] = 0x20;   // ���ұ߽�ȥ������
 	 	FPGAInfor[3] = 0x82;   //��ֵ����ֵȡ1

 	 	StartInfor[0] = 0x2000;
 	 	StartInfor[1] = 0x0120;

	for(i=0;i<2;i++)
	{
		*(Config_FPGA +i ) = StartInfor[i];
	}
}


