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
#include "../Engine/Common/Wave.h"

namespace DK
{
	struct RenderItem
	{
		RenderItem() = default;

		int NumFramesDirty = 0;

		int nCBIndex = -1;
		GameObject* pGameObject = nullptr;
		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		XMFLOAT4X4 TexTransform = MathUtils::Identity4x4();
	};

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

		void LoadTexture();
		void CreateGeometry();
		void CreateMaterial();
		void CreateGameObject();
		void CreateFrameResource();
		void LoadTexture(std::wstring fileName);

		void BuildDescriptor();
		void BuildInputLayoutAndShader();
		void BuildRootSignature();
		void BuildPSO();

		void AnimateMaterials(const GameTimer& gt);
		void UpdateWave(const GameTimer& gt);
		void UpdateRenderPassCB();
		void UpdateObjectCBs();
		void UpdateMaterialCBs();

		void DrawGameObjects(ID3D12GraphicsCommandList* cmdList);
		void DrawRenderItem(ID3D12GraphicsCommandList* cmdList, RenderItem renderItem);

		DirectX::XMFLOAT3 GetHillNormal(float x, float z);


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
		std::vector<GameObject*> m_vecGameObjects;		// object
		std::vector<RenderItem> m_vecRenderItem;
		std::vector<RenderItem> m_vecRenderItemTransparent;

		// frame resource
		const int m_nFrameReesourceCount = 3;
		std::vector<std::unique_ptr<FrameResource>> m_vecFrameResoruce;
		FrameResource* m_pCurrFrameResource = nullptr;
		int m_nCurrFrameResourceIndex = 0;

		// constants
		RenderPassConstants m_renderPassCB;

		std::unique_ptr<Wave> m_waves;
		GameObject* m_pGoWave;

		Gizmo m_gizmo;
	};
}