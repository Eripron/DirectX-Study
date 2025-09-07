#pragma once

#include "../Engine/GraphicEngine.h"

#include "../Engine/Component/Component.h"
#include "../Engine/Component/MeshFilter.h"

#include "../Engine/Utils/MathUtils.h"
#include "../Engine/Utils/GeometryGenerator.h"
#include "../Engine/Utils/DDSTextureLoader.h"

#include "../Engine/Component/GameObject.h"
#include "../Engine/Resource/FrameResource.h"
#include "../Engine/Common/Wave.h"

#include <fstream>

namespace DK
{
	enum class RenderLayer : int
	{
		Opaque = 0,
		Transparent,
		AlphaTested,
		AlphaTestedTreeSprites,
		Count
	};

	class ExTexture : public GraphicEngine
	{
	public:
		ExTexture(HWND hWnd);
		~ExTexture();

	protected:
		virtual void Init() override;
		virtual bool Update() override;
		virtual bool Render() override;

		void LoadTexture();
		void LoadTexture(std::wstring fileName, int texCBIndex);

		void CreateMaterial();
		void CreateGeometry();
		void CreateGameObject();
		void CreateFrameResource();

		void BuildDescriptor();
		void BuildRootSignature();
		void BuildInputLayoutAndShader();
		void BuildPSO();

		void AnimateMaterials(const GameTimer& gt);
		void UpdateWave(const GameTimer& gt);
		void UpdateObjectCBs();
		void UpdateMaterialCBs();
		void UpdateRenderPassCB();

		void DrawGameObjects(ID3D12GraphicsCommandList* cmdList, std::vector<GameObject*> vecGameObject);

		DirectX::XMFLOAT3 GetHillNormal(float x, float z);

	private:

		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_spRootSignature;
		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_mapPSO;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_spHeapSRV;

		std::vector<D3D12_INPUT_ELEMENT_DESC> m_vecInputLayout;
		std::vector<D3D12_INPUT_ELEMENT_DESC> m_vecInputLayoutTree;

		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> m_mapShaders;

		// resource
		std::unordered_map<std::string, std::unique_ptr<Texture>> m_mapTextures;	// texture
		std::unordered_map<std::string, std::unique_ptr<Material>> m_mapMaterials;	// material

		std::vector<GameObject*> m_vecGameObjects[(int)RenderLayer::Count];

		// frame resource
		const int m_nFrameReesourceCount = 3;
		std::vector<std::unique_ptr<FrameResource>> m_vecFrameResoruce;
		FrameResource* m_pCurrFrameResource = nullptr;
		int m_nCurrFrameResourceIndex = 0;

		// constants
		RenderPassConstants m_renderPassCB;

	};
}