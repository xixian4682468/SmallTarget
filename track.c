/************************************************************************
	filename:   track.c
	purpose:   ���ٹ��ܵ�ʵ��
*************************************************************************/

#include "track.h"
#include "struct.h"
#include "Detect.h"
#include "match.h"

unsigned short 	        g_trackFrameCounter = 1;

int 			g_lostFrameNum = 0;
int				g_bfullTrackWin = 1;
int				g_trackWinLeftBound, g_trackWinRightBound, g_trackWinTopBound, g_trackWinBottomBound;
int				g_bFirstMatch = 1;
unsigned int   * TrackWin_Enable  =  (unsigned int *) 0xC0000004;
int			    center_x;
int		        center_y;
int             lostnum=0;
unsigned short	        SEARCHWITH;
short					LastRow=256,LastCol=320,LastRow_DSP4 = 0,LastCol_DSP4 = 0;

extern volatile int g_bTrackMatch,CALIBRATIONX,CALIBRATIONY;              //�Ƿ���и���ƥ��

//volatile int g_iTargetNumInChain = 0;
//volatile int g_iTargetNumInCurChain = 0;
//volatile int g_iTrackObjCount=0;
//volatile int g_iFrameCount = 0;
unsigned short	g_mobanWidth = 48;				//MOBANWITH;48
unsigned short	g_mobanHeight = 30;  			//MOBANHITH;30

extern volatile int			m_PossTarNum,m_CandTarNum,RealTarNum;
extern volatile int            g_iTrackObjCount;

extern unsigned char  	     g_bSearchInTrackState;
extern TARFEATURE 		*g_Candidate;
extern TARFEATURE 		*g_sel_Candidate;
extern TARFEATURE 		g_ZeroFeat;
extern TARFEATURE      *tempFeat;

//==========================================================================================================

