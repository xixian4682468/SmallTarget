#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_


//#define     SearchTrackAllRegion    0xAA  //ȫ֡�Ѹ�
#define     SearchTrackInGate       0xBB  // �������Ѹ�
#define     SearchAlarm             0xCC  // �����澯
#define     JumpAlarm               0x11  // ��Ծ�澯
#define     ExitTrack               0x55  // �˳�����

void Send_To_SifuAngle();
void ReceiveAnglefromSiFu();
void ReceiveCommandfromPC();
void SiFuTrackStart();
void  SiFuSearchStart();
void SiFuJump(long Fuyangjiao);
void SiFuTrackStop();
void SendTargetInfoToPC();
void DrawTrackWindow();
void ClearSearchWindow();
void SendTargetInfoToSiFu(short Y,short X,float Yzz,float Xzz);
void ClearTrackWindow();
void DrawSearchWindow();
void  SiFuSearchStop();


#endif
