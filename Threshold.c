
#include"Threshold.h"

int Element=0;
int l;
int m_imgWidth  = 640;
int m_imgHeight = 512;
extern unsigned char *imgBufDstZone;                //做差后的图像                
extern unsigned char *imgBufSrcZone;
extern unsigned char *imgBufSrc; 

void Convolution(unsigned char *DstImage, unsigned char *SrcImage)
{
    #define Dst(ROW,COL)   DstImage[ROW*m_imgWidth +COL]
    #define Src(ROW,COL)   SrcImage[ROW*m_imgWidth +COL]
	int i,j;
	int row1,row2,row3,col1,col2,col3;
	
	for(i = 0; i < m_imgHeight; i++)
	{
		for(j = 0; j < m_imgWidth; j++)
		{
			if((i< 18)||(i> 272)||(j<86)||(j> 634))///2014.04.25--1原为if((i< 18)||(i> 272)||(j<86)||(j> 634))
			{

				Dst(i,j) = Src(i,j);
			}
			else
			{
				row1 = i-1; row2 = i; row3 = i+1;
				col1 = j-1; col2 = j; col3 = j+1;		 
				Dst(i,j) = (Src(row1,col1) + 2*Src(row1,col2) + Src(row1,col3) +
							2*Src(row2,col1) + 4*Src(row2,col2) + 2*Src(row2,col3) +
							Src(row3,col1) + 2*Src(row3,col2) + Src(row3,col3))/16;
			}
		}
	}
}  

//**********************************************************************************
//腐蝕
//**********************************************************************************
void ImgErosion_Gray(unsigned char * imgBufIn,unsigned char * imgBufOut,int imgWidth,int imgHeight) 
{
	int i=0,j=0;

	unsigned char *yuansu=(unsigned char *)0x00870000;
	unsigned short erow1,erow2,erow3,erow4,erow5,ecol1,ecol2,ecol3,ecol4,ecol5;
#define EroOut(ROW,COL)   imgBufOut[ROW*m_imgWidth +COL]
#define EroIn(ROW,COL)    imgBufIn[ROW*m_imgWidth +COL]

	for(i = 0; i < m_imgHeight; i++)
	{
		for(j = 0; j < m_imgWidth; j++)
		{
			if((i< 30)||(i> 250)||(j<55)||(j> 650))
			{
				EroOut(i,j) = EroIn(i,j);
			}
			else
			{
				erow1 = i-2; erow2 = i-1; erow3 = i;erow4 = i+1; erow5 = i+2;
				ecol1 = j-2; ecol2 = j-1; ecol3 = j;ecol4 = j+1; ecol5 = j+2;	
				yuansu[0]  = EroIn(erow1,ecol1); yuansu[1]  = EroIn(erow1,ecol3); yuansu[2]  = EroIn(erow1,ecol5);
				yuansu[3]  = EroIn(erow2,ecol2); yuansu[4]  = EroIn(erow2,ecol3); yuansu[5]  = EroIn(erow2,ecol4);
				yuansu[6]  = EroIn(erow3,ecol1); yuansu[7]  = EroIn(erow3,ecol2); yuansu[8]  = EroIn(erow3,ecol3);
				yuansu[9]  = EroIn(erow3,ecol4); yuansu[10] = EroIn(erow3,ecol5); yuansu[11] = EroIn(erow4,ecol2);
				yuansu[12] = EroIn(erow4,ecol3); yuansu[13] = EroIn(erow4,ecol4); yuansu[14] = EroIn(erow5,ecol1);
				yuansu[15] = EroIn(erow5,ecol3); yuansu[16] = EroIn(erow5,ecol5);
				Element = 17;	 
				EroOut(i,j) = min(yuansu);
			}
		}
	}
}

