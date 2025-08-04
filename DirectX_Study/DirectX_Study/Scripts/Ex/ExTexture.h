#pragma once

#include "../Engine/GraphicEngine.h"
#include "../Engine/Data/DataTypes.h"

#include "../Engine/Utils/D3DUtils.h"
#include "../Engine/Utils/MathUtils.h"
#include "../Engine/Utils/GeometryGenerator.h"

#include "../Engine/Component/GameObject.h"
#include "../Engine/Component/Camera.h"
#include "../Engine/Resource/FrameResource.h"

namespace DK
{
	class ExTexture : public GraphicEngine
	{
	public:
		ExTexture(HWND hWnd);
		~ExTexture();

		virtual bool Init() override;

	protected:
		// Init
		void CreateGeometry();
		void CreateMaterial();
		void CreateGameObject();
		void CreateFrameResource();

		void BuildDescriptor();
		void BuildInputLayoutAndShader();
		void BuildRootSignature();
		void BuildPSO();

	protected:
		virtual bool OnResize(int width, int height, bool force);

		virtual bool Update() override;
		void UpdateRenderPassCB();
		void UpdateObjectCBs();
		void UpdateMaterialCBs();

		virtual bool Render() override;
		void DrawGameObjects(ID3D12GraphicsCommandList* cmdList);

	private:

		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_spPSO;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_spRootSignature;
		
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_spHeapSRV;

		std::vector<D3D12_INPUT_ELEMENT_DESC> m_vecInputLayout;

		// resource
		std::vector<std::unique_ptr<MeshBuffer>> m_vecMeshBuffers;						// mesh
		std::unordered_map<std::string, std::unique_ptr<Material>> m_mapMaterials;		// material
		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> m_mapShaders;	// shader code
		std::vector<GameObject> m_vecGameObjects;		// object

		// frame resource
		const int m_nFrameReesourceCount = 3;
		std::vector<std::unique_ptr<FrameResource>> m_vecFrameResoruce;
		FrameResource* m_pCurrFrameResource = nullptr;
		int m_nCurrFrameResourceIndex = 0;

		// constants
		RenderPassConstants m_renderPassCB;
	};
}