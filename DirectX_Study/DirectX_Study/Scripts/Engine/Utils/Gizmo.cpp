#include "Gizmo.h"

using namespace DK;

DK::Gizmo::Gizmo()
{
}

DK::Gizmo::~Gizmo()	{}

void DK::Gizmo::Init(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCmdList, int nFrameResourceCount, DXGI_FORMAT eBackBufferFormat, DXGI_FORMAT eDSFormat)
{
	BuildMeshBuffer(pDevice, pCmdList);
	BuildFrameResource(pDevice, nFrameResourceCount);
	BuildShaderAndInputLayout();
	BuildRootSignature(pDevice);
	BuildPSO(pDevice, eBackBufferFormat, eDSFormat);
}

void DK::Gizmo::Update(Camera* pCamera)
{
	m_nCurIndexUploadBuffer = (m_nCurIndexUploadBuffer + 1) % m_vecGizmoConstant.size();

	UpdateBaseGridConstant(pCamera);
}

void DK::Gizmo::PreRender(ID3D12GraphicsCommandList* pCmdList)
{
	pCmdList->SetGraphicsRootSignature(m_pRootSig.Get());
	pCmdList->SetPipelineState(m_pPSOBlend.Get());

	DrawBaseGrid(pCmdList);
}

DirectX::XMFLOAT4 DK::Gizmo::SetGizmoColor(DirectX::XMFLOAT4 color)
{
	DirectX::XMFLOAT4 orgColor = m_gizmoColor;
	m_gizmoColor = DirectX::XMFLOAT4(color);
	
	return orgColor;
}

void DK::Gizmo::BuildMeshBuffer(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCmdList)
{
	MeshData<GizmoVertex> mesh;

	int halfLen = 500;
	for (int pos = -halfLen; pos <= halfLen; pos += 10)
	{
		int vertexIndex = mesh.Vertices.size();

		GizmoVertex top;
		top.Position = DirectX::XMFLOAT3(pos, 0, halfLen);
		top.Color = m_gizmoColor;

		GizmoVertex bottom;
		bottom.Position = DirectX::XMFLOAT3(pos, 0, -halfLen);
		bottom.Color = m_gizmoColor;

		GizmoVertex left;
		left.Position = DirectX::XMFLOAT3(halfLen, 0, pos);
		left.Color = m_gizmoColor;

		GizmoVertex right;
		right.Position = DirectX::XMFLOAT3(-halfLen, 0, pos);
		right.Color = m_gizmoColor;

		mesh.Vertices.push_back(top);
		mesh.Vertices.push_back(bottom);
		mesh.Vertices.push_back(left);
		mesh.Vertices.push_back(right);

		mesh.Indices32.push_back(vertexIndex);
		mesh.Indices32.push_back(vertexIndex + 1);
		mesh.Indices32.push_back(vertexIndex + 2);
		mesh.Indices32.push_back(vertexIndex + 3);
	}

	std::unique_ptr<MeshBuffer<GizmoVertex>> sceneBuffer = std::make_unique<MeshBuffer<GizmoVertex>>();
	sceneBuffer->AddMeshData(m_kMeshNameBaseGrid, mesh);
	sceneBuffer->CreateMeshBuffer(pDevice, pCmdList);

	m_pMeshBuffer = std::move(sceneBuffer);
}

void DK::Gizmo::BuildFrameResource(ID3D12Device* pDevice, int nFrameResourceCount)
{
	for (int i = 0; i < nFrameResourceCount; ++i)
	{
		m_vecGizmoConstant.push_back(
			std::make_unique<UploadBuffer<GizmoConstant>>(pDevice, 1, true));
	}
}

void DK::Gizmo::BuildShaderAndInputLayout()
{
	m_vecInputLayout = GizmoVertex::GetInputLayout();

	m_pByteVS = D3DUtils::CompileShader(L"Shaders\\Gizmo.hlsl", nullptr, "VS", "vs_5_0");
	m_pBytePS = D3DUtils::CompileShader(L"Shaders\\Gizmo.hlsl", nullptr, "PS", "ps_5_0");
}

