
#ifndef THHHH
#define THHHH
#include<stdlib.h>
void Convolution(unsigned char *DstImage, unsigned char *SrcImage);
void ImgErosion_Grey(unsigned char *imgBufIn,unsigned char *imgBufOut,int imgWidth,int imgHeight);//∏Ø ¥	
void ImgDilation_Grey(unsigned char *imgBufIn,unsigned char *imgBufOut,int imgWidth,int imgHeight);//≈Ú’Õ
void ImgDilation_Bin(unsigned char *imgBufIn,unsigned char *imgBufOut,int imgWidth,int imgHeight);
void Open();
void Close();
void ImgSubtract(unsigned char *imgSrc,unsigned char *imgDst,int imgWidth,int imgHeight);
float mean(unsigned char *imgSrc,int imgWidth,int imgHeight);
float std(unsigned char *imgSrc,int imgWidth,int imgHeight,float pingjun);
void BinValue(unsigned char *imgSrc,int imgWidth,int imgHeight,float yuzhi);
unsigned char max(unsigned char yuansu[69]);
unsigned char min(unsigned char yuansu[69]);

#endif
