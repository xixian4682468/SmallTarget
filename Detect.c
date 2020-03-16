#include"Detect.h"
#include   "struct.h"
#include"Communication.h"
#include "Feature.h"
//#include "Lable.h"


TARFEATURE 		g_ZeroFeat;
TARFEATURE      *tempFeat        = (TARFEATURE *)(CHAIN_ADDR+0x000012C0);
TARFEATURE 		*g_sel_Candidate = (TARFEATURE *)(0x0c230892);	//len:300*7*4 0x20d0
TARFEATURE     	*CandObject    	= (TARFEATURE *)(OBJ_ADDR);
TARFEATURE     	*Candidate     	= (TARFEATURE *)(OBJ_ADDR+0x00002BC0);
TARFEATURE 		*g_Candidate 	 = (TARFEATURE *)(CHAIN_ADDR);			//len:300*7*4		  存放300个目标，直接阈值

unsigned short	*block        	= (unsigned short*)0x0c250000;//(6000*2*4=0xBB80)
unsigned short	*linsum        	= (unsigned short*)(0x0c2A1800);//
unsigned short	*M        	    = (unsigned short*)(0x0c2A0000);//(3000*2=0X1770,可存3000个目标面积)
unsigned int	*posxy        	= (unsigned int*)0x0c28BB80;//(3000*2*8=0xBB80)
unsigned char   *pSrc1  ;

volatile int	 m_PossTarNum,m_CandTarNum,RealTarNum;
unsigned short	 g_sendPCFangWeiAngle;						//发送到上位机的方位角
unsigned short	 g_sendPCFuYangAngle;						//发送到上位机俯仰角
unsigned char  	 g_bSearchInTrackState = 1;
unsigned short	 SegNum_CurLine,SegNum_LastLine,Num_Seg,LabNum;
volatile int     m_ThreshMax,m_ThreshMean,m_ThreshCount,m_ThreshVar,m_Threshold=0x3fff;
unsigned short   *NeiTable0     = (unsigned short  *)(SEG_ADDR+0x00008CA0);
unsigned short   *NeiTable1     = (unsigned short  *)(SEG_ADDR+0x00009858);
unsigned short   *MapTable      = (unsigned short  *)(SEG_ADDR+0x0000A410);
unsigned short			NeiTabLeng,True_Obj;


extern RUNLENGTHCODE    *Seg_Code;
extern volatile int 			Seg_Count;
extern unsigned short          FPGA_Frame_Num,Pro_Frame_Num;
extern int          			g_fangweijiao;
extern int          			g_fuyangjiao;
extern int						g_ReceiveCommand;
extern float                   XFov,YFov;                 // 默认为大视场
extern volatile int        	XiuZhengZhi_X,XiuZhengZhi_Y,m_GuideState,StoreFrame;
extern volatile unsigned char     *pmoban;


