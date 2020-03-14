/************************************************************************
	filename:   Associate.h
	purpose:   ����Ŀ�������
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
#define		MAX_FEAT_NUM			200		    //����Ŀ�������Ŀ����(�ݶ�20,���Ŀ����Ŀֻ��16--cui��//200
CHAIN           *g_TargetFeatureChain   = (CHAIN *)(CHAIN_ADDR+0x00001320);	//Ŀ����
CHAIN           *g_CurTargetFeatureChain= (CHAIN *)(CHAIN_ADDR+0x000022F6);/////////��ǰ֡�й����ϵ�Ŀ�꼰�¼�⵽��Ŀ��///2013.8.14
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
/*�������ã���Ŀ��tarfeature��ֵ����Ϊ��ʼ��ֵ
  ���룺tarfeature��Ҫɾ����Ŀ��
  �������ʼ��ֵ֮���tarfeature
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
/*�������ã���Ŀ������posλ�ô���Ŀ���ֵ����Ϊ��ʼ��ֵ
  ���룺pos��Ŀ������Ҫɾ����Ŀ���λ��
  �����g_TargetFeatureChain������֮���Ŀ����
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
/*�������ã��ѵ�ǰ֡ͼ��Ŀ������posλ�ô���Ŀ���ֵ����Ϊ��ʼ��ֵ
  ���룺pos��Ŀ������Ҫɾ����Ŀ���λ��
  �����g_TargetFeatureChain������֮���Ŀ����
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
/*�������ã���ʼ��Ŀ����
  ���룺��
  �������ʼ��֮���Ŀ����
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
/*�������ã���ʼ����ǰͼ��Ŀ����
  ���룺��
  �������ʼ��֮��ĵ�ǰͼ��Ŀ����
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
/*�������ã���Ŀ��tarFeature���뵽Ŀ����g_TargetFeatureChain��posλ�ô�
  ���룺tarFeature��Ҫ���뵽Ŀ�����е�Ŀ��
        pos��Ҫ����Ŀ������Ŀ����Ŀ�����е�λ��
  �����g_TargetFeatureChain��������Ŀ����Ŀ����
        g_iTargetNumInChain��Ŀ������Ŀ�����Ŀ
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
/*�������ã��ѵ�һ֡�м�⵽��Ŀ��tarFeature���뵽Ŀ����g_TargetFeatureChain��posλ�ô�
  ���룺tarFeature��Ҫ���뵽Ŀ�����е�Ŀ��
        pos��Ҫ����Ŀ������Ŀ����Ŀ�����е�λ��
  �����g_TargetFeatureChain��������Ŀ����Ŀ����
        g_iTargetNumInChain��Ŀ������Ŀ�����Ŀ
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
/*�������ã���Ŀ��tarFeature���뵽СĿ����g_CurTargetFeatureChain��posλ�ô�
  ���룺tarFeature��Ҫ���뵽СĿ�����е�Ŀ��
        pos��Ҫ����СĿ������Ŀ����СĿ�����е�λ��
  �����g_CurTargetFeatureChain��������Ŀ����Ŀ����
        g_iTargetNumInCurChain��Ŀ������Ŀ�����Ŀ
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
/*�������ã���Ŀ��tarFeature���뵽СĿ����g_CurTargetFeatureChain��posλ�ô�
  ���룺tarFeature��Ҫ���뵽СĿ�����е�Ŀ��
        pos��Ҫ����СĿ������Ŀ����СĿ�����е�λ��
  �����g_CurTargetFeatureChain��������Ŀ����Ŀ����
        g_iTargetNumInCurChain��Ŀ������Ŀ�����Ŀ
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
/*�������ã���Ŀ��tarFeature����Ŀ������posλ�ô���Ŀ��
  ���룺tarFeature���µ�Ŀ��
        pos����Ҫ���и��µ�Ŀ����Ŀ�����е�λ��
  �����g_TargetFeatureChain������֮���Ŀ����
*/
void UpdateTargetInChain(TARFEATURE tarFeature, int pos)
{
	g_TargetFeatureChain[pos].s_iChainSum++;
	g_TargetFeatureChain[pos].s_Feature = tarFeature;
	g_TargetFeatureChain[pos].s_all_x += g_TargetFeatureChain[pos].s_Feature.s_C_Col - g_TargetFeatureChain[pos].s_Last_C_Col;//����all_x
	g_TargetFeatureChain[pos].s_all_y += g_TargetFeatureChain[pos].s_Feature.s_C_Row - g_TargetFeatureChain[pos].s_Last_C_Row;//����all_y
	g_TargetFeatureChain[pos].s_Last_C_Col = g_TargetFeatureChain[pos].s_Feature.s_C_Col;
	g_TargetFeatureChain[pos].s_Last_C_Row = g_TargetFeatureChain[pos].s_Feature.s_C_Row;
	g_TargetFeatureChain[pos].s_featureError = 1000.0f;
	g_TargetFeatureChain[pos].s_UpdateState = 0;
	g_TargetFeatureChain[pos].s_TargetNumForUpdate = 0;

}
//==========================================================================================
/*�������ã���û�����ŵ�Ŀ���������
  ���룺NewPH��Ҫ���������
  �����g_TargetFeatureChain������֮���Ŀ����
        g_iTargetNumInChain������֮���Ŀ������Ŀ�����Ŀ
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
/*�������ã�ɾ��Ŀ������posλ�ô���Ŀ�꣬��ʹ������Ŀ������ǰ��һ��λ��
  ���룺pos��Ŀ������Ҫɾ����Ŀ���λ��
  �����g_TargetFeatureChain������֮���Ŀ����
        g_iTargetNumInChain������֮���Ŀ������Ŀ�����Ŀ
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
/*�������ã�����Ŀ������ӳ����ĵľ���
  ���룺srcFeat��Ҫ�Ƚϵ�Ŀ��
  �����fDifference��Ŀ�굽�ӳ����ĵľ���
*/
float  DiffDis(TARFEATURE srcFeat)
{
	float fDifference = 0; 
	fDifference = abs_f(srcFeat.s_C_fangweiPixel, 351.0) + abs_f(srcFeat.s_C_fuyangPixel, 144.0); 

	return fDifference;
}
//==========================================================================================
/*�������ã���������Ŀ��֮���λ��ƫ�Ҳ�������ɼ�������Ŀ��֮��ľ���
  ���룺srcFeat��destFeat��Ҫ�Ƚϵ�����Ŀ��
  �����fDifference������Ŀ��֮���λ��ƫ��
*/
float  DiffFeat(TARFEATURE srcFeat, TARFEATURE destFeat)
{
	float fDifference = 0; 
	fDifference = abs_f(srcFeat.s_C_fangweiPixel, destFeat.s_C_fangweiPixel) + abs_f(srcFeat.s_C_fuyangPixel, destFeat.s_C_fuyangPixel); 

	return fDifference;
}

