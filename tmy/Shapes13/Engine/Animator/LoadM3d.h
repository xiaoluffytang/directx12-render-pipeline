#ifndef LOADM3D_H
#define LOADM3D_H

#include "SkinnedData.h"

//持有动画实例
class SkinnedModelInstance
{
public:
	SkinnedData* SkinnedInfo = nullptr;
	std::vector<DirectX::XMFLOAT4X4> FinalTransforms;
	std::string ClipName;
	float TimePos = 0.0f;
	float speed = 0.5f;
	// Called every frame and increments the time position, interpolates the 
	// animations for each bone based on the current animation clip, and 
	// generates the final transforms which are ultimately set to the effect
	// for processing in the vertex shader.
	void UpdateSkinnedAnimation(float dt)
	{
		TimePos += dt * speed;

		// Loop animation
		if (TimePos > SkinnedInfo->GetClipEndTime(ClipName))
			TimePos = 0.0f;
		TimePos = 0;
		// Compute the final transforms for this time position.
		SkinnedInfo->GetFinalTransforms(ClipName, TimePos, FinalTransforms);
	}
};

//动画的顶点数据
struct SkinnedVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexC;
	DirectX::XMFLOAT3 TangentU;
	DirectX::XMFLOAT3 Color = { 1,1,1 };
	DirectX::XMFLOAT3 BoneWeights;   //顶点蒙皮时，每根骨骼的权重，只需存储3个即可，第4个用1-前三个
	BYTE BoneIndices[4];  //受到哪几根骨骼影响（索引）
};

//模型加载器
class M3DLoader
{
public:
    struct Subset
    {
        UINT Id = -1;
        UINT VertexStart = 0;
        UINT VertexCount = 0;
        UINT FaceStart = 0;
        UINT FaceCount = 0;
    };

    struct M3dMaterial
    {
        std::string Name;

        DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
        DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
        float Roughness = 0.8f;
        bool AlphaClip = false;

        std::string MaterialTypeName;
        std::string DiffuseMapName;
        std::string NormalMapName;
    };

	bool LoadM3d(const std::string& filename, 
		std::vector<SkinnedVertex>& vertices,
		std::vector<USHORT>& indices,
		std::vector<Subset>& subsets,
		std::vector<M3dMaterial>& mats,
		SkinnedData& skinInfo, ID3D12Device* device);

private:
	void ReadMaterials(std::ifstream& fin, UINT numMaterials, std::vector<M3dMaterial>& mats);
	void ReadSubsetTable(std::ifstream& fin, UINT numSubsets, std::vector<Subset>& subsets);
	void ReadSkinnedVertices(std::ifstream& fin, UINT numVertices, std::vector<SkinnedVertex>& vertices);
	void ReadTriangles(std::ifstream& fin, UINT numTriangles, std::vector<USHORT>& indices);
	void ReadBoneOffsets(std::ifstream& fin, UINT numBones, std::vector<DirectX::XMFLOAT4X4>& boneOffsets);
	void ReadBoneHierarchy(std::ifstream& fin, UINT numBones, std::vector<int>& boneIndexToParentIndex);
	void ReadAnimationClips(std::ifstream& fin, UINT numBones, UINT numAnimationClips, std::unordered_map<std::string, AnimationClip>& animations);
	void ReadBoneKeyframes(std::ifstream& fin, UINT numBones, BoneAnimation& boneAnimation);
};



#endif // LOADM3D_H