void AutoTarDetect(unsigned short StartRow,unsigned short EndRow,unsigned short StartCol,unsigned short EndCol)
{
	unsigned short label;
	unsigned short start,stop;
	int temp;
	int i,j,k,l,m;
	char repeat;
	int maxTarNum=-1;

	block[0]=0;
	block[1]=0;
	k=0;

	if(Seg_Count==0)
	{
		maxTarNum=0;
	}
	if(Seg_Count>3000)
	{
		Seg_Count=3000;
	}

	for(j=0;j<512;j++)                //     512行 初始化
	{
		linsum[j]=0;                                   //  确定地址空间？
	}
	for(i=0;i<3000;i++)               //     初始化3000个目标
	{
		block[i*4]=0;                   //  确定地址空间？
		block[i*4+1]=0;
		block[i*4+2]=0;
		block[i*4+3]=0;
		M[i]=0;                                        //  确定地址空间？
	}
	for(i = 0;i < Seg_Count;i++)    //每个段对应一个block
	{
		linsum[Seg_Code[i].Row]++;                 // 每行游程段数数统计，一段加1
		block[i*4]=(short)Seg_Code[i].Row;         //
		block[i*4+1]=(short)Seg_Code[i].CStart;
		block[i*4+2]=(short)Seg_Code[i].CEnd;
		block[i*4+3]=0;
	}
	if(Seg_Count != 0)
	{
		label=1;
		block[3]=1;
		j=512;
		for(i=0;i<512;i++)
		{
			if((linsum[i]>0))    //搜索第一个有游程的行
			{
				j=i;   //用j记录该行
				break;
			}
		}
		for(i=1;i<linsum[j];i++)    //统计第一个有游程的行的段数，并进行标号
		{
			label++;       //编号
			block[i*4+3]=label;   //记录每一段的编号
		}
		start=0;
		stop=linsum[j];    //有游程的行
		repeat=0;
		for(k=j+1;k<EndRow;k++)   //从有目标的行开始计算，直到窗口最后一行
		{
			for(i=stop;i<stop+linsum[k];i++)   //第一个有游程的行之后，每一个段
			{
				repeat=0;
				for(l=start;l<stop;l++)
				{
					if(((block[i*4+1]>=(block[l*4+1]-1))&&(block[i*4+1]<=(block[l*4+2]+1)))
							||((block[i*4+2]>=(block[l*4+1]-1))&&(block[i*4+2]<=(block[l*4+2]+1)))
							||((block[i*4+1]<=(block[l*4+1]-1))&&(block[i*4+2]>=(block[l*4+2]+1))))
					{
						if(repeat==0)   //                          标号相同
						{
							block[i*4+3]=block[l*4+3];
							repeat=1;
						}
						else
						{
							for(m=0;m<i;m++)
							{
								if(block[m*4+3]==block[l*4+3])
								{
									block[m*4+3]=block[i*4+3];
								}
							}
						}
					}
				}
				if(repeat==0)
				{
					label++;
					block[i*4+3]=label;
				}
			}
			start=stop;
			stop=stop+linsum[k];
		}
		for(i=0;i<label;i++)
		{
			posxy[i*8]=0;
			posxy[i*8+1]=0;
			posxy[i*8+2]=0;
			posxy[i*8+3]=0;
			posxy[i*8+4]=512;
			posxy[i*8+5]=0;
			posxy[i*8+6]=640;
			posxy[i*8+7]=0;
		}
		for(i=0;i<Seg_Count-1;i++)
		{
			temp=block[i*4+3]-1;   //每一个段的标号
			posxy[temp*8]=posxy[temp*8]+block[i*4]; // Row
			posxy[temp*8+1]=posxy[temp*8+1]+(block[i*4+1]+block[i*4+2])/2;// Col 中心点位置
			posxy[temp*8+2]=posxy[temp*8+2]+1;
			for(j=block[i*4+1];j<block[i*4+2];j++)
			{
				M[temp]=M[temp]+1;    //计算每一段的像素个数
			}
			posxy[temp*8+3]=posxy[temp*8+3]+1;
			posxy[temp*8+4]=(block[i*4]<=posxy[temp*8+4])? block[i*4]:posxy[temp*8+4];//目标上边界
			posxy[temp*8+5]=(posxy[temp*8+5]<=block[i*4])? block[i*4]:posxy[temp*8+5];//目标下边界
			posxy[temp*8+6]=(block[i*4+1]<=posxy[temp*8+6])? block[i*4+1]:posxy[temp*8+6];//目标左边界
			posxy[temp*8+7]=(posxy[temp*8+7]<=block[i*4+2])? block[i*4+2]:posxy[temp*8+7];//目标右下边界
			maxTarNum=maxTarNum>(temp+1)?maxTarNum:(temp+1);
		}
	}
	GetTarInformation(maxTarNum,StartRow,EndRow,StartCol,EndCol);
}


