#include "ModelObj.h"



ModelObj::ModelObj(string modelPath,int modelFToTri)
{
	path = modelPath;
	objPath = path + ".obj";
	mtlPath = path + ".mtl";
	if (modelFToTri < 3)
	{
		modelFToTri = 3;
	}
	fToTri = modelFToTri;
}


ModelObj::~ModelObj()
{
}

void ModelObj::getModelLineNum()
{
//	ifstream fin("Models/skull.txt");
	ifstream infile(objPath.c_str()); //打开指定文件
	string sline;//每一行
	int ddd = 0;
	while (getline(infile, sline)) {//从指定文件逐行读取
		ddd++;
		if (sline[0] == 'v') {
			if (sline[1] == 'n')
				vnNum++;
			else if (sline[1] == 't')
				vtNum++;
			else
				vNum++;
		}
		if (sline[0] == 'f')
			fNum += (fToTri - 2);
	}

	infile.close();
}

void ModelObj::getMaterialLineNum()
{
	ifstream ifile(mtlPath.c_str());
	string sline;//每一行
	string value, newmtlName, map_Kd;   //当前的数据，材质名,贴图路径
	float d, Ns, Ni, Km;
	float KaX, KaY, KaZ;
	float KdX, KdY, KdZ;
	float KsX, KsY, KsZ;
	Newmtl* material = nullptr;
	
	while (getline(ifile, sline)) {
		istringstream ins(sline);
		ins >> value;
		if (value == "newmtl") {
			ins >> newmtlName;
			material = new Newmtl();
			newmtls[newmtlName] = std::move(material);
		}
		else if (material != nullptr && value == "d") {  
			ins >> d;   
			material->d = d;
		}
		else if (material != nullptr && value == "Ns") {    //透明度
			ins >> Ns;
			material->Ns = Ns;
		}
		else if (material != nullptr && value == "Ka") {    //当没有光线照射时的表面颜色。通常它是和表面的漫反射颜色是一样的
			ins >> KaX >> KaY >> KaZ;
			material->Ka = XMFLOAT3(KaX, KaY, KaZ);
		}
		else if (material != nullptr && value == "Kd") {    //漫反射反照率
			ins >> KdX >> KdY >> KdZ;
			material->Kd = XMFLOAT3(KdX, KdY, KdZ);
		}
		else if (material != nullptr && value == "Ks") {    //漫反射反照率
			ins >> KsX >> KsY >> KsZ;
			material->Ks = XMFLOAT3(KsX, KsY, KsZ);
		}
		else if (material != nullptr && value == "map_Kd") {    //漫反射反照率
			ins >> map_Kd;
			material->map_Kd = map_Kd;
		}
	}
	ifile.close();
}

void ModelObj::readfile()
{
	getModelLineNum();
	getMaterialLineNum();
	vertices = vector<Vertex>(fNum * 3);
	indices = vector<int32_t>(fNum * 3);
	
	//new二维数组
	vArr = new float*[vNum];
	for (int i = 0; i < vNum; i++)
		vArr[i] = new float[3];

	vnArr = new float*[vnNum];
	for (int i = 0; i < vnNum; i++)
		vnArr[i] = new float[3];

	vtArr = new float*[vtNum];
	for (int i = 0; i < vtNum; i++)
		vtArr[i] = new float[3];

	fvArr = new int*[fNum];
	ftArr = new int*[fNum];
	fnArr = new int*[fNum];
	for (int i = 0; i < fNum; i++) {
		fvArr[i] = new int[3];
		ftArr[i] = new int[3];
		fnArr[i] = new int[3];
	}
	ifstream infile(objPath.c_str());
	string sline;//每一行
	int ii = 0, tt = 0, jj = 0, kk = 0;

	std::string s1;
	float f2, f3, f4;
	//int fIndex = 0;   //表示当前是第几个三角形
	while (getline(infile, sline)) {
		if (sline[0] == 'v') {
			if (sline[1] == 'n') {//vn
				istringstream ins(sline);
				ins >> s1 >> f2 >> f3 >> f4;
				vnArr[ii][0] = f2;
				vnArr[ii][1] = f3;
				vnArr[ii][2] = f4;
				ii++;
			}
			else if (sline[1] == 't') {//vt
				istringstream ins(sline);
				ins >> s1 >> f2 >> f3 >> f4;
				vtArr[tt][0] = f2;
				vtArr[tt][1] = 1 - f3;
				vtArr[tt][2] = f4;
				tt++;
			}
			else {//v
				istringstream ins(sline);
				ins >> s1 >> f2 >> f3 >> f4;
				vArr[jj][0] = f2;
				vArr[jj][1] = f3;
				vArr[jj][2] = f4;
				jj++;
			}
		}
		if (sline[0] == 'f') { //存储面
		//	fIndex += (fToTri - 2);
			istringstream in(sline);
			float a;
			in >> s1;//去掉f
			int i, k, length;
			
			vector<int> v = vector<int>(fToTri);
			vector<int> vt = vector<int>(fToTri);
			vector<int> vn = vector<int>(fToTri);
			for (i = 0; i < fToTri; i++) {
				in >> s1;
				length = s1.size();
				//取出第一个顶点和法线索引
				a = 0;
				for (k = 0; s1[k] != '/' && k < length; k++)
					a = a * 10 + (s1[k] - 48);
				v[i] = a;
			//	fvArr[kk][i] = a;

				a = 0;
				for (k = k + 1; s1[k] != '/' && k < length; k++)
					a = a * 10 + (s1[k] - 48);
				vt[i] = a;
			//	ftArr[kk][i] = a;

				a = 0;
				for (k = k + 1; k < length; k++)
					a = a * 10 + (s1[k] - 48);
				vn[i] = a;
			//	fnArr[kk][i] = a;
			}
			for (i = 0; i < fToTri - 2; i++)
			{
				fvArr[kk][0] = v[0];
				fvArr[kk][1] = v[i + 1];
				fvArr[kk][2] = v[i + 2];
				ftArr[kk][0] = vt[0];
				ftArr[kk][1] = vt[i + 1];
				ftArr[kk][2] = vt[i + 2];
				fnArr[kk][0] = vn[0];
				fnArr[kk][1] = vn[i + 1];
				fnArr[kk][2] = vn[i + 2];
				kk++;
			}
			
		}
		if (sline[0] == 'u')
		{
			istringstream in(sline);
			in >> s1;
			if (s1 == "usemtl")
			{
				in >> s1;
				newmtlNames.push_back(s1);
				fStartIndexs.push_back(kk);
			}
		}
	}
	infile.close();
	initTriangles();
}