/*
void SetTrackWindowSize(int left, int right, int top, int down);
void MobanMatch();
void AutoAriSmallTarTrack();

*/
//==========================================================================================================
/*�������ã�Ŀ����٣�����ǰΪ����״̬�������SmallSearch()�������ҵ���ǰ֡ͼ���е�Ŀ�꣬����ð�ݷ����ҳ������������Ŀ�꣬
            ���������Ŀ�굱������ܵ�Ŀ�ꡣ����ǰΪ����״̬����ǰ50֡�ĸ���ͼ���У����ȵ���SmallSearch()�����ҵ���ǰ֡��
            ��Ŀ�꣬Ȼ���ҵ��������Ŀ����������Ŀ�ꡣ������Ŀ��ľ���С����ֵ������Ϊ���ٵ�Ŀ�꣬�ø��ٿ�����Ŀ�꣬����
            ���������͸��ٵ�Ŀ�����Ϣ��������Ŀ��ľ��������ֵ������Ϊû���ҵ������ٵ�Ŀ�꣬��ʾǰһ�����ٵ���Ŀ���λ�ÿ�
            �����������ǰһ�θ��ٵ���Ŀ�����Ϣ����������������5ʱ����������Ŀ�ꡣ
			�Ը���״̬��50֮֡���ͼ�����ȸ��ݴ�����Ŀ���λ�ù�����ٴ��ڵ�λ�ã�Ѱ�Ҹ��ٴ����ڵ��������Ŀ�꣬����Ŀ���
			���������ֵ������ƥ��ķ�ʽ��֤��Ŀ���Ƿ�Ϊ����Ŀ�꣬��Ŀ�����С����ֵ�����������Ŀ���λ�ü��������Ŀ���λ�ã�
			�������ٿ������������Ŀ�����Ϣ��
*/
void AllImageSmallTarTrack()
{
	int i, j;
	float minDif, tempDif;
	int tempPos = 0;
	int maxArea;
	int mindex;
	if(g_bSearchInTrackState == 1)
	{

		//--------------------------------����-----------------------------------
      if( m_PossTarNum > 0)
       {
         g_bfullTrackWin			= 1;
		 g_lostFrameNum 			= 0;
	     g_bFirstMatch 			= 1;
         for(i = 0; i < MAX_AREA_COUNT_NUM; i++)
	      {
	       for(j = m_PossTarNum-1; j > i; j--)
	       {
		       if( g_Candidate[j].s_Area > g_Candidate[j-1].s_Area)
	        	{
		        	tempFeat[0] 	 = g_Candidate[j];
		        	g_Candidate[j]   = g_Candidate[j-1];
		        	g_Candidate[j-1] = tempFeat[0];
	        	}
        	}
    	  }
    	 LastRow = g_Candidate[0].s_C_fuyangPixel;
         LastCol = g_Candidate[0].s_C_fangweiPixel;
         g_iTrackObjCount=1;
         g_sel_Candidate[0] = g_Candidate[0];
    	 g_bSearchInTrackState 	= 0;
	   }
      else
       {
		 g_bSearchInTrackState 	= 1;
       }
	}
	else
	{
       //-----------------------------����------------------------//
       minDif = 1000;
       if(g_trackFrameCounter < 50)
	   {
          for(i = 0; i < m_PossTarNum; i++)
		  {
			tempDif = DiffFeat(g_sel_Candidate[0], g_Candidate[i]);
			if(tempDif < minDif)
			{
				minDif = tempDif;
				tempPos = i;
			}
		  }
          if(minDif < 50 )
		  {
		    g_sel_Candidate[0] = g_Candidate[tempPos];
			g_lostFrameNum = 0;
			LastRow = g_sel_Candidate[0].s_C_fuyangPixel;
			LastCol = g_sel_Candidate[0].s_C_fangweiPixel;
			g_iTrackObjCount=1;
		  }
		  else
		  {
		    g_lostFrameNum++;
			g_iTrackObjCount=0;
		  }
		  if(g_lostFrameNum > 2)
		  {
				g_sel_Candidate[0] = g_ZeroFeat;
                g_bSearchInTrackState = 1;
				g_iTrackObjCount=0;
		  }
	   }
	   else
	   {
	        maxArea =0;
			minDif = 1000;
			g_bfullTrackWin = 0;
			//����һ����������64*40������С��������
			g_trackWinLeftBound   = g_sel_Candidate[0].s_C_fangweiPixel - TRACKWIN_COL;
			g_trackWinRightBound  = g_sel_Candidate[0].s_C_fangweiPixel + TRACKWIN_COL;
			g_trackWinTopBound    = g_sel_Candidate[0].s_C_fuyangPixel - TRACKWIN_ROW;
			g_trackWinBottomBound = g_sel_Candidate[0].s_C_fuyangPixel + TRACKWIN_ROW;
			SetTrackWindowSize(g_trackWinLeftBound, g_trackWinRightBound, g_trackWinTopBound, g_trackWinBottomBound);
			for(i = 0; i < m_PossTarNum; i++)
			{
				if ((g_Candidate[i].s_C_fangweiPixel > g_trackWinLeftBound)
				  &&(g_Candidate[i].s_C_fangweiPixel < g_trackWinRightBound)
				  &&(g_Candidate[i].s_C_fuyangPixel > g_trackWinTopBound)
				  &&(g_Candidate[i].s_C_fuyangPixel < g_trackWinBottomBound))
				{
					if(g_Candidate[i].s_Area > maxArea)
					{
						maxArea = g_Candidate[i].s_Area;
						mindex = i;
					}

				}
			}
			if(maxArea > 1000)
			 {
				g_bTrackMatch = 1;
				g_sel_Candidate[0] = g_Candidate[mindex];
			 }
			else
				g_bTrackMatch = 0;

			if(g_bTrackMatch == 0)
			{
			   if(maxArea > 0)
				{
					g_sel_Candidate[0] = g_Candidate[mindex];
					g_lostFrameNum = 0;
					LastRow =  0.0 * LastRow + 1.0 * g_sel_Candidate[0].s_C_fuyangPixel;/////////2014.02.12
					LastCol = 0.0 * LastCol + 1.0 * g_sel_Candidate[0].s_C_fangweiPixel;
					g_iTrackObjCount=1;

				}
				else
				{
					g_lostFrameNum++;
					g_iTrackObjCount=0;
				}

				if(g_lostFrameNum > 2)
				{
					g_sel_Candidate[0] = g_ZeroFeat;
					g_bfullTrackWin = 1;
					g_bSearchInTrackState = 1;
					TrackWin_Enable[0]=0x00000000;	//���ٿ�4��3��2��1��ֹ��
			    	g_iTrackObjCount=0;
				}

			}
			else
			{
		    	MobanMatch( );
			}
	   }
	   g_trackFrameCounter++;
	}
}




