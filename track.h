/************************************************************************
	filename:   track.h	
	purpose:   ���ٹ��ܵ�ʵ��
*************************************************************************/
#ifndef _TRACK_H_
#define _TRACK_H_


#define 	MAX_AREA_COUNT_NUM    3
#define 	TRACKWIN_ROW 		  20   ///2014.02.25
#define 	TRACKWIN_COL          20
#define 	TRACKWIN_ROW1 		  40   ///2014.02.25
#define 	TRACKWIN_COL1         40

#define		IMAGEWIDTH				640			//ͼ����
#define		IMAGEHIGTH				512			//ͼ����
#define		LEFTBORDER				1			//ͼ������߽�
#define		TOPBORDER				1			//ͼ�����ϱ߽�
#define		RIGHTBORDER				640			//ͼ�����ұ߽�
#define		BOTTOMBORDER			512			//ͼ�����±߽�

#define		MINASSOCIATEERROR       20          //Ŀ��ͼ�������������/////2013.8.4,ԭΪ#define		MINASSOCIATEERROR       60

void AllImageSmallTarTrack();
void MobanMatch();
void SetTrackWindowSize(int left, int right, int top, int down);
void AutoAriSmallTarTrack();

#endif

