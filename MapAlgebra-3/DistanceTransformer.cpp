#include "stdafx.h"
#include "DistanceTransformer.h"


DistanceTransformer::DistanceTransformer(FILE* SrcFileStream, int Height, int Width, int LineBytes)
{
	this->Height = Height;
	this->Width = Width;
	this->LineBytes = LineBytes;
	this->SrcFileStream = SrcFileStream;
}


DistanceTransformer::~DistanceTransformer()
{
	// 清理，释放内存
	if(LocMtx != 0x00)
	for (int j = 0; j < Height; j++)
	{
		delete[]LocMtx[j];
	}
	delete[]LocMtx;

	if (DisMtx != 0x00)
		for (int j = 0; j < Height; j++)
		{
			delete[]DisMtx[j];
		}
	delete[]DisMtx;
}

void DistanceTransformer::OctDistanceO()
{
	this->OTmp = new COctTmp();
	TmpDistanceTransformO(OTmp);
}

void DistanceTransformer::EuDistanceO()
{
	this->E5Tmp = new CEu5Tmp();
	TmpDistanceTransformO(E5Tmp);
}


int DistanceTransformer::TmpDistanceTransformO(CDistanceTemplet *pTemplet)
{
	// 1. 建立初始距离场、分配场
	LocMtx = new unsigned char *[Height];
	DisMtx = new float *[Height];

	for (int j = 0; j < Height; j++)
	{
		LocMtx[j] = new unsigned char[LineBytes];
		DisMtx[j] = new float[Width];
		fread(LocMtx[j], sizeof(char), LineBytes, SrcFileStream);
		for (int i = 0; i < Width; i++)
		{
			if (LocMtx[j][i] == 255) DisMtx[j][i] = MaxFloat;
			else DisMtx[j][i] = 0;
		}
	}

	// 2. 模板距离变换
	// 2.1 从左上到右下方向（Forward mask）
	for (int j = 0; j < Height; j++)
		for (int i = 0; i < Width; i++)
		{
			if (fabs(DisMtx[j][i]) < MinFloat) continue;
			float MinDis = DisMtx[j][i];

			for (int k = 0; k < pTemplet->TmpSize() / 2 + 1; k++)
			{
				int OffX = pTemplet->GetOffX(k);
				int OffY = pTemplet->GetOffY(k);
				float TmpDis = pTemplet->GetTmpDis(k);

				int x = i + OffX;
				int y = j + OffY;

				if (x < 0 || y < 0 || x > Width - 1 || y > Height - 1) continue;

				if (fabs(DisMtx[y][x] - MaxFloat) < MinFloat) continue; // 该位置如果是无穷大不可计算

				if (MinDis > DisMtx[y][x] + TmpDis)
				{
					MinDis = DisMtx[y][x] + TmpDis;
					DisMtx[j][i] = MinDis;
					LocMtx[j][i] = LocMtx[y][x];
				}
			}
		}
	// 2.2 从右下到左上方向（Backward mask）
	for (int j = Height - 1; j > 0; j--)
		for (int i = Width - 1; i > 0; i--)
		{
			if (fabs(DisMtx[j][i]) < MinFloat) continue;
			float MinDis = DisMtx[j][i];

			for (int k = pTemplet->TmpSize() / 2; k < pTemplet->TmpSize(); k++)
			{
				int OffX = pTemplet->GetOffX(k);
				int OffY = pTemplet->GetOffY(k);
				float TmpDis = pTemplet->GetTmpDis(k);

				int x = i + OffX;
				int y = j + OffY;

				if (x < 0 || y < 0 || x > Width - 1 || y > Height - 1) continue;

				if (fabs(DisMtx[y][x] - MaxFloat) < MinFloat) continue; // 该位置如果是无穷大不可计算

				if (MinDis > DisMtx[y][x] + TmpDis)
				{
					MinDis = DisMtx[y][x] + TmpDis;
					DisMtx[j][i] = MinDis;
					LocMtx[j][i] = LocMtx[y][x];
				}
			}
		}
	return 1;
};

void DistanceTransformer::EuDistanceD()
{
	this->E5Tmp = new CEu5Tmp();
	TmpDistanceTransformD(E5Tmp);
}

