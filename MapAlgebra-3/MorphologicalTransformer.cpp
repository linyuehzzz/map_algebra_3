#include "stdafx.h"
#include "MorphologicalTransformer.h"


MorphologicalTransformer::MorphologicalTransformer(const char* SrcBmpName1, const char* SrcBmpName2, 
	const char* OutBufName, const char* OutVorName, const char* OutAxisName, const char* OutDelName)
{
	this->SrcBmpName1 = SrcBmpName1;
	this->SrcBmpName2 = SrcBmpName2;
	this->OutBufName = OutBufName;
	this->OutVorName = OutVorName;
	this->OutAxisName = OutAxisName;
	this->OutDelName = OutDelName;
}


MorphologicalTransformer::~MorphologicalTransformer()
{

}

/// <summary>
/// 缓冲区变换
/// </summary>
int MorphologicalTransformer::Buffer(float radius)
{
	// 1. 读取bmp
	unsigned i, j; // 循环变量
	FILE* SrcFileStream = fopen(SrcBmpName1, "rb");
	if (SrcFileStream == NULL) return -1; // 出错代码1，文件不存在

	BITMAPFILEHEADER SrcFileHead;
	BITMAPINFOHEADER SrcFileInfo;
	unsigned char ColorIdx[1024];

	if (fread(&SrcFileHead, sizeof(BITMAPFILEHEADER), 1, SrcFileStream) != 1)
	{
		fclose(SrcFileStream);
		return -1;
	}
	if (fread(&SrcFileInfo, sizeof(BITMAPINFOHEADER), 1, SrcFileStream) != 1)
	{
		fclose(SrcFileStream);
		return -1;
	}
	if (fread(ColorIdx, 1024, 1, SrcFileStream) != 1)
	{
		fclose(SrcFileStream);
		return -1;
	}

	if (SrcFileInfo.biBitCount != 8) // 不是8bit位图
	{
		fclose(SrcFileStream);
		return -2; // 出错代码2，文件格式不合要求
	}

	int Height = SrcFileInfo.biHeight;
	int Width = SrcFileInfo.biWidth;
	int LineBytes = (SrcFileInfo.biWidth + 3) / 4 * 4;

	// 2. 距离变换
	DistanceTransformer* dt = new DistanceTransformer(SrcFileStream, Height, Width, LineBytes);
	dt->EuDistanceO();

	// 3. 创建缓冲区矩阵
	BufMtx = new float *[Height];
	for (int j = 0; j < Height; j++)
	{
		BufMtx[j] = new float[Width];
		for (int i = 0; i < Width; i++)
		{
			if (dt->DisMtx[j][i] <= radius) BufMtx[j][i] = dt->DisMtx[j][i];
			else BufMtx[j][i] = MaxFloat;
		}
	}

	// 4. 写入缓冲区矩阵32bitfloat型
	BITMAPFILEHEADER IdxFileHead; // 32bit位图头结构
	BITMAPINFOHEADER IdxFileInfo;

	IdxFileHead.bfType = SrcFileHead.bfType;
	IdxFileHead.bfSize = 54 + SrcFileInfo.biWidth * 4 * SrcFileInfo.biHeight;//
	IdxFileHead.bfReserved1 = 0;
	IdxFileHead.bfReserved2 = 0;
	IdxFileHead.bfOffBits = 54;//

	IdxFileInfo.biSize = 40;
	IdxFileInfo.biWidth = SrcFileInfo.biWidth;//
	IdxFileInfo.biHeight = SrcFileInfo.biHeight;//
	IdxFileInfo.biPlanes = 1;
	IdxFileInfo.biBitCount = 32;//
	IdxFileInfo.biCompression = 0;
	IdxFileInfo.biSizeImage = 0;
	IdxFileInfo.biXPelsPerMeter = 0;
	IdxFileInfo.biYPelsPerMeter = 0;
	IdxFileInfo.biClrUsed = 0;
	IdxFileInfo.biClrImportant = 0;

	FILE * bmpWrite = fopen(OutBufName, "wb");
	fwrite(&IdxFileHead, sizeof(BITMAPFILEHEADER), 1, bmpWrite);
	fwrite(&IdxFileInfo, sizeof(BITMAPINFOHEADER), 1, bmpWrite);
	for (int y = 0; y < Height; y++)
	{
		fwrite(BufMtx[y], sizeof(float), Width, bmpWrite);
	}
	fclose(bmpWrite);

	// 5. 清理，释放内存
	if (BufMtx != 0x00)
		for (int j = 0; j < Height; j++)
		{
			delete[]BufMtx[j];
		}
	delete[]BufMtx;
	if (dt != 0x00)
		delete dt;
	return 1;
};

