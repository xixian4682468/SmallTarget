/************************************************************************
	filename:   Detect.h	
	purpose:   计算游程并获得候选目标
*************************************************************************/
#ifndef _DETECT_H_
#define _DETECT_H_
#include   "struct.h"

#define		SEG_ADDR				0x0c208400  //游程码起始地址   ?
#define		OBJ_ADDR				0x0c22A0A0  //目标初始信息地址
#define		CHAIN_ADDR				0x0c22DF20  //链地址
#define		MAXPOSSNUM				16			//最大目标数目
#define		NORMAL					0x00
#define		IMAGEWIDTH				640			//图像宽度
#define		IMAGEHIGTH				512			//图像宽度
#define		MAPSTACK_OVER       	0x100
#define		NEIGHTABLE_OVER     	0xb00
#define		MAPTABLE_OVER       	0xb00
#define		OVERFLOW				0xff
#define		RUNLENGTHCODE_OVER  	3000		//1000


void AutoTarDetect(unsigned short StartRow,unsigned short EndRow,unsigned short StartCol,unsigned short EndCol);
void GetTarInformation(int tarnum,short StartRow,short EndRow,short StartCol,short EndCol);
//void CopyCandidate(TARFEATURE* g_Candidate, TARFEATURE* Candidate);
//void ConvertTargetCood2Angle(int col, int row);
//float abs_f(float a,float b);
//float  DiffFeat(TARFEATURE srcFeat, TARFEATURE destFeat);
//int TarFeatCredit_FPGA(TARFEATURE m_CFeature,short StartRow,short EndRow,short StartCol,short EndCol);
unsigned char LBPBaseImage(int col,int row);
void SmallSearchBaseImage(short StartRow,short EndRow, short StartCol,short EndCol);

void InitLabelPara();
int Sobelv(volatile unsigned char *Proc_Image_Col);
int Map();
int SmallRunlengthBaseYouCheng();
int SmallRunlengthBaseImage(int m_ProcWinStartRow,int m_ProcWinEndRow,int m_ProcWinStartCol,int m_ProcWinEndCol);
void AdaptThreshHist();

#endif


