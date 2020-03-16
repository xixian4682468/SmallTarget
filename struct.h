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

/////MapĿ�����
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
    short	  s_C_fangweiPixel;			//�������ֵ
	short	  s_C_fuyangPixel;			//�������ֵ
	short     Frame_Code;

}TARFEATURE;

typedef struct
{
	TARFEATURE s_Feature;
	int        s_iChainSum;  		//��������
	int        s_iOldChainSum; 		//֮ǰ�Ĺ�����
	int        s_missFrameSum; 		//Ŀ�궪ʧ֡��

	float      s_all_x;
	float      s_all_y;
	float      s_Last_C_Col;		//ǰһ֡��Col
	float      s_Last_C_Row;		//ǰһ֡��Row
	char       s_UpdateState;       //�Ƿ���Ҫ����״̬
	char       s_TargetNumForUpdate;
	float      s_featureError;
	char       s_Pihao;            //��ǰĿ�������//2013.8.14
}CHAIN;

#endif
