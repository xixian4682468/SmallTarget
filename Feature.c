/************************************************************************
	filename:   Feature.h
	purpose:   ��ȡ����
*************************************************************************/
#include "Feature.h"
#include   "struct.h"
#include"Communication.h"
#include"Detect.h"
#include"Threshold.h"
/////////////////////////////////////////////////////////////
extern TARFEATURE     	*Candidate ;
extern TARFEATURE     	*CandObject;
extern TARFEATURE 		*g_Candidate;
extern unsigned char  	     g_bSearchInTrackState;
extern int          			g_fangweijiao;
extern int          			g_fuyangjiao;
extern RUNLENGTHCODE    *Seg_Code;
extern int						g_ReceiveCommand;
extern unsigned short			g_sendPCFangWeiAngle;
extern unsigned short			g_sendPCFuYangAngle;
extern volatile int			m_PossTarNum,m_CandTarNum,RealTarNum;
extern int m_imgWidth;
extern int m_imgHeight;
extern unsigned short			SegNum_CurLine,SegNum_LastLine,Num_Seg,LabNum;
extern unsigned short          FPGA_Frame_Num,Pro_Frame_Num;
extern unsigned short			NeiTabLeng,True_Obj;
extern float                   XFov,YFov;                 // Ĭ��Ϊ���ӳ�
extern volatile int        	XiuZhengZhi_X,XiuZhengZhi_Y,m_GuideState,StoreFrame;
////////////////////////////////////////////////////////////////////////
short			*colstart      	= (short      *)(OBJ_ADDR+0x00002580);
short			*colend        	= (short      *)(OBJ_ADDR+0x00002710);
short			*rowstart      	= (short      *)(OBJ_ADDR+0x000028A0);
short			*rowend        	= (short      *)(OBJ_ADDR+0x00002A30);

//==========================================================================================================
/*�������ã���CandObject�еĸ���Ŀ������ж���ȥ��������������Ŀ��
  ���룺ͼ���е�Ŀ��
  �������
  ����ֵ��m_Credit��m_Credit=0 ��ʾ��Ŀ������������ж�����
                    m_Credit=1 ��ʾ��Ŀ��������������ж��������Ӷ���Ŀ�겻��������Ŀ��
*/
int TarFeatCredit(TARFEATURE m_CFeature,short StartRow,short EndRow,short StartCol,short EndCol)
{
	char  m_Credit;
	unsigned short MinArea = 0,a1=0,a2=0,a3=0;

    if(g_bSearchInTrackState < 5)
        MinArea = 4;
	else
	    MinArea = 4;

	m_Credit = 0;
	a1=m_CFeature.s_MaxHeight;
	a2=m_CFeature.s_MaxWidth;

	if(m_CFeature.s_MaxHeight/m_CFeature.s_MaxWidth>9)
		m_Credit++;
	if(m_CFeature.s_MaxWidth/m_CFeature.s_MaxHeight>9)
		m_Credit++;
	
	
	if(m_CFeature.s_MaxHeight<=0)
		m_Credit++;

	if(m_CFeature.s_MaxWidth<=0)
		m_Credit++;
    a3=m_CFeature.s_Area;
	if((m_CFeature.s_Area < MinArea))
		m_Credit++;
	
	if(m_CFeature.s_Area > 4000)
		m_Credit++;

	/*if(m_CFeature.s_C_Col>SearchWinEndLie || m_CFeature.s_C_Col<SearchWinStartLie)
	    m_Credit++;
    
	if(m_CFeature.s_C_Row>SearchWinEndHang || m_CFeature.s_C_Row<SearchWinStartHang)
	    m_Credit++;*/
    if(m_CFeature.s_C_Col>EndCol || m_CFeature.s_C_Col<StartCol)
	    m_Credit++;
    
	if(m_CFeature.s_C_Row>EndRow || m_CFeature.s_C_Row<StartRow)
	    m_Credit++;



	return m_Credit; 
}
//==========================================================================================================
/*�������ã���CandObject�еĸ���Ŀ������ж���ȥ��������������Ŀ��
  ���룺ͼ���е�Ŀ��
  �������
  ����ֵ��m_Credit��m_Credit=0 ��ʾ��Ŀ������������ж�����
                    m_Credit=1 ��ʾ��Ŀ��������������ж��������Ӷ���Ŀ�겻��������Ŀ��
*/
int TarFeatCredit_FPGA(TARFEATURE m_CFeature,short StartRow,short EndRow,short StartCol,short EndCol)
{
	char  m_Credit;
	unsigned short MinArea = 0,a1=0,a2=0,a3=0;
    unsigned char LbpCode;
    MinArea = 36;

	m_Credit = 0;
	a1=m_CFeature.s_MaxHeight;
	a2=m_CFeature.s_MaxWidth;

	if(m_CFeature.s_MaxHeight/m_CFeature.s_MaxWidth>7)
		m_Credit++;
	if(m_CFeature.s_MaxWidth/m_CFeature.s_MaxHeight>7)
		m_Credit++;
	if((m_CFeature.s_MaxWidth*m_CFeature.s_MaxHeight)/m_CFeature.s_Area>3)	
		m_Credit++;
	if(m_CFeature.s_MaxHeight<=0)
		m_Credit++;

	if(m_CFeature.s_MaxWidth<=0)
		m_Credit++;
    a3=m_CFeature.s_Area;
	if((m_CFeature.s_Area < MinArea))
		m_Credit++;
	
	if(m_CFeature.s_Area > 4000)
		m_Credit++;
	if(g_bSearchInTrackState==1)
	 {
	   LbpCode=LBPBaseImage(m_CFeature.s_C_Col+XiuZhengZhi_X,m_CFeature.s_C_Row+XiuZhengZhi_Y);
       if(LbpCode>0)
	       m_Credit++;
	 }
	/*if(m_CFeature.s_C_Col>SearchWinEndLie || m_CFeature.s_C_Col<SearchWinStartLie)
	    m_Credit++;
    
	if(m_CFeature.s_C_Row>SearchWinEndHang || m_CFeature.s_C_Row<SearchWinStartHang)
	    m_Credit++;*/
    if(m_CFeature.s_C_Col>EndCol || m_CFeature.s_C_Col<StartCol)
	    m_Credit++;
    
	if(m_CFeature.s_C_Row>EndRow || m_CFeature.s_C_Row<StartRow)
	    m_Credit++;



	return m_Credit; 
}