//==========================================================================================
/*�������ã���������Ŀ��֮���λ��ƫ�Ҳ�������ɼ�������Ŀ��֮��ľ���
  �룺srcFeat��destFeat��Ҫ�Ƚϵ�����Ŀ��
  �����fDifference������Ŀ��֮���λ��ƫ��
*/
float  DiffFeatAngle(TARFEATURE srcFeat, TARFEATURE destFeat)
{
	float fDifference = 0; 
	fDifference = abs_f(srcFeat.s_C_Row, destFeat.s_C_Row) + abs_f(srcFeat.s_C_Col, destFeat.s_C_Col); 

	return fDifference;
}
//==========================================================================================
/*�������ã���������Ŀ��֮������ƫ��
  �룺srcFeat��destFeat��Ҫ�Ƚϵ�����Ŀ��
  �����fDifference������Ŀ��֮������ƫ��
*/
float  DiffFeatArea(TARFEATURE srcFeat, TARFEATURE destFeat)
{
	float fDifference = 0; 
	fDifference = abs_f(srcFeat.s_Area, destFeat.s_Area);

	return fDifference;
}
//==========================================================================================
/*�������ã���Ŀ�����е�ÿ��Ŀ���뵱ǰ֡�м�⵽��ÿ��Ŀ����бȽϣ��ҵ���Ŀ������ÿ��Ŀ������ͬһ֡ͼ���Ҿ�����С��Ŀ�꣬
            ��������Ŀ��֮��ľ���С��ĳ����ֵ������Ϊ������Ŀ����ͬһ��Ŀ�꣬����Ŀ�����е��Ǹ�Ŀ������Ϊ��ǰ֡�м�⵽��������
            ����ǰ֡�е�ĳ��Ŀ����Ŀ�����е�����Ŀ�궼��ƥ�䣬����Ϊ��Ŀ��Ϊ��Ŀ�꣬���뵽Ŀ������
  ���룺candidate[]����ǰ֡�м�⵽��Ŀ������
        PossTarNum����ǰ֡�м�⵽��Ŀ����Ŀ
  �����g_TargetFeatureChain������֮���Ŀ����
        g_iTargetNumInChain������֮���Ŀ������Ŀ�����Ŀ
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
			featureAngleError = DiffFeatAngle(candidate[i], g_TargetFeatureChain[j].s_Feature);////��������Ŀ��Ŀռ�����ƫ��
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
/*�������ã����Ŀ�����е�Ŀ������Ĵ�����������������ֵ��������Ŀ���Ŀ������ɾ��
  ���룺��
  �����g_TargetFeatureChain������֮���Ŀ����
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
/*�������ã����Ƚ���Ŀ������Ȼ����ݺ�����Ƶͼ���м�⵽��Ŀ�겻�ϵĸ���Ŀ����
  ���룺tarFeature��ͼ���м�⵽��Ŀ��g_Candidate
        PossTarNum��ͼ���м�⵽��Ŀ�����
  �����tarFeature��ͼ���м�⵽��Ŀ��g_Candidate
*/
void Associate(TARFEATURE tarFeature[], int PossTarNum)
{
    
	int i;
    
	if(PossTarNum>0)
	   g_iFrameCount++;

	if (g_iFrameCount <= CHANGE_MODE_FRAME)
	{
		/************************���ۻ�Ŀ�꣬�����׶�*********************************/

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
		/**************�����׶���ϣ�Ŀ�궪ʧ�ļ�⿪ʼ********************************/

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






