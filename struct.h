// struct.h
#ifndef  _STRUCT_
#define  _STRUCT_
////////////////////////////////////////////////////////////////////

typedef struct
{
	short Row;
	short CStart;
	short CEnd;
	short Label;
	short MaxGray;
	short MeanGray;
}RUNLENGTHCODE;

typedef struct
{
   short m_StartRow;
   short m_EndRow;
   short m_StartCol;
   short m_EndCol;
}PROCWIN;

/////Map目标聚类
typedef struct
{
	float s_Area;
	float s_PeakLight;
	float s_MaxHeight;
	float s_MaxWidth ;
	float s_MeanLight;
	//float s_StdLight_Big;
	//float s_StdLight_Small;
	int s_C_Row;
	int s_C_Col;
	//float s_EntryBi;
    short	  s_C_fangweiPixel;			//存放像素值
	short	  s_C_fuyangPixel;			//存放像素值
	short     Frame_Code;

}TARFEATURE;

typedef struct
{
	TARFEATURE s_Feature;
	int        s_iChainSum;  		//关联次数
	int        s_iOldChainSum; 		//之前的关联次
	int        s_missFrameSum; 		//目标丢失帧数

	float      s_all_x;
	float      s_all_y;
	float      s_Last_C_Col;		//前一帧的Col
	float      s_Last_C_Row;		//前一帧的Row
	char       s_UpdateState;       //是否需要更新状态
	char       s_TargetNumForUpdate;
	float      s_featureError;
	char       s_Pihao;            //当前目标的批号//2013.8.14
}CHAIN;

#endif
