/************************************************************************
	filename:   Associate.h
	purpose:   建立目标关联链
*************************************************************************/
#include "associate.h"
#include "match.h"
#include   "struct.h"
#include "track.h"
//==========================================================================================
volatile int g_iTargetNumInChain = 0;
volatile int g_iTargetNumInCurChain = 0;
volatile int            g_iTrackObjCount=0;  
volatile int g_iFrameCount = 0;	
unsigned char           MaxPiHao=0;
unsigned char           NewObjPH[10]={0};
unsigned char           NewObjPHNum = 0;
unsigned char           NewObjPiHao=0,target_capture=0;

//==========================================================================================
#define		MAX_FEAT_NUM			200		    //关联目标链最大目标数(暂定20,最大目标数目只有16--cui）//200
CHAIN           *g_TargetFeatureChain   = (CHAIN *)(CHAIN_ADDR+0x00001320);	//目标链
CHAIN           *g_CurTargetFeatureChain= (CHAIN *)(CHAIN_ADDR+0x000022F6);/////////当前帧中关联上的目标及新检测到的目标///2013.8.14
//==========================================================================================
float abs_f(float a,float b)
{
	float r;

	if(a > b)
		r = a - b;
	else
		r = b - a;

	return r;
}

//==========================================================================================
/*函数作用：把目标tarfeature的值设置为初始化值
  输入：tarfeature：要删除的目标
  输出：初始化值之后的tarfeature
*/
void SetTargetFeatureZero(TARFEATURE tarfeature)
{
	tarfeature.s_Area			= 0;
	tarfeature.s_C_Col			= 0;
	tarfeature.s_C_Row			= 0;
	tarfeature.s_MaxHeight		= 0;
	tarfeature.s_MaxWidth		= 0;
	tarfeature.s_MeanLight		= 0;
	tarfeature.s_PeakLight		= 0;
}

//==========================================================================================
/*函数作用：把目标链中pos位置处的目标的值设置为初始化值
  输入：pos：目标链中要删除的目标的位置
  输出：g_TargetFeatureChain：更新之后的目标链
*/
void SetTargetInChainZero(int pos)
{
	g_TargetFeatureChain[pos].s_iChainSum	= 0;
	g_TargetFeatureChain[pos].s_all_x		= 0;
	g_TargetFeatureChain[pos].s_all_y		= 0;
	g_TargetFeatureChain[pos].s_Last_C_Col  = 0;
	g_TargetFeatureChain[pos].s_Last_C_Row  = 0;
	g_TargetFeatureChain[pos].s_missFrameSum = 0;
	g_TargetFeatureChain[pos].s_featureError = 1000.0f;
	g_TargetFeatureChain[pos].s_UpdateState = 0;
	g_TargetFeatureChain[pos].s_TargetNumForUpdate = 0;

	SetTargetFeatureZero(g_TargetFeatureChain[pos].s_Feature);
}
//==========================================================================================
/*函数作用：把当前帧图像目标链中pos位置处的目标的值设置为初始化值
  输入：pos：目标链中要删除的目标的位置
  输出：g_TargetFeatureChain：更新之后的目标链
*/
void SetTargetInCurChainZero(int pos)
{
	g_CurTargetFeatureChain[pos].s_iChainSum	= 0;
	g_CurTargetFeatureChain[pos].s_all_x=0;
	g_CurTargetFeatureChain[pos].s_all_y		= 0;
	g_CurTargetFeatureChain[pos].s_Last_C_Col  = 0;
	g_CurTargetFeatureChain[pos].s_Last_C_Row  = 0;
	g_CurTargetFeatureChain[pos].s_missFrameSum = 0;
	g_CurTargetFeatureChain[pos].s_featureError = 1000.0f;
	g_CurTargetFeatureChain[pos].s_UpdateState = 0;
	g_CurTargetFeatureChain[pos].s_TargetNumForUpdate = 0;
    g_CurTargetFeatureChain[pos].s_Pihao=0;
	SetTargetFeatureZero(g_CurTargetFeatureChain[pos].s_Feature);
}
//==========================================================================================
/*函数作用：初始化目标链
  输入：无
  输出：初始化之后的目标链
*/
void InitTargetInChain()
{
	int i;

	for(i = 0; i < MAX_FEAT_NUM; i++)
	{
		SetTargetInChainZero(i);
	}
	g_iTargetNumInChain = 0;
}