/// <summary>
/// Voronoi图
/// </summary>
int MorphologicalTransformer::Voronoi()
{
	// 1. 读取bmp
	unsigned i, j; // 循环变量
	FILE* SrcFileStream = fopen(SrcBmpName1, "rb");
	if (SrcFileStream == NULL) return -1; // 出错代码1，文件不存在

	BITMAPFILEHEADER SrcFileHead;
	BITMAPINFOHEADER SrcFileInfo;
	unsigned char ColorIdx[1024];

	if (fread(&SrcFileHead, sizeof(BITMAPFILEHEADER), 1, SrcFileStream) != 1)
	{
		fclose(SrcFileStream);
		return -1;
	}
	if (fread(&SrcFileInfo, sizeof(BITMAPINFOHEADER), 1, SrcFileStream) != 1)
	{
		fclose(SrcFileStream);
		return -1;
	}
	if (fread(ColorIdx, 1024, 1, SrcFileStream) != 1)
	{
		fclose(SrcFileStream);
		return -1;
	}

	if (SrcFileInfo.biBitCount != 8) // 不是8bit位图
	{
		fclose(SrcFileStream);
		return -2; // 出错代码2，文件格式不合要求
	}

	int Height = SrcFileInfo.biHeight;
	int Width = SrcFileInfo.biWidth;
	int LineBytes = (SrcFileInfo.biWidth + 3) / 4 * 4;

	// 2. 距离变换
	DistanceTransformer* dt = new DistanceTransformer(SrcFileStream, Height, Width, LineBytes);
	dt->EuDistanceO();

	// 3. 创建颜色边界矩阵，即Voronoi图
	vMtx = new unsigned char *[Height];
	for (j = 0; j < Height; j++)
	{
		vMtx[j] = new unsigned char[LineBytes];
		for (i = 0; i < LineBytes; i++)
		{
			if (i == 0 || j == 0 || i == LineBytes - 1 || j == Height - 1)
			{
				vMtx[j][i] = 255;
				continue;
			}
			if (dt->LocMtx[j][i] != dt->LocMtx[j - 1][i - 1] || dt->LocMtx[j][i] != dt->LocMtx[j - 1][i]
				|| dt->LocMtx[j][i] != dt->LocMtx[j - 1][i + 1] || dt->LocMtx[j][i] != dt->LocMtx[j][i - 1]
				|| dt->LocMtx[j][i] != dt->LocMtx[j][i + 1] || dt->LocMtx[j][i] != dt->LocMtx[j + 1][i - 1]
				|| dt->LocMtx[j][i] != dt->LocMtx[j + 1][i] || dt->LocMtx[j][i] != dt->LocMtx[j + 1][i + 1])
				vMtx[j][i] = 0;
			else vMtx[j][i] = 255;
		}
	}

	// 4. 写入Voronoi矩阵8bitfloat型
	FILE * vWrite = fopen(OutVorName, "wb");
	fwrite(&SrcFileHead, sizeof(BITMAPFILEHEADER), 1, vWrite);
	fwrite(&SrcFileInfo, sizeof(BITMAPINFOHEADER), 1, vWrite);
	fwrite(ColorIdx, 1024, 1, vWrite);
	for (int y = 0; y < Height; y++)
	{
		fwrite(vMtx[y], sizeof(char), LineBytes, vWrite);
	}
	fclose(vWrite);


	// 5. 清理，释放内存
	if (vMtx != 0x00)
		for (int j = 0; j < Height; j++)
		{
			delete[]vMtx[j];
		}
	delete[]vMtx;
	if (dt != 0x00)
		delete dt;
	return 1;
};

