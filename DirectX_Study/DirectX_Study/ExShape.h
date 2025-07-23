#pragma once

#include "GraphicEngine.h"
#include "GeometryGenerator.h"
#include "FrameResource.h"
#include "GameObject.h"
#include "MeshRenderData.h"

namespace DK
{
	const int gNumFrameResources = 3;

	struct RenderObject
	{
		RenderObject() = default;

		int NumFramesDirty = gNumFrameResources;
		GameObject* pRenderObject = nullptr;
		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	};

	class ExShape : public GraphicEngine
	{
	public:
		ExShape(HWND hWnd);
		~ExShape();

		virtual bool Init() override;

	protected:

		virtual void OnResize(int width, int height) override;
		virtual void Update() override;
		virtual void Render() override;

		void OnKeyboardInput(const GameTimer& gt);
		void UpdateCamera(const GameTimer& gt);
		void UpdateObjectCBs(const GameTimer& gt);
		void UpdateMainPassCB(const GameTimer& gt);

		void BuildRootSignature();
		void BuildShadersAndInputLayout();
		void BuildShapeGeometry();
		void BuildRenderItems();
		void BuildFrameResources();
		void BuildDescriptorHeaps();
		void BuildConstantBufferViews();
		void BuildPSOs();
		void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderObject>& renderObjects);

	private:

		std::vector<std::unique_ptr<FrameResource>> mFrameResources;
		FrameResource* mCurrFrameResource = nullptr;
		int mCurrFrameResourceIndex = 0;

		Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> mShaders;
		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> mPSOs;

		std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

		PassConstants mMainPassCB;
		UINT mPassCbvOffset = 0;

		Transform camera;

		DirectX::XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT4X4 mView = MathUtils::Identity4x4();
		DirectX::XMFLOAT4X4 mProj = MathUtils::Identity4x4();

		float mTheta = 1.5f * DirectX::XM_PI;
		float mPhi = 0.2f * DirectX::XM_PI;
		float mRadius = 15.0f;

	private:
		MeshRenderData mMeshRenderData;
		std::vector<std::unique_ptr<GameObject>> mGameObjects;
		std::vector<RenderObject> mRenderObjects;

	private:
		std::unordered_map<std::string, GeometryGenerator::MeshData> mMeshDatas;

	};
}