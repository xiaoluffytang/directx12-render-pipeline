#include "SceneManager.h"
#include "Manager.h"


SceneManager::SceneManager(Manager* manager)
{
	this->manager = manager;
}


SceneManager::~SceneManager()
{
}

void SceneManager::Init()
{
	BuildDefaultTextures();  //����һЩĬ�ϵ�ͼƬ
	BuildDefaultMaterials();   //����һЩĬ�ϵĲ���
	BuildAllGeometry();    //����һЩ�����е���Ϸ����
	BuildAllRenderGameObject();    //����һЩ�����е���Ϸ����
}

//����һЩĬ�ϵ�ͼƬ
void SceneManager::BuildDefaultTextures()
{
	std::vector<string> paths = { "grasscube1024","grass" ,"ice","water1" ,"tree01S" ,"NoiseTex" ,"stone" };
	for (int i = 0; i < paths.size(); i++)
	{
		manager->textureManager->LoadTexture("../../Textures/"+ paths[i] + ".dds");
	}
}

//����һЩĬ�ϵĲ���
void SceneManager::BuildDefaultMaterials()
{
	int setAside = manager->textureManager->setAside;   //ǰ����������������ĳ������� 
	std::vector<string> materialNames = { "screen","skyBox" ,"opaque_back","specular" ,"uiSprite" ,"uiShadowMap" ,"uiScreenPost","uiDepth","font","dynFont",
		"opaque_None","animator","cubeTargetReflect" ,"water","font3d","aniTex","skull","particlePosOffsetTex","moutain","uiText" };
	std::vector<int> srvIndex = { 1, setAside, setAside + 1, setAside + 2, setAside + 4 , 0, 1, 2, 5, 6,
		setAside + 1, setAside + 1, 6, setAside + 5, setAside + 6 ,18,19,19, setAside + 6,5};
	std::vector<string> psoNames = { "screenDefault","skyBox" ,"opaque_back","Transparent" ,"uiSprite" ,"uiSprite" ,"uiSprite","uiSprite","uiSprite","uiSprite",
		"opaque_None","animator","cubeTargetReflect" ,"lake","opaque_None","uiSprite" ,"skull","uiSprite","opaque_back","uiText"};
	for (int i = 0; i < materialNames.size(); i++)
	{
		auto material = manager->materialManager->CreateMaterial(materialNames[i]);
		material->MatCBIndex = manager->materialManager->mMaterials.size() - 1;
		material->DiffuseSrvHeapIndex = srvIndex[i];
		material->pso = manager->psoManager->mPSOs[psoNames[i]];
	}
}