void GetTarInformation(int tarnum,short StartRow,short EndRow,short StartCol,short EndCol)
{
	int i,k11=0;
	short SuitNum=0;
	for(i = 0; i <= 100; i++)
	{
		CandObject[i].s_Area          	= 0;
		CandObject[i].s_MeanLight     	= 0;
		CandObject[i].s_PeakLight     	= 0;
		CandObject[i].s_MaxWidth      	= 0;
		CandObject[i].s_MaxHeight     	= 0;
		CandObject[i].s_C_Row         	= 0;
		CandObject[i].s_C_Col         	= 0;
	}
	m_CandTarNum=0;
	for(i=0;i<tarnum;i++)
	{
		if((posxy[i*8+2]>0) && (M[i]>30)&&(M[i]<4000))
		{
			CandObject[k11].s_Area=M[i];
			CandObject[k11].s_C_Row = (posxy[i*8]/posxy[i*8+2]);
			CandObject[k11].s_C_Col = (posxy[i*8+1]/posxy[i*8+2]);
			CandObject[k11].s_MeanLight = 255;
			CandObject[k11].s_MaxHeight = posxy[i*8+5]-posxy[i*8+4]+1;
			CandObject[k11].s_MaxWidth  = posxy[i*8+7]-posxy[i*8+6]+1;
			k11++;
		}
	}
	SuitNum=k11;
	for(i=0;i<SuitNum;i++)
	{
		if((m_CandTarNum<16)&&(TarFeatCredit_FPGA(CandObject[i],StartRow,EndRow,StartCol,EndCol)==0))
		{
			Candidate[m_CandTarNum] = CandObject[i];
			m_CandTarNum++;
		}
	}

	m_PossTarNum = (m_CandTarNum < 16) ? m_CandTarNum : MAXPOSSNUM;
	CopyCandidate(g_Candidate, Candidate);


}



/////////////////////////////////////////////////////////////////////////////////
//LBP算子,以疑似木彼岸
/////////////////////////////////////////////////////////////////////////////////
unsigned char LBPBaseImage(int col,int row)
{
	int Center_Gray_Sum,LeftTop_Gray_Sum,Top_Gray_Sum,RightTop_Gray_Sum;
	int Left_Gray_Sum,LeftBottom_Gray_Sum,Right_Gray_Sum,Bottom_Gray_Sum;
	int RightBottom_Gray_Sum;
	unsigned char LBPCODE;
	int i,j;
	Center_Gray_Sum=0;
	LeftTop_Gray_Sum=0;
	Top_Gray_Sum=0;
	RightTop_Gray_Sum=0;
	Left_Gray_Sum=0;
	LeftBottom_Gray_Sum=0;
	Right_Gray_Sum=0;
	Bottom_Gray_Sum=0;
	RightBottom_Gray_Sum=0;
	LBPCODE=0;
	for(i=-1;i<2;i++)
		for(j=-1;j<2;j++)
			Center_Gray_Sum=Center_Gray_Sum+pSrc1[(row+i)*640+col+j];
	for(i=-1;i<2;i++)
		for(j=-1;j<2;j++)
			LeftTop_Gray_Sum=LeftTop_Gray_Sum+pSrc1[(row-3+i)*640+col-3+j];
	for(i=-1;i<2;i++)
		for(j=-1;j<2;j++)
			Top_Gray_Sum=Top_Gray_Sum+pSrc1[(row-3+i)*640+col+j];
	for(i=-1;i<2;i++)
		for(j=-1;j<2;j++)
			RightTop_Gray_Sum=RightTop_Gray_Sum+pSrc1[(row-3+i)*640+col+3+j];
	for(i=-1;i<2;i++)
		for(j=-1;j<2;j++)
			Left_Gray_Sum=Left_Gray_Sum+pSrc1[(row+i)*640+col-3+j];
	for(i=-1;i<2;i++)
		for(j=-1;j<2;j++)
			Right_Gray_Sum=Right_Gray_Sum+pSrc1[(row-3+i)*640+col+3+j];
	for(i=-1;i<2;i++)
		for(j=-1;j<2;j++)
			LeftBottom_Gray_Sum=LeftBottom_Gray_Sum+pSrc1[(row+3+i)*640+col-3+j];
	for(i=-1;i<2;i++)
		for(j=-1;j<2;j++)
			Bottom_Gray_Sum=Bottom_Gray_Sum+pSrc1[(row+3+i)*640+col+j];
	for(i=-1;i<2;i++)
		for(j=-1;j<2;j++)
			RightBottom_Gray_Sum=RightBottom_Gray_Sum+pSrc1[(row+3+i)*640+col+3+j];

	if (Center_Gray_Sum>LeftTop_Gray_Sum)
		LBPCODE=LBPCODE;
	else
		LBPCODE=LBPCODE+(0x1<<8);
	if (Center_Gray_Sum>Top_Gray_Sum)
		LBPCODE=LBPCODE;
	else
		LBPCODE=LBPCODE+(0x1<<7);
	if (Center_Gray_Sum>RightTop_Gray_Sum)
		LBPCODE=LBPCODE;
	else
		LBPCODE=LBPCODE+(0x1<<6);
	if (Center_Gray_Sum>Left_Gray_Sum)
		LBPCODE=LBPCODE;
	else
		LBPCODE=LBPCODE+(0x1<<5);
	if (Center_Gray_Sum>Right_Gray_Sum)
		LBPCODE=LBPCODE;
	else
		LBPCODE=LBPCODE+(0x1<<4);
	if (Center_Gray_Sum>LeftBottom_Gray_Sum)
		LBPCODE=LBPCODE;
	else
		LBPCODE=LBPCODE+(0x1<<3);
	if (Center_Gray_Sum>Bottom_Gray_Sum)
		LBPCODE=LBPCODE;
	else
		LBPCODE=LBPCODE+(0x1<<2);
	if (Center_Gray_Sum>RightBottom_Gray_Sum)
		LBPCODE=LBPCODE;
	else
		LBPCODE=LBPCODE+(0x1);
	return LBPCODE;

}