void ModelObj::initTriangles()
{
	for (int i = 0; i < fNum; i++) {
		int v1Index = i * 3;
		int v2Index = i * 3 + 1;
		int v3Index = i * 3 + 2;

		XMVECTOR P;
		
		vertices[v1Index].Pos.x = vArr[fvArr[i][0] - 1][0];
		vertices[v1Index].Pos.y = vArr[fvArr[i][0] - 1][1];
		vertices[v1Index].Pos.z = vArr[fvArr[i][0] - 1][2];
		vertices[v1Index].Normal.x = vnArr[fnArr[i][0] - 1][0];
		vertices[v1Index].Normal.y = vnArr[fnArr[i][0] - 1][1];
		vertices[v1Index].Normal.z = vnArr[fnArr[i][0] - 1][2];
		vertices[v1Index].UV.x = vtArr[ftArr[i][0] - 1][0];
		vertices[v1Index].UV.y = vtArr[ftArr[i][0] - 1][1];

		P = XMLoadFloat3(&vertices[v1Index].Pos);
		vMin = XMVectorMin(P, vMin);
		vMax = XMVectorMax(P, vMax);

		vertices[v2Index].Pos.x = vArr[fvArr[i][1] - 1][0];
		vertices[v2Index].Pos.y = vArr[fvArr[i][1] - 1][1];
		vertices[v2Index].Pos.z = vArr[fvArr[i][1] - 1][2];
		vertices[v2Index].Normal.x = vnArr[fnArr[i][1] - 1][0];
		vertices[v2Index].Normal.y = vnArr[fnArr[i][1] - 1][1];
		vertices[v2Index].Normal.z = vnArr[fnArr[i][1] - 1][2];
		vertices[v2Index].UV.x = vtArr[ftArr[i][1] - 1][0];
		vertices[v2Index].UV.y = vtArr[ftArr[i][1] - 1][1];

		P = XMLoadFloat3(&vertices[v2Index].Pos);
		vMin = XMVectorMin(P, vMin);
		vMax = XMVectorMax(P, vMax);

		vertices[v3Index].Pos.x = vArr[fvArr[i][2] - 1][0];
		vertices[v3Index].Pos.y = vArr[fvArr[i][2] - 1][1];
		vertices[v3Index].Pos.z = vArr[fvArr[i][2] - 1][2];
		vertices[v3Index].Normal.x = vnArr[fnArr[i][2] - 1][0];
		vertices[v3Index].Normal.y = vnArr[fnArr[i][2] - 1][1];
		vertices[v3Index].Normal.z = vnArr[fnArr[i][2] - 1][2];
		vertices[v3Index].UV.x = vtArr[ftArr[i][2] - 1][0];
		vertices[v3Index].UV.y = vtArr[ftArr[i][2] - 1][1];

		P = XMLoadFloat3(&vertices[v3Index].Pos);
		vMin = XMVectorMin(P, vMin);
		vMax = XMVectorMax(P, vMax);

		indices[i * 3] = v1Index;
		indices[i * 3 + 1] = v2Index;
		indices[i * 3 + 2] = v3Index;
	}

	clearTriangles();
}

void ModelObj::clearTriangles()
{
	for (int i = 0; i < vNum; i++)
		delete[] * (vArr + i);
	for (int i = 0; i < vnNum; i++)
		delete[] * (vnArr + i);
	for (int i = 0; i < vtNum; i++)
		delete[] * (vtArr + i);
	for (int i = 0; i < fNum; i++) {
		delete[] * (fvArr + i);
		delete[] * (ftArr + i);
		delete[] * (fnArr + i);
	}

	delete[] vArr;
	delete[] vnArr;
	delete[] vtArr;
	delete[] fvArr;
	delete[] ftArr;
	delete[] fnArr;
}
