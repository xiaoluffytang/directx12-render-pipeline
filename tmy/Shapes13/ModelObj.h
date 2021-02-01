

#include "../../Common/d3dApp.h"
#include "../../Common/MathHelper.h"
#include "../../Common/UploadBuffer.h"
#include "../../Common/GeometryGenerator.h"
#include "FrameResource.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace std;

struct Newmtl
{
	float d;
	float Ns;
	float Ni;
	XMFLOAT3 Ka;
	XMFLOAT3 Kd;
	XMFLOAT3 Ks;
	float Km;
	string map_Kd = "";     //这个模型的漫反射反照率贴图的相对路径
};

#pragma once
class ModelObj
{
public:
	ModelObj(string modelpath,int fToTri);
	~ModelObj();
	int fNum = 0;    //表示三角形的数量
	vector<int32_t> indices;//顶点索引数组
	vector<Vertex> vertices;  //所有三角形对应的顶点数组（3个顶点为一个数组）
	std::unordered_map<std::string, Newmtl*> newmtls;    //读取的所有的材质数据
	vector<int> fStartIndexs;  //每个材质对应的三角形的起始的下标
	vector<string> newmtlNames;   //材质名

	void readfile();
	///一个f代表几个三角形，顶点下标是第几个，法线下标是第几个，uv坐标是第几个
	void setType(int fToTri, int vIndex, int vnIndex, int vtIndex);
	XMVECTOR vMax;
	XMVECTOR vMin;

private:
	void getModelLineNum();   //得到模型的各种数据有多少个
	void getMaterialLineNum();   //得到材质的数据
	void initTriangles();
	void clearTriangles();
	///一个f代表几个三角形
	int fToTri = 3;

	int vNum = 0;   //表示顶点数量
	int vnNum = 0;   //表示顶点的法线数量
	int vtNum = 0;   //表示顶点的uv坐标数量	

	float** vArr;  //顶点数组
	float** vnArr;   //顶点法线数组
	float** vtArr;  //uv坐标数组
	
	int** fvArr;
	int** fnArr;
	int** ftArr;

	string path;
	string objPath;
	string mtlPath;
};