// 利用先用原始图像进行滤波，再对滤波结果进行跑长码编码聚类，提取目标
void SmallSearchBaseImage(short StartRow,short EndRow, short StartCol,short EndCol)
{

	InitLabelPara();
	m_GuideState = SmallRunlengthBaseImage(StartRow,EndRow,StartCol,EndCol);//label the runlength
	AdaptThreshHist();
	if(m_GuideState == NORMAL)
	{
		m_GuideState = Map();//聚类出目标，目标标号
		if(m_GuideState == NORMAL)
		{
			ExtractFeature(StartRow,EndRow,StartCol,EndCol); //把满足条件的目标信息放到g_candidate[m_possTarNum]
		}
		else
			m_PossTarNum = 0;
	}
	else
		m_PossTarNum = 0;
}

//==========================================================================================
//初始化label参数
void InitLabelPara()
{
	SegNum_CurLine          = 0;
	SegNum_LastLine         = 0;
	Num_Seg                 = 0;
	LabNum                  = 0;
	NeiTabLeng              = 0;
	True_Obj                = 0;

	Seg_Code[0].Row         = -1;
	Seg_Code[0].CStart      = -1;
	Seg_Code[0].CEnd        = -1;
	Seg_Code[0].Label       = -1;
	Seg_Code[0].MaxGray     = 0;
	Seg_Code[0].MeanGray    = 0;

	NeiTable0[0]            = 0xff;
	NeiTable1[0]            = 0xff;
	MapTable [0]            = 0xff;

	m_GuideState            = NORMAL;
	m_ThreshMax             = 0;
	m_ThreshMean            = 0;
	m_ThreshCount           = 0;
	m_ThreshVar             = 0;
	m_PossTarNum            = 0;
}