void SceneManager::useScriptLand(GeometryGenerator::MeshData& mesh)
{
	int vertexCount = mesh.Vertices.size();
	int indexSize = mesh.GetIndices16().size();
	Vertex* vertices = new Vertex[vertexCount];
	uint16_t* indices = new uint16_t[indexSize];
	for (size_t i = 0; i < vertexCount; i++)
	{
		vertices[i].Pos = mesh.Vertices[i].Position;
		vertices[i].Normal = mesh.Vertices[i].Normal;
		float u = mesh.Vertices[i].TexC.x ;
		float v = mesh.Vertices[i].TexC.y ;
		vertices[i].UV = XMFLOAT2(u, v);
	}
	for (size_t i = 0; i < indexSize; i++)
	{
		indices[i] = mesh.GetIndices16()[i];
	}
	const UINT vbByteSize = (UINT)vertexCount * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indexSize * sizeof(std::uint16_t);

	auto geo = new MeshGeometry();
	geo->Name = "moutain";

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(manager->commonInterface.md3dDevice.Get(),
		manager->commonInterface.mCommandList.Get(), vertices, vbByteSize, geo->VertexBufferUploader, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(manager->commonInterface.md3dDevice.Get(),
		manager->commonInterface.mCommandList.Get(), indices, ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = indexSize;
	submesh.BaseVertexLocation = 0;
	submesh.modelBindMaterialName = "moutain";
	submesh.StartIndexLocation = 0;
	geo->DrawArgs[geo->Name] = std::move(submesh);

	mGeometries[geo->Name] = std::move(geo);
	//delete[] vertices;
	//delete[] indices;

	auto command = manager->commonInterface.mCommandList.Get();
	command->SetComputeRootSignature(manager->psoManager->mScriptLandSignature.Get());
	command->SetComputeRootUnorderedAccessView(0, geo->VertexBufferGPU->GetGPUVirtualAddress());
	command->SetPipelineState(manager->psoManager->mPSOs["Moutain"]);
	command->Dispatch(8, 8, 1);
}

void SceneManager::BuildOneGeometry(GeometryGenerator::MeshData& mesh, std::string geoName, std::string materialName, float scale, XMFLOAT3 color = {1,1,1})
{
	int vertexCount = mesh.Vertices.size();
	int indexSize = mesh.GetIndices16().size();
	Vertex* vertices = new Vertex[vertexCount];
	uint16_t* indices = new uint16_t[indexSize];
	for (size_t i = 0; i < vertexCount; i++)
	{
		vertices[i].Pos = mesh.Vertices[i].Position;
		vertices[i].Normal = mesh.Vertices[i].Normal;
		float u = mesh.Vertices[i].TexC.x * scale;
		float v = mesh.Vertices[i].TexC.y * scale;
		vertices[i].UV = XMFLOAT2(u, v);
		vertices[i].Color = color;
	}
	for (size_t i = 0; i < indexSize; i++)
	{
		indices[i] = mesh.GetIndices16()[i];
	}
	const UINT vbByteSize = (UINT)vertexCount * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indexSize * sizeof(std::uint16_t);

	auto geo = new MeshGeometry();
	geo->Name = geoName;

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(manager->commonInterface.md3dDevice.Get(),
		manager->commonInterface.mCommandList.Get(), vertices, vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(manager->commonInterface.md3dDevice.Get(),
		manager->commonInterface.mCommandList.Get(), indices, ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = indexSize;
	submesh.BaseVertexLocation = 0;
	submesh.modelBindMaterialName = materialName;
	submesh.StartIndexLocation = 0;
	geo->DrawArgs[geoName] = std::move(submesh);

	mGeometries[geo->Name] = std::move(geo);
	delete[] vertices;
	delete[] indices;
}

void SceneManager::BuildAllGeometry()
{
	//�������е�ģ�ͼ����壬������ģ�͵Ķ��㣬���ߣ�uv���꣬���㻺�����������������ȣ���Щ�� MeshGeometry �����������
	//ע�⣬���ﲢ����������Ҫ��Ⱦ����Ϸ���壨��Ⱦ���ֻ�Ǵ���һЩģ�͵�һЩ����
	 
	//���ﴴ���Լ��ô���д��ģ��
	GeometryGenerator::MeshData screenPlane = geoGen.CreatePlane(2, 2);
	BuildOneGeometry(screenPlane, "screen", "screen", 1);
	GeometryGenerator::MeshData plane = geoGen.CreatePlane(10, 10);
	BuildOneGeometry(plane, "plane", "opaque_back", 10);
	BuildOneGeometry(plane, "specular","specular", 1);
	GeometryGenerator::MeshData box = geoGen.CreateSphere(0.5f,200,200);
	BuildOneGeometry(box, "skyBox", "skyBox", 1);
	GeometryGenerator::MeshData sprite = geoGen.CreateUISprite(1, 1);
	BuildOneGeometry(sprite, "ui", "uiSprite", 1);
	BuildOneGeometry(sprite, "text", "uiText", 1,XMFLOAT3(0,0,0));
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 200, 200);
	BuildOneGeometry(sphere, "sphere", "cubeTargetReflect", 1);
	GeometryGenerator::MeshData water = geoGen.CreateGrid(200,200,201,201);
	BuildOneGeometry(water, "water", "water", 1);
	GeometryGenerator::MeshData moutain = geoGen.CreateGrid(63, 63, 64, 64);
	useScriptLand(moutain);


	//���ﴴ��obj��ʽ��ģ��
	BuildModel("Models/door/door", "Models/door/", 4, "opaque_back");
	BuildModel("Models/GaiLun/GaiLun", "Models/GaiLun/", 3, "opaque_back");
	BuildModel("Models/WindGreen/wildGreen_medium_V01", "Models/WindGreen/", 4, "opaque_None");
	//���ﴴ��3d����ģ��
	BuildText3D("һ�亮â�ȵ������ǹ������");
	//���ﴴ������ģ��
	BuildM3dModel("Models/soldier.m3d");
	//����txt�ļ���ģ��
	BuildParticleFromTxtFile("Models/skull.txt","skull");
}

void SceneManager::BuildParticleFromTxtFile(const string& filepath,const string& gemName)
{
	std::ifstream fin(filepath);
	if (!fin)
	{
		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
		return;
	}
	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;
	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	manager->textureManager->particle->pos = new XMFLOAT3[vcount];
	Vertex* vertices = new Vertex[vcount];
	
	manager->textureManager->particle->posCount = vcount;
	for (UINT i = 0; i < vcount; ++i)
	{	
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Color.x >> vertices[i].Color.y >> vertices[i].Color.z;
		XMFLOAT3& v = vertices[i].Pos;
		manager->textureManager->particle->pos[i] = XMFLOAT3(v.x, v.y, v.z);
	}
	manager->textureManager->particle->InitPos(manager->commonInterface.md3dDevice.Get(), manager->commonInterface.mCommandList.Get());
	fin >> ignore;
	fin >> ignore;
	fin >> ignore;
	int indicesCount = 3 * tcount;
	int32_t* indices = new int32_t[indicesCount];
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}
	fin.close();

	const UINT vbByteSize = (UINT)vcount * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indicesCount * sizeof(std::int32_t);

	auto geo = new MeshGeometry();
	geo->Name = gemName;

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(manager->commonInterface.md3dDevice.Get(),
		manager->commonInterface.mCommandList.Get(), vertices, vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(manager->commonInterface.md3dDevice.Get(),
		manager->commonInterface.mCommandList.Get(), indices, ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indicesCount;
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;
	submesh.modelBindMaterialName = gemName;

	geo->DrawArgs[gemName] = submesh;
	mGeometries[geo->Name] = std::move(geo);
	delete[] vertices;
	delete[] indices;
}

void SceneManager::BuildM3dModel(const string& mSkinnedModelFilename)
{
	std::vector<SkinnedVertex> vertices;
	std::vector<std::uint16_t> indices;
	std::vector<M3DLoader::Subset> mSkinnedSubsets;
	std::vector<M3DLoader::M3dMaterial> mSkinnedMats;
	std::vector<std::string> mSkinnedTextureNames;
	SkinnedData* mSkinnedInfo = manager->textureManager->skin;

	M3DLoader m3dLoader;
	m3dLoader.LoadM3d(mSkinnedModelFilename, vertices, indices,
		mSkinnedSubsets, mSkinnedMats,* mSkinnedInfo,manager->commonInterface.md3dDevice.Get());
	mSkinnedModelInst = new SkinnedModelInstance();
	mSkinnedModelInst->SkinnedInfo = mSkinnedInfo;
	mSkinnedModelInst->FinalTransforms.resize(mSkinnedInfo->BoneCount());
	mSkinnedModelInst->ClipName = "Take1";
	mSkinnedModelInst->TimePos = 0.0f;


	const UINT vbByteSize = (UINT)vertices.size() * sizeof(SkinnedVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = new MeshGeometry();
	geo->Name = mSkinnedModelFilename;

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(manager->commonInterface.md3dDevice.Get(),
		manager->commonInterface.mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(manager->commonInterface.md3dDevice.Get(),
		manager->commonInterface.mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(SkinnedVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	//�������ģ�͵����еĲ���
	for (UINT i = 0; i < mSkinnedMats.size(); ++i)
	{
		string path1 = "../../Textures/" + mSkinnedMats[i].DiffuseMapName;
		string path2 = "../../Textures/" + mSkinnedMats[i].NormalMapName;
		auto material = manager->materialManager->CreateMaterial(mSkinnedMats[i].Name);
		material->MatCBIndex = manager->materialManager->mMaterials.size() - 1;
		material->pso = manager->psoManager->mPSOs["animator"];
		material->DiffuseSrvHeapIndex = manager->textureManager->LoadTexture(path1);
		material->NormalSrvHeapIndex = manager->textureManager->LoadTexture(path2);
	}

	int submeshCount = mSkinnedSubsets.size();
	for (int i = 0; i < submeshCount; ++i)
	{
		SubmeshGeometry submesh;
		std::string name = "sm_" + std::to_string(i);

		submesh.IndexCount = (UINT)mSkinnedSubsets[i].FaceCount * 3;
		submesh.StartIndexLocation =  mSkinnedSubsets[i].FaceStart * 3;
		submesh.BaseVertexLocation = 0;
		submesh.modelBindMaterialName = mSkinnedMats[i].Name;
		geo->DrawArgs[name] = submesh;
	}
	mGeometries[geo->Name] = std::move(geo);

	int instanceCount = 100;
	std::vector<float> timeOffsets;
	std::vector<float> speeds;
	timeOffsets.resize(instanceCount);
	speeds.resize(instanceCount);
	for (int i = 0; i < instanceCount; i++)
	{
		timeOffsets[i] = MathHelper::RandF() * 5;
		speeds[i] = 0.5 + MathHelper::RandF() * 4;
	}
	for (auto& f : geo->DrawArgs)
	{
		AnimatorRole* go = new AnimatorRole(geo, itemCount++, f.first, manager->materialManager->GetMaterial(f.second.modelBindMaterialName, true),
			timeOffsets,speeds);
		manager->gameobjectManager->gameobjectList.push_back(go);
		manager->renderManager->OpaqueAnimatorList1.push_back(go);
		go->mSkinnedModelInst = mSkinnedModelInst;
	}
	timeOffsets.clear();
	timeOffsets.shrink_to_fit();
	speeds.clear();
	speeds.shrink_to_fit();
	vertices.clear();
	vertices.shrink_to_fit();
	indices.clear();
	indices.shrink_to_fit();
}

void SceneManager::BuildText3D(const string& str)
{
	Font3D* font3d = new Font3D("../../font/simhei.ttf");
	auto tris = font3d->getVectoriser(str);
	
	int size = tris.size() * 3;
	std::vector<Vertex> vertices(size);
	std::vector<std::uint16_t> indices(size);
	for (size_t i = 0; i < tris.size(); i++)
	{
		int index0 = i * 3 + 0;
		int index1 = i * 3 + 1;
		int index2 = i * 3 + 2;
		vertices[index0].Pos = XMFLOAT3(tris[i].a.x, tris[i].a.y, tris[i].a.z);
		vertices[index1].Pos = XMFLOAT3(tris[i].b.x, tris[i].b.y, tris[i].b.z);
		vertices[index2].Pos = XMFLOAT3(tris[i].c.x, tris[i].c.y, tris[i].c.z);
		vertices[index0].UV = XMFLOAT2(tris[i].a.x / 5, tris[i].a.y / 5);
		vertices[index1].UV = XMFLOAT2(tris[i].b.x / 5, tris[i].b.y / 5);
		vertices[index2].UV = XMFLOAT2(tris[i].c.x / 5, tris[i].c.y / 5);

		
		XMVECTOR v1 = XMLoadFloat3(&XMFLOAT3(tris[i].a.x - tris[i].b.x, tris[i].a.y - tris[i].b.y, tris[i].a.z - tris[i].b.z));
		XMVECTOR v2 = XMLoadFloat3(&XMFLOAT3(tris[i].a.x - tris[i].c.x, tris[i].a.y - tris[i].c.y, tris[i].a.z - tris[i].c.z));
		XMVECTOR cross = XMVector3Normalize(XMVector3Cross(v1, v2));
		XMFLOAT3 normal;
		XMStoreFloat3(&normal, cross);
		
		vertices[index0].Normal.x += normal.x;
		vertices[index0].Normal.y += normal.y;
		vertices[index0].Normal.z += normal.z;
		vertices[index1].Normal.x += normal.x;
		vertices[index1].Normal.y += normal.y;
		vertices[index1].Normal.z += normal.z;
		vertices[index2].Normal.x += normal.x;
		vertices[index2].Normal.y += normal.y;
		vertices[index2].Normal.z += normal.z;
	}
	
	for (size_t i = 0; i < size; i++)
	{
		indices[i] = i;
		XMVECTOR v = XMVector3Normalize(XMLoadFloat3(&vertices[i].Normal));
		XMStoreFloat3(&vertices[i].Normal, v);
	}
	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = new MeshGeometry();
	string name = "3d�ı�";
	geo->Name = name;

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(manager->commonInterface.md3dDevice.Get(),
		manager->commonInterface.mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(manager->commonInterface.md3dDevice.Get(),
		manager->commonInterface.mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = size;
	submesh.BaseVertexLocation = 0;
	submesh.modelBindMaterialName = "font3d";
	submesh.StartIndexLocation = 0;
	geo->DrawArgs[name] = std::move(submesh);

	mGeometries[geo->Name] = std::move(geo);

	vertices.clear();
	vertices.shrink_to_fit();
	indices.clear();
	indices.shrink_to_fit();
	delete font3d;
}

//������ȡ��ojb��ʽ��ģ�ͼ���������
void SceneManager::BuildModel(string path, string textureOppPath, int fToTri,string pso)
{
	ModelObj* model = new ModelObj(path, fToTri);
	model->readfile();
	
	BoundingBox bounds;
	XMStoreFloat3(&bounds.Center, 0.5f*(model->vMin + model->vMax));
	XMStoreFloat3(&bounds.Extents, 0.5f*(model->vMax - model->vMin));

	const UINT vbByteSize = (UINT)model->vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)model->indices.size() * sizeof(std::int32_t);
	auto geo = new MeshGeometry();
	geo->Name = path;

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(manager->commonInterface.md3dDevice.Get(),
		manager->commonInterface.mCommandList.Get(), model->vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(manager->commonInterface.md3dDevice.Get(),
		manager->commonInterface.mCommandList.Get(), model->indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	int submeshCount = model->fStartIndexs.size();
	//������ͼ�����е���Ϸ��������
	for (int i = 0; i < submeshCount; i++)
	{
		SubmeshGeometry submesh;
		int startF = model->fStartIndexs[i] * 3;   //�ӵڼ����濪ʼ
		int endF = i == submeshCount - 1 ? model->fNum * 3 - 1 : model->fStartIndexs[i + 1] * 3 - 1;    //�ӵڼ��������
		string subName = path + model->newmtlNames[i];
		submesh.IndexCount = endF - startF;    //����������һ��
		submesh.StartIndexLocation = startF;    //ÿ��mesh�Ķ�����ʼ�㲻һ������Ҫ����ƫ��
		submesh.BaseVertexLocation = 0;    //�õ���ͬһ�����㻺�������ݣ�����Ҫ���ƫ��
		submesh.Bounds = bounds;
		submesh.modelBindMaterialName = subName;
		geo->DrawArgs[to_string(i)] = submesh;
	}
	mGeometries[geo->Name] = std::move(geo);

	//�������ģ�͵����еĲ���
	for (auto& newmtl : model->newmtls)
	{
		string name = path + newmtl.first;
		Newmtl* mtl = newmtl.second;
		auto material = manager->materialManager->CreateMaterial(name);
		material->MatCBIndex = manager->materialManager->mMaterials.size() - 1;
		string texturePath = mtl->map_Kd;
		material->DiffuseSrvHeapIndex = -1;
		material->pso = manager->psoManager->mPSOs[pso];
		if (texturePath != "")
		{
			int index = manager->textureManager->LoadTexture(textureOppPath + texturePath);
			material->DiffuseSrvHeapIndex = index;
		}
	}
	model->vertices.clear();
	model->vertices.shrink_to_fit();
	model->indices.clear();
	model->indices.shrink_to_fit();
	delete model;
}


//����������Ҫ��Ⱦ����Ⱦ���Ϸ���壩
void SceneManager::BuildAllRenderGameObject()
{
	for (auto& a : mGeometries)
	{
		MeshGeometry* mesh = a.second;
		if (mesh->Name == "Models/WindGreen/wildGreen_medium_V01")
		{
			for (auto& f : mesh->DrawArgs)
			{
				Grass1* go = new Grass1(mesh, itemCount++, f.first, manager->materialManager->GetMaterial(f.second.modelBindMaterialName, true));
				manager->gameobjectManager->gameobjectList.push_back(go);
				manager->renderManager->OpaqueList1.push_back(go);
			}
		}
		else if (mesh->Name == "Models/door/door")
		{
			for (auto& f : mesh->DrawArgs)
			{
				CityDoor* go = new CityDoor(mesh, itemCount++, f.first, manager->materialManager->GetMaterial(f.second.modelBindMaterialName, true));
				manager->gameobjectManager->gameobjectList.push_back(go);
				manager->renderManager->OpaqueList1.push_back(go);
			}		
		}
		else if (mesh->Name == "plane")
		{
			for (auto& f : mesh->DrawArgs)
			{
				Plane* go = new Plane(mesh, itemCount++, f.first, manager->materialManager->GetMaterial(f.second.modelBindMaterialName, true));
				manager->gameobjectManager->gameobjectList.push_back(go);
				manager->renderManager->OpaqueList1.push_back(go);
			}
		}
		else if (mesh->Name == "moutain")
		{
			for (auto& f : mesh->DrawArgs)
			{
				ScriptLand* go = new ScriptLand(mesh, itemCount++, f.first, manager->materialManager->GetMaterial(f.second.modelBindMaterialName, true));
				manager->gameobjectManager->gameobjectList.push_back(go);
				manager->renderManager->OpaqueList1.push_back(go);
			}
		}
		else if (mesh->Name == "sphere")
		{
			for (auto& f : mesh->DrawArgs)
			{
				Sphere* go = new Sphere(mesh, itemCount++, f.first, manager->materialManager->GetMaterial(f.second.modelBindMaterialName, true));
				manager->gameobjectManager->gameobjectList.push_back(go);
				manager->renderManager->EnvironmentCube.push_back(go);
			}
		}
		else if (mesh->Name == "Models/GaiLun/GaiLun")
		{
			for (auto& f : mesh->DrawArgs)
			{
				GaiLun* go = new GaiLun(mesh, itemCount++, f.first, manager->materialManager->GetMaterial(f.second.modelBindMaterialName, true));
				manager->gameobjectManager->gameobjectList.push_back(go);
				manager->renderManager->OpaqueList2.push_back(go);
			}

			for (auto& f : mesh->DrawArgs)
			{
				GaiLun* go = new GaiLun(mesh, itemCount++, f.first, manager->materialManager->GetMaterial(f.second.modelBindMaterialName, true));
				for (int i = 0; i < go->instanceCount; i++)
				{
					InstanceData& data = go->Instances[i];
					data.World._22 = -data.World._22;
				}
				manager->gameobjectManager->gameobjectList.push_back(go);
				manager->renderManager->OpaqueList3.push_back(go);
			}
			
		}
		else if (mesh->Name == "specular")
		{
			for (auto& f : mesh->DrawArgs)
			{
				Specular* go = new Specular(mesh, itemCount++, f.first, manager->materialManager->GetMaterial(f.second.modelBindMaterialName, true));
				manager->gameobjectManager->gameobjectList.push_back(go);
				manager->renderManager->TransparentList2.push_back(go);
			}
		}
		else if (mesh->Name == "water")
		{
			for (auto& f : mesh->DrawArgs)
			{
				Water* go = new Water(mesh, itemCount++, f.first, manager->materialManager->GetMaterial(f.second.modelBindMaterialName, true));
				manager->gameobjectManager->gameobjectList.push_back(go);
				manager->renderManager->TransparentList3.push_back(go);
			}
		}
		else if (mesh->Name == "skyBox")
		{
			for (auto& f : mesh->DrawArgs)
			{
				SkyBox* go = new SkyBox(mesh, itemCount++, f.first, manager->materialManager->GetMaterial(f.second.modelBindMaterialName, true));
				manager->gameobjectManager->gameobjectList.push_back(go);
				manager->renderManager->SkyBoxList.push_back(go);
			}
		}
		else if (mesh->Name == "screen")
		{
			for (auto& f : mesh->DrawArgs)
			{
				ScreenRenderObj* go = new ScreenRenderObj(mesh, itemCount++, f.first, manager->materialManager->GetMaterial(f.second.modelBindMaterialName, true));
				manager->gameobjectManager->gameobjectList.push_back(go);
				manager->renderManager->postEffect.push_back(go);
			}
		}
		else if (mesh->Name == "ui")
		{
			for (auto& f : mesh->DrawArgs)
			{
				UISprite* go = new UISprite(mesh, itemCount++, f.first, manager->materialManager->GetMaterial(f.second.modelBindMaterialName, true));
				manager->gameobjectManager->gameobjectList.push_back(go);
				manager->renderManager->UIList.push_back(go);
			}
		}
		else if (mesh->Name == "text")
		{
			for (auto& f : mesh->DrawArgs)
			{
				Font* font = manager->fontManager->font;
				UIText* go = new UIText(mesh, itemCount++, f.first, manager->materialManager->GetMaterial(f.second.modelBindMaterialName, true), font);
				manager->gameobjectManager->gameobjectList.push_back(go);
				manager->renderManager->UIList.push_back(go);
				string s1 = "�������������������WASD�����������λ��|";
				string s2 = "����Z�ر���Ļ����X������Ч��C������˹ģ����V�����Աȶȱ��Ͷ����ȣ�B����BloomЧ��|";
				string s3 = "����O��Ⱦ���棬����P��Ⱦ���󣬰���L������Ⱦ���������壬����K�ر���Ⱦ����������";
				go->SetText(s1 + s2 + s3);
			}
		}
		else if (mesh->Name == "3d�ı�")
		{
			for (auto& f : mesh->DrawArgs)
			{
				Text3D* go = new Text3D(mesh, itemCount++, f.first, manager->materialManager->GetMaterial(f.second.modelBindMaterialName, true));
				manager->gameobjectManager->gameobjectList.push_back(go);
				manager->renderManager->OpaqueList1.push_back(go);
			}
		}
		else if (mesh->Name == "skull")
		{
			for (auto& f : mesh->DrawArgs)
			{
				SkullParticle* go = new SkullParticle(mesh, itemCount++, f.first, manager->materialManager->GetMaterial(f.second.modelBindMaterialName, true));
				manager->gameobjectManager->gameobjectList.push_back(go);
				manager->renderManager->OpaqueList1.push_back(go);
			}
		}
	}
}

void SceneManager::Update(const GameTimer& gt)
{
	for (auto& gameobject : manager->gameobjectManager->gameobjectList)
	{
		gameobject->Update(gt);
	}
}

void SceneManager::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void SceneManager::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void SceneManager::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		manager->cameraManager->mainCamera->Pitch(dy);
		manager->cameraManager->mainCamera->RotateY(dx);
	}
	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void SceneManager::OnKeyboardInput(const GameTimer& gt)
{
	const float dt = gt.DeltaTime();
	float speed = 30.0f;
	Camera* camera = manager->cameraManager->mainCamera;
	if (GetAsyncKeyState('W') & 0x8000)
		camera->Walk(speed*dt);

	if (GetAsyncKeyState('S') & 0x8000)
		camera->Walk(-speed * dt);

	if (GetAsyncKeyState('A') & 0x8000)
		camera->Strafe(-speed * dt);

	if (GetAsyncKeyState('D') & 0x8000)
		camera->Strafe(speed*dt);

	camera->UpdateViewMatrix();
}