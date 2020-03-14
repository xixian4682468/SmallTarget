#include"Communication.h"
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
extern unsigned char           StartSearchLabel;
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
    recTemp[3] = 0x02;   // ��������λ����
	recTemp[4] = 0x03;   // ת̨�豸��
	recTemp[5] = 0x21;   // �����룬��ʾ��ת̨���ͽǶ���Ϣ

	checkSum = recTemp[4]+ recTemp[5];
    checkvalue = checkSum&0xff;
    if((checkvalue==0xCF)||(checkvalue==0xC0)||(checkvalue==0xCA))
	{
	    recTemp[kk]=0xCA;
	    recTemp[kk+1]=checkvalue;
        kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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

//	GPIO_OUT[0]  |=  0x00000001<<9;			//ȷ�� GPIO9 �����Ϊ �� ��ƽ
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//����ת̨��ת�ĽǶ�ֵ
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
	   //��ת̨��lx
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

     //��ת̨��ly
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

	//��ת̨��lz
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
///��У����
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
		YouChengOrImage=0;
		StartSearchLabel=1;
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
        StartSearchLabel=0;
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
	recTemp[3] = 0x03;   // ��������λ����
	recTemp[4] = 0x03;   // �豸����(ת̨������)
	recTemp[5] = 0x0A;   // ���ܴ���(����ģʽ)
	recTemp[6] = 0xAA;   // �����룺��ʼ���?
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
   /* GPIO_OUT[0]  &=  (~(0x00000001<<9));	//ȷ��GPIO9Ϊ �� ��ƽ
	delay(10);
	GPIO_OUT[0]  |=  0x00000001<<9;			//ȷ�� GPIO9 �����Ϊ �� ��ƽ*/
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
    recTemp[3] = 0x0A;   // ��������λ����
	recTemp[4] = 0x03;   // ת̨�豸��
	recTemp[5] = 0x10;   // �����룬��ʾ��ת̨�����������ٶ�300��/s

	fangweisudu=-300000000;
   // �Ѱ����ȷ���λ���ٷ���λ
    X_4=  fangweisudu & 0x000000ff;//Ŀ�귽λ�Ͱ�λ
	X_3= (fangweisudu & 0x0000ff00)>>8;//Ŀ�귽λ�εͰ�λ
	X_2= (fangweisudu & 0x00ff0000)>>16;// Ŀ�귽λ�θ߰�λ
    X_1= (fangweisudu & 0xff000000)>>24;// Ŀ�귽λ�߰�λ

    Y_4=  0x00000000 & 0x000000ff;//Ŀ�긩���Ͱ�λ
	Y_3= (0x00000000 & 0x0000ff00)>>8;//Ŀ�긩���εͰ�λ
	Y_2= (0x00000000 & 0x00ff0000)>>16;// Ŀ�긩���θ߰�λ
    Y_1= (0x00000000 & 0xff000000)>>24;// Ŀ�긩���߰�λ

    if ((X_1==0xCF)||(X_1==0xC0)||(X_1==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=X_1;
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
        kk=kk+2;  // ��ת���ַ��������λ��д��һ���?
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
        kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
        kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
//	GPIO_OUT[0]  |=  0x00000001<<9;			//ȷ�� GPIO9 �����Ϊ �� ��ƽ
	GPIO_OUT[0]  &=  (~(0x00000001<<9));	//ȷ��GPIO9Ϊ �� ��ƽ
	delay(5000);
	GPIO_OUT[0]  |=  0x00000001<<9;			//ȷ�� GPIO9 �����Ϊ �� ��ƽ
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
    recTemp[3] = 0x0A;   // ��������λ����
	recTemp[4] = 0x03;   // ת̨�豸��
	recTemp[5] = 0x13;   // �����룬��ʾ��ת̨���ͽǶ���Ϣ

   // �Ѱ����ȷ���λ���ٷ���λ
    X_4=  0x80000000 & 0x000000ff;//Ŀ�귽λ�Ͱ�λ
	X_3= (0x80000000 & 0x0000ff00)>>8;//Ŀ�귽λ�εͰ�λ
	X_2= (0x80000000 & 0x00ff0000)>>16;// Ŀ�귽λ�θ߰�λ
    X_1= (0x80000000 & 0xff000000)>>24;// Ŀ�귽λ�߰�λ

    Y_4=  Fuyangjiao & 0x000000ff;//Ŀ�긩���Ͱ�λ
	Y_3= (Fuyangjiao & 0x0000ff00)>>8;//Ŀ�긩���εͰ�λ
	Y_2= (Fuyangjiao & 0x00ff0000)>>16;// Ŀ�긩���θ߰�λ
    Y_1= (Fuyangjiao & 0xff000000)>>24;// Ŀ�긩���߰�λ

    if ((X_1==0xCF)||(X_1==0xC0)||(X_1==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=X_1;
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
        kk=kk+2;  // ��ת���ַ��������λ��д��һ���?
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
        kk=kk+2;  // ���ת���ַ�����׽�λ��д��һ�ֽ�ֵ
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
        kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
//	GPIO_OUT[0]  |=  0x00000001<<9;			//ȷ�� GPIO9 �����Ϊ �� ��ƽ
	GPIO_OUT[0]  &=  (~(0x00000001<<9));	//ȷ��GPIO9Ϊ �� ��ƽ
	delay(5000);
	GPIO_OUT[0]  |=  0x00000001<<9;			//ȷ�� GPIO9 �����Ϊ �� ��ƽ
*/
}

//  void TrackStop():����������תֹ̨ͣ����Ŀ��
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
	recTemp[3] = 0x03;   // ��������λ����
	recTemp[4] = 0x03;   // �豸����(ת̨������)
	recTemp[5] = 0x0A;   // ���ܴ���(����ģʽ)
	recTemp[6] = 0x55;   // ������룺ֹͣ����
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
    GPIO_OUT[0]  &=  (~(0x00000001<<9));	//ȷ��GPIO9Ϊ �� ��ƽ
	delay(10);
	GPIO_OUT[0]  |=  0x00000001<<9;			//ȷ�� GPIO9 �����Ϊ �� ��ƽ
*/
}

// SendTargetInfoToPC():����Ŀ����Ϣ����λ��
void SendTargetInfoToPC()
{
    short FangWeiPianCha=0,FuYangPianCha=0;
	int i;
	int checkSum,kk=5; // kk=5��ʾ�ӵ�(kk+1)���ֽڴ�Ŀ��λ����Ϣ�����ֽ���ǰ�����ֽ��ں�
	unsigned char checkvalue,XLow,XHigh,YLow,YHigh;
 	unsigned char recTemp[20]={0},DetectLabel=0 ;
 	unsigned short SendObj[10]={0};
 	unsigned short*   SifuAngle_ADDR_BASE = (unsigned short*) 0x74000000;

    recTemp[0] = 0xc0;
	recTemp[1] = 0xc0;
	recTemp[2] = 0x06;   // ��������λ����
	recTemp[3] = 0x00; // ��λ���豸����

    if(g_iTrackObjCount == 1)
	{
	   FangWeiPianCha = LastRow+XiuZhengZhi_X;
	   FuYangPianCha = LastCol+XiuZhengZhi_Y;
	   lostnum=0;
	   DetectLabel=0x01;   // ��⵽Ŀ���־����λ��ά��ԭ�����
	}
	else
	{
	   FangWeiPianCha = 0x00;///��Ŀ����ٶ�ʧ�������Ѱ���0
	   FuYangPianCha = 0x00;
	   if(lostnum>9)
	   {
	      DetectLabel=0x00;  // ����10֡δ��⵽Ŀ�꣬������λ������Ŀ�궪ʧ״̬��
		  lostnum=0;
	   }
	}
	recTemp[4] =  DetectLabel;

    XLow=  FangWeiPianCha & 0x000000ff;//Ŀ�귽λ�Ͱ�λ
	XHigh=(FangWeiPianCha & 0x0000ff00)>>8;//Ŀ�귽λ�߰�λ
    YLow=  FuYangPianCha & 0x000000ff;//Ŀ�긩���Ͱ�λ
	YHigh=(FuYangPianCha & 0x000000ff00)>>8;//Ŀ�긩���߰�λ

    if ((XLow==0xCF)||(XLow==0xC0)||(XLow==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=XLow;
      kk=kk+2;    // ������ַ����ֽ�λ��д��һ�ֽ��?
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
        kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
        kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
        kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
    GPIO_OUT[0]  &=  (~(0x00000001<<11));	//ȷ��GPIO11Ϊ �� ��ƽ
	delay(10);
	GPIO_OUT[0]  |=  0x00000001<<11;			//ȷ�� GPIO11 �����Ϊ  ��ƽ
	*/
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

/////////////////////////////////////////////////////////////////////////////////////
	    FPGA_Config_RAM[18]= 0x00000100;
		FPGA_Config_RAM[19]= 0x00000000;
	    FPGA_Config_RAM[12]=ObjCoordinate;
////////////////////////////////////////////////////////////////////////////////////////////
	}

}

// ClearSearchWindow():����ʾ������
void ClearSearchWindow()
{
   FPGA_Config_RAM[18]= 0x00000000;
   FPGA_Config_RAM[19]= 0x00000000;
}

//  void SendTargetInfoToSiFu():��Ŀ�귽λ������Ϣ������ת̨
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
    recTemp[3] = 0x12;   // ��������λ����
	recTemp[4] = 0x03;   // ת̨�豸��
	recTemp[5] = 0x17;   // �����룬��ʾ��ת̨���ͽǶ���Ϣ

   // �Ѱ����ȷ���λ���ٷ���λ
    X_4=  FangWeiPianCha & 0x000000ff;//Ŀ�귽λ�Ͱ�λ
	X_3= (FangWeiPianCha & 0x0000ff00)>>8;//Ŀ�귽λ�εͰ�λ
	X_2= (FangWeiPianCha & 0x00ff0000)>>16;// Ŀ�귽λ�θ߰�λ
    X_1= (FangWeiPianCha & 0xff000000)>>24;// Ŀ�귽λ�߰�λ

    Y_4=  FuYangPianCha & 0x000000ff;//Ŀ�긩���Ͱ�λ
	Y_3= (FuYangPianCha & 0x0000ff00)>>8;//Ŀ�긩���εͰ�λ
	Y_2= (FuYangPianCha & 0x00ff0000)>>16;// Ŀ�긩���θ߰�λ
    Y_1= (FuYangPianCha & 0xff000000)>>24;// Ŀ�긩���߰�λ


	Z_4=  SuDuXiuZheng_X & 0x000000ff;        // X�����ٶ�����ֵ�Ͱ�λ
	Z_3= (SuDuXiuZheng_X & 0x0000ff00)>>8;    // X�����ٶ�����ֵ�εͰ�λ
	Z_2= (SuDuXiuZheng_X & 0x00ff0000)>>16;   // X�����ٶ�����ֵ�θ߰�λ
    Z_1= (SuDuXiuZheng_X & 0xff000000)>>24;   // X�����ٶ�����ֵ�߰�λ

	L_4=  SuDuXiuZheng_Y & 0x000000ff;        // Y�����ٶ�����ֵ�Ͱ�λ
	L_3= (SuDuXiuZheng_Y & 0x0000ff00)>>8;    // Y�����ٶ�����ֵ�εͰ�λ
	L_2= (SuDuXiuZheng_Y & 0x00ff0000)>>16;   // Y�����ٶ�����ֵ�θ߰�λ
    L_1= (SuDuXiuZheng_Y & 0xff000000)>>24;   // Y�����ٶ�����ֵ�߰�λ

    if ((X_1==0xCF)||(X_1==0xC0)||(X_1==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=X_1;
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
        kk=kk+2;  // ����ת�����ֽ�λֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
        kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
        kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
        kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
        kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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


//	GPIO_OUT[0]  |=  0x00000001<<9;			//ȷ�� GPIO9 �����Ϊ �� ��ƽ
/*
	GPIO_OUT[0]  &=  (~(0x00000001<<9));	//ȷ��GPIO9Ϊ �� ��ƽ
	delay(5000);
	GPIO_OUT[0]  |=  0x00000001<<9;			//ȷ�� GPIO9 �����Ϊ �� ��ƽ
	*/

}

/////////////////////////////////////////////////////////////////////////////
//���͸澯��Ϣ����λ��
//////////////////////////////////////////////////////////////////////////////

void  SendAlarmTarInfoToPC()
{

    int FangWeiPianCha=0,FuYangPianCha=0;
	int i;
	int checkSum,kk=4; // kk=5��ʾ�ӵ�(kk+1)���ֽڴ�Ŀ��λ����Ϣ�����ֽ���ǰ�����ֽ��ں�
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
	recTemp[2] = TarNum;   // ����Ŀ����Ϣ����
	recTemp[3] = 0x00;     // ��λ���豸����
 	checkSum =  recTemp[3];
    for(i=0;i<TarNum;i++)
	 {

        X1  =  (g_Candidate[i].s_C_Col & 0x000000ff);
        X2  =  (g_Candidate[i].s_C_Col & 0x0000ff00)>>8;
		X3  =  (g_Candidate[i].s_C_Col & 0x00ff0000)>>16;
		X4  =  (g_Candidate[i].s_C_Col & 0xff000000)>>24;//Ŀ�귽λ��

        Y1  =  (g_Candidate[i].s_C_Row & 0x000000ff);
        Y2  =  (g_Candidate[i].s_C_Row & 0x0000ff00)>>8;
		Y3  =  (g_Candidate[i].s_C_Row & 0x00ff0000)>>16;
		Y4  =  (g_Candidate[i].s_C_Row & 0xff000000)>>24; //Ŀ�긩����


        XLow =((g_Candidate[i].s_C_fuyangPixel+XiuZhengZhi_Y) & 0x000000ff);//���ط�λ�Ͱ�λ
	    XHigh=((g_Candidate[i].s_C_fuyangPixel+XiuZhengZhi_Y) & 0x0000ff00)>>8;//���ط�λ�߰�λ
        YLow =((640-(g_Candidate[i].s_C_fangweiPixel+XiuZhengZhi_X)) & 0x000000ff);//���ط�λ�Ͱ�λ
	    YHigh=((640-(g_Candidate[i].s_C_fangweiPixel+XiuZhengZhi_Y)) & 0x0000ff00)>>8;//���ط�λ�߰�λ

        CodeL =(0xAA  & 0x000000ff);//���ŵͰ�λ
	    CodeH =(0xAA  & 0x0000ff00)>>8;//���Ÿ߰�λ
        FrameL=(g_Candidate[i].Frame_Code   & 0x000000ff);//֡�ŵͰ�λ
	    FrameH=(g_Candidate[i].Frame_Code   & 0x0000ff00)>>8;//֡�Ÿ߰�λ

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
            kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
            kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
            kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
            kk=kk+2;      // ��
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
        kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
    GPIO_OUT[0]  &=  (~(0x00000001<<11));	//ȷ��GPIO11Ϊ �� ��ƽ
	delay(10);
	GPIO_OUT[0]  |=  0x00000001<<11;			//ȷ�� GPIO11 �����Ϊ  ��ƽ
*/
}


/*�������ã�ClearTrackWindow() �����ٿ�����FPGA_Config_RAM[18]
*/
void ClearTrackWindow()
{

    FPGA_Config_RAM[18]= 0x00000000;
    FPGA_Config_RAM[19]= 0x00000000;
}


// DrawSearchWindow():���������������СΪ128*128��λ���ӳ�����
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
    recTemp[3] = 0x0A;   // ��������λ����
	recTemp[4] = 0x03;   // ת̨�豸��
	recTemp[5] = 0x10;   // �����룬��ʾ��ת̨����ֹͣ��������

	fangweisudu=300000000;
   // �Ѱ����ȷ���λ���ٷ���λ
    X_4=  0x00000000 & 0x000000ff;//Ŀ�귽λ�Ͱ�λ
	X_3= (0x00000000 & 0x0000ff00)>>8;//Ŀ�귽λ�εͰ�λ
	X_2= (0x00000000 & 0x00ff0000)>>16;// Ŀ�귽λ�θ߰�λ
    X_1= (0x00000000 & 0xff000000)>>24;// Ŀ�귽λ�߰�λ

    Y_4=  0x00000000 & 0x000000ff;//Ŀ�긩���Ͱ�λ
	Y_3= (0x00000000 & 0x0000ff00)>>8;//Ŀ�긩���εͰ�λ
	Y_2= (0x00000000 & 0x00ff0000)>>16;// Ŀ�긩���θ߰�λ
    Y_1= (0x00000000 & 0xff000000)>>24;// Ŀ�긩���߰�λ

    if ((X_1==0xCF)||(X_1==0xC0)||(X_1==0xCA))
	{
	  recTemp[kk]=0xCA;
	  recTemp[kk+1]=X_1;
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
        kk=kk+2;  // ��ת���ַ��������λ��д��һ���?
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
      kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
        kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
        kk=kk+2;  // ����ת���ַ��������ֽ�λ��д��һ�ֽ�ֵ
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
//	GPIO_OUT[0]  |=  0x00000001<<9;			//ȷ�� GPIO9 �����Ϊ �� ��ƽ
	GPIO_OUT[0]  &=  (~(0x00000001<<9));	//ȷ��GPIO9Ϊ �� ��ƽ
	delay(5000);
	GPIO_OUT[0]  |=  0x00000001<<9;			//ȷ�� GPIO9 �����Ϊ �� ��ƽ

	*/
}



