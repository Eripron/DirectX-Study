#pragma once

#include "GraphicEngine.h"
#include "GeometryGenerator.h"
#include "FrameResource.h"

namespace DK
{
	const int gNumFrameResources = 3;

	struct RenderItem
	{
		RenderItem() = default;

		DirectX::XMFLOAT4X4 World = MathUtils::Identity4x4();

		int NumFramesDirty = gNumFrameResources;

		UINT ObjCBIndex = -1;

		MeshGeometry* Geo = nullptr;

		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		UINT IndexCount = 0;
		UINT StartIndexLocation = 0;
		int BaseVertexLocation = 0;
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
		void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);

	private:

		std::vector<std::unique_ptr<FrameResource>> mFrameResources;
		FrameResource* mCurrFrameResource = nullptr;
		int mCurrFrameResourceIndex = 0;

		Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

		std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> mShaders;
		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> mPSOs;

		std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
		std::vector<std::unique_ptr<RenderItem>> mAllRitems;

		std::vector<RenderItem*> mOpaqueRitems;

		PassConstants mMainPassCB;

		UINT mPassCbvOffset = 0;

		bool mIsWireframe = false;

		DirectX::XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT4X4 mView = MathUtils::Identity4x4();
		DirectX::XMFLOAT4X4 mProj = MathUtils::Identity4x4();

		float mTheta = 1.5f * DirectX::XM_PI;
		float mPhi = 0.2f * DirectX::XM_PI;
		float mRadius = 15.0f;
	};
}