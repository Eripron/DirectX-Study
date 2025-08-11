#include "ExBox.h"
#include <DirectXPackedVector.h>

namespace DK
{
	ExBox::ExBox(HWND hWnd) : GraphicEngine(hWnd)
	{
	}

	ExBox::~ExBox()
	{
	}

	void ExBox::Init()
	{
		BuildDescriptorHeaps();
		BuildConstantBuffers();
		BuildRootSignature();
		BuildShadersAndInputLayout();
		BuildBoxGeometry();
		BuildPSO();
	}

	bool ExBox::OnResize(int width, int height, bool force)
	{
		GraphicEngine::OnResize(width, height, force);

		DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI * 0.25f, AspectRatio(), 1.0f, 1000.0f);
		DirectX::XMStoreFloat4x4(&mProj, P);

		return true;
		return false;
	}

	bool ExBox::Update()
	{
		//if (GraphicEngine::Update() == false)
		//	return false;

		//mTheta += GetXAxisInput() * 0.001f;
		//mPhi += GetYAxisInput() * 0.001f;
		//if (mPhi < 0.1f)
		//	mPhi = 0.1f;
		//else if (mPhi > DirectX::XM_PI - 0.1f)
		//	mPhi = DirectX::XM_PI - 0.1f;

		//// Convert Spherical to Cartesian coordinates.
		//float x = mRadius * sinf(mPhi) * cosf(mTheta);
		//float z = mRadius * sinf(mPhi) * sinf(mTheta);
		//float y = mRadius * cosf(mPhi);

		//// Build the view matrix.
		//DirectX::XMVECTOR pos = DirectX::XMVectorSet(x, y, z, 1.0f);
		//DirectX::XMVECTOR target = DirectX::XMVectorZero();
		//DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		//DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
		//XMStoreFloat4x4(&mView, view);

		//DirectX::XMMATRIX world = XMLoadFloat4x4(&mWorld);
		//DirectX::XMMATRIX proj = XMLoadFloat4x4(&mProj);
		//DirectX::XMMATRIX worldViewProj = world * view * proj;

		//// Update the constant buffer with the latest worldViewProj matrix.
		//ObjectConstants objConstants;
		//XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
		//XMStoreFloat4x4(&wvp.WorldViewProj, XMMatrixTranspose(worldViewProj));
		//wvp.Color = DirectX::XMFLOAT4(DirectX::Colors::White);
		//wvp.time = m_gameTimer.DeltaTimef();
		////mObjectCB->CopyData(0, objConstants);

		return true;
	}

	bool ExBox::Render()
	{
		//// command allocator / command list 초기화 (순서 중요)
		//THROW_IF_FAILED(mDirectCmdListAlloc->Reset());
		//THROW_IF_FAILED(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSO.Get()));

		//// view 설정
		//// RSSetViewports: 화면으로 출력할 영역(viewport) 설정
		//// RSSetScissorRects: 출력 시 제외할 영역(rect) 설정
		//mCommandList->RSSetViewports(1, &mScreenViewport);
		//mCommandList->RSSetScissorRects(1, &mScissorRect);

		//// back buffer 리소스 상태 Render Target으로 전환
		//D3D12_RESOURCE_BARRIER rscBarrierTransition = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		//mCommandList->ResourceBarrier(1, &rscBarrierTransition);

		//// 렌더 타겟과 뎁스.스탠실 초기화
		//mCommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);
		//mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		//// output merge 단계에서 사용할 render, depth.stencil buffer binding
		//D3D12_CPU_DESCRIPTOR_HANDLE back = CurrentBackBufferView();
		//D3D12_CPU_DESCRIPTOR_HANDLE handle = DepthStencilView();
		//mCommandList->OMSetRenderTargets(1, &back, true, &handle);

		//// 힙을 GPU에 바인딩
		//ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
		//mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		//// 셰이더에서 사용할 root signature를 바인딩
		//mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
		//mCommandList->SetGraphicsRootDescriptorTable(0, mCbvHeap->GetGPUDescriptorHandleForHeapStart());

		//D3D12_VERTEX_BUFFER_VIEW bufferView = mBoxGeo->VertexBufferView();
		//mCommandList->IASetVertexBuffers(0, 1, &bufferView);
		//D3D12_VERTEX_BUFFER_VIEW colorView = mBoxGeo->ColorBufferView();
		//mCommandList->IASetVertexBuffers(1, 1, &colorView);

		//D3D12_INDEX_BUFFER_VIEW inxBufferView = mBoxGeo->IndexBufferView();
		//mCommandList->IASetIndexBuffer(&inxBufferView);
		//mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//XMStoreFloat4x4(&mWorld, DirectX::XMMatrixTranslation(-2, 0, 0));
		//Update();
		//mObjectCB->CopyData(0, wvp);
		//mCommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

		//XMStoreFloat4x4(&mWorld, DirectX::XMMatrixTranslation(2, 0, 0));
		//Update();
		//mObjectCB->CopyData(1, wvp);
		//D3D12_GPU_DESCRIPTOR_HANDLE han = mCbvHeap->GetGPUDescriptorHandleForHeapStart();
		//han.ptr += mCbvSrvUavDescriptorSize;
		//mCommandList->SetGraphicsRootDescriptorTable(0, han);
		//mCommandList->DrawIndexedInstanced(18, 1, 36, 8, 0);

		//// Indicate a state transition on the resource usage.
		//D3D12_RESOURCE_BARRIER backBufferTransition = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		//	D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		//mCommandList->ResourceBarrier(1, &backBufferTransition);

		//// Done recording commands.
		//THROW_IF_FAILED(mCommandList->Close());

		//// Add the command list to the queue for execution.
		//ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		//mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		//// swap the back and front buffers
		//THROW_IF_FAILED(mSwapChain->Present(0, 0));
		//mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

		//// Wait until frame commands are complete.  This waiting is inefficient and is
		//// done for simplicity.  Later we will show how to organize our rendering code
		//// so we do not have to wait per frame.
		//FlushCommandQueue();

		return true;
	}

	void ExBox::BuildDescriptorHeaps()
	{
		// Const Buffer 리소스에 대한 서술자(Descriptor)를 저장할 Heap을 할당한다.
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
		cbvHeapDesc.NumDescriptors = 2;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		/*
		* D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
		  : GPU측에서 해당 힙에 접근할 수 있는지를 나타내는 flag
		    Type이 CBV_SRV_UAV와 SAMPLER만 해당 플래그 가능
		*/
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.NodeMask = 0;

		THROW_IF_FAILED(m_d3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mCbvHeap)));
	}

	void ExBox::BuildConstantBuffers()
	{
		// const buffer 생성
		mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(m_d3dDevice.Get(), 2, true);

		UINT objCBByteSize = D3DUtils::CalcConstBufferByteSize(sizeof(ObjectConstants));

		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->GetBuffer()->GetGPUVirtualAddress();

		// upload buffer를 개별로 만드는 것이 아니라 데이터를 연속적으로 저장하므로
		// 여러 object 저장 시 직접 위치 이동해야한다.

		D3D12_CPU_DESCRIPTOR_HANDLE handle = mCbvHeap->GetCPUDescriptorHandleForHeapStart();

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc[2];
		for (int i = 0; i < 2; ++i)
		{
			int boxCBufIndex = i;
			cbAddress += (boxCBufIndex * objCBByteSize);

			cbvDesc[i].BufferLocation = cbAddress;
			cbvDesc[i].SizeInBytes = objCBByteSize;

			m_d3dDevice->CreateConstantBufferView(&cbvDesc[i], handle);
			handle.ptr += m_uCbvSrvUavDescriptorSize;
		}
	}

	void ExBox::BuildRootSignature()
	{
		CD3DX12_ROOT_PARAMETER slotRootParameter[1];

		slotRootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		slotRootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		D3D12_DESCRIPTOR_RANGE descriptorRange;
		descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		descriptorRange.NumDescriptors = 2;
		descriptorRange.BaseShaderRegister = 0;
		descriptorRange.RegisterSpace = 0;
		descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_DESCRIPTOR_TABLE rdt;
		rdt.NumDescriptorRanges = 1;
		rdt.pDescriptorRanges = &descriptorRange;

		slotRootParameter[0].DescriptorTable = rdt;

		// Create a single descriptor table of CBVs.
		/*CD3DX12_DESCRIPTOR_RANGE cbvTable;
		cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);*/

		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		THROW_IF_FAILED(hr);

		m_d3dDevice->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(&mRootSignature));
	}

	void ExBox::BuildShadersAndInputLayout()
	{
		HRESULT hr = S_OK;

		mvsByteCode = D3DUtils::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
		mpsByteCode = D3DUtils::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

		mInputLayout = 
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
	}

	void ExBox::BuildBoxGeometry()
	{
		//// 정육면체의 vertex 배열 생성
		//array<VPosData, 8> boxVer
		//{
		//	VPosData({ DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f) }),
		//	VPosData({ DirectX::XMFLOAT3(-1.0f, +1.0f, -1.0f) }),
		//	VPosData({ DirectX::XMFLOAT3(+1.0f, +1.0f, -1.0f) }),
		//	VPosData({ DirectX::XMFLOAT3(+1.0f, -1.0f, -1.0f) }),
		//	VPosData({ DirectX::XMFLOAT3(-1.0f, -1.0f, +1.0f) }),
		//	VPosData({ DirectX::XMFLOAT3(-1.0f, +1.0f, +1.0f) }),
		//	VPosData({ DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f) }),
		//	VPosData({ DirectX::XMFLOAT3(+1.0f, -1.0f, +1.0f) })
		//};

		//array<VColorData, 8> boxColor
		//{
		//	VColorData({ DirectX::PackedVector::XMCOLOR(DirectX::Colors::Yellow) }),
		//	VColorData({ DirectX::PackedVector::XMCOLOR(DirectX::Colors::Yellow) }),
		//	VColorData({ DirectX::PackedVector::XMCOLOR(DirectX::Colors::Yellow) }),
		//	VColorData({ DirectX::PackedVector::XMCOLOR(DirectX::Colors::Yellow) }),
		//	VColorData({ DirectX::PackedVector::XMCOLOR(DirectX::Colors::Yellow) }),
		//	VColorData({ DirectX::PackedVector::XMCOLOR(DirectX::Colors::Yellow) }),
		//	VColorData({ DirectX::PackedVector::XMCOLOR(DirectX::Colors::Yellow) }),
		//	VColorData({ DirectX::PackedVector::XMCOLOR(DirectX::Colors::Yellow) })
		//};

		//array<uint16_t, 36> boxIndex =
		//{
		//	0, 1, 2,
		//	0, 2, 3,

		//	// back face
		//	4, 6, 5,
		//	4, 7, 6,

		//	// left face
		//	4, 5, 1,
		//	4, 1, 0,

		//	// right face
		//	3, 2, 6,
		//	3, 6, 7,

		//	// top face
		//	1, 5, 6,
		//	1, 6, 2,

		//	// bottom face
		//	4, 0, 3,
		//	4, 3, 7
		//};

		//// 사면체 
		//array<VPosData, 5> vertices
		//{
		//	VPosData({ DirectX::XMFLOAT3(0.0f, 2.0f, 0.0f) }),
		//	VPosData({ DirectX::XMFLOAT3(+1.0f, -1.0f, +1.0f) }),
		//	VPosData({ DirectX::XMFLOAT3(+1.0f, -1.0f, -1.0f) }),
		//	VPosData({ DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f) }),
		//	VPosData({ DirectX::XMFLOAT3(-1.0f, -1.0f, +1.0f) }),
		//};

		//array<VColorData, 5> colors
		//{
		//	VColorData({ DirectX::PackedVector::XMCOLOR(DirectX::Colors::Red) }),
		//	VColorData({ DirectX::PackedVector::XMCOLOR(DirectX::Colors::Green) }),
		//	VColorData({ DirectX::PackedVector::XMCOLOR(DirectX::Colors::Green) }),
		//	VColorData({ DirectX::PackedVector::XMCOLOR(DirectX::Colors::Green) }),
		//	VColorData({ DirectX::PackedVector::XMCOLOR(DirectX::Colors::Green) }),
		//};

		//// index 배열 생성 (6면 * 삼각형 2개 * 점3개 = 36개)
		//array<uint16_t, 18> indices =
		//{
		//	0, 1, 2,
		//	0, 2, 3,
		//	0, 3, 4,
		//	0, 4, 1,
		//	1, 4, 3,
		//	1, 3, 2,
		//};

		//std::vector<VPosData> vertexs;
		//vertexs.insert(vertexs.end(), boxVer.begin(), boxVer.end());
		//vertexs.insert(vertexs.end(), vertices.begin(), vertices.end());

		//std::vector<VColorData> vecColors;
		//vecColors.insert(vecColors.end(), boxColor.begin(), boxColor.end());
		//vecColors.insert(vecColors.end(), colors.begin(), colors.end());

		//std::vector<uint16_t> vecIndexs;
		//vecIndexs.insert(vecIndexs.end(), boxIndex.begin(), boxIndex.end());
		//vecIndexs.insert(vecIndexs.end(), indices.begin(), indices.end());

		//const UINT vbByteSize = (UINT)vertexs.size() * sizeof(VPosData);
		//const UINT cbByteSize = (UINT)vecColors.size() * sizeof(VColorData);
		//const UINT ibByteSize = (UINT)vecIndexs.size() * sizeof(uint16_t);

		//mBoxGeo = std::make_unique<MeshGeometry>();
		//mBoxGeo->Name = "boxGeo";

		//// 임시 buffer에 데이터 복사
		//THROW_IF_FAILED(D3DCreateBlob(vbByteSize, &mBoxGeo->VertexBufferCPU));
		//CopyMemory(mBoxGeo->VertexBufferCPU->GetBufferPointer(), vertexs.data(), vbByteSize);

		//THROW_IF_FAILED(D3DCreateBlob(cbByteSize, &mBoxGeo->ColorBufferCPU));
		//CopyMemory(mBoxGeo->ColorBufferCPU->GetBufferPointer(), vecColors.data(), cbByteSize);

		//THROW_IF_FAILED(D3DCreateBlob(ibByteSize, &mBoxGeo->IndexBufferCPU));
		//CopyMemory(mBoxGeo->IndexBufferCPU->GetBufferPointer(), vecIndexs.data(), ibByteSize);

		//// 데이터를 default buffer에 생성
		//mBoxGeo->VertexBufferGPU = D3DUtils::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertexs.data(), vbByteSize, mBoxGeo->VertexBufferUploader);
		//mBoxGeo->ColorBufferGPU = D3DUtils::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vecColors.data(), cbByteSize, mBoxGeo->ColorBufferUploader);
		//mBoxGeo->IndexBufferGPU = D3DUtils::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vecIndexs.data(), ibByteSize, mBoxGeo->IndexBufferUploader);

		//mBoxGeo->VertexByteStride = sizeof(VPosData);
		//mBoxGeo->VertexBufferByteSize = vbByteSize;

		//mBoxGeo->ColorByteStride = sizeof(VColorData);
		//mBoxGeo->ColorBufferByteSize = cbByteSize;

		//mBoxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
		//mBoxGeo->IndexBufferByteSize = ibByteSize;

		//SubmeshGeometry submesh;
		//submesh.IndexCount = (UINT)indices.size();
		//submesh.StartIndexLocation = 0;
		//submesh.BaseVertexLocation = 0;

		//mBoxGeo->DrawArgs["box"] = submesh;
	}

	void ExBox::BuildPSO()
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

		// std::vector<T>::data(): Returns a pointer to the underlying array serving as element storage.
		psoDesc.pRootSignature = mRootSignature.Get();
		psoDesc.VS =
		{
			reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),
			mvsByteCode->GetBufferSize()
		};
		psoDesc.PS =
		{
			reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),
			mpsByteCode->GetBufferSize()
		};
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;

		D3D12_RASTERIZER_DESC rd;
		rd.FillMode = D3D12_FILL_MODE_SOLID;
		rd.CullMode = D3D12_CULL_MODE_BACK;
		rd.FrontCounterClockwise = FALSE;
		rd.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		rd.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		rd.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rd.DepthClipEnable = TRUE;
		rd.MultisampleEnable = FALSE;
		rd.AntialiasedLineEnable = FALSE;
		rd.ForcedSampleCount = 0;
		rd.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		psoDesc.RasterizerState = rd;

		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = m_eBackBufferFormat;
		psoDesc.DSVFormat = m_eDepthStencilFormat;
		psoDesc.SampleDesc.Count = m_b4xMsaaState ? 4 : 1;
		psoDesc.SampleDesc.Quality = m_b4xMsaaState ? (m_u4xMsaaQuality - 1) : 0;
		THROW_IF_FAILED(m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));
	}

}