void DK::Gizmo::BuildRootSignature(ID3D12Device* pDevice)
{
	D3D12_ROOT_PARAMETER parameter;
	parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	parameter.Descriptor.ShaderRegister = 0;
	parameter.Descriptor.RegisterSpace = 0;

	D3D12_ROOT_SIGNATURE_DESC rootSigDesc;
	rootSigDesc.NumParameters = 1;
	rootSigDesc.pParameters = &parameter;
	rootSigDesc.NumStaticSamplers = 0;
	rootSigDesc.pStaticSamplers = nullptr;
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, 
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	THROW_IF_FAILED(hr);

	pDevice->CreateRootSignature(0,
		serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_pRootSig));
}

void DK::Gizmo::BuildPSO(ID3D12Device* pDevice, DXGI_FORMAT eBackBufferFormat, DXGI_FORMAT eDSFormat)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	// opaque pso
	psoDesc.pRootSignature = m_pRootSig.Get();
	psoDesc.VS = 
	{
		m_pByteVS->GetBufferPointer(),
		m_pByteVS->GetBufferSize()
	};
	psoDesc.PS =
	{
		m_pBytePS->GetBufferPointer(),
		m_pBytePS->GetBufferSize()
	};
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.InputLayout = { m_vecInputLayout.data(), (UINT)m_vecInputLayout.size() };
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = eBackBufferFormat;
	psoDesc.DSVFormat = eDSFormat;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;

	pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPSO));

	// blend pso
	D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc;
	rtBlendDesc.BlendEnable = true;
	rtBlendDesc.LogicOpEnable = false;
	rtBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	rtBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	rtBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	rtBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	rtBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	rtBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0] = rtBlendDesc;

	psoDesc.BlendState = blendDesc;

	pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPSOBlend));

}

void DK::Gizmo::UpdateBaseGridConstant(Camera* pCamera)
{
	DirectX::XMFLOAT3 camPosition = pCamera->GetTransform().GetPosition();
	XMMATRIX matrixWorld = DirectX::XMMatrixTranslation((int)camPosition.x / 10 * 10, 0, (int)camPosition.z / 10 * 10);

	DirectX::XMFLOAT4X4 viewMatrix = pCamera->GetViewMatrixf4();
	DirectX::XMFLOAT4X4 projMatrix = pCamera->GetProjMatrixf4();
	DirectX::XMMATRIX view = XMLoadFloat4x4(&viewMatrix);
	DirectX::XMMATRIX proj = XMLoadFloat4x4(&projMatrix);
	DirectX::XMMATRIX matrixViewProj = XMMatrixMultiply(view, proj);

	GizmoConstant gizmoConstant;
	XMStoreFloat4x4(&gizmoConstant.matrixWorld, XMMatrixTranspose(matrixWorld));
	XMStoreFloat4x4(&gizmoConstant.matrixViewProj, XMMatrixTranspose(matrixViewProj));
	gizmoConstant.camPos = camPosition;

	auto uploadBuffer = m_vecGizmoConstant[m_nCurIndexUploadBuffer].get();
	uploadBuffer->CopyData(0, gizmoConstant);
}

void DK::Gizmo::DrawBaseGrid(ID3D12GraphicsCommandList* pCmdList)
{
	D3D12_VERTEX_BUFFER_VIEW vbView = m_pMeshBuffer->VertexBufferView();
	D3D12_INDEX_BUFFER_VIEW ibView = m_pMeshBuffer->IndexBufferView();

	pCmdList->IASetVertexBuffers(0, 1, &vbView);
	pCmdList->IASetIndexBuffer(&ibView);
	pCmdList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);

	ID3D12Resource* constBuffer = m_vecGizmoConstant[m_nCurIndexUploadBuffer]->GetBuffer();
	pCmdList->SetGraphicsRootConstantBufferView(0, constBuffer->GetGPUVirtualAddress());

	MeshSection section;
	m_pMeshBuffer->GetMeshSection(m_kMeshNameBaseGrid, section);
	pCmdList->DrawIndexedInstanced(section.IndexCount, 1, section.StartIndexLocation, section.BaseVertexLocation, 0);
}
