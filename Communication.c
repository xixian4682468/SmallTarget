#include "Communication.h"
#include   "struct.h"
// #include"variable.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//���ŷ����ͽǶȲ�ѯָ��
///////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int			*COM_RAM_R0		= (unsigned int*)0xD0100000;  	//������λ��������ַ
unsigned int			*COM_RAM_R1		= (unsigned int*)0xD0000000;  	//����ת̨������ַ
unsigned int			*COM_RAM_T0		= (unsigned int*)0xD0300000;  	//����Ŀ����Ϣ����λ�������ַ
unsigned int			*COM_RAM_T1		= (unsigned int*)0xD0200000;  	//����Ŀ����Ϣ��ת̨�����ַ

unsigned int            *FPGA_Config_RAM= (unsigned int*)0xD0380000;    //����FPGA�γ�

int                     g_gunzhuanjiao=0;
extern int          			g_fangweijiao;
extern int          			g_fuyangjiao;
extern unsigned char           ExitOpen;
extern unsigned char           StartJumpLabel;

extern unsigned char           YouChengOrImage;
extern volatile int            g_iTrackObjCount;
extern TARFEATURE 		*g_Candidate;
extern volatile int			m_PossTarNum,m_CandTarNum,RealTarNum;

extern short					LastRow,LastCol,LastRow_DSP4,LastCol_DSP4 ;
extern int                     lostnum;
volatile unsigned char  FovLabel=0x0B,StartTrackLabel=0,SiFuTrackOpen=1,inipro=0,FirstDetectLabel=0;
float                   XFov=7.66,YFov=6.18;                 // Ĭ��Ϊ���ӳ�
long                    SuDuXiuZheng_X=0,SuDuXiuZheng_Y=0; // ���ŷ����͵��ٶȽ�����ֵ,����Ϊ0.000001�ȣ���ʵ��ֵ����1000000��������ֵ�����ĸ��ֽڵĳ�����
int						g_ReceiveCommand = 0;							//���յĴ����շ�������
unsigned char          Idel = 0;
volatile int			    g_bTrackMatch=0,CALIBRATIONX=320,CALIBRATIONY=256;              //�Ƿ���и���ƥ��
volatile int        	XiuZhengZhi_X=0,XiuZhengZhi_Y=0,m_GuideState,StoreFrame=0;


void ReceiveCommandfromPC()
{
  int i,temp=0,piancha_x=0,piancha_y=0;
  /*
  unsigned int  pc_rx_buff[3];
  unsigned char* recTmp=(unsigned char*)pc_rx_buff;
  */
  unsigned char  g_pReceiveCommand;
  unsigned char  recTemp[12]={0};
  unsigned char* Rec = (unsigned char *)0x7C000000;
  unsigned char* recTmp=(unsigned char*)Rec;

  if((recTmp[1]==0xC0)&&(recTmp[2]==0xA0)&&(recTmp[6]==0x60))
  {
	 XFov=3.83;
	 YFov=3.09;
	 SuDuXiuZheng_X=10000000;
	 SuDuXiuZheng_Y=7500000;

/*	 else if(FovLabel==0x0B)               // FovLabel=0x0B:���ӳ�
    {
	 XFov=7.66;
	 YFov=6.18;
	 SuDuXiuZheng_X=6000000;
	 SuDuXiuZheng_Y=6000000;
	}
*/
  }
  else if((recTmp[1]==0xC0)&&(recTmp[2]==0xA0)&&(recTmp[6]==0x5D))  //����
  {
      g_ReceiveCommand=SearchAlarm;        //����
		YouChengOrImage=0;                 //����ʹ��FPGA������γ�

		ExitOpen=1;
	    Idel = 0x01;  //����״̬
  }
  else if((recTmp[1]==0xC0)&&(recTmp[2]==0xA0)&&(recTmp[6]==0x5D))  //����
  {
	    g_ReceiveCommand=SearchTrackInGate;    //����
		YouChengOrImage=1;  //�����γ�
		StartTrackLabel=1;  //
		ExitOpen=1;
	    Idel = 0x01;
  }
  else if((recTmp[1]==0xC0)&&(recTmp[2]==0xA0)&&(recTmp[6]==0x5F))   //�˳�
  {
      g_ReceiveCommand = ExitTrack;
	    StartTrackLabel=0;

		StartJumpLabel=0;
	    Idel = 0x00;
  }

  if(YouChengOrImage==1)                //  YouChengOrImage=0����ʾ���γ���Ϣ����õ���Ŀ����������ʵλ����ƫ��ø�ƫ��ֵ��������
    {
         XiuZhengZhi_X=0;
         XiuZhengZhi_Y=0;
    }
   else                                  //  YouChengOrImage=1����ʾ��Ŀ��ԭʼ��Ϣ����õ���Ŀ����������ʵλ����ƫ�����Ҫ����
    {
         XiuZhengZhi_X=-35;
         XiuZhengZhi_Y=-9;
    }

}