int DistanceTransformer::TmpDistanceTransformD(CDistanceTemplet *pTemplet)
{
	// 1. 建立初始距离场、分配场，并提取点坐标
	LocMtx = new unsigned char *[Height];
	DisMtx = new float *[Height];
	for (int j = 0; j < Height; j++)
	{
		LocMtx[j] = new unsigned char[LineBytes];
		DisMtx[j] = new float[Width];
		Point* pt = new Point();
		fread(LocMtx[j], sizeof(char), LineBytes, SrcFileStream);
		for (int i = 0; i < Width; i++)
		{
			if (LocMtx[j][i] == 255) DisMtx[j][i] = MaxFloat;
			else
			{
				DisMtx[j][i] = 0;
				pt->x = j;
				pt->y = i;
				pt->color = LocMtx[j][i];
				pts.push_back(pt);		
			}
		}
	}

	// 2. 模板距离变换
	// 2.1 从左上到右下方向（Forward mask）
	for (int j = 0; j < Height; j++)
		for (int i = 0; i < Width; i++)
		{
			if (fabs(DisMtx[j][i]) < MinFloat) continue;
			float MinDis = DisMtx[j][i];

			for (int k = 0; k < pTemplet->TmpSize() / 2 + 1; k++)
			{
				int OffX = pTemplet->GetOffX(k);
				int OffY = pTemplet->GetOffY(k);
				float TmpDis = pTemplet->GetTmpDis(k);

				int x = i + OffX;
				int y = j + OffY;

				if (x < 0 || y < 0 || x > Width - 1 || y > Height - 1) continue;

				if (fabs(DisMtx[y][x] - MaxFloat) < MinFloat) continue; // 该位置如果是无穷大不可计算

				if (MinDis > DisMtx[y][x] + TmpDis)
				{
					MinDis = DisMtx[y][x] + TmpDis;
					DisMtx[j][i] = MinDis;
					LocMtx[j][i] = LocMtx[y][x];
				}
			}
		}
	// 2.2 从右下到左上方向（Backward mask）
	for (int j = Height - 1; j > 0; j--)
		for (int i = Width - 1; i > 0; i--)
		{
			if (fabs(DisMtx[j][i]) < MinFloat) continue;
			float MinDis = DisMtx[j][i];

			for (int k = pTemplet->TmpSize() / 2; k < pTemplet->TmpSize(); k++)
			{
				int OffX = pTemplet->GetOffX(k);
				int OffY = pTemplet->GetOffY(k);
				float TmpDis = pTemplet->GetTmpDis(k);

				int x = i + OffX;
				int y = j + OffY;

				if (x < 0 || y < 0 || x > Width - 1 || y > Height - 1) continue;

				if (fabs(DisMtx[y][x] - MaxFloat) < MinFloat) continue; // 该位置如果是无穷大不可计算

				if (MinDis > DisMtx[y][x] + TmpDis)
				{
					MinDis = DisMtx[y][x] + TmpDis;
					DisMtx[j][i] = MinDis;
					LocMtx[j][i] = LocMtx[y][x];
				}
			}
		}
	return 1;
};

void DistanceTransformer::OctDistanceI()
{
	this->OTmp = new COctTmp();
	TmpDistanceTransformI(OTmp);
}

void DistanceTransformer::EuDistanceI()
{
	this->E5Tmp = new CEu5Tmp();
	TmpDistanceTransformI(E5Tmp);
}


int DistanceTransformer::TmpDistanceTransformI(CDistanceTemplet *pTemplet)
{
	// 1. 建立初始距离场、分配场
	LocMtx = new unsigned char *[Height];
	DisMtx = new float *[Height];

	for (int j = 0; j < Height; j++)
	{
		LocMtx[j] = new unsigned char[LineBytes];
		DisMtx[j] = new float[Width];
		fread(LocMtx[j], sizeof(char), LineBytes, SrcFileStream);
		for (int i = 1; i < Width; i++)
		{
			if (LocMtx[j][i] == 255) DisMtx[j][i] = 0;
			else DisMtx[j][i] = MaxFloat;
		}
	}

	for (int j = 0; j < Height; j++)
	{
		for (int i = 0; i < Width; i++)
		{
			if (i == 0 || j == 0 || i == Width - 1 || j == Height - 1) continue;
			if(LocMtx[j][i] != 255)
			if (LocMtx[j - 1][i - 1] ==  255 || LocMtx[j - 1][i] == 255
				|| LocMtx[j - 1][i + 1] == 255 || LocMtx[j][i - 1] == 255
				|| LocMtx[j][i + 1] == 255 || LocMtx[j + 1][i - 1] == 255
				|| LocMtx[j + 1][i] == 255 || LocMtx[j + 1][i + 1] == 255)
				DisMtx[j][i] = 0;
		}
	}
	// 2. 模板距离变换
	// 2.1 从左上到右下方向（Forward mask）
	for (int j = 0; j < Height; j++)
		for (int i = 0; i < Width; i++)
		{
			if (fabs(DisMtx[j][i]) < MinFloat) continue;
			float MinDis = DisMtx[j][i];

			for (int k = 0; k < pTemplet->TmpSize() / 2 + 1; k++)
			{
				int OffX = pTemplet->GetOffX(k);
				int OffY = pTemplet->GetOffY(k);
				float TmpDis = pTemplet->GetTmpDis(k);

				int x = i + OffX;
				int y = j + OffY;

				if (x < 0 || y < 0 || x > Width - 1 || y > Height - 1) continue;

				if (fabs(DisMtx[y][x] - MaxFloat) < MinFloat) continue; // 该位置如果是无穷大不可计算

				if (MinDis > DisMtx[y][x] + TmpDis)
				{
					MinDis = DisMtx[y][x] + TmpDis;
					DisMtx[j][i] = MinDis;
					LocMtx[j][i] = LocMtx[y][x];
				}
			}
		}
	// 2.2 从右下到左上方向（Backward mask）
	for (int j = Height - 1; j > 0; j--)
		for (int i = Width - 1; i > 0; i--)
		{
			if (fabs(DisMtx[j][i]) < MinFloat) continue;
			float MinDis = DisMtx[j][i];

			for (int k = pTemplet->TmpSize() / 2; k < pTemplet->TmpSize(); k++)
			{
				int OffX = pTemplet->GetOffX(k);
				int OffY = pTemplet->GetOffY(k);
				float TmpDis = pTemplet->GetTmpDis(k);

				int x = i + OffX;
				int y = j + OffY;

				if (x < 0 || y < 0 || x > Width - 1 || y > Height - 1) continue;

				if (fabs(DisMtx[y][x] - MaxFloat) < MinFloat) continue; // 该位置如果是无穷大不可计算

				if (MinDis > DisMtx[y][x] + TmpDis)
				{
					MinDis = DisMtx[y][x] + TmpDis;
					DisMtx[j][i] = MinDis;
					LocMtx[j][i] = LocMtx[y][x];
				}
			}
		}
	return 1;
};