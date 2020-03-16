#include "Communication.h"
#include   "struct.h"
// #include"variable.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//向伺服发送角度查询指令
///////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int			*COM_RAM_R0		= (unsigned int*)0xD0100000;  	//接收上位机命令缓存地址
unsigned int			*COM_RAM_R1		= (unsigned int*)0xD0000000;  	//接收转台命令缓存地址
unsigned int			*COM_RAM_T0		= (unsigned int*)0xD0300000;  	//发送目标信息至上位机缓存地址
unsigned int			*COM_RAM_T1		= (unsigned int*)0xD0200000;  	//发送目标信息至转台缓存地址

unsigned int            *FPGA_Config_RAM= (unsigned int*)0xD0380000;    //配置FPGA游程

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
float                   XFov=7.66,YFov=6.18;                 // 默认为大视场
long                    SuDuXiuZheng_X=0,SuDuXiuZheng_Y=0; // 给伺服发送的速度角修正值,精度为0.000001度，故实际值乘以1000000，并将该值定义四个字节的长整型
int						g_ReceiveCommand = 0;							//接收的串口收发的命令
unsigned char          Idel = 0;
volatile int			    g_bTrackMatch=0,CALIBRATIONX=320,CALIBRATIONY=256;              //是否进行跟踪匹配
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

/*	 else if(FovLabel==0x0B)               // FovLabel=0x0B:大视场
    {
	 XFov=7.66;
	 YFov=6.18;
	 SuDuXiuZheng_X=6000000;
	 SuDuXiuZheng_Y=6000000;
	}
*/
  }
  else if((recTmp[1]==0xC0)&&(recTmp[2]==0xA0)&&(recTmp[6]==0x5D))  //搜索
  {
      g_ReceiveCommand=SearchAlarm;        //搜索
		YouChengOrImage=0;                 //搜索使用FPGA计算的游程

		ExitOpen=1;
	    Idel = 0x01;  //工作状态
  }
  else if((recTmp[1]==0xC0)&&(recTmp[2]==0xA0)&&(recTmp[6]==0x5D))  //跟踪
  {
	    g_ReceiveCommand=SearchTrackInGate;    //跟踪
		YouChengOrImage=1;  //不用游程
		StartTrackLabel=1;  //
		ExitOpen=1;
	    Idel = 0x01;
  }
  else if((recTmp[1]==0xC0)&&(recTmp[2]==0xA0)&&(recTmp[6]==0x5F))   //退出
  {
      g_ReceiveCommand = ExitTrack;
	    StartTrackLabel=0;

		StartJumpLabel=0;
	    Idel = 0x00;
  }

  if(YouChengOrImage==1)                //  YouChengOrImage=0：表示用游程信息聚类得到的目标中心与真实位置有偏差，用该偏差值进行修正
    {
         XiuZhengZhi_X=0;
         XiuZhengZhi_Y=0;
    }
   else                                  //  YouChengOrImage=1：表示用目标原始信息聚类得到的目标中心与真实位置无偏差，不需要修正
    {
         XiuZhengZhi_X=-35;
         XiuZhengZhi_Y=-9;
    }

}


/*函数作用：DrawTrackWindow() 画跟踪框，配置FPGA_Config_RAM[18]
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
	    Coordinate[0] =  (int)(FangWei)&0x000000ff;//d八位
	    Coordinate[1] =  ((int)(FangWei)&0x0000ff00)>>8;//gao八位
		Coordinate[2] =  (int)(FuYang)&0x000000ff;//d八位
		Coordinate[3] =  ((int)(FuYang)&0x000000ff00)>>8;//g八位
		ObjCoordinate =(Coordinate[3]<<24)|(Coordinate[2]<<16)|(Coordinate[1]<<8) | Coordinate[0];
	    FPGA_Config_RAM[18]= 0x00000100;
		FPGA_Config_RAM[19]= 0x00000000;
	    FPGA_Config_RAM[12]=ObjCoordinate;
	}
	else
	{
	   FangWei = 0x78FF;///若目标跟踪丢失，则发送脱靶量0x78FF
	   FuYang  = 0x78FF;
	   /*if(FirstDetectLabel==1)
	   {
			Coordinate[0] =  (int)(FangWei)&0x000000ff;//d八位
		    Coordinate[1] =  ((int)(FangWei)&0x0000ff00)>>8;//gao八位
			Coordinate[2] =  (int)(FuYang)&0x000000ff;//d八位
			Coordinate[3] =  ((int)(FuYang)&0x000000ff00)>>8;//g八位
			ObjCoordinate =(Coordinate[3]<<24)|(Coordinate[2]<<16)|(Coordinate[1]<<8) | Coordinate[0];
		    FPGA_Config_RAM[18]= 0x01010100;
			FPGA_Config_RAM[19]= 0x00000101;
		    FPGA_Config_RAM[12]=ObjCoordinate;
	   }*/
		Coordinate[0] =  (int)(FangWei)&0x000000ff;//d八位
	    Coordinate[1] =  ((int)(FangWei)&0x0000ff00)>>8;//gao八位
		Coordinate[2] =  (int)(FuYang)&0x000000ff;//d八位
		Coordinate[3] =  ((int)(FuYang)&0x000000ff00)>>8;//g八位
		ObjCoordinate =(Coordinate[3]<<24)|(Coordinate[2]<<16)|(Coordinate[1]<<8) | Coordinate[0];

	    FPGA_Config_RAM[18]= 0x00000100;
		FPGA_Config_RAM[19]= 0x00000000;
	    FPGA_Config_RAM[12]=ObjCoordinate;
	}

}

// ClearSearchWindow():不显示搜索框
void ClearSearchWindow()
{

}

/*函数作用：ClearTrackWindow() 画跟踪框，配置FPGA_Config_RAM[18]
*/
void ClearTrackWindow()
{

}

// DrawSearchWindow():画搜索框，搜索框大小为128*128，位于视场中心
void DrawSearchWindow()
{

}