float CmEntry(unsigned char *src,unsigned short row, unsigned short col)
{
  #define SrcImage(ROW,COL)   *(src + ROW*m_imgWidth +COL)
  float LocEntry = 0.0f,sum = 0.0f;
  char i,j ;
  int crow,ccol;
  for(i = -2;i<=2;i++)
  {
    for(j=-2;j<=2;j++)
	{
	  crow = row + i;
	  ccol = col + j;
	  sum = sum + SrcImage(crow,ccol)*SrcImage(crow,ccol);
	}
  }
  sum = sqrt(sum);
  LocEntry = SrcImage(row,col)/sum;
  
  return LocEntry;  
}

float CmStd(unsigned char *src,unsigned short row, unsigned short col,unsigned char radius)
{
  float LocVar = 0.0f,LocMean = 0.0f;
  unsigned char  PixelNum = (2*radius + 1)*(2*radius + 1);
  char i,j;
  int crow,ccol;
  #define SrcImage(ROW,COL)   *(src + ROW*m_imgWidth +COL)

  for(i = -radius;i<=radius;i++)
  {
    for(j=-radius;j<=radius;j++)
	{
	  crow = row + i;
	  ccol = col + j;
	  LocMean = LocMean + SrcImage(crow,ccol);
	}
  }

  LocMean = LocMean/PixelNum;

  for(i = -radius;i<=radius;i++)
  {
    for(j=-radius;j<=radius;j++)
	{
	  crow = row + i;
	  ccol = col + j;
	  LocVar = LocVar + (SrcImage(crow,ccol) - LocMean)*(SrcImage(crow,ccol) - LocMean);
	}
  }

  LocVar = LocVar/PixelNum;
  LocVar = sqrt(LocVar);
  return LocVar;
} 
//==========================================================================================================
/*�������ã���ȡ����Ŀ��������������������ж�����ȥ��һ��������Ŀ�꣬�����������ж�������Ŀ�����g_Candidate������
  ���룺�����ž����˸����γ̵����ǩ������Seg_Code
  �����m_PossTarNum����ǰͼ���п��ܵ�Ŀ����
        g_Candidate����ǰͼ���п��ܵ�Ŀ��
*/
void ExtractFeature(short StartRow,short EndRow, short StartCol,short EndCol)
{
	int i,j;
	int m_Lab,m_Candrow,m_Candcol,m_Candarea;
	for(i = 0; i <= 100; i++)
	{
		CandObject[i].s_Area          	= 0;                 
		CandObject[i].s_MeanLight     	= 0;
		CandObject[i].s_PeakLight     	= 0;
		CandObject[i].s_MaxWidth      	= 0;
		CandObject[i].s_MaxHeight     	= 0;
		CandObject[i].s_C_Row         	= 0;
		CandObject[i].s_C_Col         	= 0;
	
		rowstart[i]                 = 490;
		rowend[i]                   = 0;
		colstart[i]                 = 620;
		colend[i]                   = 0;
	}

	for(i = 1; i <=Num_Seg; i++)//2014.04.24--3ԭΪ��for(i = 0; i <Num_Seg; i++)
	{     
		m_Lab = Seg_Code[i].Label -		 1;   
		m_Candrow = 0;
		m_Candcol = 0;
		m_Candarea = 0;
	
		CandObject[m_Lab].s_MeanLight += Seg_Code[i].MeanGray;      
		CandObject[m_Lab].s_PeakLight = (Seg_Code[i].MaxGray > CandObject[m_Lab].s_PeakLight) ? Seg_Code[i].MaxGray : CandObject[m_Lab].s_PeakLight;

		for(j = Seg_Code[i].CStart; j <= Seg_Code[i].CEnd; j++)
		{      
			m_Candrow += Seg_Code[i].Row;
			m_Candcol += j;
			m_Candarea++;    
		}

		CandObject[m_Lab].s_C_Row += m_Candrow; 
		CandObject[m_Lab].s_C_Col += m_Candcol;
		CandObject[m_Lab].s_Area  += m_Candarea; 
		if(colstart[m_Lab] > Seg_Code[i].CStart)
			colstart[m_Lab] = Seg_Code[i].CStart;       
		if(colend[m_Lab] < Seg_Code[i].CEnd)
			colend[m_Lab]=Seg_Code[i].CEnd;
		if(rowstart[m_Lab]>Seg_Code[i].Row)
			rowstart[m_Lab]=Seg_Code[i].Row;       
		if(rowend[m_Lab]<Seg_Code[i].Row)
			rowend[m_Lab]=Seg_Code[i].Row;
	}  
	m_CandTarNum=0;
	for(i=0;i<True_Obj;i++)
	{
		if(CandObject[i].s_Area>0)
		{
			CandObject[i].s_C_Row = (int)(CandObject[i].s_C_Row/CandObject[i].s_Area);
			CandObject[i].s_C_Col = (int)(CandObject[i].s_C_Col/CandObject[i].s_Area);        
        	CandObject[i].s_MeanLight = (int)(CandObject[i].s_MeanLight/CandObject[i].s_Area);
			CandObject[i].s_MaxHeight = rowend[i]-rowstart[i]+1;
			CandObject[i].s_MaxWidth  = colend[i]-colstart[i]+1;           
			if((m_CandTarNum<MAXPOSSNUM)&&(TarFeatCredit(CandObject[i],StartRow,EndRow,StartCol,EndCol)==0))
			{           
				Candidate[m_CandTarNum] = CandObject[i];               
				m_CandTarNum++;
			}
		}       
	}

	m_PossTarNum = (m_CandTarNum < MAXPOSSNUM) ? m_CandTarNum : MAXPOSSNUM;

	CopyCandidate(g_Candidate, Candidate);
}
//==========================================================================================================
/*�������ã���ȡ����Ŀ��������������������ж�����ȥ��һ��������Ŀ�꣬�����������ж�������Ŀ�����g_Candidate������
  ���룺�����ž����˸����γ̵����ǩ������Seg_Code
  �����m_PossTarNum����ǰͼ���п��ܵ�Ŀ����
        g_Candidate����ǰͼ���п��ܵ�Ŀ��
*/
void ExtractFeature_FPGA(short StartRow,short EndRow, short StartCol,short EndCol)
{
	int i,j;
	int m_Lab,m_Candrow,m_Candcol,m_Candarea;
	for(i = 0; i <= 100; i++)
	{
		CandObject[i].s_Area          	= 0;                 
		CandObject[i].s_MeanLight     	= 0;
		CandObject[i].s_PeakLight     	= 0;
		CandObject[i].s_MaxWidth      	= 0;
		CandObject[i].s_MaxHeight     	= 0;
		CandObject[i].s_C_Row         	= 0;
		CandObject[i].s_C_Col         	= 0;
	
		rowstart[i]                 = 490;
		rowend[i]                   = 0;
		colstart[i]                 = 620;
		colend[i]                   = 0;
	}

	for(i = 1; i <=Num_Seg; i++)//2014.04.24--3ԭΪ��for(i = 0; i <Num_Seg; i++)
	{     
		m_Lab = Seg_Code[i].Label -		 1;   
		m_Candrow = 0;
		m_Candcol = 0;
		m_Candarea = 0;
	
		CandObject[m_Lab].s_MeanLight += Seg_Code[i].MeanGray;      
		CandObject[m_Lab].s_PeakLight = (Seg_Code[i].MaxGray > CandObject[m_Lab].s_PeakLight) ? Seg_Code[i].MaxGray : CandObject[m_Lab].s_PeakLight;

		for(j = Seg_Code[i].CStart; j <= Seg_Code[i].CEnd; j++)
		{      
			m_Candrow += Seg_Code[i].Row;
			m_Candcol += j;
			m_Candarea++;    
		}

		CandObject[m_Lab].s_C_Row += m_Candrow; 
		CandObject[m_Lab].s_C_Col += m_Candcol;
		CandObject[m_Lab].s_Area  += m_Candarea; 
		if(colstart[m_Lab] > Seg_Code[i].CStart)
			colstart[m_Lab] = Seg_Code[i].CStart;       
		if(colend[m_Lab] < Seg_Code[i].CEnd)
			colend[m_Lab]=Seg_Code[i].CEnd;
		if(rowstart[m_Lab]>Seg_Code[i].Row)
			rowstart[m_Lab]=Seg_Code[i].Row;       
		if(rowend[m_Lab]<Seg_Code[i].Row)
			rowend[m_Lab]=Seg_Code[i].Row;
	}  
	m_CandTarNum=0;
	for(i=0;i<True_Obj;i++)
	{
		if(CandObject[i].s_Area>0)
		{
			CandObject[i].s_C_Row = (int)(CandObject[i].s_C_Row/CandObject[i].s_Area);
			CandObject[i].s_C_Col = (int)(CandObject[i].s_C_Col/CandObject[i].s_Area);        
        	CandObject[i].s_MeanLight = (int)(CandObject[i].s_MeanLight/CandObject[i].s_Area);
			CandObject[i].s_MaxHeight = rowend[i]-rowstart[i]+1;
			CandObject[i].s_MaxWidth  = colend[i]-colstart[i]+1;           
			if((m_CandTarNum<MAXPOSSNUM)&&(TarFeatCredit_FPGA(CandObject[i],StartRow,EndRow,StartCol,EndCol)==0))
			{           
				Candidate[m_CandTarNum] = CandObject[i];               
				m_CandTarNum++;
			}
		}       
	}

	m_PossTarNum = (m_CandTarNum < MAXPOSSNUM) ? m_CandTarNum : MAXPOSSNUM;

	CopyCandidate(g_Candidate, Candidate);
}

