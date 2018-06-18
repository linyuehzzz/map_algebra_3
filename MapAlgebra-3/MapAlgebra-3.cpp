// MapAlgebra-3.cpp : 定义控制台应用程序的入口点。
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

