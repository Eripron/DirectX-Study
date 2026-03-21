#pragma once

#include "../Engine/EngineBase.h"
#include "../Engine/Common/AnimationHelper.h"
#include "../Engine/Utils/LoadM3d.h"

namespace DK
{
	struct SkinnedModelInstance
	{
		SkinnedData* SkinnedInfo = nullptr;
		std::vector<DirectX::XMFLOAT4X4> FinalTransforms;
		std::string ClipName;
		float TimePos = 0.0f;

		// Called every frame and increments the time position, interpolates the 
		// animations for each bone based on the current animation clip, and 
		// generates the final transforms which are ultimately set to the effect
		// for processing in the vertex shader.
		void UpdateSkinnedAnimation(float dt)
		{
			TimePos += dt;

			// Loop animation
			if (TimePos > SkinnedInfo->GetClipEndTime(ClipName))
				TimePos = 0.0f;

			// Compute the final transforms for this time position.
			SkinnedInfo->GetFinalTransforms(ClipName, TimePos, FinalTransforms);
		}
	};

	class Testing : public EngineBase
	{
	public:
		Testing(HWND hWnd);
		~Testing();

	protected:
		virtual void Init() override;
		virtual bool Update() override;
		virtual void Render(ID3D12GraphicsCommandList* cmdList) override;

		virtual bool OnResize(int width, int height, bool force) override;

		virtual void CreateMesh() override;
		virtual void LoadTextures() override;
		virtual void CreateMaterial() override;
		virtual void CreateGameObject() override;
		virtual void CreateRenderObjectInfo() override;

		virtual void BuildInputLayoutAndShader() override;
		virtual void BuildRootSignature() override;
		virtual void BuildPSO() override;

	protected:
		void RenderCubeMap(ID3D12GraphicsCommandList* cmdList, int i);

	// region animation
	private:

		std::unique_ptr<SkinnedModelInstance> _skinnedModelInst;

		SkinnedData _skinnedInfo;
		std::vector<M3DLoader::Subset> _skinnedSubsets;		// subMesh 정보
		std::vector<M3DLoader::M3dMaterial> _skinnedMats;	// material 정보
		std::vector<std::string> _skinnedTextureNames;		// texture 정보
		vector<D3D12_INPUT_ELEMENT_DESC> _skinnedInputLayouts;

		MeshBuffer<M3DLoader::SkinnedVertex> _modelMeshBuffer;	// model mesh 데이터

		void LoadModelData();


	// endregion animation

	private:
		float _accumTime = 0.0f;

		vector<int> _boneIndexing;
		vector<float> _planetRadius;
		vector<float> _orbitSpeed;
		vector<RenderObjectInfo*> _boneObjects;
		vector<XMMATRIX> _parentWorlds;
		unordered_map<int, vector<BoneAnimation>> _boneAnimations;

		vector<Transform*> _playerTransform;
		vector<RenderObjectInfo*> _playerObjects;
		float _playerRot = 0.0f;

		void DefineAnimationKeyFrames();
		void UpdateAnimation(BoneAnimation animation, float deltaTime, XMFLOAT4X4& M);

		BoneAnimation CreateOrbitAnimation(float time, float radius);
		BoneAnimation CreateTiltAnimation(XMVECTOR axis, float angle);
		BoneAnimation CreateSpinAnimation(float time, int spinDir = 1);
	};

}