//==========================================================================================================
/*�������ã���Candidate��ֵ��g_Candidate,�����������Ŀ��Ŀռ䷽λ�ǺͿռ丩����
  ���룺g_Candidate�����ͼ��Ŀ����������
        Candidate:ͼ������ȡ����Ŀ��
  �����g_Candidate����ǰ֡ͼ������ȡ�ĸ���Ŀ�꣬�Ҽ�����˸���Ŀ��Ŀռ䷽λ�ǺͿռ丩����
*/
void CopyCandidate(TARFEATURE* g_Candidate, TARFEATURE* Candidate)
{
	int i;
	for(i = 0; i < m_PossTarNum; i++)
	{
		g_Candidate[i] = Candidate[i];
		
 		ConvertTargetCood2Angle(g_Candidate[i].s_C_Col, g_Candidate[i].s_C_Row);
		g_Candidate[i].s_C_fangweiPixel = g_Candidate[i].s_C_Col;
		g_Candidate[i].s_C_fuyangPixel  = g_Candidate[i].s_C_Row;
		g_Candidate[i].s_C_Col = g_sendPCFangWeiAngle;//��ʱ��Ϊ�Ǵ�Ŀ���ڿռ伫����ϵ�еķ�λ��
		g_Candidate[i].s_C_Row = g_sendPCFuYangAngle;//��ʱ��Ϊ�Ǵ�Ŀ���ڿռ伫����ϵ�еĸ�����
        g_Candidate[i].Frame_Code=Pro_Frame_Num;//֡��
	} 
}

//==========================================================================================================
/*�������ã����ݸ���ͼ��Ŀ�������λ�����ڵ��У���������������Ŀ��Ŀռ䷽λ�ǺͿռ丩����
  ���룺col��Ŀ�������λ���ڵ���
        row��Ŀ�������λ�����ڵ���
  �����g_sendPCFangWeiAngleĿ��Ŀռ䷽λ��
        g_sendPCFuYangAngle:Ŀ��Ŀռ丩����
*/
void ConvertTargetCood2Angle(int col, int row)
{	

	if((g_ReceiveCommand == SearchAlarm)||(g_ReceiveCommand == JumpAlarm))
	  {
	   	g_sendPCFangWeiAngle = g_fangweijiao + (256-(row+XiuZhengZhi_Y))*YFov*1000000/512;//	g_fangweijiao���ŷ�������
		g_sendPCFuYangAngle  = g_fuyangjiao  + (320-(640-col-XiuZhengZhi_X))*XFov*1000000/640;
	  }
	else
	  {
	    g_sendPCFangWeiAngle=row;
	    g_sendPCFuYangAngle=640-col;
      }
   
}



