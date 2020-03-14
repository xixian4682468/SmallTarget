#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_


//#define     SearchTrackAllRegion    0xAA  //È«Ö¡ËÑ¸ú
#define     SearchTrackInGate       0xBB  // ²¨ÃÅÄÚËÑ¸ú
#define     SearchAlarm             0xCC  // ËÑË÷¸æ¾¯
#define     JumpAlarm               0x11  // ½×Ô¾¸æ¾¯
#define     ExitTrack               0x55  // ÍË³ö¸ú×Ù

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