//==========================================================================================
/*函数作用：初始化当前图像目标链
  输入：无
  输出：初始化之后的当前图像目标链
*/
void InitTargetInCurChain()
{
	int i;

	for(i = 0; i < MAX_FEAT_NUM; i++)
	{
		SetTargetInCurChainZero(i);
	}
	g_iTargetNumInCurChain = 0;
}
//==========================================================================================
/*函数作用：把目标tarFeature加入到目标链g_TargetFeatureChain的pos位置处
  输入：tarFeature：要加入到目标链中的目标
        pos：要加入目标链的目标在目标链中的位置
  输出：g_TargetFeatureChain：加入新目标后的目标链
        g_iTargetNumInChain：目标链中目标的数目
*/
void AddTargetInChain_PH(TARFEATURE tarFeature, int pos, unsigned char PiHao)
{

	if (pos + 1 <= MAX_FEAT_NUM)
	{
		g_TargetFeatureChain[pos].s_Feature = tarFeature;	
		g_TargetFeatureChain[pos].s_iChainSum = 0;
		g_TargetFeatureChain[pos].s_iOldChainSum = 0;
		g_TargetFeatureChain[pos].s_all_x = 0;
		g_TargetFeatureChain[pos].s_all_y = 0;
		g_TargetFeatureChain[pos].s_Last_C_Col = 0;
		g_TargetFeatureChain[pos].s_Last_C_Row = 0;
		g_TargetFeatureChain[pos].s_missFrameSum = 0;
		g_TargetFeatureChain[pos].s_featureError = 1000.0f;
		g_TargetFeatureChain[pos].s_UpdateState = 0;
		g_TargetFeatureChain[pos].s_TargetNumForUpdate = 0;
	
        g_TargetFeatureChain[pos].s_Pihao=PiHao;////2013.8.14
		g_iTargetNumInChain++;
	
	}	
}
//==========================================================================================
/*函数作用：把第一帧中检测到的目标tarFeature加入到目标链g_TargetFeatureChain的pos位置处
  输入：tarFeature：要加入到目标链中的目标
        pos：要加入目标链的目标在目标链中的位置
  输出：g_TargetFeatureChain：加入新目标后的目标链
        g_iTargetNumInChain：目标链中目标的数目
*/
void AddTargetInChain_NPH(TARFEATURE tarFeature, int pos)
{
  
	if (pos + 1 <= MAX_FEAT_NUM)
	{
		g_TargetFeatureChain[pos].s_Feature = tarFeature;	
		g_TargetFeatureChain[pos].s_iChainSum = 0;
		g_TargetFeatureChain[pos].s_iOldChainSum = 0;
		g_TargetFeatureChain[pos].s_all_x = 0;
		g_TargetFeatureChain[pos].s_all_y = 0;
		g_TargetFeatureChain[pos].s_Last_C_Col = 0;
		g_TargetFeatureChain[pos].s_Last_C_Row = 0;
		g_TargetFeatureChain[pos].s_missFrameSum = 0;
		g_TargetFeatureChain[pos].s_featureError = 1000.0f;
		g_TargetFeatureChain[pos].s_UpdateState = 0;
		g_TargetFeatureChain[pos].s_TargetNumForUpdate = 0;
	
        g_TargetFeatureChain[pos].s_Pihao = 0;////2013.8.14
		g_iTargetNumInChain++;
	}	
}

//==========================================================================================
/*函数作用：把目标tarFeature加入到小目标链g_CurTargetFeatureChain的pos位置处
  输入：tarFeature：要加入到小目标链中的目标
        pos：要加入小目标链的目标在小目标链中的位置
  输出：g_CurTargetFeatureChain：加入新目标后的目标链
        g_iTargetNumInCurChain：目标链中目标的数目
*/
void AddTargetInCurChain(TARFEATURE tarFeature, int pos)
{
	if (pos + 1 <= MAX_FEAT_NUM)
	{
		g_CurTargetFeatureChain[pos].s_Feature = tarFeature;	
		g_CurTargetFeatureChain[pos].s_iChainSum = 0;
		g_CurTargetFeatureChain[pos].s_iOldChainSum = 0;
		g_CurTargetFeatureChain[pos].s_all_x = 0;
		g_CurTargetFeatureChain[pos].s_all_y = 0;
		g_CurTargetFeatureChain[pos].s_Last_C_Col = 0;
		g_CurTargetFeatureChain[pos].s_Last_C_Row = 0;
		g_CurTargetFeatureChain[pos].s_missFrameSum = 0;
		g_CurTargetFeatureChain[pos].s_featureError = 1000.0f;
		g_CurTargetFeatureChain[pos].s_UpdateState = 0;
		g_CurTargetFeatureChain[pos].s_TargetNumForUpdate = 0;

        g_CurTargetFeatureChain[pos].s_Pihao=NewObjPiHao;////2013.8.14
		g_iTargetNumInCurChain++;
	}	
}

