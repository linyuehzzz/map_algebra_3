/**************************************
Author:�֫h
Date:2018/5/4
Update:2018/5/4
Description:����任����ŷʽ����ģ��
***************************************/
#pragma once
#include "DistanceTemplet.h"
class CEu5Tmp :
	public CDistanceTemplet
{
public:
	CEu5Tmp();
	~CEu5Tmp();
	int TmpSize();
	float GetTmpDis(int i);
	int GetOffX(int i);
	int GetOffY(int i);

private:
	float a_Mtx[9];
};

