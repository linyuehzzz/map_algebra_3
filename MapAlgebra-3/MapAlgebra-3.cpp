// MapAlgebra-3.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "MorphologicalTransformer.h"

int main()
{
	MorphologicalTransformer* mf = new MorphologicalTransformer("point.bmp", "tst.bmp", "buf.bmp", "voronoi.bmp", "axis.bmp", "delaunay.bmp");
	mf->Buffer(10);
	mf->Voronoi();
	mf->Axis();
	mf->Delaunay();
    return 0;
}

