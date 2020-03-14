#include"Communication.h"
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
extern unsigned char           StartSearchLabel;
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


void Send_To_SifuAngle()
{
	unsigned short*   SifuAngle_ADDR_BASE = (unsigned short*) 0x74000000;
	int i;
	int checkSum=0,kk=6;
 	unsigned char recTemp[48]={0} ;
 	unsigned short SendObj[12]={0};
	unsigned char checkvalue;

    recTemp[1] = 0xc0;
    recTemp[2] = 0xc0;
    recTemp[3] = 0x02;   // 发送数据位长度
	recTemp[4] = 0x03;   // 转台设备码
	recTemp[5] = 0x21;   // 命令码，表示给转台发送角度信息

	checkSum = recTemp[4]+ recTemp[5];
    checkvalue = checkSum&0xff;
    if((checkvalue==0xCF)||(checkvalue==0xC0)||(checkvalue==0xCA))
	{
	    recTemp[kk]=0xCA;
	    recTemp[kk+1]=checkvalue;
        kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=checkvalue;
	   kk=kk+1;
    }
    recTemp[kk]=0xCF;
    recTemp[0] =kk;
    for(i = 0; i < 3; i++)
	{
		SendObj[i] =(recTemp[2*i+1]<<8) | recTemp[2*i];
	}
	for(i=0;i<3;i++)
	{
		*(SifuAngle_ADDR_BASE + 2*i) = SendObj[i];
//	  	COM_RAM_T1[i] = SendObj[i];
	}

//	GPIO_OUT[0]  |=  0x00000001<<9;			//确保 GPIO9 的输出为 高 电平
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//接收转台旋转的角度值
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ReceiveAnglefromSiFu()
{
  int i,temp=0,piancha_x=0,piancha_y=0;
  unsigned int  pc_rx_buff[20]={0};
  unsigned int  Check_Sum,kk;
  unsigned char* recTmp=(unsigned char*)pc_rx_buff;
  unsigned char  g_pReceiveCommand;
  unsigned char  X[4],Y[4],Z[4];
///////////////////////////////////////////////////////////
  for(i=0;i<20;i++)   //?
  {
      pc_rx_buff[i] = COM_RAM_R1[i];
	  Check_Sum =0;
  }
  ///////////////////////////////////////////////////////////
  kk=6;
  Check_Sum = Check_Sum+recTmp[4]+recTmp[5];

  if((recTmp[1]==0xC0)&&(recTmp[2]==0xC0)&&(recTmp[4]==0x03)&&(recTmp[5]==0x21))
  {
	   //收转台的lx
    for(i=0;i<4;i++)
	 {
       if (recTmp[kk]==0xCA)
	    {
	       kk=kk+1;
	    }
	   else
	    {
	       kk=kk;
	    }
	   X[i]=recTmp[kk];
       Check_Sum= Check_Sum+recTmp[kk];
	   kk=kk+1;
	 }

     //收转台的ly
    for(i=0;i<4;i++)
	 {
       if (recTmp[kk]==0xCA)
	    {
	       kk=kk+1;
	    }
	   else
	    {
	       kk=kk;
	    }
	   Y[i]=recTmp[kk];
       Check_Sum= Check_Sum+recTmp[kk];
	   kk=kk+1;
	 }

	//收转台的lz
    for(i=0;i<4;i++)
	 {
       if (recTmp[kk]==0xCA)
	    {
	       kk=kk+1;
	    }
	   else
	    {
	       kk=kk;
	    }
	   Z[i]=recTmp[kk];
       Check_Sum= Check_Sum+recTmp[kk];
	   kk=kk+1;
	 }
///读校验码
    if (recTmp[kk]==0xCA)
	 {
	    kk=kk+1;
	 }
	else
	 {
	    kk=kk;
	 }

    if(recTmp[kk]==(Check_Sum&0xff))
	 {
	    g_fangweijiao=((X[0]<<24)+(X[1]<<16)+(X[2]<<8)+(X[0]));
		g_fuyangjiao=((Y[0]<<24)+(Y[1]<<16)+(Y[2]<<8)+(Y[0]));
        g_gunzhuanjiao=((Z[0]<<24)+(Z[1]<<16)+(Z[2]<<8)+(Z[0]));

	 }
  }
}


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
		YouChengOrImage=0;
		StartSearchLabel=1;
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
        StartSearchLabel=0;
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


void SiFuTrackStart()
{
	int i;
	int checkSum=0;
	unsigned char checkvalue;
 	unsigned char recTemp[12]={0};
 	unsigned short StartInfor[6]={0};
 	unsigned short*   SifuAngle_ADDR_BASE = (unsigned short*) 0x74000000;

    recTemp[0] = 0x08;
    recTemp[1] = 0xc0;
	recTemp[2] = 0xc0;
	recTemp[3] = 0x03;   // 发送数据位长度
	recTemp[4] = 0x03;   // 设备代码(转台控制器)
	recTemp[5] = 0x0A;   // 功能代码(跟踪模式)
	recTemp[6] = 0xAA;   // 命令代耄嚎几?
	for (i = 4; i < 7; i++)
	{
		checkSum += recTemp[i];
	}
	checkvalue = checkSum&0xff;
	recTemp[7] = checkvalue;
	recTemp[8] = 0xCF;
	for(i = 0; i < 6; i++)
	{
		StartInfor[i] =(recTemp[2*i+1]<<8) | recTemp[2*i];
	}
	for(i=0;i<6;i++)
	{
	  	*(SifuAngle_ADDR_BASE + 2*i) = StartInfor[i];
	}
   /* GPIO_OUT[0]  &=  (~(0x00000001<<9));	//确保GPIO9为 低 电平
	delay(10);
	GPIO_OUT[0]  |=  0x00000001<<9;			//确保 GPIO9 的输出为 高 电平*/
}


void  SiFuSearchStart()
{
 	int i,fangweisudu;
	int checkSum=0,kk=6;
 	unsigned char recTemp[48]={0} ;
 	unsigned short SendObj[24]={0};
	unsigned char checkvalue,X_1,X_2,X_3,X_4,Y_1,Y_2,Y_3,Y_4,Vx_1,Vx_2,Vx_3,Vx_4,Vy_1,Vy_2,Vy_3,Vy_4;
	unsigned short*   SifuAngle_ADDR_BASE = (unsigned short*) 0x74000000;
	recTemp[1] = 0xc0;
    recTemp[2] = 0xc0;
    recTemp[3] = 0x0A;   // 发送数据位长度
	recTemp[4] = 0x03;   // 转台设备码
	recTemp[5] = 0x10;   // 命令码，表示给转台启动搜索，速度300度/s

	fangweisudu=-300000000;
   // 脱靶量先发高位，再发低位
    X_4=  fangweisudu & 0x000000ff;//目标方位低八位
	X_3= (fangweisudu & 0x0000ff00)>>8;//目标方位次低八位
	X_2= (fangweisudu & 0x00ff0000)>>16;// 目标方位次高八位
    X_1= (fangweisudu & 0xff000000)>>24;// 目标方位高八位

    Y_4=  0x00000000 & 0x000000ff;//目标俯仰低八位
	Y_3= (0x00000000 & 0x0000ff00)>>8;//目标俯仰次低八位
	Y_2= (0x00000000 & 0x00ff0000)>>16;// 目标俯仰次高八位
    Y_1= (0x00000000 & 0xff000000)>>24;// 目标俯仰高八位

    if ((X_1==0xCF)||(X_1==0xC0)||(X_1==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=X_1;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=X_1;
	   kk=kk+1;
    }

	if ((X_2==0xCF)||(X_2==0xC0)||(X_2==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=X_2;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=X_2;
	   kk=kk+1;
    }

	if ((X_3==0xCF)||(X_3==0xC0)||(X_3==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=X_3;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=X_3;
	   kk=kk+1;
    }

    if((X_4==0xCF)||(X_4==0xC0)||(X_4==0xCA))
	{
	    recTemp[kk]=0xCA;
	    recTemp[kk+1]=X_4;
        kk=kk+2;  // 出转义字符，两字谖恢眯聪乱唤谥?
	}
	else
    {
	   recTemp[kk]=X_4;
	   kk=kk+1;
    }
	if ((Y_1==0xCF)||(Y_1==0xC0)||(Y_1==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=Y_1;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=Y_1;
	   kk=kk+1;
    }

	if ((Y_2==0xCF)||(Y_2==0xC0)||(Y_2==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=Y_2;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=Y_2;
	   kk=kk+1;
    }

	if ((Y_3==0xCF)||(Y_3==0xC0)||(Y_3==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=Y_3;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=Y_3;
	   kk=kk+1;
    }

    if((Y_4==0xCF)||(Y_4==0xC0)||(Y_4==0xCA))
	{
	    recTemp[kk]=0xCA;
	    recTemp[kk+1]=Y_4;
        kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=Y_4;
	   kk=kk+1;
    }
	checkSum = recTemp[4]+ recTemp[5]+X_1+X_2+X_3+X_4+Y_1+Y_2+Y_3+Y_4;
    checkvalue = checkSum&0xff;
    if((checkvalue==0xCF)||(checkvalue==0xC0)||(checkvalue==0xCA))
	{
	    recTemp[kk]=0xCA;
	    recTemp[kk+1]=checkvalue;
        kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=checkvalue;
	   kk=kk+1;
    }
    recTemp[kk]=0xCF;
    recTemp[0] =kk;
    for(i = 0; i < 24; i++)
	{
		SendObj[i] =(recTemp[2*i+1]<<8) | recTemp[2*i];
	}
	for(i=0;i<24;i++)
	{
		*(SifuAngle_ADDR_BASE + 2*i) =  SendObj[i];
	}

/*
//	GPIO_OUT[0]  |=  0x00000001<<9;			//确保 GPIO9 的输出为 高 电平
	GPIO_OUT[0]  &=  (~(0x00000001<<9));	//确保GPIO9为 低 电平
	delay(5000);
	GPIO_OUT[0]  |=  0x00000001<<9;			//确保 GPIO9 的输出为 高 电平
*/
}


void SiFuJump(long Fuyangjiao)
{

	int i;
	int checkSum=0,kk=6;
 	unsigned char recTemp[48]={0} ;
 	unsigned short SendObj[24]={0};
	unsigned char checkvalue,X_1,X_2,X_3,X_4,Y_1,Y_2,Y_3,Y_4,Vx_1,Vx_2,Vx_3,Vx_4,Vy_1,Vy_2,Vy_3,Vy_4;
	unsigned short*   SifuAngle_ADDR_BASE = (unsigned short*) 0x74000000;
    recTemp[1] = 0xc0;
    recTemp[2] = 0xc0;
    recTemp[3] = 0x0A;   // 发送数据位长度
	recTemp[4] = 0x03;   // 转台设备码
	recTemp[5] = 0x13;   // 命令码，表示给转台发送角度信息

   // 脱靶量先发高位，再发低位
    X_4=  0x80000000 & 0x000000ff;//目标方位低八位
	X_3= (0x80000000 & 0x0000ff00)>>8;//目标方位次低八位
	X_2= (0x80000000 & 0x00ff0000)>>16;// 目标方位次高八位
    X_1= (0x80000000 & 0xff000000)>>24;// 目标方位高八位

    Y_4=  Fuyangjiao & 0x000000ff;//目标俯仰低八位
	Y_3= (Fuyangjiao & 0x0000ff00)>>8;//目标俯仰次低八位
	Y_2= (Fuyangjiao & 0x00ff0000)>>16;// 目标俯仰次高八位
    Y_1= (Fuyangjiao & 0xff000000)>>24;// 目标俯仰高八位

    if ((X_1==0xCF)||(X_1==0xC0)||(X_1==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=X_1;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=X_1;
	   kk=kk+1;
    }

	if ((X_2==0xCF)||(X_2==0xC0)||(X_2==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=X_2;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=X_2;
	   kk=kk+1;
    }

	if ((X_3==0xCF)||(X_3==0xC0)||(X_3==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=X_3;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=X_3;
	   kk=kk+1;
    }

    if((X_4==0xCF)||(X_4==0xC0)||(X_4==0xCA))
	{
	    recTemp[kk]=0xCA;
	    recTemp[kk+1]=X_4;
        kk=kk+2;  // 出转义字符，两字谖恢眯聪乱唤谥?
	}
	else
    {
	   recTemp[kk]=X_4;
	   kk=kk+1;
    }
	if ((Y_1==0xCF)||(Y_1==0xC0)||(Y_1==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=Y_1;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=Y_1;
	   kk=kk+1;
    }

	if ((Y_2==0xCF)||(Y_2==0xC0)||(Y_2==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=Y_2;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=Y_2;
	   kk=kk+1;
    }

	if ((Y_3==0xCF)||(Y_3==0xC0)||(Y_3==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=Y_3;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=Y_3;
	   kk=kk+1;
    }

    if((Y_4==0xCF)||(Y_4==0xC0)||(Y_4==0xCA))
	{
	    recTemp[kk]=0xCA;
	    recTemp[kk+1]=Y_4;
        kk=kk+2;  // 鱿肿遄址隔阶节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=Y_4;
	   kk=kk+1;
    }


	checkSum = recTemp[4]+ recTemp[5]+X_1+X_2+X_3+X_4+Y_1+Y_2+Y_3+Y_4;
    checkvalue = checkSum&0xff;
    if((checkvalue==0xCF)||(checkvalue==0xC0)||(checkvalue==0xCA))
	{
	    recTemp[kk]=0xCA;
	    recTemp[kk+1]=checkvalue;
        kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=checkvalue;
	   kk=kk+1;
    }
    recTemp[kk]=0xCF;
    recTemp[0] =kk;
    for(i = 0; i < 24; i++)
	{
		SendObj[i] =recTemp[2*i+1]<<8 | recTemp[2*i];
	}
	for(i=0;i<24;i++)
	{
		*(SifuAngle_ADDR_BASE + 2*i) =  SendObj[i];
	}

/*
//	GPIO_OUT[0]  |=  0x00000001<<9;			//确保 GPIO9 的输出为 高 电平
	GPIO_OUT[0]  &=  (~(0x00000001<<9));	//确保GPIO9为 低 电平
	delay(5000);
	GPIO_OUT[0]  |=  0x00000001<<9;			//确保 GPIO9 的输出为 高 电平
*/
}

//  void TrackStop():发送命令至转台停止跟踪目标
void SiFuTrackStop()
{
	int i;
	int checkSum=0;
	unsigned char checkvalue;
 	unsigned char recTemp[12]={0};
 	unsigned short StopInfor[6]={0};
 	unsigned short*   SifuAngle_ADDR_BASE = (unsigned short*) 0x74000000;

    recTemp[0] = 0x08;
	recTemp[1] = 0xc0;
	recTemp[2] = 0xc0;
	recTemp[3] = 0x03;   // 发送数据位长度
	recTemp[4] = 0x03;   // 设备代码(转台控制器)
	recTemp[5] = 0x0A;   // 功能代码(跟踪模式)
	recTemp[6] = 0x55;   // 命令代码：停止跟踪
	for (i = 4; i < 7; i++)
	{
		checkSum += recTemp[i];
	}
	checkvalue = checkSum&0xff;
	recTemp[7] = checkvalue;
	recTemp[8] = 0xCF;
	for(i = 0; i < 6; i++)
	{
		StopInfor[i] =(recTemp[2*i+1]<<8) | recTemp[2*i];
	}
	for(i=0;i<6;i++)
	{
		*(SifuAngle_ADDR_BASE + 2*i)  = StopInfor[i];
	}
/*
    GPIO_OUT[0]  &=  (~(0x00000001<<9));	//确保GPIO9为 低 电平
	delay(10);
	GPIO_OUT[0]  |=  0x00000001<<9;			//确保 GPIO9 的输出为 高 电平
*/
}

// SendTargetInfoToPC():发送目标信息至上位机
void SendTargetInfoToPC()
{
    short FangWeiPianCha=0,FuYangPianCha=0;
	int i;
	int checkSum,kk=5; // kk=5表示从第(kk+1)个字节传目标位置信息，低字节在前，高字节在后
	unsigned char checkvalue,XLow,XHigh,YLow,YHigh;
 	unsigned char recTemp[20]={0},DetectLabel=0 ;
 	unsigned short SendObj[10]={0};
 	unsigned short*   SifuAngle_ADDR_BASE = (unsigned short*) 0x74000000;

    recTemp[0] = 0xc0;
	recTemp[1] = 0xc0;
	recTemp[2] = 0x06;   // 发送数据位长度
	recTemp[3] = 0x00; // 上位机设备代码

    if(g_iTrackObjCount == 1)
	{
	   FangWeiPianCha = LastRow+XiuZhengZhi_X;
	   FuYangPianCha = LastCol+XiuZhengZhi_Y;
	   lostnum=0;
	   DetectLabel=0x01;   // 检测到目标标志，上位机维持原命令不变
	}
	else
	{
	   FangWeiPianCha = 0x00;///若目标跟踪丢失，则发送脱靶量0
	   FuYangPianCha = 0x00;
	   if(lostnum>9)
	   {
	      DetectLabel=0x00;  // 连续10帧未检测到目标，则向上位机发送目标丢失状态字
		  lostnum=0;
	   }
	}
	recTemp[4] =  DetectLabel;

    XLow=  FangWeiPianCha & 0x000000ff;//目标方位低八位
	XHigh=(FangWeiPianCha & 0x0000ff00)>>8;//目标方位高八位
    YLow=  FuYangPianCha & 0x000000ff;//目标俯仰低八位
	YHigh=(FuYangPianCha & 0x000000ff00)>>8;//目标俯仰高八位

    if ((XLow==0xCF)||(XLow==0xC0)||(XLow==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=XLow;
      kk=kk+2;    // 出现遄址袅阶纸谖恢眯聪乱蛔纸谥?
	}
	else
    {
	   recTemp[kk]=XLow;
	   kk=kk+1;
    }

    if((XHigh==0xCF)||(XHigh==0xC0)||(XHigh==0xCA))
	{
	    recTemp[kk]=0xCA;
	    recTemp[kk+1]=XHigh;
        kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=XHigh;
	   kk=kk+1;
    }

     if ((YLow==0xCF)||(YLow==0xC0)||(YLow==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=YLow;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=YLow;
	   kk=kk+1;
    }

    if((YHigh==0xCF)||(YHigh==0xC0)||(YHigh==0xCA))
	{
	    recTemp[kk]=0xCA;
	    recTemp[kk+1]=YHigh;
        kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=YHigh;
	   kk=kk+1;
    }

	checkSum =  recTemp[3]+ recTemp[4]+XLow+XHigh+YLow+YHigh;
    checkvalue = checkSum&0xff;
    if((checkvalue==0xCF)||(checkvalue==0xC0)||(checkvalue==0xCA))
	{
	    recTemp[kk]=0xCA;
	    recTemp[kk+1]=checkvalue;
        kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=checkvalue;
	   kk=kk+1;
    }
    recTemp[kk]=0xCF;

    for(i = 0; i < 10; i++)
	{
		SendObj[i] =(recTemp[2*i+1]<<8) | recTemp[2*i];
	}

	for(i=0;i<100;i++)
	{
		*(SifuAngle_ADDR_BASE + 2*i) =  SendObj[i];
	}
/*
    GPIO_OUT[0]  &=  (~(0x00000001<<11));	//确保GPIO11为 低 电平
	delay(10);
	GPIO_OUT[0]  |=  0x00000001<<11;			//确保 GPIO11 的输出为  电平
	*/
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

/////////////////////////////////////////////////////////////////////////////////////
	    FPGA_Config_RAM[18]= 0x00000100;
		FPGA_Config_RAM[19]= 0x00000000;
	    FPGA_Config_RAM[12]=ObjCoordinate;
////////////////////////////////////////////////////////////////////////////////////////////
	}

}

// ClearSearchWindow():不显示搜索框
void ClearSearchWindow()
{
   FPGA_Config_RAM[18]= 0x00000000;
   FPGA_Config_RAM[19]= 0x00000000;
}

//  void SendTargetInfoToSiFu():将目标方位俯仰信息发送至转台
void SendTargetInfoToSiFu(short Y,short X,float Yzz,float Xzz)
{
    long FangWeiPianCha=0,FuYangPianCha=0;
	int i;
	int checkSum=0,kk=6;
 	unsigned char recTemp[48]={0} ;
 	unsigned short SendObj[24]={0};
	unsigned char checkvalue,X_1,X_2,X_3,X_4,Y_1,Y_2,Y_3,Y_4,Z_1,Z_2,Z_3,Z_4,L_1,L_2,L_3,L_4;
	unsigned short*   SifuAngle_ADDR_BASE = (unsigned short*) 0x74000000;
    FangWeiPianCha=(CALIBRATIONY-Y)*Yzz*1000000/512;
    FuYangPianCha=(CALIBRATIONX-(640-X))*Xzz*1000000/640;
    recTemp[1] = 0xc0;
    recTemp[2] = 0xc0;
    recTemp[3] = 0x12;   // 发送数据位长度
	recTemp[4] = 0x03;   // 转台设备码
	recTemp[5] = 0x17;   // 命令码，表示给转台发送角度信息

   // 脱靶量先发高位，再发低位
    X_4=  FangWeiPianCha & 0x000000ff;//目标方位低八位
	X_3= (FangWeiPianCha & 0x0000ff00)>>8;//目标方位次低八位
	X_2= (FangWeiPianCha & 0x00ff0000)>>16;// 目标方位次高八位
    X_1= (FangWeiPianCha & 0xff000000)>>24;// 目标方位高八位

    Y_4=  FuYangPianCha & 0x000000ff;//目标俯仰低八位
	Y_3= (FuYangPianCha & 0x0000ff00)>>8;//目标俯仰次低八位
	Y_2= (FuYangPianCha & 0x00ff0000)>>16;// 目标俯仰次高八位
    Y_1= (FuYangPianCha & 0xff000000)>>24;// 目标俯仰高八位


	Z_4=  SuDuXiuZheng_X & 0x000000ff;        // X方向速度修正值低八位
	Z_3= (SuDuXiuZheng_X & 0x0000ff00)>>8;    // X方向速度修正值次低八位
	Z_2= (SuDuXiuZheng_X & 0x00ff0000)>>16;   // X方向速度修正值次高八位
    Z_1= (SuDuXiuZheng_X & 0xff000000)>>24;   // X方向速度修正值高八位

	L_4=  SuDuXiuZheng_Y & 0x000000ff;        // Y方向速度修正值低八位
	L_3= (SuDuXiuZheng_Y & 0x0000ff00)>>8;    // Y方向速度修正值次低八位
	L_2= (SuDuXiuZheng_Y & 0x00ff0000)>>16;   // Y方向速度修正值次高八位
    L_1= (SuDuXiuZheng_Y & 0xff000000)>>24;   // Y方向速度修正值高八位

    if ((X_1==0xCF)||(X_1==0xC0)||(X_1==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=X_1;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=X_1;
	   kk=kk+1;
    }

	if ((X_2==0xCF)||(X_2==0xC0)||(X_2==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=X_2;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=X_2;
	   kk=kk+1;
    }

	if ((X_3==0xCF)||(X_3==0xC0)||(X_3==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=X_3;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=X_3;
	   kk=kk+1;
    }

    if((X_4==0xCF)||(X_4==0xC0)||(X_4==0xCA))
	{
	    recTemp[kk]=0xCA;
	    recTemp[kk+1]=X_4;
        kk=kk+2;  // 出现转义阶纸谖恢节值
	}
	else
    {
	   recTemp[kk]=X_4;
	   kk=kk+1;
    }
	 if ((Z_1==0xCF)||(Z_1==0xC0)||(Z_1==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=Z_1;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=Z_1;
	   kk=kk+1;
    }

	if ((Z_2==0xCF)||(Z_2==0xC0)||(Z_2==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=Z_2;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=Z_2;
	   kk=kk+1;
    }

	if ((Z_3==0xCF)||(Z_3==0xC0)||(Z_3==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=Z_3;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=Z_3;
	   kk=kk+1;
    }

    if((Z_4==0xCF)||(Z_4==0xC0)||(Z_4==0xCA))
	{
	    recTemp[kk]=0xCA;
	    recTemp[kk+1]=Z_4;
        kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=Z_4;
	   kk=kk+1;
    }


	if ((Y_1==0xCF)||(Y_1==0xC0)||(Y_1==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=Y_1;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=Y_1;
	   kk=kk+1;
    }

	if ((Y_2==0xCF)||(Y_2==0xC0)||(Y_2==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=Y_2;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=Y_2;
	   kk=kk+1;
    }

	if ((Y_3==0xCF)||(Y_3==0xC0)||(Y_3==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=Y_3;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=Y_3;
	   kk=kk+1;
    }

    if((Y_4==0xCF)||(Y_4==0xC0)||(Y_4==0xCA))
	{
	    recTemp[kk]=0xCA;
	    recTemp[kk+1]=Y_4;
        kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=Y_4;
	   kk=kk+1;
    }


	if ((L_1==0xCF)||(L_1==0xC0)||(L_1==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=L_1;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=L_1;
	   kk=kk+1;
    }

	if ((L_2==0xCF)||(L_2==0xC0)||(L_2==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=L_2;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=L_2;
	   kk=kk+1;
    }

	if ((L_3==0xCF)||(L_3==0xC0)||(L_3==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=L_3;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=L_3;
	   kk=kk+1;
    }

    if((L_4==0xCF)||(L_4==0xC0)||(L_4==0xCA))
	{
	    recTemp[kk]=0xCA;
	    recTemp[kk+1]=L_4;
        kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=L_4;
	   kk=kk+1;
    }

	checkSum = recTemp[4]+ recTemp[5]+X_1+X_2+X_3+X_4+Y_1+Y_2+Y_3+Y_4+Z_1+Z_2+Z_3+Z_4+L_1+L_2+L_3+L_4;
    checkvalue = checkSum&0xff;
    if((checkvalue==0xCF)||(checkvalue==0xC0)||(checkvalue==0xCA))
	{
	    recTemp[kk]=0xCA;
	    recTemp[kk+1]=checkvalue;
        kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=checkvalue;
	   kk=kk+1;
    }
    recTemp[kk]=0xCF;
    recTemp[0] =kk;
    for(i = 0; i < 24; i++)
	{
		SendObj[i] =(recTemp[2*i+1]<<8) | recTemp[2*i];
	}
	for(i=0;i<24;i++)
	{
		*(SifuAngle_ADDR_BASE + 2*i) =  SendObj[i];
	}

    /*COM_RAM_T1[0]=0x12C0C017;
	COM_RAM_T1[1]=0xFBFF1705;
	COM_RAM_T1[2]=0x3101D47D;
	COM_RAM_T1[3]=0x0400002D;
	COM_RAM_T1[4]=0x3101E20F;
	COM_RAM_T1[5]=0xCF1A002D;*/


//	GPIO_OUT[0]  |=  0x00000001<<9;			//确保 GPIO9 的输出为 高 电平
/*
	GPIO_OUT[0]  &=  (~(0x00000001<<9));	//确保GPIO9为 低 电平
	delay(5000);
	GPIO_OUT[0]  |=  0x00000001<<9;			//确保 GPIO9 的输出为 高 电平
	*/

}

/////////////////////////////////////////////////////////////////////////////
//传送告警信息给上位机
//////////////////////////////////////////////////////////////////////////////

void  SendAlarmTarInfoToPC()
{

    int FangWeiPianCha=0,FuYangPianCha=0;
	int i;
	int checkSum,kk=4; // kk=5表示从第(kk+1)个字节传目标位置信息，低字节在前，高字节在后
	unsigned char checkvalue,X1,X2,X3,X4,Y1,Y2,Y3,Y4,XLow,XHigh,YLow,YHigh,CodeH,CodeL,FrameH,FrameL;
 	unsigned char* recTemp=(unsigned char*) 0x008C2000;
    unsigned char TarNum;
    for(i=0;i<1000;i++)
	  recTemp[i]=0;
    if(m_PossTarNum>12)
	  TarNum=0x0C;
	else
	  TarNum=m_PossTarNum;
    recTemp[0] = 0xc0;
	recTemp[1] = 0xc0;
	recTemp[2] = TarNum;   // 发送目标信息个数
	recTemp[3] = 0x00;     // 上位机设备代码
 	checkSum =  recTemp[3];
    for(i=0;i<TarNum;i++)
	 {

        X1  =  (g_Candidate[i].s_C_Col & 0x000000ff);
        X2  =  (g_Candidate[i].s_C_Col & 0x0000ff00)>>8;
		X3  =  (g_Candidate[i].s_C_Col & 0x00ff0000)>>16;
		X4  =  (g_Candidate[i].s_C_Col & 0xff000000)>>24;//目标方位角

        Y1  =  (g_Candidate[i].s_C_Row & 0x000000ff);
        Y2  =  (g_Candidate[i].s_C_Row & 0x0000ff00)>>8;
		Y3  =  (g_Candidate[i].s_C_Row & 0x00ff0000)>>16;
		Y4  =  (g_Candidate[i].s_C_Row & 0xff000000)>>24; //目标俯仰角


        XLow =((g_Candidate[i].s_C_fuyangPixel+XiuZhengZhi_Y) & 0x000000ff);//像素方位低八位
	    XHigh=((g_Candidate[i].s_C_fuyangPixel+XiuZhengZhi_Y) & 0x0000ff00)>>8;//像素方位高八位
        YLow =((640-(g_Candidate[i].s_C_fangweiPixel+XiuZhengZhi_X)) & 0x000000ff);//像素方位低八位
	    YHigh=((640-(g_Candidate[i].s_C_fangweiPixel+XiuZhengZhi_Y)) & 0x0000ff00)>>8;//像素方位高八位

        CodeL =(0xAA  & 0x000000ff);//批号低八位
	    CodeH =(0xAA  & 0x0000ff00)>>8;//批号高八位
        FrameL=(g_Candidate[i].Frame_Code   & 0x000000ff);//帧号低八位
	    FrameH=(g_Candidate[i].Frame_Code   & 0x0000ff00)>>8;//帧号高八位

        if ((CodeL==0xCF)||(CodeL==0xC0)||(CodeL==0xCA))
         {
	        recTemp[kk]=0xCA;
	        recTemp[kk+1]=CodeL;
            kk=kk+2;    //
	     }
	    else
         {
	        recTemp[kk]=CodeL;
	        kk=kk+1;
         }
        if ((CodeH==0xCF)||(CodeH==0xC0)||(CodeH==0xCA))
         {
	        recTemp[kk]=0xCA;
	        recTemp[kk+1]=CodeH;
            kk=kk+2;    //
	     }
	    else
         {
	        recTemp[kk]=CodeH;
	        kk=kk+1;
         }

        if ((FrameL==0xCF)||(FrameL==0xC0)||(FrameL==0xCA))
         {
	        recTemp[kk]=0xCA;
	        recTemp[kk+1]=FrameL;
            kk=kk+2;    //
	     }
	    else
         {
	        recTemp[kk]=FrameL;
	        kk=kk+1;
         }
        if ((FrameH==0xCF)||(FrameH==0xC0)||(FrameH==0xCA))
         {
	        recTemp[kk]=0xCA;
	        recTemp[kk+1]=FrameH;
            kk=kk+2;    //
	     }
	    else
         {
	        recTemp[kk]=FrameH;
	        kk=kk+1;
         }

        if ((XLow==0xCF)||(XLow==0xC0)||(XLow==0xCA))
         {
	        recTemp[kk]=0xCA;
	        recTemp[kk+1]=XLow;
            kk=kk+2;    //
	     }
	    else
         {
	        recTemp[kk]=XLow;
	        kk=kk+1;
         }

        if((XHigh==0xCF)||(XHigh==0xC0)||(XHigh==0xCA))
	     {
	        recTemp[kk]=0xCA;
	        recTemp[kk+1]=XHigh;
            kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	     }
	    else
         {
	        recTemp[kk]=XHigh;
	        kk=kk+1;
         }

        if ((YLow==0xCF)||(YLow==0xC0)||(YLow==0xCA))
	     {
	        recTemp[kk]=0xCA;
	        recTemp[kk+1]=YLow;
            kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	     }
	    else
         {
	        recTemp[kk]=YLow;
	        kk=kk+1;
         }

        if((YHigh==0xCF)||(YHigh==0xC0)||(YHigh==0xCA))
	     {
	        recTemp[kk]=0xCA;
	        recTemp[kk+1]=YHigh;
            kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	     }
	    else
         {
	        recTemp[kk]=YHigh;
	        kk=kk+1;
         }
		if ((X1==0xCF)||(X1==0xC0)||(X1==0xCA))
         {
	        recTemp[kk]=0xCA;
	        recTemp[kk+1]=X1;
            kk=kk+2;      // 出
	     }
	    else
         {
	        recTemp[kk]=X1;
	        kk=kk+1;
         }

        if ((X2==0xCF)||(X2==0xC0)||(X2==0xCA))
         {
	        recTemp[kk]=0xCA;
	        recTemp[kk+1]=X2;
            kk=kk+2;    //
	     }
	    else
         {
	        recTemp[kk]=X2;
	        kk=kk+1;
         }

        if ((X3==0xCF)||(X3==0xC0)||(X3==0xCA))
         {
	        recTemp[kk]=0xCA;
	        recTemp[kk+1]=X3;
            kk=kk+2;    //
	     }
	    else
         {
	        recTemp[kk]=X3;
	        kk=kk+1;
         }
        if ((X4==0xCF)||(X4==0xC0)||(X4==0xCA))
         {
	        recTemp[kk]=0xCA;
	        recTemp[kk+1]=X4;
            kk=kk+2;    //
	     }
	    else
         {
	        recTemp[kk]=X4;
	        kk=kk+1;
         }
		if ((Y1==0xCF)||(Y1==0xC0)||(Y1==0xCA))
         {
	        recTemp[kk]=0xCA;
	        recTemp[kk+1]=Y1;
            kk=kk+2;      //
	     }
	    else
         {
	        recTemp[kk]=Y1;
	        kk=kk+1;
         }

        if ((Y2==0xCF)||(Y2==0xC0)||(Y2==0xCA))
         {
	        recTemp[kk]=0xCA;
	        recTemp[kk+1]=Y2;
            kk=kk+2;    //
	     }
	    else
         {
	        recTemp[kk]=Y2;
	        kk=kk+1;
         }

        if ((Y3==0xCF)||(Y3==0xC0)||(Y3==0xCA))
         {
	        recTemp[kk]=0xCA;
	        recTemp[kk+1]=Y3;
            kk=kk+2;    //
	     }
	    else
         {
	        recTemp[kk]=Y3;
	        kk=kk+1;
         }
        if ((Y4==0xCF)||(Y4==0xC0)||(Y4==0xCA))
         {
	        recTemp[kk]=0xCA;
	        recTemp[kk+1]=Y4;
            kk=kk+2;    //
	     }
	    else
         {
	        recTemp[kk]=Y4;
	        kk=kk+1;
         }

    	checkSum =  X1+X2+X3+X4+Y1+Y2+Y3+Y4+XLow+XHigh+YLow+YHigh+CodeH+CodeL+FrameH+FrameL;
	  }
    checkvalue = checkSum & 0xff;
    if((checkvalue==0xCF)||(checkvalue==0xC0)||(checkvalue==0xCA))
	{
	    recTemp[kk]=0xCA;
	    recTemp[kk+1]=checkvalue;
        kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=checkvalue;
	   kk=kk+1;
    }
    recTemp[kk]=0xCF;

/*    for(i = 0; i < 250; i++)
	{
		Send_OB[i] =(recTemp[4*i+3]<<24)|(recTemp[4*i+2]<<16)|(recTemp[4*i+1]<<8) | recTemp[4*i];
	}*/

	for(i=0;i<60;i++)
	{
	  	COM_RAM_T0[i] = (recTemp[4*i+3]<<24)|(recTemp[4*i+2]<<16)|(recTemp[4*i+1]<<8) | recTemp[4*i];
	}
/*
    GPIO_OUT[0]  &=  (~(0x00000001<<11));	//确保GPIO11为 低 电平
	delay(10);
	GPIO_OUT[0]  |=  0x00000001<<11;			//确保 GPIO11 的输出为  电平
*/
}


/*函数作用：ClearTrackWindow() 画跟踪框，配置FPGA_Config_RAM[18]
*/
void ClearTrackWindow()
{

    FPGA_Config_RAM[18]= 0x00000000;
    FPGA_Config_RAM[19]= 0x00000000;
}


// DrawSearchWindow():画搜索框，搜索框大小为128*128，位于视场中心
void DrawSearchWindow()
{
  /* if(g_ReceiveCommand==SearchTrackInGate)
   {
      FPGA_Config_RAM[18]= 0x00000000;
      FPGA_Config_RAM[19]= 0x00010000;
   }
   else
   {
      FPGA_Config_RAM[18]= 0x00000000;
      FPGA_Config_RAM[19]= 0x00000000;
   }*/
    FPGA_Config_RAM[18]= 0x00000000;
    FPGA_Config_RAM[19]= 0x00010000;
}

void  SiFuSearchStop()
{
 	int i,fangweisudu;
	int checkSum=0,kk=6;
 	unsigned char recTemp[48]={0} ;
 	unsigned int SendObj[12]={0};
	unsigned char checkvalue,X_1,X_2,X_3,X_4,Y_1,Y_2,Y_3,Y_4,Vx_1,Vx_2,Vx_3,Vx_4,Vy_1,Vy_2,Vy_3,Vy_4;
    recTemp[1] = 0xc0;
    recTemp[2] = 0xc0;
    recTemp[3] = 0x0A;   // 发送数据位长度
	recTemp[4] = 0x03;   // 转台设备码
	recTemp[5] = 0x10;   // 命令码，表示给转台发送停止搜索命令

	fangweisudu=300000000;
   // 脱靶量先发高位，再发低位
    X_4=  0x00000000 & 0x000000ff;//目标方位低八位
	X_3= (0x00000000 & 0x0000ff00)>>8;//目标方位次低八位
	X_2= (0x00000000 & 0x00ff0000)>>16;// 目标方位次高八位
    X_1= (0x00000000 & 0xff000000)>>24;// 目标方位高八位

    Y_4=  0x00000000 & 0x000000ff;//目标俯仰低八位
	Y_3= (0x00000000 & 0x0000ff00)>>8;//目标俯仰次低八位
	Y_2= (0x00000000 & 0x00ff0000)>>16;// 目标俯仰次高八位
    Y_1= (0x00000000 & 0xff000000)>>24;// 目标俯仰高八位

    if ((X_1==0xCF)||(X_1==0xC0)||(X_1==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=X_1;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=X_1;
	   kk=kk+1;
    }

	if ((X_2==0xCF)||(X_2==0xC0)||(X_2==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=X_2;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=X_2;
	   kk=kk+1;
    }

	if ((X_3==0xCF)||(X_3==0xC0)||(X_3==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=X_3;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=X_3;
	   kk=kk+1;
    }

    if((X_4==0xCF)||(X_4==0xC0)||(X_4==0xCA))
	{
	    recTemp[kk]=0xCA;
	    recTemp[kk+1]=X_4;
        kk=kk+2;  // 出转义字符，两字谖恢眯聪乱唤谥?
	}
	else
    {
	   recTemp[kk]=X_4;
	   kk=kk+1;
    }
	if ((Y_1==0xCF)||(Y_1==0xC0)||(Y_1==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=Y_1;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=Y_1;
	   kk=kk+1;
    }

	if ((Y_2==0xCF)||(Y_2==0xC0)||(Y_2==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=Y_2;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=Y_2;
	   kk=kk+1;
    }

	if ((Y_3==0xCF)||(Y_3==0xC0)||(Y_3==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=Y_3;
      kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=Y_3;
	   kk=kk+1;
    }

    if((Y_4==0xCF)||(Y_4==0xC0)||(Y_4==0xCA))
	{
	    recTemp[kk]=0xCA;
	    recTemp[kk+1]=Y_4;
        kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=Y_4;
	   kk=kk+1;
    }


	checkSum = recTemp[4]+ recTemp[5]+X_1+X_2+X_3+X_4+Y_1+Y_2+Y_3+Y_4;
    checkvalue = checkSum&0xff;
    if((checkvalue==0xCF)||(checkvalue==0xC0)||(checkvalue==0xCA))
	{
	    recTemp[kk]=0xCA;
	    recTemp[kk+1]=checkvalue;
        kk=kk+2;  // 出现转义字符，隔两字节位置写下一字节值
	}
	else
    {
	   recTemp[kk]=checkvalue;
	   kk=kk+1;
    }
    recTemp[kk]=0xCF;
    recTemp[0] =kk;
    for(i = 0; i < 12; i++)
	{
		SendObj[i] =(recTemp[4*i+3]<<24)|(recTemp[4*i+2]<<16)|(recTemp[4*i+1]<<8) | recTemp[4*i];
	}
	for(i=0;i<12;i++)
	{
	  	COM_RAM_T1[i] = SendObj[i];
	}

	/*
//	GPIO_OUT[0]  |=  0x00000001<<9;			//确保 GPIO9 的输出为 高 电平
	GPIO_OUT[0]  &=  (~(0x00000001<<9));	//确保GPIO9为 低 电平
	delay(5000);
	GPIO_OUT[0]  |=  0x00000001<<9;			//确保 GPIO9 的输出为 高 电平

	*/
}