//==========================================================================================
/*函数作用：把目标tarFeature加入到小目标链g_CurTargetFeatureChain的pos位置处
  输入：tarFeature：要加入到小目标链中的目标
        pos：要加入小目标链的目标在小目标链中的位置
  输出：g_CurTargetFeatureChain：加入新目标后的目标链
        g_iTargetNumInCurChain：目标链中目标的数目
*/
void AddTargetInCurChain2(TARFEATURE tarFeature, int pos,CHAIN tarChain)
{
	if (pos + 1 <= MAX_FEAT_NUM)
	{
		g_CurTargetFeatureChain[pos].s_Feature = tarFeature;		
        g_CurTargetFeatureChain[pos].s_Pihao=tarChain.s_Pihao;
		g_iTargetNumInCurChain++;
	}	
}

//==========================================================================================
/*函数作用：用目标tarFeature更新目标链中pos位置处的目标
  输入：tarFeature：新的目标
        pos：需要进行更新的目标在目标链中的位置
  输出：g_TargetFeatureChain：更新之后的目标链
*/
void UpdateTargetInChain(TARFEATURE tarFeature, int pos)
{
	g_TargetFeatureChain[pos].s_iChainSum++;
	g_TargetFeatureChain[pos].s_Feature = tarFeature;
	g_TargetFeatureChain[pos].s_all_x += g_TargetFeatureChain[pos].s_Feature.s_C_Col - g_TargetFeatureChain[pos].s_Last_C_Col;//更新all_x
	g_TargetFeatureChain[pos].s_all_y += g_TargetFeatureChain[pos].s_Feature.s_C_Row - g_TargetFeatureChain[pos].s_Last_C_Row;//更新all_y
	g_TargetFeatureChain[pos].s_Last_C_Col = g_TargetFeatureChain[pos].s_Feature.s_C_Col;
	g_TargetFeatureChain[pos].s_Last_C_Row = g_TargetFeatureChain[pos].s_Feature.s_C_Row;
	g_TargetFeatureChain[pos].s_featureError = 1000.0f;
	g_TargetFeatureChain[pos].s_UpdateState = 0;
	g_TargetFeatureChain[pos].s_TargetNumForUpdate = 0;

}
//==========================================================================================
/*函数作用：给没有批号的目标加入批号
  输入：NewPH：要加入的批号
  输出：g_TargetFeatureChain：更新之后的目标链
        g_iTargetNumInChain：更新之后的目标链中目标的数目
*/
void AddNewPH(unsigned char NewPH)
{
	int i,NewIndex = 200;
    unsigned int MaxAssociateCount = 0;
	for (i = 0; i < g_iTargetNumInChain; i++)
	{
	  if(g_TargetFeatureChain[i].s_Pihao == 0 && g_TargetFeatureChain[i].s_iChainSum > MaxAssociateCount)
	  {
		  MaxAssociateCount = g_TargetFeatureChain[i].s_iChainSum;
          NewIndex = i;
	  }
	}
    if(NewIndex<200)
	{
      g_TargetFeatureChain[NewIndex].s_Pihao = NewPH;
	  NewObjPiHao = 0;
    }

	
}
//==========================================================================================
/*函数作用：删除目标链中pos位置处的目标，并使其后面的目标依次前移一个位置
  输入：pos：目标链中要删除的目标的位置
  输出：g_TargetFeatureChain：更新之后的目标链
        g_iTargetNumInChain：更新之后的目标链中目标的数目
*/
void DeleteTargetInChain(int pos)
{
	int i;
	
	if (pos <= g_iTargetNumInChain-1)
	{
		for (i = pos; i < g_iTargetNumInChain-1; i++)
		{
			g_TargetFeatureChain[i] = g_TargetFeatureChain[i+1];
		}
		SetTargetInChainZero(g_iTargetNumInChain-1);
		g_iTargetNumInChain--;

		if(g_iTargetNumInChain==0)
		{
         g_iFrameCount=0;
		 MaxPiHao = 0;
        }

	}
}
//==========================================================================================
/*函数作用：计算目标距离视场中心的距离
  输入：srcFeat：要比较的目标
  输出：fDifference：目标到视场中心的距离
*/
float  DiffDis(TARFEATURE srcFeat)
{
	float fDifference = 0; 
	fDifference = abs_f(srcFeat.s_C_fangweiPixel, 351.0) + abs_f(srcFeat.s_C_fuyangPixel, 144.0); 

	return fDifference;
}
//==========================================================================================
/*函数作用：计算两个目标之间的位置偏差，也可以理解成计算两个目标之间的距离
  输入：srcFeat，destFeat：要比较的两个目标
  输出：fDifference：两个目标之间的位置偏差
*/
float  DiffFeat(TARFEATURE srcFeat, TARFEATURE destFeat)
{
	float fDifference = 0; 
	fDifference = abs_f(srcFeat.s_C_fangweiPixel, destFeat.s_C_fangweiPixel) + abs_f(srcFeat.s_C_fuyangPixel, destFeat.s_C_fuyangPixel); 

	return fDifference;
}

