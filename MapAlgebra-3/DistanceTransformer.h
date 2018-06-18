#pragma once
#include <stdio.h>
#include <windows.h>
#include <assert.h>
#include <math.h>
#include <vector>
#include "OctTmp.h"
#include "Eu5Tmp.h"

#define MaxFloat 9999999999999.0
#define MinFloat 0.0000000001
using namespace std;
class Point
{
public:
	int x;
	int y;
	unsigned char color;
};

class DistanceTransformer
{
public:
	DistanceTransformer(FILE* SrcFileStream, int Height, int Width, int LineBytes);
	~DistanceTransformer();
	void OctDistanceO();
	void EuDistanceO();
	int TmpDistanceTransformO(CDistanceTemplet *pTemplet);
	void EuDistanceD();
	int TmpDistanceTransformD(CDistanceTemplet *pTemplet);
	void OctDistanceI();
	void EuDistanceI();
	int TmpDistanceTransformI(CDistanceTemplet *pTemplet);

	unsigned char ** LocMtx;
	float ** DisMtx;
	vector<Point* > pts;

private:	
	COctTmp* OTmp;
	CEu5Tmp* E5Tmp;
	int Height;
	int Width;
	int LineBytes;
	FILE* SrcFileStream;
};