/// <summary>
/// 中轴线提取
/// </summary>
int MorphologicalTransformer::Axis()
{
	// 1. 读取bmp
	unsigned i, j; // 循环变量
	FILE* SrcFileStream = fopen(SrcBmpName2, "rb");
	if (SrcFileStream == NULL) return -1; // 出错代码1，文件不存在

	BITMAPFILEHEADER SrcFileHead;
	BITMAPINFOHEADER SrcFileInfo;
	unsigned char ColorIdx[1024];

	if (fread(&SrcFileHead, sizeof(BITMAPFILEHEADER), 1, SrcFileStream) != 1)
	{
		fclose(SrcFileStream);
		return -1;
	}
	if (fread(&SrcFileInfo, sizeof(BITMAPINFOHEADER), 1, SrcFileStream) != 1)
	{
		fclose(SrcFileStream);
		return -1;
	}
	if (fread(ColorIdx, 1024, 1, SrcFileStream) != 1)
	{
		fclose(SrcFileStream);
		return -1;
	}

	if (SrcFileInfo.biBitCount != 8) // 不是8bit位图
	{
		fclose(SrcFileStream);
		return -2; // 出错代码2，文件格式不合要求
	}

	int Height = SrcFileInfo.biHeight;
	int Width = SrcFileInfo.biWidth;
	int LineBytes = (SrcFileInfo.biWidth + 3) / 4 * 4;

	// 2. 距离变换
	DistanceTransformer* dt = new DistanceTransformer(SrcFileStream, Height, Width, LineBytes);
	dt->EuDistanceI();

	// 3. 骨架提取
	aMtx = new unsigned char *[Height];
	for (j = 0; j < Height; j++)
	{
		aMtx[j] = new unsigned char[LineBytes];
		for (i = 0; i < LineBytes; i++)
		{
			if (i == 0 || j == 0 || i == LineBytes - 1 || j == Height - 1)
			{
				aMtx[j][i] = 255;
				continue;
			}
			if (dt->LocMtx[j][i] != dt->LocMtx[j - 1][i - 1] || dt->LocMtx[j][i] != dt->LocMtx[j - 1][i]
				|| dt->LocMtx[j][i] != dt->LocMtx[j - 1][i + 1] || dt->LocMtx[j][i] != dt->LocMtx[j][i - 1]
				|| dt->LocMtx[j][i] != dt->LocMtx[j][i + 1] || dt->LocMtx[j][i] != dt->LocMtx[j + 1][i - 1]
				|| dt->LocMtx[j][i] != dt->LocMtx[j + 1][i] || dt->LocMtx[j][i] != dt->LocMtx[j + 1][i + 1])
				aMtx[j][i] = 0;
			else aMtx[j][i] = 255;
		}
	}

	// 4. 写入Axis矩阵8bitfloat型
	FILE * aWrite = fopen(OutAxisName, "wb");
	fwrite(&SrcFileHead, sizeof(BITMAPFILEHEADER), 1, aWrite);
	fwrite(&SrcFileInfo, sizeof(BITMAPINFOHEADER), 1, aWrite);
	fwrite(ColorIdx, 1024, 1, aWrite);
	for (int y = 0; y < Height; y++)
	{
		fwrite(aMtx[y], sizeof(char), LineBytes, aWrite);
	}
	fclose(aWrite);


	// 5. 清理，释放内存
	if (aMtx != 0x00)
		for (int j = 0; j < Height; j++)
		{
			delete[]aMtx[j];
		}
	delete[]aMtx;
	if(dt != 0x00)
		delete dt;
	return 1;
};

