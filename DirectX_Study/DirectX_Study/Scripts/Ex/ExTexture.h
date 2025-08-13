#pragma once

#include "../Engine/GraphicEngine.h"
#include "../Engine/Data/DataTypes.h"

#include "../Engine/Utils/D3DUtils.h"
#include "../Engine/Utils/MathUtils.h"
#include "../Engine/Utils/GeometryGenerator.h"
#include "../Engine/Utils/DDSTextureLoader.h"
#include "../Engine/Utils/Gizmo.h"

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

	protected:
		virtual void Init() override;
		virtual bool OnResize(int width, int height, bool force) override;
		virtual bool Update() override;
		virtual bool Render() override;

		void CreateGeometry();
		void CreateMaterial();
		void CreateGameObject();
		void CreateFrameResource();
		void LoadTexture();
		void LoadTexture(std::wstring fileName);

		void BuildDescriptor();
		void BuildInputLayoutAndShader();
		void BuildRootSignature();
		void BuildPSO();

		void UpdateRenderPassCB();
		void UpdateObjectCBs();
		void UpdateMaterialCBs();

		void DrawGameObjects(ID3D12GraphicsCommandList* cmdList);

	private:
		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_mapPSO;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_spRootSignature;
		
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_spHeapSRV;

		std::vector<D3D12_INPUT_ELEMENT_DESC> m_vecInputLayout;

		// resource
		std::vector<std::unique_ptr<MeshBuffer<Vertex>>> m_vecMeshBuffers;						// mesh
		std::unordered_map<std::string, std::unique_ptr<Material>> m_mapMaterials;		// material
		std::unordered_map<std::string, std::unique_ptr<Texture>> m_mapTextures;
		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> m_mapShaders;	// shader code
		std::vector<GameObject> m_vecGameObjects;		// object

		// frame resource
		const int m_nFrameReesourceCount = 3;
		std::vector<std::unique_ptr<FrameResource>> m_vecFrameResoruce;
		FrameResource* m_pCurrFrameResource = nullptr;
		int m_nCurrFrameResourceIndex = 0;

		// constants
		RenderPassConstants m_renderPassCB;

		Gizmo m_gizmo;
	};
}