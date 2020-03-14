/*
 * net.h
 *
 *  Created on: 2014-4-15
 *      Author: 771
 */

#ifndef NET_H_
#define NET_H_


//void TaskNetInit(unsigned char NetType);
void TaskNetInit();
void NetworkOpen();
void daemonTCP(IPN LocalAddress, unsigned int LocalPort);
void daemonUDP();
void CreatServer();
void NetworkClose();
void NetworkIPAddr( IPN IPAddr, uint IfIdx, uint fAdd );
void MainFun();
void MainProcess();
void SelectInitialTrackRegion();
void ProcWinAdapt();
void SUANFAFUWEI();

void FPGA_Config();

void ReceiveCommfromPC1();
void ReceiveCommfromPC2();
void DDR_Init();
void SendCommandToPC();
void SendCommandToPC1();
void SendCommandToPC2();
void SendCommandToPC4();
void SendCommandToPC3();


#endif /* NET_H_ */
