#pragma once

// base engine
#include "../Engine/GraphicEngine.h"

// load texture
#include "../Engine/Utils/DDSTextureLoader.h"	

// mesh
#include "../Engine/Manager/MeshManager.h"
#include "../Engine/Utils/GeometryGenerator.h"

// component
#include "../Engine/Component/GameObject.h"
#include "../Engine/Component/Component.h"
#include "../Engine/Component/MeshFilter.h"

// frame resource
#include "../Engine/Resource/FrameResource.h"

// only
#include "BlurFilter.h"

using namespace Microsoft::WRL;

namespace DK
{
	class ExBlur : public GraphicEngine
	{
	public:
		ExBlur(HWND hWnd);
		~ExBlur();

	protected:
		virtual void Init() override;
		virtual bool Update() override;
		virtual bool Render() override;

		virtual bool OnResize(int width, int height, bool force) override;

		// init
		void LoadTextures();

		void CreateMesh();
		void CreateMaterial();
		void CreateGameObject();
		void CreateFrameResource();

		void BuildDescriptorHeap();
		void BuildRootSignature();
		void BuildPostProcessRootSignature();
		void BuildInputLayoutAndShader();
		void BuildPSO();


		// update
		void UpdateObjectCBs();
		void UpdateMaterialCBs();
		void UpdateMainPassCB();

		// render
		void RenderGameObjects(ID3D12GraphicsCommandList* cmdList, std::vector<GameObject*> vecGameObject);

	private:
		// pso
		ComPtr<ID3D12RootSignature> m_spRootSignature;							// rootsignature
		std::vector<D3D12_INPUT_ELEMENT_DESC> m_vecInputLayout;					// input layout
		std::unordered_map<std::string, ComPtr<ID3DBlob>> m_mapShaders;			// shader
		std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> m_mapPSO;	// pso

		std::unordered_map<std::string, std::unique_ptr<Texture>> m_mapTextures;	// texture resource
		ComPtr<ID3D12DescriptorHeap> m_spHeapCbvSrvUav;

		std::unordered_map<std::string, std::unique_ptr<Material>> m_mapMaterials;	// material

		std::vector<GameObject*> m_vecGameObject[(int)RenderLayer::Count];	// gameobject

		// frame resource
		const int FRAME_RESOURCE_COUNT = 3;
		std::vector<std::unique_ptr<FrameResource>> m_vecFrameResoruce;
		FrameResource* m_pCurrFrameResource = nullptr;
		int m_nCurrFrameResourceIndex = 0;
		
		RenderPassConstants m_renderPassCB;

	private:
		void LoadTexture(std::wstring path);

		std::unique_ptr<BlurFilter> m_blurFilter;
		ComPtr<ID3D12RootSignature> m_postProcessRootSignature;

	};
}