int Sobelv(volatile unsigned char *Proc_Image_Col)
{
	unsigned short m_nGrayLast1,m_nGrayLast2,m_nGrayLast3;
	unsigned short m_nGrayLast4,m_nGrayLast5,m_nGrayLast6,m_nGrayLast7,m_nGrayLast8;
	unsigned short m_nGrayLast9,m_nGrayLast10,m_nGrayLast11;
	unsigned short m_nGrayLast12,m_nGrayLast13,m_nGrayLast14;
	unsigned short  m_nGrayLastz1,m_nGrayLastz2,m_nGrayLastz3,m_nGrayLastz4,m_nGrayLastz5,m_nGrayLastz6,m_nGrayLastz7;
	unsigned short  m_nGrayLastz8,m_nGrayLastz9,m_nGrayLastz10,m_nGrayLastz11,m_nGrayLastz12,m_nGrayLastz13,m_nGrayLastz14;
	int   m_nPreProcGray;

	m_nGrayLast1=*(Proc_Image_Col-1);
	m_nGrayLast2=*(Proc_Image_Col-2);
	m_nGrayLast3=*(Proc_Image_Col-3);
	m_nGrayLast4=*(Proc_Image_Col-4);
	m_nGrayLast5=*(Proc_Image_Col-5);
	m_nGrayLast6=*(Proc_Image_Col-6);
	m_nGrayLast7=*(Proc_Image_Col-7);

	m_nGrayLast8=*(Proc_Image_Col-8);
	m_nGrayLast9=*(Proc_Image_Col-9);
	m_nGrayLast10=*(Proc_Image_Col-10);
	m_nGrayLast11=*(Proc_Image_Col-11);
	m_nGrayLast12=*(Proc_Image_Col-12);
	m_nGrayLast13=*(Proc_Image_Col-13);
	m_nGrayLast14=*(Proc_Image_Col-14);


	m_nGrayLastz1=*(Proc_Image_Col+1);
	m_nGrayLastz2=*(Proc_Image_Col+2);
	m_nGrayLastz3=*(Proc_Image_Col+3);
	m_nGrayLastz4=*(Proc_Image_Col+4);
	m_nGrayLastz5=*(Proc_Image_Col+5);
	m_nGrayLastz6=*(Proc_Image_Col+6);
	m_nGrayLastz7=*(Proc_Image_Col+7);
	m_nGrayLastz8=*(Proc_Image_Col+8);
	m_nGrayLastz9=*(Proc_Image_Col+9);
	m_nGrayLastz10=*(Proc_Image_Col+10);
	m_nGrayLastz11=*(Proc_Image_Col+11);
	m_nGrayLastz12=*(Proc_Image_Col+12);
	m_nGrayLastz13=*(Proc_Image_Col+13);
	m_nGrayLastz14=*(Proc_Image_Col+14);


	m_nPreProcGray=28*(*Proc_Image_Col)-(m_nGrayLast1+m_nGrayLast2+m_nGrayLast3+m_nGrayLast4+m_nGrayLast5+m_nGrayLast6+m_nGrayLast7
			+m_nGrayLast8+m_nGrayLast9+m_nGrayLast10+m_nGrayLast11+m_nGrayLast12+m_nGrayLast13+m_nGrayLast14
			+m_nGrayLastz1+m_nGrayLastz2+m_nGrayLastz3+m_nGrayLastz4+m_nGrayLastz5+m_nGrayLastz6+m_nGrayLastz7
			+m_nGrayLastz8+m_nGrayLastz9+m_nGrayLastz10+m_nGrayLastz11+m_nGrayLastz12+m_nGrayLastz13+m_nGrayLastz14);
	m_nPreProcGray/=28;


	return m_nPreProcGray;
}

