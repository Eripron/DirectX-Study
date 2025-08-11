#pragma once

#include "../Engine/GraphicEngine.h"
#include "../Engine/Utils/MathUtils.h"
#include "../Engine/Resource/UploadBuffer.h"

#include <array>
#include <vector>
#include <memory>

using namespace std;

namespace DK
{
	/*struct ObjectConstants
	{
		DirectX::XMFLOAT4X4 WorldViewProj = MathUtils::Identity4x4();
		DirectX::XMFLOAT4 Color;
		float time;
	};*/

	class ExBox : public GraphicEngine
	{
	public:
		ExBox(HWND hWnd);
		~ExBox();


	protected:
		virtual void Init() override;
		virtual bool OnResize(int width, int height, bool force);
		virtual bool Update() override;
		virtual bool Render() override;

		void BuildDescriptorHeaps();
		void BuildConstantBuffers();
		void BuildRootSignature();
		void BuildShadersAndInputLayout();
		void BuildBoxGeometry();
		void BuildPSO();

	protected:
		Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

		std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;

		Microsoft::WRL::ComPtr<ID3DBlob> mvsByteCode = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> mpsByteCode = nullptr;

		std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

		Microsoft::WRL::ComPtr<ID3D12PipelineState> mPSO = nullptr;

		DirectX::XMFLOAT4X4 mWorld = MathUtils::Identity4x4();
		DirectX::XMFLOAT4X4 mView = MathUtils::Identity4x4();
		DirectX::XMFLOAT4X4 mProj = MathUtils::Identity4x4();
		ObjectConstants wvp;


		float mTheta = DirectX::XM_PIDIV4;	// 수평각도
		float mPhi = DirectX::XM_PIDIV4;	// 수직각도
		float mRadius = 5.0f;

		POINT mLastMousePos;
	};

}