/*�������ã��Լ�⵽��Ŀ�����ƥ����١���Ϊ��һ��ƥ�䣬��ֻ����ģ�壻����ͨ����ģ���뵱ǰ֡ͼ��ƥ��
            ����ø���Ŀ���λ�ã�����Ŀ���÷���������
 ���룺g_sel_Candidate[0]������ٵĿ?
 �����LastRow����Ŀ�����λ��
       LastCol����Ŀ�����λ��
*/
void MobanMatch( )
{
	if(g_bFirstMatch == 1)
	{
		g_bFirstMatch = 0;

		g_mobanWidth  = 64;
        g_mobanHeight = 64;

        SEARCHWITH    = 32;

		getmoban_wlp(g_sel_Candidate[0].s_C_fangweiPixel, g_sel_Candidate[0].s_C_fuyangPixel);
		LastRow = 0.0 * LastRow + 1.0 * g_sel_Candidate[0].s_C_fuyangPixel;
		LastCol = 0.0 * LastCol + 1.0 * g_sel_Candidate[0].s_C_fangweiPixel;
		center_x=LastCol;
		center_y=LastRow;
		g_iTrackObjCount=1;

	}
	else
	{
		match_MAD();
		g_sel_Candidate[0].s_C_fuyangPixel = center_y;
		g_sel_Candidate[0].s_C_fangweiPixel = center_x;
		LastRow=0.0 * LastRow + 1.0* g_sel_Candidate[0].s_C_fuyangPixel;
		LastCol=0.0 * LastCol + 1.0* g_sel_Candidate[0].s_C_fangweiPixel;
		g_iTrackObjCount=1;

	}
}


//���ø��ٿ򴰿ڳߴ�
void SetTrackWindowSize(int left, int right, int top, int bottom)
{
	if(left < LEFTBORDER)
		g_trackWinLeftBound = LEFTBORDER;
	else
		g_trackWinLeftBound = left;

	if(right > RIGHTBORDER)
		g_trackWinRightBound = RIGHTBORDER;
	else
		g_trackWinRightBound = right;

	if(top < TOPBORDER)
		g_trackWinTopBound = TOPBORDER;
	else
		g_trackWinTopBound = top;

	if(bottom > BOTTOMBORDER)
		g_trackWinBottomBound = BOTTOMBORDER;
	else
		g_trackWinBottomBound = bottom;
}


