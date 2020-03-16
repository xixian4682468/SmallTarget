#include "Communication.h"
#include "struct.h"
#include "Detect.h"
#include "track.h"
#include "associate.h"


PROCWIN PWin;    //   ������������

#define KICK0  0x02620038
#define KICK1  0x0262003C
#define DEVICE_REG32_W(x,y) *(volatile unsigned long int *)(x) = (y)

/////////////////////////////////////////////////////////////////////////////////
extern volatile unsigned char   FovLabel,StartTrackLabel,SiFuTrackOpen,inipro,FirstDetectLabel;
extern unsigned char 	        Seg_Code_Over;
extern unsigned short           FPGA_Frame_Num,Pro_Frame_Num;
extern unsigned char            SiFuSearchOpen;

extern unsigned char            StartJumpLabel;
extern unsigned char            SiFuJumpOpen;
extern unsigned int             Jump_Num;
extern int                      Jump_Start_fuyangjiao;
extern int          			g_fuyangjiao ;
extern int						g_ReceiveCommand;
extern  PROCWIN      			PWin;
extern volatile int 			g_iTrackObjCount;
extern int		        		center_y,lostnum;
extern volatile unsigned char   FovLabel,StartTrackLabel,SiFuTrackOpen,inipro,FirstDetectLabel;
extern short					LastRow,LastCol,LastRow_DSP4,LastCol_DSP4;
extern volatile int 			g_iTrackObjCount;
extern float                    XFov,YFov;                 // Ĭ��Ϊ���ӳ�
extern volatile int        		XiuZhengZhi_X,XiuZhengZhi_Y,m_GuideState,StoreFrame;
extern unsigned char  	        g_bSearchInTrackState;
extern volatile int			    m_PossTarNum,m_CandTarNum,RealTarNum;
extern unsigned char            ExitOpen;
extern int						g_bFirstMatch ;
extern volatile int 			g_iFrameCount ;


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
			//����Ŀ��
			//SendAlarmTarInfoToPC();
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
		//�����γ�========================================================================
		SmallSearchBaseImage(PWin.m_StartRow,PWin.m_EndRow,PWin.m_StartCol,PWin.m_EndCol);
		//�����γ�========================================================================
		AutoAriSmallTarTrack();
		if(g_iTrackObjCount == 0)
		{
			lostnum++;    //  Ŀ��������ʧ֡��
			SiFuTrackOpen   = 1;
			StartTrackLabel = 0;
			if(lostnum>9) //�ۼƶ�֡����������ֵ
			{
				SiFuTrackStop(); //ֹͣ����
			}
		}
		else if(g_iTrackObjCount == 1)
		{
			lostnum=0;
			SendTargetInfoToSiFu(LastRow+XiuZhengZhi_Y,LastCol+XiuZhengZhi_X,YFov,XFov);
		}
		//����Ŀ��͵��Ӳ���=============================================================
//		SendTargetInfoToPC();
//		DrawTrackWindow();
		break;

	case ExitTrack:
		SUANFAFUWEI();
//		ClearTrackWindow();
//		DrawSearchWindow();
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
	if(g_ReceiveCommand == SearchTrackInGate)
	{
		PWin.m_StartRow  = 256-64;  //512������
		PWin.m_EndRow    = 256+64;
		PWin.m_StartCol  = 320-64;  //640������
		PWin.m_EndCol    = 320+64;
	}
	else
	{
		PWin.m_StartRow  = 20;
		PWin.m_EndRow    = 492;
		PWin.m_StartCol  = 30;
		PWin.m_EndCol    = 610;
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