//==========================================================================================
/*函数作用：计算两个目标之间的位置偏差，也可以理解成计算两个目标之间的距离
  入：srcFeat，destFeat：要比较的两个目标
  输出：fDifference：两个目标之间的位置偏差
*/
float  DiffFeatAngle(TARFEATURE srcFeat, TARFEATURE destFeat)
{
	float fDifference = 0; 
	fDifference = abs_f(srcFeat.s_C_Row, destFeat.s_C_Row) + abs_f(srcFeat.s_C_Col, destFeat.s_C_Col); 

	return fDifference;
}
//==========================================================================================
/*函数作用：计算两个目标之间的面积偏差
  入：srcFeat，destFeat：要比较的两个目标
  输出：fDifference：两个目标之间的面积偏差
*/
float  DiffFeatArea(TARFEATURE srcFeat, TARFEATURE destFeat)
{
	float fDifference = 0; 
	fDifference = abs_f(srcFeat.s_Area, destFeat.s_Area);

	return fDifference;
}
//==========================================================================================
/*函数作用：把目标链中的每个目标与当前帧中检测到的每个目标进行比较，找到与目标链中每个目标属于同一帧图像且距离最小的目标，
            若这两个目标之间的距离小于某个阈值，则认为这两个目标是同一个目标，更新目标链中的那个目标特征为当前帧中检测到的特征。
            若当前帧中的某个目标与目标链中的所有目标都不匹配，则认为此目标为新目标，加入到目标链中
  输入：candidate[]：当前帧中检测到的目标数组
        PossTarNum：当前帧中检测到的目标数目
  输出：g_TargetFeatureChain：更新之后的目标链
        g_iTargetNumInChain：更新之后的目标链中目标的数目
*/
void FeatAssociate(TARFEATURE candidate[], int PossTarNum)
{
	int i, j, k;
	int Object_Count=0;
	float featureError = 0.0f;
	float featureAngleError = 0.0f;
	float featureAreaError=0.0f;
	int addcount = 0,tempTargetNumInChain = 0;

	tempTargetNumInChain = g_iTargetNumInChain;///2014.03.03-3
	for (i = 0; i < PossTarNum; i++)
	{
		for(j = 0; j < tempTargetNumInChain; j++)///2014.03.03-3
		{
			featureAngleError = DiffFeatAngle(candidate[i], g_TargetFeatureChain[j].s_Feature);////计算两个目标的空间坐标偏差
			featureError = DiffFeat(candidate[i], g_TargetFeatureChain[j].s_Feature);
			featureAreaError=DiffFeatArea(candidate[i], g_TargetFeatureChain[j].s_Feature);
			if (featureError < MINASSOCIATEERROR)//featureAngleError< MINASSOCIATEANGLEERROR)
			{
				if (featureAreaError < g_TargetFeatureChain[j].s_featureError)
				{				 
					g_TargetFeatureChain[j].s_featureError = featureAreaError;
					g_TargetFeatureChain[j].s_UpdateState = 1;
					g_TargetFeatureChain[j].s_TargetNumForUpdate = i;						
				}
			}
			else
			{
				addcount++;
				if (addcount == g_iTargetNumInChain)
				{
				    if(NewObjPHNum > 0)///2014.03.03-5
					{
					  NewObjPiHao = NewObjPH[0];///2014.03.03-5
					  for(k=0;k<NewObjPHNum-1;k++)///2014.03.03-5
					  {
					   NewObjPH[k] = NewObjPH[k+1];///2014.03.03-5
					  }

                      NewObjPHNum--;///2014.03.03-5
					  AddTargetInChain_PH(candidate[i], g_iTargetNumInChain,NewObjPiHao);
                      
					}
					else
					{
					  if(MaxPiHao < 10)
				      {	
					    MaxPiHao++;
				        AddTargetInChain_PH(candidate[i], g_iTargetNumInChain,MaxPiHao);
				      }
				      else
					  {
					    AddTargetInChain_NPH(candidate[i], g_iTargetNumInChain);
				      }
					}

				}
			}			
		}
		addcount = 0;
	}

	for (i = 0; i < g_iTargetNumInChain; i++)
	{
		if (g_TargetFeatureChain[i].s_UpdateState == 1)
		{
		    Object_Count=g_TargetFeatureChain[i].s_TargetNumForUpdate;
			UpdateTargetInChain(candidate[ Object_Count], i);
			if(g_TargetFeatureChain[i].s_iChainSum > 0)///2014.02.13
			{///////2014.02.13
			  AddTargetInCurChain2(candidate[Object_Count],g_iTargetNumInCurChain,g_TargetFeatureChain[i]);
			}///////2014.02.13
			
		}	
	}
}