/*�������ã�Ŀ����٣�����ǰΪ����״̬�������SmallSearch()�������ҵ���ǰ֡ͼ���е�Ŀ�꣬����ð�ݷ����ҳ������������Ŀ�꣬
            ���������Ŀ�굱������ܵ�Ŀ�ꡣ����ǰΪ����״̬����ǰ50֡�ĸ���ͼ���У����ȵ���SmallSearch()�����ҵ���ǰ֡��
            ��Ŀ�꣬Ȼ���ҵ��������Ŀ����������Ŀ�ꡣ������Ŀ��ľ���С����ֵ������Ϊ���ٵ�Ŀ�꣬�ø��ٿ�����Ŀ�꣬����
            ���������͸��ٵ�Ŀ�����Ϣ��������Ŀ��ľ��������ֵ������Ϊû���ҵ������ٵ�Ŀ�꣬��ʾǰһ�����ٵ���Ŀ���λ�ÿ�
            �����������ǰһ�θ��ٵ���Ŀ�����Ϣ����������������5ʱ����������Ŀ�ꡣ
			�Ը���״̬��50֮֡���ͼ�����ȸ��ݴ�����Ŀ���λ�ù�����ٴ��ڵ�λ�ã�Ѱ�Ҹ��ٴ����ڵ��������Ŀ�꣬����Ŀ���
			���������ֵ������ƥ��ķ�ʽ��֤��Ŀ���Ƿ�Ϊ����Ŀ�꣬��Ŀ�����С����ֵ�����������Ŀ���λ�ü��������Ŀ���λ�ã�
			�������ٿ������������Ŀ�����Ϣ��
*/
void AutoAriSmallTarTrack()
{
	int i, j;
	float minDif, tempDif;
	int tempPos = 0;
	int maxArea;
	int mindex;
	if(g_bSearchInTrackState == 1)
	{
		//--------------------------------����-----------------------------------
        g_bfullTrackWin			= 1;
		g_lostFrameNum 			= 0;
	    g_bFirstMatch 			= 1;
       for(i = 0; i < MAX_AREA_COUNT_NUM; i++)   //����
	   {
	       for(j = m_PossTarNum-1; j > i; j--)
	       {
		       if( g_Candidate[j].s_Area > g_Candidate[j-1].s_Area)
	        	{
		        	tempFeat[0] 	 = g_Candidate[j];
		        	g_Candidate[j]   = g_Candidate[j-1];
		        	g_Candidate[j-1] = tempFeat[0];
	        	}
        	}
    	}
		if( m_PossTarNum > 0)  //ѡ������Ŀ�꣬ѡ����Ŀ������һ֡����
     	{
        	LastRow = g_Candidate[0].s_C_fuyangPixel;
        	LastCol = g_Candidate[0].s_C_fangweiPixel;
            g_iTrackObjCount=1;
	        g_sel_Candidate[0] = g_Candidate[0];
	    	g_bSearchInTrackState 	= 0;

    	}
    	else
    	{
		    g_bSearchInTrackState 	= 1;
    	}
	}
	else
	{
       //-----------------------------����------------------------//
       minDif = 1000;
       if(g_trackFrameCounter < 50)
	   {
          for(i = 0; i < m_PossTarNum; i++)
		  {
			tempDif = DiffFeat(g_sel_Candidate[0], g_Candidate[i]);
			if(tempDif < minDif)
			{
				minDif = tempDif;
				tempPos = i;
			}
		  }
          if(minDif < MINASSOCIATEERROR )
		  {
		    g_sel_Candidate[0] = g_Candidate[tempPos];
			g_lostFrameNum = 0;
			LastRow = g_sel_Candidate[0].s_C_fuyangPixel;
			LastCol = g_sel_Candidate[0].s_C_fangweiPixel;
			g_iTrackObjCount=1;
		  }
		  else
		  {
		    g_lostFrameNum++;
			g_iTrackObjCount=0;
		  }
		  if(g_lostFrameNum > 2)
		  {
				g_sel_Candidate[0] = g_ZeroFeat;
                g_bSearchInTrackState = 1;
				g_iTrackObjCount=0;
		  }
	   }
	   else
	   {
	        maxArea =0;
			minDif = 1000;
			g_bfullTrackWin = 0;
			//����һ����������64*40������С��������
			g_trackWinLeftBound   = g_sel_Candidate[0].s_C_fangweiPixel - TRACKWIN_COL;
			g_trackWinRightBound  = g_sel_Candidate[0].s_C_fangweiPixel + TRACKWIN_COL;
			g_trackWinTopBound    = g_sel_Candidate[0].s_C_fuyangPixel - TRACKWIN_ROW;
			g_trackWinBottomBound = g_sel_Candidate[0].s_C_fuyangPixel + TRACKWIN_ROW;
			SetTrackWindowSize(g_trackWinLeftBound, g_trackWinRightBound, g_trackWinTopBound, g_trackWinBottomBound);
			for(i = 0; i < m_PossTarNum; i++)
			{
				if ((g_Candidate[i].s_C_fangweiPixel > g_trackWinLeftBound)
				  &&(g_Candidate[i].s_C_fangweiPixel < g_trackWinRightBound)
				  &&(g_Candidate[i].s_C_fuyangPixel > g_trackWinTopBound)
				  &&(g_Candidate[i].s_C_fuyangPixel < g_trackWinBottomBound))
				{
					if(g_Candidate[i].s_Area > maxArea)
					{
						maxArea = g_Candidate[i].s_Area;
						mindex = i;
					}

				}
			}
			if(maxArea > 1000)
			 {
				g_bTrackMatch = 1;
				g_sel_Candidate[0] = g_Candidate[mindex];
			 }
			else
				g_bTrackMatch = 0;

			if(g_bTrackMatch == 0)
			{
			   if(maxArea > 0)
				{
					g_sel_Candidate[0] = g_Candidate[mindex];
					g_lostFrameNum = 0;
					LastRow =  0.0 * LastRow + 1.0 * g_sel_Candidate[0].s_C_fuyangPixel;/////////2014.02.12
					LastCol = 0.0 * LastCol + 1.0 * g_sel_Candidate[0].s_C_fangweiPixel;
					g_iTrackObjCount=1;

				}
				else
				{
					g_lostFrameNum++;
					g_iTrackObjCount=0;
				}

				if(g_lostFrameNum > 2)
				{
					g_sel_Candidate[0] = g_ZeroFeat;
					g_bfullTrackWin = 1;
					g_bSearchInTrackState = 1;
					TrackWin_Enable[0]=0x00000000;	//���ٿ�4��3��2��1��ֹ��
			    	g_iTrackObjCount=0;
				}

			}
			else
			{
		    	MobanMatch( );
			}
	   }
	   g_trackFrameCounter++;
	}
}

