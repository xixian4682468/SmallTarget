/************************************************************************
	filename:   track.c
	purpose:   跟踪功能的实现
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

extern volatile int g_bTrackMatch,CALIBRATIONX,CALIBRATIONY;              //是否进行跟踪匹配

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
/*函数作用：目标跟踪：若当前为搜索状态，则调用SmallSearch()函数，找到当前帧图像中的目标，并用冒泡方法找出面积最大的三个目标，
            把面积最大的目标当做最可能的目标。若当前为跟踪状态，在前50帧的跟踪图像中，首先调用SmallSearch()函数找到当前帧中
            的目标，然后找到与待跟踪目标距离最近的目标。若两个目标的距离小于阈值，则认为跟踪到目标，用跟踪框标出该目标，并向
            服务器发送跟踪到目标的信息。若两个目标的距离大于阈值，则认为没有找到待跟踪的目标，显示前一个跟踪到的目标的位置框，
            向服务器发送前一次跟踪到的目标的信息，当跟丢次数大于5时，重新搜索目标。
			对跟踪状态的50帧之后的图像，首先根据待跟踪目标的位置构造跟踪窗口的位置，寻找跟踪窗口内的面积最大的目标，若此目标的
			面积大于阈值，采用匹配的方式验证此目标是否为跟踪目标，若目标面积小于阈值，则根据两个目标的位置计算出跟踪目标的位置，
			画出跟踪框并向服务器发送目标的信息。
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

		//--------------------------------搜索-----------------------------------
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
       //-----------------------------跟踪------------------------//
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
			//开启一个搜索窗口64*40，进行小窗口搜索
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
					TrackWin_Enable[0]=0x00000000;	//跟踪框4、3、2、1禁止；
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




/*函数作用：对检测到的目标进行匹配跟踪。若为第一次匹配，则只创建模板；否则，通过把模板与当前帧图像匹配
            来获得跟踪目标的位置，并把目标用方框框出来。
 输入：g_sel_Candidate[0]：俚目?
 输出：LastRow：新目标的行位置
       LastCol：新目标的列位置
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


//设置跟踪框窗口尺寸
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


/*函数作用：目标跟踪：若当前为搜索状态，则调用SmallSearch()函数，找到当前帧图像中的目标，并用冒泡方法找出面积最大的三个目标，
            把面积最大的目标当做最可能的目标。若当前为跟踪状态，在前50帧的跟踪图像中，首先调用SmallSearch()函数找到当前帧中
            的目标，然后找到与待跟踪目标距离最近的目标。若两个目标的距离小于阈值，则认为跟踪到目标，用跟踪框标出该目标，并向
            服务器发送跟踪到目标的信息。若两个目标的距离大于阈值，则认为没有找到待跟踪的目标，显示前一个跟踪到的目标的位置框，
            向服务器发送前一次跟踪到的目标的信息，当跟丢次数大于5时，重新搜索目标。
			对跟踪状态的50帧之后的图像，首先根据待跟踪目标的位置构造跟踪窗口的位置，寻找跟踪窗口内的面积最大的目标，若此目标的
			面积大于阈值，采用匹配的方式验证此目标是否为跟踪目标，若目标面积小于阈值，则根据两个目标的位置计算出跟踪目标的位置，
			画出跟踪框并向服务器发送目标的信息。
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
		//--------------------------------搜索-----------------------------------
        g_bfullTrackWin			= 1;
		g_lostFrameNum 			= 0;
	    g_bFirstMatch 			= 1;
       for(i = 0; i < MAX_AREA_COUNT_NUM; i++)   //排序
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
		if( m_PossTarNum > 0)  //选面积最大目标，选出的目标在下一帧搜索
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
       //-----------------------------跟踪------------------------//
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
			//开启一个搜索窗口64*40，进行小窗口搜索
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
					TrackWin_Enable[0]=0x00000000;	//跟踪框4、3、2、1禁止；
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

