#pragma once
#include "BlmSerialComm.h"
#include <iostream>

class PELCO_D{
public:
	bool Init(int com);
	void Up();
	void Down();
	void Right();
	void Left();
	void Stop();
	void FocusNear();//焦距
	void FocusFar();
	void ZoomOut();//变倍
	void ZoomIn();
	void ApertureLarge();//光圈
	void ApertureSmall();
	bool Available();//判断云台是否可用
	void SetSpeed(int speed);
	int  GetSpeed();
private:
	SerialComm comm;
	int speed;
};