/*�������ã�DrawTrackWindow() �����ٿ�����FPGA_Config_RAM[18]
*/
void DrawTrackWindow()
{
   unsigned char Coordinate[4]={0} ;
   unsigned int ObjCoordinate=0;
   short FangWei=0,FuYang=0;

   if(g_iTrackObjCount == 1)
	{
	   FangWei = LastCol+XiuZhengZhi_X;
	   FuYang = LastRow+XiuZhengZhi_Y;
	   ClearSearchWindow();
	   FirstDetectLabel=1;
	   /*if(FirstDetectLabel==0)
	   {
	       FirstDetectLabel=1;
	   }*/
	    Coordinate[0] =  (int)(FangWei)&0x000000ff;//d��λ
	    Coordinate[1] =  ((int)(FangWei)&0x0000ff00)>>8;//gao��λ
		Coordinate[2] =  (int)(FuYang)&0x000000ff;//d��λ
		Coordinate[3] =  ((int)(FuYang)&0x000000ff00)>>8;//g��λ
		ObjCoordinate =(Coordinate[3]<<24)|(Coordinate[2]<<16)|(Coordinate[1]<<8) | Coordinate[0];
	    FPGA_Config_RAM[18]= 0x00000100;
		FPGA_Config_RAM[19]= 0x00000000;
	    FPGA_Config_RAM[12]=ObjCoordinate;
	}
	else
	{
	   FangWei = 0x78FF;///��Ŀ����ٶ�ʧ�������Ѱ���0x78FF
	   FuYang  = 0x78FF;
	   /*if(FirstDetectLabel==1)
	   {
			Coordinate[0] =  (int)(FangWei)&0x000000ff;//d��λ
		    Coordinate[1] =  ((int)(FangWei)&0x0000ff00)>>8;//gao��λ
			Coordinate[2] =  (int)(FuYang)&0x000000ff;//d��λ
			Coordinate[3] =  ((int)(FuYang)&0x000000ff00)>>8;//g��λ
			ObjCoordinate =(Coordinate[3]<<24)|(Coordinate[2]<<16)|(Coordinate[1]<<8) | Coordinate[0];
		    FPGA_Config_RAM[18]= 0x01010100;
			FPGA_Config_RAM[19]= 0x00000101;
		    FPGA_Config_RAM[12]=ObjCoordinate;
	   }*/
		Coordinate[0] =  (int)(FangWei)&0x000000ff;//d��λ
	    Coordinate[1] =  ((int)(FangWei)&0x0000ff00)>>8;//gao��λ
		Coordinate[2] =  (int)(FuYang)&0x000000ff;//d��λ
		Coordinate[3] =  ((int)(FuYang)&0x000000ff00)>>8;//g��λ
		ObjCoordinate =(Coordinate[3]<<24)|(Coordinate[2]<<16)|(Coordinate[1]<<8) | Coordinate[0];

	    FPGA_Config_RAM[18]= 0x00000100;
		FPGA_Config_RAM[19]= 0x00000000;
	    FPGA_Config_RAM[12]=ObjCoordinate;
	}

}

// ClearSearchWindow():����ʾ������
void ClearSearchWindow()
{

}

/*�������ã�ClearTrackWindow() �����ٿ�����FPGA_Config_RAM[18]
*/
void ClearTrackWindow()
{

}

// DrawSearchWindow():���������������СΪ128*128��λ���ӳ�����
void DrawSearchWindow()
{

}




