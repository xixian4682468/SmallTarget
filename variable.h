
#ifndef  _VARIABLE_
#define  _VARIABLE_
////////////////////////////////////////////////////////////////////

#define		SEG_ADDR	  0x0c208400  //游程码起始地址   ?

char			         *pImageInDSP;

unsigned char           SiFuSearchOpen=1;
unsigned char           StartJumpLabel=0;
unsigned char           SiFuJumpOpen=1;
unsigned char           ExitOpen=1;
unsigned char 	        Seg_Code_Over = 0;
unsigned char           YouChengOrImage=0;

unsigned short          FPGA_Frame_Num,Pro_Frame_Num;

unsigned short         *Seg_PaDuan;
unsigned short          Seg_Judge[8] = {0,0,0,0,0,0,0,0};
unsigned short	        *SEG_RAM;
volatile int 			Seg_Count;

int          			g_fangweijiao = 0;
int          			g_fuyangjiao = 0;
unsigned int            Jump_Num=0;
int                     Jump_Start_fuyangjiao;


#endif
