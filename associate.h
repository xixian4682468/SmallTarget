/************************************************************************
	filename:   Associate.h
	purpose:   建立目标关联链
*************************************************************************/
#ifndef _ASSOCIATE_H_
#define _ASSOCIATE_H_
#include   "struct.h"

#define		CHAIN_ADDR				0x0082DF20  //链地址
#define		MINASSOCIATEERROR       20          //目标图像坐标关联距离/////2013.8.4,原为#define		MINASSOCIATEERROR       60
#define		MINASSOCIATEAREAERROR       200
#define     MINASSOCIATEANGLEERROR  50       //目标空间角度关联距离///2013.8.14
#define		MAX_FEAT_NUM			200		    //关联目标链最大目标数(暂定20,最大目标数目只有16--cui）//200
#define		MIN_DIFF_ERROR          1000.0      //计算最近邻的最小误差
#define		MISSFRAME               5     //目标丢失帧数//设定目标720帧不更新就删除（30度每秒，1秒24帧，12秒转完一周360度）
#define 	CHANGE_MODE_FRAME		1			//累积目标建链 帧数

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