//==========================================================================================================
/*函数作用：检测目标链中的目标跟丢的次数，若次数大于阈值，则把这个目标从目标链中删除
  输入：无
  输出：g_TargetFeatureChain：更新之后的目标链
*/
void ProcessLostTarget()
{
	int i;

	for (i = 0; i < g_iTargetNumInChain; i++)
	{
		if (g_TargetFeatureChain[i].s_iChainSum - g_TargetFeatureChain[i].s_iOldChainSum <= 0)
		{
			g_TargetFeatureChain[i].s_missFrameSum++;
		}
		else
		{
			g_TargetFeatureChain[i].s_missFrameSum = 0;
		}

		g_TargetFeatureChain[i].s_iOldChainSum = g_TargetFeatureChain[i].s_iChainSum;

		if (g_TargetFeatureChain[i].s_missFrameSum >= MISSFRAME)
		{
		    NewObjPiHao = g_TargetFeatureChain[i].s_Pihao;///2014.02.24
			AddNewPH(NewObjPiHao);////2014.02.24
            if(NewObjPiHao > 0)///2014.03.03-5
			{
			 NewObjPH[NewObjPHNum] = NewObjPiHao;///2014.03.03-5
             NewObjPHNum++;///2014.03.03-5
			}///2014.03.03-5

			DeleteTargetInChain(i);				
		}
	}
}

//==========================================================================================================
/*函数作用：首先建立目标链，然后根据后续视频图像中检测到的目标不断的更新目标链
  输入：tarFeature：图像中检测到的目标g_Candidate
        PossTarNum：图像中检测到的目标个数
  输出：tarFeature：图像中检测到的目标g_Candidate
*/
void Associate(TARFEATURE tarFeature[], int PossTarNum)
{
    
	int i;
    
	if(PossTarNum>0)
	   g_iFrameCount++;

	if (g_iFrameCount <= CHANGE_MODE_FRAME)
	{
		/************************纯累积目标，建链阶段*********************************/

		if(g_iFrameCount == 1)
		{
			if (g_iTargetNumInChain == 0)
			{
				if (PossTarNum > MAX_FEAT_NUM)
				{
					return;
				}
                if(PossTarNum <= 10)
				{
				   for (i = 0; i < PossTarNum; i++)
				   {
				        MaxPiHao++;
				    	AddTargetInChain_PH(tarFeature[i], i, MaxPiHao);			   	  
				   }	
				}
				else
				{
				  for (i = 0; i < 10; i++)
				   {
				        MaxPiHao++;
				    	AddTargetInChain_PH(tarFeature[i], i, MaxPiHao);
			   	   
				   }
				  for (i = 10; i < PossTarNum; i++)
				   {
				    	AddTargetInChain_NPH(tarFeature[i], i);
				   }
				}		
			}
		}
		else
		{
			FeatAssociate(tarFeature, PossTarNum);
		}
	}
	else
	{
		/**************建链阶段完毕，目标丢失的检测开始********************************/

		for(i = 0; i < g_iTargetNumInChain; i++)
		{
			g_TargetFeatureChain[i].s_iOldChainSum = g_TargetFeatureChain[i].s_iChainSum;
		}

		FeatAssociate(tarFeature, PossTarNum);
		ProcessLostTarget();

		if(g_iFrameCount > 65535)
		{
			g_iFrameCount = CHANGE_MODE_FRAME+1;
		}
	}
	
	for (i = 0; i < g_iTargetNumInChain; i++)
	{
		if(g_TargetFeatureChain[i].s_iChainSum > 65535)
			g_TargetFeatureChain[i].s_iChainSum = 0;					
	}
}






