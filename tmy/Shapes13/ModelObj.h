

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
	string map_Kd = "";     //���ģ�͵������䷴������ͼ�����·��
};

#pragma once
class ModelObj
{
public:
	ModelObj(string modelpath,int fToTri);
	~ModelObj();
	int fNum = 0;    //��ʾ�����ε�����
	vector<int32_t> indices;//������������
	vector<Vertex> vertices;  //���������ζ�Ӧ�Ķ������飨3������Ϊһ�����飩
	std::unordered_map<std::string, Newmtl*> newmtls;    //��ȡ�����еĲ�������
	vector<int> fStartIndexs;  //ÿ�����ʶ�Ӧ�������ε���ʼ���±�
	vector<string> newmtlNames;   //������

	void readfile();
	///һ��f�����������Σ������±��ǵڼ����������±��ǵڼ�����uv�����ǵڼ���
	void setType(int fToTri, int vIndex, int vnIndex, int vtIndex);
	XMVECTOR vMax;
	XMVECTOR vMin;

private:
	void getModelLineNum();   //�õ�ģ�͵ĸ��������ж��ٸ�
	void getMaterialLineNum();   //�õ����ʵ�����
	void initTriangles();
	void clearTriangles();
	///һ��f������������
	int fToTri = 3;

	int vNum = 0;   //��ʾ��������
	int vnNum = 0;   //��ʾ����ķ�������
	int vtNum = 0;   //��ʾ�����uv��������	

	float** vArr;  //��������
	float** vnArr;   //���㷨������
	float** vtArr;  //uv��������
	
	int** fvArr;
	int** fnArr;
	int** ftArr;

	string path;
	string objPath;
	string mtlPath;
};