//**********************************************************************************
//腐蝕
//**********************************************************************************
void ImgDilation_Gray(unsigned char * imgBufIn,unsigned char * imgBufOut,int imgWidth, int imgHeight) 
{
	int i=0,j=0;

	unsigned char *yuansu = (unsigned char *)0x00870000; //???
	unsigned short erow1,erow2,erow3,erow4,erow5,ecol1,ecol2,ecol3,ecol4,ecol5;
#define EroOut(ROW,COL)   imgBufOut[ROW*m_imgWidth +COL]
#define EroIn(ROW,COL)    imgBufIn[ROW*m_imgWidth +COL]
	for(i = 0; i < m_imgHeight; i++)
	{
		for(j = 0; j < m_imgWidth; j++)
		{
			if((i< 30)||(i> 250)||(j<55)||(j> 650))
			{
				EroOut(i,j) = EroIn(i,j);
			}
			else
			{
				erow1 = i-2; erow2 = i-1; erow3 = i;erow4 = i+1; erow5 = i+2;
				ecol1 = j-2; ecol2 = j-1; ecol3 = j;ecol4 = j+1; ecol5 = j+2;	
				yuansu[0]  = EroIn(erow1,ecol1); yuansu[1]  = EroIn(erow1,ecol3); yuansu[2]  = EroIn(erow1,ecol5);
				yuansu[3]  = EroIn(erow2,ecol2); yuansu[4]  = EroIn(erow2,ecol3); yuansu[5]  = EroIn(erow2,ecol4);
				yuansu[6]  = EroIn(erow3,ecol1); yuansu[7]  = EroIn(erow3,ecol2); yuansu[8]  = EroIn(erow3,ecol3);
				yuansu[9]  = EroIn(erow3,ecol4); yuansu[10] = EroIn(erow3,ecol5); yuansu[11] = EroIn(erow4,ecol2);
				yuansu[12] = EroIn(erow4,ecol3); yuansu[13] = EroIn(erow4,ecol4); yuansu[14] = EroIn(erow5,ecol1);
				yuansu[15] = EroIn(erow5,ecol3); yuansu[16] = EroIn(erow5,ecol5);
				Element = 17;	 
				EroOut(i,j) = max(yuansu);
			}
		}
	}
}
							 
void ImgDilation_Bin(unsigned char * imgBufIn, unsigned char * imgBufOut, int imgWidth, int imgHeight) 
{
	int i=0,j=0,s=0,t=0;
	char value = 0;
	//unsigned char *yuansu=(unsigned char *)0x00870000;
	unsigned short erow,ecol;
#define EroOut(ROW,COL)   imgBufOut[ROW*m_imgWidth +COL]
#define EroIn(ROW,COL)    imgBufIn[ROW*m_imgWidth +COL]
	for(i = 0; i < m_imgHeight; i++)
	{
		for(j = 0; j < m_imgWidth; j++)
		{
			if((i< 30)||(i> 250)||(j<55)||(j> 650))
			{
				EroOut(i,j) = EroIn(i,j);
			}
			else
			{
				value = 0;
				for(s=-3;s<=3;s++)
				{
					for(t=-5;t<=5;t++)
					{
						erow = i+s;
						ecol = j+t;

						if(EroIn(erow,ecol)==255)
						{
							value = 255;
							t = 5;
							s = 3;
						}
					}
				}
				EroOut(i,j) = value;
			}
		}
	}
} 

void ImgErosion(unsigned char * imgBufIn,unsigned char * imgBufOut,int imgWidth, int imgHeight) 
{
    int i=0,j=0;
    
	unsigned char *yuansu=(unsigned char *)0x00870000;
	unsigned short erow1,erow2,erow3,ecol1,ecol2,ecol3;
#define EroOutS(ROW,COL)   imgBufOut[ROW*imgWidth +COL]
#define EroInS(ROW,COL)    imgBufIn[ROW*imgWidth +COL]
	for(i = 0; i < imgHeight; i++)
    {
      for(j = 0; j < imgWidth; j++)
	  {
	    if((i== 0)||(i== imgHeight-1)||(j==0)||(j==imgWidth-1))
		{
		 
		  EroOutS(i,j) = EroInS(i,j);
		}
		else
		{
		  erow1 = i-1; erow2 = i;erow3 = i+1;
		  ecol1 = j-1; ecol2 = j;ecol3 = j+1; 
		  yuansu[0] = EroInS(erow1,ecol1);yuansu[1] = EroInS(erow1,ecol2);yuansu[2] = EroInS(erow1,ecol3);
		  yuansu[3] = EroInS(erow2,ecol1);yuansu[4] = EroInS(erow2,ecol2);yuansu[5] = EroInS(erow2,ecol3);
		  yuansu[6] = EroInS(erow3,ecol1);yuansu[7] = EroInS(erow3,ecol2);yuansu[8] = EroInS(erow3,ecol3);

		  Element = 9;	 
		  EroOutS(i,j) = min(yuansu);
		}
	  }
    }

}

void ImgDilation(unsigned char * imgBufIn,unsigned char * imgBufOut,int imgWidth, int imgHeight) 
{
    int i=0,j=0;
    
	unsigned char *yuansu=(unsigned char *)0x00870000;
	unsigned short erow1,erow2,erow3,ecol1,ecol2,ecol3;
#define DilOut(ROW,COL)   imgBufOut[ROW*imgWidth +COL]
#define DilIn(ROW,COL)    imgBufIn[ROW*imgWidth +COL]
	for(i = 0; i < imgHeight; i++)
	{
		for(j = 0; j < imgWidth; j++)
		{
			if((i== 0)||(i== imgHeight)||(j==0)||(j==imgWidth))
			{

				DilOut(i,j) = DilIn(i,j);
			}
			else
			{
				erow1 = i-1; erow2 = i;erow3 = i+1;
				ecol1 = j-1; ecol2 = j;ecol3 = j+1; 
				yuansu[0] = DilIn(erow1,ecol1);yuansu[1] = DilIn(erow1,ecol2);yuansu[2] = DilIn(erow1,ecol3);
				yuansu[3] = DilIn(erow2,ecol1);yuansu[4] = DilIn(erow2,ecol2);yuansu[5] = DilIn(erow2,ecol3);
				yuansu[6] = DilIn(erow3,ecol1);yuansu[7] = DilIn(erow3,ecol2);yuansu[8] = DilIn(erow3,ecol3);

				Element = 9;	 
				DilOut(i,j) = max(yuansu);
			}
		}
	}
} 


