/************************************************************************
	filename:   track.h	
	purpose:   跟踪功能的实现
*************************************************************************/
#ifndef _TRACK_H_
#define _TRACK_H_


#define 	MAX_AREA_COUNT_NUM    3
#define 	TRACKWIN_ROW 		  20   ///2014.02.25
#define 	TRACKWIN_COL          20
#define 	TRACKWIN_ROW1 		  40   ///2014.02.25
#define 	TRACKWIN_COL1         40

#define		IMAGEWIDTH				640			//图像宽度
#define		IMAGEHIGTH				512			//图像宽度
#define		LEFTBORDER				1			//图像处理左边界
#define		TOPBORDER				1			//图像处理上边界
#define		RIGHTBORDER				640			//图像处理右边界
#define		BOTTOMBORDER			512			//图像处理下边界

#define		MINASSOCIATEERROR       20          //目标图像坐标关联距离/////2013.8.4,原为#define		MINASSOCIATEERROR       60

void AllImageSmallTarTrack();
void MobanMatch();
void SetTrackWindowSize(int left, int right, int top, int down);
void AutoAriSmallTarTrack();

#endif