//==========================================================================================
int Map()
{
	short Lab_Stack[MAPSTACK_OVER],Cur_Lab;
	short Nei_Index,NeiSe_Index,Stack_Index,Seg_Index;

	Stack_Index=0;
	True_Obj=0;

	for(Nei_Index=1;Nei_Index<=NeiTabLeng;Nei_Index++)
	{
		if((NeiTable0[Nei_Index]!=0)&&(Stack_Index==0))
		{
			Stack_Index++;

			if(Stack_Index>MAPSTACK_OVER-2)
				return OVERFLOW;

			Lab_Stack[Stack_Index] = NeiTable0[Nei_Index];
			True_Obj++;
			if(True_Obj>MAPTABLE_OVER-2)
				return OVERFLOW;
			NeiTable0[Nei_Index]=0;
			NeiTable1[Nei_Index]=0;
		}
		while(Stack_Index>0)
		{
			Cur_Lab=Lab_Stack[Stack_Index];
			Stack_Index--;
			MapTable[Cur_Lab]=True_Obj;

			for(NeiSe_Index=1;NeiSe_Index<=NeiTabLeng;NeiSe_Index++)
			{
				if(NeiTable0[NeiSe_Index]>0)
				{
					if((NeiTable0[NeiSe_Index]==Cur_Lab)&&(NeiTable1[NeiSe_Index]==Cur_Lab))
					{
						NeiTable0[NeiSe_Index]=0;
						NeiTable1[NeiSe_Index]=0;
					}
					else
					{
						if(NeiTable0[NeiSe_Index]==Cur_Lab)
						{
							Stack_Index++;
							if(Stack_Index>MAPSTACK_OVER-2)
								return OVERFLOW;
							Lab_Stack[Stack_Index]=NeiTable1[NeiSe_Index];
							NeiTable0[NeiSe_Index]=0;
							NeiTable1[NeiSe_Index]=0;
						}
						else
						{
							if(NeiTable1[NeiSe_Index]==Cur_Lab)
							{
								Stack_Index++;
								if(Stack_Index>MAPSTACK_OVER-2)
									return OVERFLOW;
								Lab_Stack[Stack_Index]=NeiTable0[NeiSe_Index];
								NeiTable0[NeiSe_Index]=0;
								NeiTable1[NeiSe_Index]=0;
							}
						}
					}
				}
			}
		}
	}

	for(Seg_Index = 1; Seg_Index < Num_Seg; Seg_Index++)
		Seg_Code[Seg_Index].Label = MapTable[Seg_Code[Seg_Index].Label];

	return NORMAL;
}


//==========================================================================================
/*函数作用：函数给Seg_Code中的各个游程贴上一个初始标签。
  输入：Seg_Code：存放着当前帧图像中检测到的各个游程。
  Seg_Code中的每一个元素分别记录了一个游程所在的行，起始列和游程长度
  输出：各个游程有了初始标签的数组Seg_Code
 */
int SmallRunlengthBaseYouCheng()
{
	unsigned short    i=0;
	volatile int      Neigh_Flag;
	int               SegLast, SegCur_Bias;
	volatile int      addlabel = 0;
	int               CurRow;
	SegNum_CurLine = 0;
	CurRow=Seg_Code[0].Row;
	while(i<Seg_Count)
	{
		Num_Seg=i+1;
		if(Seg_Code[i].Row==CurRow)
		{

			Neigh_Flag=0;
			SegNum_CurLine++;
			for(SegLast=1;SegLast<=SegNum_LastLine;SegLast++)
			{
				SegCur_Bias=Num_Seg-SegNum_CurLine-SegNum_LastLine+SegLast-1;
				if((Seg_Code[Num_Seg-1].CStart-1<=Seg_Code[SegCur_Bias].CEnd)&&
						(Seg_Code[Num_Seg-1].CEnd+1>=Seg_Code[SegCur_Bias].CStart))
				{
					Neigh_Flag++;
					if(Neigh_Flag==1)
						Seg_Code[Num_Seg-1].Label=Seg_Code[SegCur_Bias].Label;
					if(Neigh_Flag>1)
					{
						if(Seg_Code[Num_Seg-1].Label!=Seg_Code[SegCur_Bias].Label)
						{
							NeiTabLeng++;
							if(NeiTabLeng>NEIGHTABLE_OVER)
							{
								return OVERFLOW;
							}
							NeiTable0[NeiTabLeng]=Seg_Code[Num_Seg-1].Label;
							NeiTable1[NeiTabLeng]=Seg_Code[SegCur_Bias].Label;
						}
					}
				}
			}
			if(Neigh_Flag==0)
			{
				LabNum++;
				Seg_Code[Num_Seg-1].Label=LabNum;
				NeiTabLeng++;
				NeiTable0[NeiTabLeng]=LabNum;
				NeiTable1[NeiTabLeng]=LabNum;
				if(NeiTabLeng>NEIGHTABLE_OVER)
				{
					return OVERFLOW;
				}
			}
			i=i+1;
		}
		else
		{
			///////////////////////////////////////////////////////////
			if(Seg_Code[i].Row-CurRow<=3)///////////2014.02.26
			{
				SegNum_LastLine=SegNum_CurLine;
			}
			else
			{
				SegNum_LastLine=0;
			}
			////////////////////////////////////////////////////////////
			CurRow= Seg_Code[i].Row;
			SegNum_CurLine=0;
		}
	}
	return NORMAL;
}