//**********************************************************************************
//開操作	
//**********************************************************************************						 
void Open(void)
{ 	
	unsigned char *buf=(unsigned char *)0x00880000;
	ImgErosion_Gray(imgBufDstZone, buf,m_imgWidth,m_imgHeight);//先腐蚀

	ImgDilation_Gray(buf, imgBufSrcZone,m_imgWidth,m_imgHeight);//后膨胀
}


//**********************************************************************************
//閉操作	
//**********************************************************************************
void Close(void)
{
    //中间结果缓冲区申请,用来存放腐蚀后的中间结果
    unsigned char *buf= (unsigned char*)malloc(m_imgWidth*m_imgHeight);

	
	//先膨胀
	ImgDilation_Gray(imgBufSrc, buf, m_imgWidth, m_imgHeight);

	//后腐蚀
	ImgErosion_Gray(buf, imgBufDstZone, m_imgWidth, m_imgHeight);

	//释放缓冲区
	free(buf);
}

unsigned char max(unsigned char yuansu[69])
{
	unsigned char maxValue=0;
	unsigned char ii=0;
	for(ii=0;ii<Element;ii++)
	{
		if(yuansu[ii]>maxValue)
			maxValue=yuansu[ii];
	}

	return maxValue;
}

unsigned char min(unsigned char yuansu[69])
{
	unsigned char minValue=255;
	unsigned char ii=0;
	for(ii=0;ii<Element;ii++)
	{
		if(yuansu[ii]<minValue)
			minValue=yuansu[ii];
	}

	return minValue;
}

void ImgSubtract(unsigned char * imgSrc,unsigned char * imgDst,int imgWidth,int imgHeight)
{
	int i=0,j=0;
	for(i=0;i<imgHeight;i++)
	{
		for(j=0;j<imgWidth;j++)
		{
			imgDst[j+i*imgWidth]=abs(imgSrc[j+i*imgWidth]-imgDst[j+i*imgWidth]);
		}
	}
}

float mean(unsigned char *  imgSrc,int imgWidth,int imgHeight)
{
	unsigned int sum=0;
	int i=0,j=0;
	float mean=0.0;
	for(i=30;i<=250;i++)
	{
		for(j=55;j<=650;j++)
		{
			sum+=imgSrc[j+i*imgWidth];
		}
	}
	mean=((float)sum)/(596*221);
	return mean;
}

float std(unsigned char * imgSrc,int imgWidth,int imgHeight,float pingjun)
{
	float std2=0.0;
	float sum=0.0;
	int i=0,j=0;
	for(i=30;i<=250;i++)
	{
		for(j=55;j<=650;j++)
		{
			sum+=(imgSrc[j+i*imgWidth]-pingjun)*(imgSrc[j+i*imgWidth]-pingjun);
		}
	}

	std2=sum/(596*221);

	std2=sqrt((int)std2);

	return std2;
} 

float mean_small(unsigned char *  imgSrc,int imgWidth,int imgHeight)
{
	unsigned int sum=0;
	int i=0,j=0;
	float mean=0.0;
	for(i=0;i<imgHeight;i++)
	{
		for(j=0;j<imgWidth;j++)
		{
			sum+=imgSrc[j+i*imgWidth];
		}
	}

	mean=((float)sum)/(imgWidth*imgHeight);

	return mean;
}

float std_small(unsigned char * imgSrc,int imgWidth,int imgHeight,float pingjun)
{
	float std2=0.0;
	float sum=0.0;
	int i=0,j=0;
	for(i=0;i<imgHeight;i++)
	{
		for(j=0;j<imgWidth;j++)
		{
			sum+=(imgSrc[j+i*imgWidth]-pingjun)*(imgSrc[j+i*imgWidth]-pingjun);
		}
	}

	std2=sum/(imgWidth*imgHeight);

	std2=sqrt(std2);
	return std2;
}

void BinValue(unsigned char * imgSrc,int imgWidth,int imgHeight,float yuzhi)
{
	int i=0,j=0;
	for(i=0;i<imgHeight;i++)
	{
		for(j=0;j<imgWidth;j++)
		{
			imgSrc[j+i*imgWidth]=imgSrc[j+i*imgWidth]>=yuzhi?255:0;
		}
	}
}

