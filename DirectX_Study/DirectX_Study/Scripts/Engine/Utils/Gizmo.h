#pragma once

#include <wrl.h>
#include <DirectXColors.h>

#include "../Data/DataTypes.h"
#include "../Resource/UploadBuffer.h"

namespace DK
{
	struct GizmoVertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT4 Color;

		static std::vector<D3D12_INPUT_ELEMENT_DESC> GetInputLayout()
		{
			std::vector< D3D12_INPUT_ELEMENT_DESC> layout = 
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			};

			return layout;
		}
	};

	struct GizmoConstant
	{
		DirectX::XMFLOAT4X4 matrixWorld = MathUtils::Identity4x4();
		DirectX::XMFLOAT4X4 matrixViewProj = MathUtils::Identity4x4();
		DirectX::XMFLOAT3 camPos;
	};

	class Gizmo
	{
	public:
		Gizmo();
		~Gizmo();

		void Init(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCmdList, int nFrameResourceCount, DXGI_FORMAT eBackBufferFormat, DXGI_FORMAT eDSFormat);
		void Update(Camera* pCamera);
		void PreRender(ID3D12GraphicsCommandList* pCmdList);

		DirectX::XMFLOAT4 SetGizmoColor(DirectX::XMFLOAT4 color);
		void OnDrawLine(DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2);

	private:
		// init
		void BuildMeshBuffer(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCmdList);
		void BuildFrameResource(ID3D12Device* pDevice, int nFrameResourceCount);
		void BuildShaderAndInputLayout();
		void BuildRootSignature(ID3D12Device* pDevice);
		void BuildPSO(ID3D12Device* pDevice, DXGI_FORMAT eBackBufferFormat, DXGI_FORMAT eDSFormat);
		void CreateGizmoBuffer(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCmdList);
		//

		// update
		void UpdateBaseGridConstant(Camera* pCamera);
		void UpdateGizmoBuffer(ID3D12GraphicsCommandList* pCmdList);
		//

		// render
		void DrawBaseGrid(ID3D12GraphicsCommandList* pCmdList);
		void DrawGizmo(ID3D12GraphicsCommandList* pCmdList);
		//

	private:
		ID3D12Device* m_device;

		const std::string m_kMeshNameBaseGrid = "BaseGrid";
		DirectX::XMFLOAT4 m_gizmoColor = DirectX::XMFLOAT4(DirectX::Colors::White);

		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pPSO = nullptr;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pPSOBlend = nullptr;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pPSOGizmo = nullptr;

		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pRootSig;

		std::vector<D3D12_INPUT_ELEMENT_DESC> m_vecInputLayout;

		Microsoft::WRL::ComPtr<ID3DBlob> m_pByteVS;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pBytePS;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pByteGizmoVS;

		std::unique_ptr<MeshBuffer<GizmoVertex>> m_pMeshBuffer;

		int m_nMaxGizmoVertexCount = 1000;
		std::vector<GizmoVertex> m_gizmoVertexs;
		std::vector<uint16_t> m_gizmoIndexs;

		Microsoft::WRL::ComPtr<ID3D12Resource> GizmoBuffer = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> GizmoIndexBuffer = nullptr;
		std::unique_ptr<UploadBuffer<GizmoVertex>> GizmoVertexUploadBuffer[3];
		std::unique_ptr<UploadBuffer<uint16_t>> GizmoIndexUploadBuffer[3];

		// frame resource
		std::vector<std::unique_ptr<UploadBuffer<GizmoConstant>>> m_vecGizmoConstant;
		int m_nCurIndexUploadBuffer = 0;
	};
}