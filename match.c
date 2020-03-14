/************************************************************************
	filename:   Match.h
	created:	
	author:		

	purpose:
*************************************************************************/
#include "match.h"

extern int			            center_x;
extern int		            	center_y,lostnum;
extern unsigned char              *pSrc1 ;
extern unsigned short	        SEARCHWITH;

float m_mobanmean=0,m_mobanmean2=0,delta_light=0;
volatile unsigned char     *pmoban	      = (unsigned char*)0x00800400;
unsigned int MAD_count=0;
unsigned char unmatch_count = 0,match_count = 0,man_match = 0,moban_flag = 0;
float		        	m_corthresh=0.70; // m_corthresh=0.80;
float CORRX=0.0f,CORRX_DSP4 = 0.0f;

extern unsigned short	g_mobanWidth;				//MOBANWITH;48
extern unsigned short	g_mobanHeight;  			//MOBANHITH;30
extern volatile int	g_bTrackMatch,CALIBRATIONX,CALIBRATIONY;


/*
*/
void getmoban_wlp(int hnx, int hny)
{
	int i,j;
	int tempPos;
	for(i = hnx - g_mobanWidth/2; i < hnx + g_mobanWidth/2; i++)
		for (j = hny - g_mobanHeight/2; j < hny + g_mobanHeight/2; j++) 
		{ 
			tempPos = i - (((unsigned int)hnx) - g_mobanWidth/2) + g_mobanWidth * (j - (((unsigned int)hny) - g_mobanHeight/2));     
			pmoban[tempPos] = pSrc1[i+j*IMAGEWIDTH];
			m_mobanmean+=pSrc1[i+j*g_mobanWidth];
		}

	m_mobanmean=m_mobanmean/(g_mobanWidth*g_mobanHeight);
}

//==========================================================================================================
/*
*/
void match_MAD_Ground()
{
	int i,j,bound_x,bound_y,k,l;
	volatile int cresult_x,cresult_y,cor_m,cor_t,temp_x,temp_y;
	volatile unsigned int add_all,add_record;
	volatile float corvalue,cornorm,corrx;


	bound_x = center_x - g_mobanWidth/2;
	bound_y = center_y - g_mobanHeight/2;
	temp_x = center_x;
	temp_y = center_y;
	add_record = 0xffffffff;

   	MAD_count++;

	for(i=-SEARCHWITH/2;i<SEARCHWITH/2;i=i+STEP)
		for(j=-SEARCHWITH/2;j<SEARCHWITH/2;j=j+STEP)
		{
			add_all=0;
			/////////////////////////////////////////////////////////////////////
			for(k=bound_x+i;k<bound_x+i+g_mobanWidth;k=k+RESOLUTION)
			{
				for(l=bound_y+j;l< bound_y+j+g_mobanHeight;l=l+RESOLUTION)
				{
					if(add_all <= add_record)
					{
						add_all += abs(pmoban[k-(bound_x+i)+g_mobanWidth*(l- (bound_y+j))]-pSrc1[k+IMAGEWIDTH*l]);
					}
					else
					{
					 l=bound_y+j+g_mobanHeight;
					 k=bound_x+i+g_mobanWidth;
					 break;
					}
				}
			}
			///////////////////////////////////////////////////////////////////////////
			if(add_all<add_record)
			{
				add_record=add_all;
				cresult_x=i;
				cresult_y=j;
			}
		}

	add_record=0xffffffff;

	bound_x=center_x+cresult_x-g_mobanWidth/2;
	bound_y=center_y+cresult_y-g_mobanHeight/2;

	for(i=-STEP;i<STEP;i++)
		for(j=-STEP;j<STEP;j++)
		{
			add_all=0;
			for(k=bound_x+i;k<bound_x+i+g_mobanWidth;k++)
				for(l=bound_y+j;l< bound_y+j+g_mobanHeight;l++)
				{
					add_all+=abs(pmoban[k-(bound_x+i)+g_mobanWidth*(l- (bound_y+j))]-pSrc1[k+IMAGEWIDTH*l]);
				}

				if(add_all < add_record)
				{
					add_record = add_all;
					temp_x = i + bound_x + g_mobanWidth/2;
					temp_y = j + bound_y + g_mobanHeight/2;
				}
		}

	center_x = temp_x;
	center_y = temp_y;
	cor_m = add_record;
	corvalue = 0;
	cornorm = 0;

	for(k = center_x-g_mobanWidth/2;k<center_x+g_mobanWidth/2;k++)
		for(l = center_y-g_mobanHeight/2;l<center_y+g_mobanHeight/2;l++)
	 	{
		 	corvalue +=pSrc1[k+IMAGEWIDTH*l]*pmoban[(unsigned int)(k-center_x+g_mobanWidth/2)+g_mobanWidth*(unsigned int)(l-center_y+g_mobanHeight/2)];
		 	cornorm +=pSrc1[k+IMAGEWIDTH*l]*pSrc1[k+IMAGEWIDTH*l];
		}
	corrx=(corvalue/cornorm);
	if(corrx>1)
		 corrx = 2-corrx;



		CORRX=corrx;

		if((center_x < 10) || (center_x > 630) || (center_y < 10) || (center_y > 500))
		{

			man_match=0;
		}

		if(corrx >= 0.98)
		{
			match_count++;
			unmatch_count = 0;
		}
		else if( corrx <0.98)
			unmatch_count++;
		else if(corrx < 0.7 && unmatch_count > 2)
		{

			man_match= 0;

		}
}

