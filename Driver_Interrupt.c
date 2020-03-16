#include  "Driver_Timer.h"
#include  "Driver_Interrupt.h"
#include <cslr_gpio.h>
#include <ti/csl/src/intc/_csl_intc.h>
#include <ti/bios/include/std.h>
#include <ti/sysbios/family/c64p/Hwi.h>

#include   "variable.h"
#include   "struct.h"


RUNLENGTHCODE      *Seg_Code      = (RUNLENGTHCODE  * )(SEG_ADDR);


unsigned long long g_startcount,g_endcount,g_count;
unsigned int g_Rval;
extern volatile unsigned int gSRIORecvDataFlag = 0;
extern volatile unsigned int gEMIFRecvDataFlag = 0;
extern RUNLENGTHCODE    *Seg_Code;
extern char Flag_Image;
///////////////////////////////////////////////////////////////////////


void ReceiveSmallRunLenth()
{
	unsigned int i = 0;
	unsigned int jj=0,k1,k2,k3,k4,k5,k6 ;
	unsigned int Seg_Num = 0;
	unsigned int SEG1,SEG2,SEG3,SEG4,SEG5,SEG6;

	unsigned short*   SmallRunLenth_ADDR_BASE = (unsigned short*) 0x74000000;  //CE1

	for(i=0;i<32;i++)
	 {
		*Seg_PaDuan = *SmallRunLenth_ADDR_BASE;
		 Seg_PaDuan ++;
		 SmallRunLenth_ADDR_BASE++;
	 }

	for(i=0;i<8;i++)
	{
	    Seg_Judge[jj] = Seg_PaDuan[i];
	}

    k1 = Seg_PaDuan[0] ;
	k2 = Seg_PaDuan[1] ;
    k3 = Seg_PaDuan[2];
	k4 = Seg_PaDuan[3];
    k5 = Seg_PaDuan[4];
	k6 = Seg_PaDuan[5];

	//if(	Seg_Judge[2] == Seg_Judge[3] && Seg_Judge[3] == Seg_Judge[4] && Seg_Judge[4] == Seg_Judge[5] && Seg_Judge[5] == Seg_Judge[6] && Seg_Judge[6] != 0 )
    if(	k2 ==k3 && k3 == k4 && k4 == k5 && k5 == k6 && k6!= 0 )
	{

		Seg_Num = k1;	// 3C  �γ̶���

		if(Seg_Num<2040 && Seg_Num>0)
		{
			for(i=0; i<16*Seg_Num; i++)
			{
				*(SEG_RAM+i) = *(SmallRunLenth_ADDR_BASE + 0x10 + i);  //16���ֽ�һ��
			}
			for(i=0;i<Seg_Num;i++)
			{
				SEG1 = SEG_RAM[0 + 8*i ];	//���
				SEG2 = SEG_RAM[1 + 8*i] ;	//���Ҷ�
				SEG3 = SEG_RAM[2 + 8*i] ;	//ƽ���Ҷ�
				SEG4 = SEG_RAM[3 + 8*i] ;	//�γ̳���
				SEG5 = SEG_RAM[4 + 8*i] ;	//��ʼ��
				SEG6 = SEG_RAM[5 + 8*i];	//��ʼ��
                Seg_Code[i].MaxGray		= SEG2;
				Seg_Code[i].MeanGray	= SEG3;
				Seg_Code[i].CStart 		= SEG5;
				Seg_Code[i].CEnd   		= SEG5+SEG4-1;
				Seg_Code[i].Row	   		= SEG6;
			}
		}
	}
	else
	{
	  Seg_Num=0;
	}

	if(Seg_Num>3000)
	{
		Seg_Count=0;
	}
	else
	{
		Seg_Count = Seg_Num;    //   ͳ�ƶ���
	}

	Seg_Code_Over = 1;
}


void ISR_YouCheng()  //CSL_GEM_GPINT9
{
	gEMIFRecvDataFlag = 1;

    if (YouChengOrImage==0)//YouChengOrImage==0:����FPGA����õ����γ̣�YouChengOrImage==1��ֱ�Ӷ�ԭʼͼ�����������γ�
	{
      ReceiveSmallRunLenth();
	}
	else
	{
	  Seg_Code_Over = 1;
	}

}



