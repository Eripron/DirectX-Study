#pragma once

#include "../Engine/EngineBase.h"
#include "../Engine/Common/AnimationHelper.h"

namespace DK
{
	class Testing : public EngineBase
	{
	public:
		Testing(HWND hWnd);
		~Testing();

	protected:
		virtual bool Update() override;
		virtual void Render(ID3D12GraphicsCommandList* cmdList) override;

		virtual bool OnResize(int width, int height, bool force) override;

		virtual void CreateMesh() override;
		virtual void LoadTextures() override;
		virtual void CreateMaterial() override;
		virtual void CreateGameObject() override;
		virtual void CreateRenderObjectInfo() override;

		virtual void BuildRootSignature() override;
		virtual void BuildPSO() override;

	protected:
		void RenderCubeMap(ID3D12GraphicsCommandList* cmdList, int i);

	private:
		float _accumTime = 0.0f;

		vector<int> _boneIndexing;
		vector<float> _planetRadius;
		vector<float> _orbitSpeed;
		vector<RenderObjectInfo*> _boneObjects;
		vector<XMMATRIX> _parentWorlds;
		unordered_map<int, vector<BoneAnimation>> _boneAnimations;

		void DefineAnimationKeyFrames();
		void UpdateAnimation(BoneAnimation animation, float deltaTime, XMFLOAT4X4& M);

		BoneAnimation CreateOrbitAnimation(float time, float radius);
		BoneAnimation CreateTiltAnimation(XMVECTOR axis, float angle);
		BoneAnimation CreateSpinAnimation(float time, int spinDir = 1);
	};

}