//==========================================================================================================
/*
*/
void match_MAD()
{
	int i,j,bound_x,bound_y,k,l;
	volatile int cresult_x,cresult_y,cor_m,cor_t,temp_x,temp_y;
	volatile unsigned int add_all,add_record;
	volatile float corvalue,cornorm,corrx;


	bound_x = center_x - g_mobanWidth/2;
	bound_y = center_y - g_mobanHeight/2;
	temp_x = center_x;
	temp_y = center_y;
	add_record = 0xffffffff;

   	MAD_count++;

	for(i=-SEARCHWITH/2;i<SEARCHWITH/2;i=i+STEP)
		for(j=-SEARCHWITH/2;j<SEARCHWITH/2;j=j+STEP)
		{
			add_all=0;
			/////////////////////////////////////////////////////////////////////
			for(k=bound_x+i;k<bound_x+i+g_mobanWidth;k=k+RESOLUTION)
			{
				for(l=bound_y+j;l< bound_y+j+g_mobanHeight;l=l+RESOLUTION)
				{
					if(add_all <= add_record)
					{
						add_all += abs(pmoban[k-(bound_x+i)+g_mobanWidth*(l- (bound_y+j))]-pSrc1[k+IMAGEWIDTH*l]);
					}
					else
					{
					 l=bound_y+j+g_mobanHeight;
					 k=bound_x+i+g_mobanWidth;
					 break;
					}
				}
			}
			///////////////////////////////////////////////////////////////////////////
			if(add_all<add_record)
			{
				add_record=add_all;
				cresult_x=i;
				cresult_y=j;
			}
		}

	add_record=0xffffffff;

	bound_x=center_x+cresult_x-g_mobanWidth/2;
	bound_y=center_y+cresult_y-g_mobanHeight/2;

	for(i=-STEP;i<STEP;i++)
		for(j=-STEP;j<STEP;j++)
		{
			add_all=0;
			for(k=bound_x+i;k<bound_x+i+g_mobanWidth;k++)
				for(l=bound_y+j;l< bound_y+j+g_mobanHeight;l++)
				{
					add_all+=abs(pmoban[k-(bound_x+i)+g_mobanWidth*(l- (bound_y+j))]-pSrc1[k+IMAGEWIDTH*l]);
				}

				if(add_all < add_record)
				{
					add_record = add_all;
					temp_x = i + bound_x + g_mobanWidth/2;
					temp_y = j + bound_y + g_mobanHeight/2;
				}
		}

	center_x = temp_x;
	center_y = temp_y;
	cor_m = add_record;
	corvalue = 0;
	cornorm = 0;

	for(k = center_x-g_mobanWidth/2;k<center_x+g_mobanWidth/2;k++)
		for(l = center_y-g_mobanHeight/2;l<center_y+g_mobanHeight/2;l++)
	 	{
		 	corvalue +=pSrc1[k+IMAGEWIDTH*l]*pmoban[(unsigned int)(k-center_x+g_mobanWidth/2)+g_mobanWidth*(unsigned int)(l-center_y+g_mobanHeight/2)];
		 	cornorm +=pSrc1[k+IMAGEWIDTH*l]*pSrc1[k+IMAGEWIDTH*l];
		}
	corrx=(corvalue/cornorm);
	if(corrx>1)
		 corrx = 2-corrx;


	if(corrx < m_corthresh)
	{
		g_bTrackMatch=0;
	}
}



