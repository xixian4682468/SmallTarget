/************************************************************************
	filename:   Associate.h
	purpose:   ����Ŀ�������
*************************************************************************/
#ifndef _ASSOCIATE_H_
#define _ASSOCIATE_H_
#include   "struct.h"

#define		CHAIN_ADDR				0x0082DF20  //����ַ
#define		MINASSOCIATEERROR       20          //Ŀ��ͼ�������������/////2013.8.4,ԭΪ#define		MINASSOCIATEERROR       60
#define		MINASSOCIATEAREAERROR       200
#define     MINASSOCIATEANGLEERROR  50       //Ŀ��ռ�Ƕȹ�������///2013.8.14
#define		MAX_FEAT_NUM			200		    //����Ŀ�������Ŀ����(�ݶ�20,���Ŀ����Ŀֻ��16--cui��//200
#define		MIN_DIFF_ERROR          1000.0      //��������ڵ���С���
#define		MISSFRAME               5     //Ŀ�궪ʧ֡��//�趨Ŀ��720֡�����¾�ɾ����30��ÿ�룬1��24֡��12��ת��һ��360�ȣ�
#define 	CHANGE_MODE_FRAME		1			//�ۻ�Ŀ�꽨�� ֡��

float abs_f(float a,float b);
void SetTargetFeatureZero(TARFEATURE tarfeature);
void SetTargetInChainZero(int pos);
void SetTargetInCurChainZero(int pos);
void InitTargetInChain();
void InitTargetInCurChain();
void AddTargetInChain_PH(TARFEATURE tarFeature, int pos, unsigned char PiHao);
void AddTargetInChain_NPH(TARFEATURE tarFeature, int pos);
void AddTargetInCurChain(TARFEATURE tarFeature, int pos);
void AddTargetInCurChain2(TARFEATURE tarFeature, int pos,CHAIN tarChain);
void UpdateTargetInChain(TARFEATURE tarFeature, int pos);
void AddNewPH(unsigned char NewPH);
void DeleteTargetInChain(int pos);
float  DiffDis(TARFEATURE srcFeat);
float  DiffFeat(TARFEATURE srcFeat, TARFEATURE destFeat);
float  DiffFeatAngle(TARFEATURE srcFeat, TARFEATURE destFeat);
float  DiffFeatArea(TARFEATURE srcFeat, TARFEATURE destFeat);
void FeatAssociate(TARFEATURE candidate[], int PossTarNum);
void ProcessLostTarget();
void Associate(TARFEATURE tarFeature[], int PossTarNum);

#endif