int SmallRunlengthBaseImage(int m_ProcWinStartRow,int m_ProcWinEndRow,int m_ProcWinStartCol,int m_ProcWinEndCol)
{

	unsigned short i,j;
	unsigned short m_rRowStart,m_rRowEnd,m_rColStart,m_rColEnd;
	volatile unsigned char *Proc_Image_Row,*Proc_Image_Col;                // 位数发生了变化为8位
	unsigned short  m_nGrayNow,m_nGrayLast1;
	volatile int   Neigh_Flag;
	int            SegLast,SegCur_Bias;
	int            m_MaxGray,m_MeanGray,m_Count,m_Var;
	volatile double         add=0;
	volatile int            addlabel=0;
	int            m_nPreProcGray,stop;

	m_rRowStart  = m_ProcWinStartRow;
	m_rRowEnd    = m_ProcWinEndRow;
	m_rColStart  = m_ProcWinStartCol;
	m_rColEnd    = m_ProcWinEndCol;

	m_MaxGray   = m_ThreshMax;
	m_MeanGray  = m_ThreshMean;
	m_Count     = m_ThreshCount;
	m_Var        = m_ThreshVar;

	SegNum_CurLine=0;
	Proc_Image_Row=pSrc1+m_rRowStart*IMAGEWIDTH;

	for(i=m_rRowStart;i<m_rRowEnd;i++)
	{
		Proc_Image_Col           = Proc_Image_Row+m_rColStart;
		m_nGrayLast1             = *Proc_Image_Col++;
		m_nGrayNow               = *Proc_Image_Col++;

		j=m_rColStart+1;
		while(j<m_rColEnd)
		{
			m_nGrayLast1         =*Proc_Image_Col;
			//m_nPreProcGray       =(Sobelh(Proc_Image_Col)+Sobelv(Proc_Image_Col))/2;
			m_nPreProcGray       =(Sobelv(Proc_Image_Col));
			//  DDRTest[(i-m_rRowStart)*128+j-m_rColStart]=(m_nPreProcGray>m_Threshold)?255:0;
			pmoban[(i-m_rRowStart)*64+j-m_rColStart]=(m_nPreProcGray>m_Threshold)?255:0;
			Proc_Image_Col++;
			if(m_nPreProcGray!=0)
			{
				stop=1;
			}
			m_MaxGray            = (m_MaxGray>m_nPreProcGray)?m_MaxGray:m_nPreProcGray;
			m_MeanGray           += m_nPreProcGray;
			m_Count++;
			m_Var                += m_nPreProcGray*m_nPreProcGray;
			if(m_Var>=0x10000000)
			{
				addlabel++;
				m_Var=m_Var&0xfffffff;
			}


			if(m_nPreProcGray>m_Threshold)
			{
				SegNum_CurLine++;
				Num_Seg++;
				if(Num_Seg>=RUNLENGTHCODE_OVER)
				{
					add               =addlabel*0x10000000+m_Var;
					m_ThreshMax       = m_MaxGray;
					m_ThreshMean      = m_MeanGray;
					m_ThreshCount     = m_Count;
					m_ThreshVar       = add/m_Count;
					return OVERFLOW;
				}
				Seg_Code[Num_Seg].Row       = i;
				Seg_Code[Num_Seg].CStart    = j;
				Seg_Code[Num_Seg].MaxGray   = m_nGrayLast1;
				Seg_Code[Num_Seg].MeanGray  = m_nGrayLast1;

				j++;
				m_nGrayLast1         =*Proc_Image_Col;
				//m_nPreProcGray       =(Sobelh(Proc_Image_Col)+Sobelv(Proc_Image_Col))/2;
				m_nPreProcGray       =Sobelv(Proc_Image_Col);
				Proc_Image_Col++;
				m_MaxGray            = (m_MaxGray>m_nPreProcGray)?m_MaxGray:m_nPreProcGray;

				while((m_nPreProcGray>m_Threshold)&&(j<m_rColEnd))
				{
					j++;
					Seg_Code[Num_Seg].MaxGray=(m_nGrayLast1>Seg_Code[Num_Seg].MaxGray)?m_nGrayLast1:Seg_Code[Num_Seg].MaxGray;
					Seg_Code[Num_Seg].MeanGray+=m_nGrayLast1;

					m_nGrayLast1         =*Proc_Image_Col;
					//m_nPreProcGray       =(Sobelh(Proc_Image_Col)+Sobelv(Proc_Image_Col))/2;
					m_nPreProcGray       =Sobelv(Proc_Image_Col);
					Proc_Image_Col++;
					m_MaxGray            = (m_MaxGray>m_nPreProcGray)?m_MaxGray:m_nPreProcGray;
				}
				Seg_Code[Num_Seg].CEnd=j-1;

				Neigh_Flag=0;
				for(SegLast=1;SegLast<=SegNum_LastLine;SegLast++)
				{
					SegCur_Bias=Num_Seg-SegNum_CurLine-SegNum_LastLine+SegLast;
					if((Seg_Code[Num_Seg].CStart-1<=Seg_Code[SegCur_Bias].CEnd)&&
							(Seg_Code[Num_Seg].CEnd+1>=Seg_Code[SegCur_Bias].CStart))
					{
						Neigh_Flag++;
						if(Neigh_Flag==1)
							Seg_Code[Num_Seg].Label=Seg_Code[SegCur_Bias].Label;
						if(Neigh_Flag>1)
						{
							if(Seg_Code[Num_Seg].Label!=Seg_Code[SegCur_Bias].Label)
							{
								NeiTabLeng++;
								if(NeiTabLeng>NEIGHTABLE_OVER)
								{
									add               =addlabel*0x10000000+m_Var;
									m_ThreshMax       = m_MaxGray;
									m_ThreshMean      = m_MeanGray;
									m_ThreshCount     = m_Count;
									m_ThreshVar       = add/m_Count;
									return OVERFLOW;
								}
								NeiTable0[NeiTabLeng]=Seg_Code[Num_Seg].Label;
								NeiTable1[NeiTabLeng]=Seg_Code[SegCur_Bias].Label;
							}
						}
					}
				}
				if(Neigh_Flag==0)
				{
					LabNum++;
					Seg_Code[Num_Seg].Label=LabNum;
					NeiTabLeng++;
					NeiTable0[NeiTabLeng]=LabNum;
					NeiTable1[NeiTabLeng]=LabNum;
					if(NeiTabLeng>NEIGHTABLE_OVER)
					{
						add               =addlabel*0x10000000+m_Var;
						m_ThreshMax       = m_MaxGray;
						m_ThreshMean      = m_MeanGray;
						m_ThreshCount     = m_Count;
						m_ThreshVar       = add/m_Count;
						return OVERFLOW;
					}
				}

			}
			j++;
		}
		SegNum_LastLine=SegNum_CurLine;
		SegNum_CurLine=0;
		Proc_Image_Row+=IMAGEWIDTH;
	}
	add               =addlabel*0x10000000+m_Var;
	m_ThreshMax       = m_MaxGray;
	m_ThreshMean      = m_MeanGray;
	m_ThreshCount     = m_Count;
	m_ThreshVar       = add/m_Count;
	return NORMAL;
}

void AdaptThreshHist()
{
	m_ThreshMean=m_ThreshMean/m_ThreshCount;
	m_ThreshVar-=m_ThreshMean*m_ThreshMean;

	if(m_ThreshVar<2)
		m_ThreshVar=2;
	if(m_GuideState != NORMAL)
		m_Threshold = m_ThreshMax;
	else
		m_Threshold = (int)(m_ThreshMean+3*sqrt(m_ThreshVar));
}
