#pragma once

#include "../Utils/D3DUtils.h"

namespace DK
{
	template<class T>
	class UploadBuffer
	{
		UploadBuffer(const UploadBuffer& rhs) = delete;
		UploadBuffer& operator=(const UploadBuffer& rhs) = delete;

	public:
		UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer)
			: m_bConstBuffer(isConstantBuffer)
		{
			m_uElementByteSize = sizeof(T);
			if (isConstantBuffer)
				m_uElementByteSize = D3DUtils::CalcConstBufferByteSize(m_uElementByteSize);

			// upload heap 속성 정의
			D3D12_HEAP_PROPERTIES heapProp;
			heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
			heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProp.CreationNodeMask = 1;
			heapProp.VisibleNodeMask = 1;

			D3D12_RESOURCE_DESC rscDesc;
			rscDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			rscDesc.Alignment = 0;
			rscDesc.Width = m_uElementByteSize * elementCount;
			rscDesc.Height = 1;
			rscDesc.DepthOrArraySize = 1;
			rscDesc.MipLevels = 1;
			rscDesc.SampleDesc.Count = 1;
			rscDesc.SampleDesc.Quality = 0;
			rscDesc.Format = DXGI_FORMAT_UNKNOWN;
			rscDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;	// buffer는 무조건 해당 layout 사용
			rscDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			THROW_IF_FAILED(device->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&rscDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_uploadBuffer)));

			// 생성된 upload buffer에 cpu가 접근하여 데이터를 쓰기 위해서 cpu 메모리를 맵핑하는 작업
			m_uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_pMappedData));
			/*
			HRESULT Map(
					  UINT              Subresource   - 버퍼는 항상 0 사용
	  [in, optional]  const D3D12_RANGE *pReadRange   - 읽기 범위, 쓰기만 하는 경우는 nullptr
	  [out, optional] void              **ppData	  - 맵핑된 버퍼 주소를 받아올 포인터
	);
			*/
		}

		~UploadBuffer()
		{
			if (m_uploadBuffer != nullptr)
				m_uploadBuffer->Unmap(0, nullptr);

			m_pMappedData = nullptr;
		}

		ID3D12Resource* GetBuffer()
		{
			return m_uploadBuffer.Get();
		}

		void CopyData(int elementIndex, const T& data)
		{
			memcpy(&m_pMappedData[elementIndex * m_uElementByteSize], &data, sizeof(T));
		}

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer;
		BYTE* m_pMappedData = nullptr;

		UINT m_uElementByteSize = 0;
		bool m_bConstBuffer = false;

	};
}