/// <summary>
/// Delauney三角网
/// </summary>
int MorphologicalTransformer::Delaunay()
{
	// 1. 读取bmp
	unsigned i, j; // 循环变量
	FILE* SrcFileStream = fopen(SrcBmpName1, "rb");
	if (SrcFileStream == NULL) return -1; // 出错代码1，文件不存在

	BITMAPFILEHEADER SrcFileHead;
	BITMAPINFOHEADER SrcFileInfo;
	unsigned char ColorIdx[1024];

	if (fread(&SrcFileHead, sizeof(BITMAPFILEHEADER), 1, SrcFileStream) != 1)
	{
		fclose(SrcFileStream);
		return -1;
	}
	if (fread(&SrcFileInfo, sizeof(BITMAPINFOHEADER), 1, SrcFileStream) != 1)
	{
		fclose(SrcFileStream);
		return -1;
	}
	if (fread(ColorIdx, 1024, 1, SrcFileStream) != 1)
	{
		fclose(SrcFileStream);
		return -1;
	}

	if (SrcFileInfo.biBitCount != 8) // 不是8bit位图
	{
		fclose(SrcFileStream);
		return -2; // 出错代码2，文件格式不合要求
	}

	int Height = SrcFileInfo.biHeight;
	int Width = SrcFileInfo.biWidth;
	int LineBytes = (SrcFileInfo.biWidth + 3) / 4 * 4;

	// 2. 距离变换
	DistanceTransformer* dt = new DistanceTransformer(SrcFileStream, Height, Width, LineBytes);
	dt->EuDistanceD();

	// 3. 提取相邻颜色
	vector<vector<unsigned char> >colors;
	size_t k;
	for (j = 0; j < Height; j++)
	{
		for (i = 0; i < LineBytes; i++)
		{
			if (i == 0 || j == 0 || i == LineBytes - 1 || j == Height - 1) continue;
			if (dt->LocMtx[j][i] != dt->LocMtx[j - 1][i - 1] || dt->LocMtx[j][i] != dt->LocMtx[j - 1][i]
				|| dt->LocMtx[j][i] != dt->LocMtx[j - 1][i + 1] || dt->LocMtx[j][i] != dt->LocMtx[j][i - 1]
				|| dt->LocMtx[j][i] != dt->LocMtx[j][i + 1] || dt->LocMtx[j][i] != dt->LocMtx[j + 1][i - 1]
				|| dt->LocMtx[j][i] != dt->LocMtx[j + 1][i] || dt->LocMtx[j][i] != dt->LocMtx[j + 1][i + 1])
			{
				if (dt->LocMtx[j][i] != dt->LocMtx[j - 1][i - 1] && dt->LocMtx[j][i] != 0 && dt->LocMtx[j - 1][i - 1] != 0)
				{
					vector<unsigned char> cls;
					unsigned char color1 = dt->LocMtx[j][i];
					unsigned char color2 = dt->LocMtx[j - 1][i - 1];
					cls.push_back(color1);
					cls.push_back(color2);
					for (k = 0; k < colors.size(); k++)
					{
						if (colors[k] == cls)
							break;
					}
					if (k == colors.size() && cls[0] != 255 && cls[1] != 255)
						colors.push_back(cls);
					else
						cls.clear();
				}

				if (dt->LocMtx[j][i] != dt->LocMtx[j - 1][i] && dt->LocMtx[j][i] != 0 && dt->LocMtx[j - 1][i] != 0)
				{
					vector<unsigned char> cls;
					unsigned char color1 = dt->LocMtx[j][i];
					unsigned char color2 = dt->LocMtx[j - 1][i];
					cls.push_back(color1);
					cls.push_back(color2);
					for (k = 0; k < colors.size(); k++)
					{
						if (colors[k] == cls)
							break;
					}
					if (k == colors.size() && cls[0] != 255 && cls[1] != 255)
						colors.push_back(cls);
					else
						cls.clear();
				}
				if (dt->LocMtx[j][i] != dt->LocMtx[j - 1][i + 1] && dt->LocMtx[j][i] != 0 && dt->LocMtx[j - 1][i + 1] != 0)
				{
					vector<unsigned char> cls;
					unsigned char color1 = dt->LocMtx[j][i];
					unsigned char color2 = dt->LocMtx[j - 1][i + 1];
					cls.push_back(color1);
					cls.push_back(color2);
					for (k = 0; k < colors.size(); k++)
					{
						if (colors[k] == cls)
							break;
					}
					if (k == colors.size() && cls[0] != 255 && cls[1] != 255)
						colors.push_back(cls);
					else
						cls.clear();
				}
				if (dt->LocMtx[j][i] != dt->LocMtx[j][i - 1] && dt->LocMtx[j][i] != 0 && dt->LocMtx[j][i - 1] != 0)
				{
					vector<unsigned char> cls;
					unsigned char color1 = dt->LocMtx[j][i];
					unsigned char color2 = dt->LocMtx[j][i - 1];
					cls.push_back(color1);
					cls.push_back(color2);
					for (k = 0; k < colors.size(); k++)
					{
						if (colors[k] == cls)
							break;
					}
					if (k == colors.size() && cls[0] != 255 && cls[1] != 255)
						colors.push_back(cls);
					else
						cls.clear();
				}
				if (dt->LocMtx[j][i] != dt->LocMtx[j][i + 1] && dt->LocMtx[j][i] != 0 && dt->LocMtx[j][i + 1] != 0)
				{
					vector<unsigned char> cls;
					unsigned char color1 = dt->LocMtx[j][i];
					unsigned char color2 = dt->LocMtx[j][i + 1];
					cls.push_back(color1);
					cls.push_back(color2);
					for (k = 0; k < colors.size(); k++)
					{
						if (colors[k] == cls)
							break;
					}
					if (k == colors.size() && cls[0] != 255 && cls[1] != 255)
						colors.push_back(cls);
					else
						cls.clear();
				}
				if (dt->LocMtx[j][i] != dt->LocMtx[j + 1][i - 1] && dt->LocMtx[j][i] != 0 && dt->LocMtx[j + 1][i - 1] != 0)
				{
					vector<unsigned char> cls;
					unsigned char color1 = dt->LocMtx[j][i];
					unsigned char color2 = dt->LocMtx[j + 1][i - 1];
					cls.push_back(color1);
					cls.push_back(color2);
					for (k = 0; k < colors.size(); k++)
					{
						if (colors[k] == cls)
							break;
					}
					if (k == colors.size() && cls[0] != 255 && cls[1] != 255)
						colors.push_back(cls);
					else
						cls.clear();
				}
				if (dt->LocMtx[j][i] != dt->LocMtx[j + 1][i] && dt->LocMtx[j][i] != 0 && dt->LocMtx[j + 1][i] != 0)
				{
					vector<unsigned char> cls;
					unsigned char color1 = dt->LocMtx[j][i];
					unsigned char color2 = dt->LocMtx[j + 1][i];
					cls.push_back(color1);
					cls.push_back(color2);
					for (k = 0; k < colors.size(); k++)
					{
						if (colors[k] == cls)
							break;
					}
					if (k == colors.size() && cls[0] != 255 && cls[1] != 255)
						colors.push_back(cls);
					else
						cls.clear();
				}
				if (dt->LocMtx[j][i] != dt->LocMtx[j + 1][i + 1] && dt->LocMtx[j][i] != 0 && dt->LocMtx[j + 1][i + 1] != 0)
				{
					vector<unsigned char> cls;
					unsigned char color1 = dt->LocMtx[j][i];
					unsigned char color2 = dt->LocMtx[j + 1][i + 1];
					cls.push_back(color1);
					cls.push_back(color2);
					for (k = 0; k < colors.size(); k++)
					{
						if (colors[k] == cls)
							break;
					}
					if (k == colors.size() && cls[0] != 255 && cls[1] != 255)
						colors.push_back(cls);
					else
						cls.clear();
				}
			}			
		}		
	}

	//4. 点对匹配
	vector<vector<Point*> >pts;
	for (vector<vector<unsigned char> >::iterator iter = colors.begin(); iter != colors.end(); iter++)
	{
		vector<Point*> p2;
		vector<unsigned char> vec = *iter;
		unsigned char color1 = vec[0];
		unsigned char color2 = vec[1]; 
		for (vector<Point* >::iterator it = dt->pts.begin(); it != dt->pts.end(); it++)
		{
			if ((*it)->color == color1)
				p2.push_back(*it);
			else if ((*it)->color == color2)
				p2.push_back(*it);
		}
		pts.push_back(p2);
	}
	pts.size();

	// 5. 点对连线
	dMtx = new unsigned char *[Height];
	for (j = 0; j < Height; j++)
	{
		dMtx[j] = new unsigned char[LineBytes];
		for (i = 0; i < LineBytes; i++)
			dMtx[j][i] = 255;
	}
	for(vector<vector<Point*> >::iterator iter = pts.begin(); iter != pts.end(); iter++)
	{
		vector<Point*> vec = *iter;
		int x0 = vec[0]->x;
		int y0 = vec[0]->y;
		int xend = vec[1]->x;
		int yend = vec[1]->y;
		DDAline(x0, y0, xend, yend);
	}
	

	// 6. 写入Delaunay矩阵8bitfloat型
	FILE * dWrite = fopen(OutDelName, "wb");
	fwrite(&SrcFileHead, sizeof(BITMAPFILEHEADER), 1, dWrite);
	fwrite(&SrcFileInfo, sizeof(BITMAPINFOHEADER), 1, dWrite);
	fwrite(ColorIdx, 1024, 1, dWrite);
	for (int y = 0; y < Height; y++)
	{
		//fwrite(dt->LocMtx[y], sizeof(char), LineBytes, dWrite);
		fwrite(dMtx[y], sizeof(char), LineBytes, dWrite);
	}
	fclose(dWrite);


	// 7. 清理，释放内存
	if (dMtx != 0x00)
		for (int j = 0; j < Height; j++)
		{
			delete[]dMtx[j];
		}
	delete[]dMtx;
	if (dt != 0x00)
		delete dt;
	return 1;
};

void MorphologicalTransformer::DDAline(int x0, int y0, int xEnd, int yEnd)
{
	int dx = xEnd - x0, dy = yEnd - y0, steps, k;
	float xIncrement, yIncrement, x = x0, y = y0;
	if (fabs(dx)>fabs(dy))
	{
		steps = fabs(dx);
	}
	else
	{
		steps = fabs(dy);
	}
	xIncrement = float(dx) / float(steps);
	yIncrement = float(dy) / float(steps);
	dMtx[int(x)][int(y)] = 0;
	//setPixel(round(x), round(y));
	for (k = 0; k<steps; k++)
	{
		x += xIncrement;
		y += yIncrement;
		dMtx[int(x)][int(y)] = 0;
	}
}