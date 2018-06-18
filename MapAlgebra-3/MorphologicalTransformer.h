#pragma once
#include "DistanceTransformer.h"
#include <cmath>

class MorphologicalTransformer
{
public:
	MorphologicalTransformer(const char* SrcBmpName1, const char* SrcBmpName2, 
		const char* OutBufName, const char* OutVorName, const char* OutAxisName, const char* OutDelName);
	~MorphologicalTransformer();

	int Buffer(float radius);
	int Voronoi();
	int Axis();
	int Delaunay();

	float ** BufMtx;
	unsigned char ** vMtx;
	unsigned char ** aMtx;
	unsigned char ** dMtx;

private:
	void DDAline(int x0, int y0, int xEnd, int yEnd);
	const char* SrcBmpName1;
	const char* SrcBmpName2;
	const char* OutBufName;
	const char* OutVorName;
	const char* OutAxisName;
	const char* OutDelName;
};

