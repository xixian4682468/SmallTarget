/************************************************************************
	filename:   Feature.h
	purpose:   Ã·»°Ãÿ’˜
*************************************************************************/
#ifndef _FEATURE_H_
#define _FEATURE_H_
#include   "struct.h"

int TarFeatCredit(TARFEATURE m_CFeature,short StartRow,short EndRow,short StartCol,short EndCol);
int TarFeatCredit_FPGA(TARFEATURE m_CFeature,short StartRow,short EndRow,short StartCol,short EndCol);
float CmEntry(unsigned char *src,unsigned short row, unsigned short col);
float CmStd(unsigned char *src,unsigned short row, unsigned short col,unsigned char radius);
void ExtractFeature(short StartRow,short EndRow, short StartCol,short EndCol);
void ExtractFeature_FPGA(short StartRow,short EndRow, short StartCol,short EndCol);
void CopyCandidate(TARFEATURE* g_Candidate, TARFEATURE* Candidate);
void ConvertTargetCood2Angle(int col, int row);

#endif

