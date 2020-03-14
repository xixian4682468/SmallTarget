
#ifndef  _VARIABLE_
#define  _VARIABLE_
////////////////////////////////////////////////////////////////////

#define		SEG_ADDR	  0x0c208400  //游程码起始地址   ?
volatile unsigned int  *DDRAddress   = (unsigned int *)0x80000000;
unsigned char           *DDRTest      = (unsigned char*)0x80000000;
char			         *pImageInDSP;

unsigned char           StartSearchLabel=0;
unsigned char           SiFuSearchOpen=1;
unsigned char           StartJumpLabel=0;
unsigned char           SiFuJumpOpen=1;
unsigned char           ExitOpen=1;
unsigned char 	         Seg_Code_Over = 0;
unsigned char           YouChengOrImage=0;

unsigned short          FPGA_Frame_Num,Pro_Frame_Num;

unsigned short         *Seg_PaDuan;
unsigned short          Seg_Judge[8] = {0,0,0,0,0,0,0,0};
unsigned short	         *SEG_RAM;
volatile int 			  Seg_Count;

int          			 g_fangweijiao = 0;
int          			 g_fuyangjiao = 0;
unsigned int            Jump_Num=0;
int                      Jump_Start_fuyangjiao;


unsigned char			*RS422_R0		= (unsigned char*)0x80100000;  	//
unsigned char			*RS422_R1		= (unsigned char*)0x80000000;  	//
unsigned char			*RS422_R2		= (unsigned char*)0x80100000;  	//
unsigned char			*RS422_R3		= (unsigned char*)0x80000000;  	//

unsigned short			*RS422_T0		= (unsigned short*)0x80300000;  	//
unsigned short			*RS422_T1		= (unsigned short*)0x80200000;  	//
unsigned short			*RS422_T2		= (unsigned short*)0x80300000;  	//
unsigned short			*RS422_T3		= (unsigned short*)0x80200000;  	//


/*
/////////////////////////////////////////////////////////////////////
unsigned int			*COM_RAM_R0		= (unsigned int*)0xD0100000;  	//接收上位机命令缓存地址
unsigned int			*COM_RAM_R1		= (unsigned int*)0xD0000000;  	//接收转台命令缓存地址
unsigned int			*COM_RAM_T0		= (unsigned int*)0xD0300000;  	//发送目标信息至上位机缓存地址
unsigned int			*COM_RAM_T1		= (unsigned int*)0xD0200000;  	//发送目标信息至转台缓存地址

int                     g_gunzhuanjiao=0;
*/

#endif
