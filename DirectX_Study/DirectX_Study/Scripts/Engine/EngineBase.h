#pragma once

#include "GraphicEngine.h"

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

using namespace std;
using namespace Microsoft::WRL;

namespace DK
{
	struct RenderItem
	{
		RenderItem() = default;
		RenderItem(const RenderItem& rhs) = delete;

		int ObjBufferIndex = -1;

		GameObject* pGameObject = nullptr;
		XMFLOAT4X4 TexTransform = MathUtils::Identity4x4();
		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		int DirtyCount = 0;

		std::vector<InstanceData> InstanceDatas;
		int RenderInstanceCount = 0;

		DirectX::BoundingBox BoundBox;
	};

	class EngineBase : public GraphicEngine
	{
	public:
		EngineBase(HWND hWnd);
		~EngineBase();

	protected:
		virtual void Init() override;
		virtual bool Update() override;
		virtual bool Render() override;
		virtual void Render(ID3D12GraphicsCommandList* cmdList);

		virtual bool OnResize(int width, int height, bool force);

		// init
		virtual void CreateMesh();
		virtual void CreateMaterial();
		virtual void CreateGameObject();
		virtual void CreateRenderItem();

		virtual void BuildFrameResource();
		virtual void BuildDescriptorHeap();
		virtual void BuildRootSignature();
		virtual void BuildInputLayoutAndShader();
		virtual void BuildPSO();

		// update
		void UpdateObjectCBs();
		void UpdateMaterialCBs();
		void UpdateMainPassCB();

		// render
		void RenderRenderItems(ID3D12GraphicsCommandList* cmdList, std::vector<RenderItem*> renderItems);

	protected:
		virtual void LoadTextures();
		void LoadTexture(std::wstring path);

	protected:
		ComPtr<ID3D12RootSignature> m_rootSignature;
		vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayouts;
		unordered_map<string, ComPtr<ID3DBlob>> m_shaders;
		unordered_map<int, ComPtr<ID3D12PipelineState>> m_psos;	

		// texture data
		ComPtr<ID3D12DescriptorHeap> m_heapCbvSrvUav;
		unordered_map<string, unique_ptr<Texture>> m_textures;

		// material
		unordered_map<string, unique_ptr<Material>> m_materials;

		// object
		vector<GameObject*> m_gameObjects[(int)RenderLayer::Count];

		// render object info list
		vector<std::unique_ptr<RenderItem>> m_renderItems;

		// frame resource
		const int FrameResourceCount = 3;
		vector<unique_ptr<FrameResource>> m_frameResources;
		FrameResource* m_curFrameResource = nullptr;
		int m_curFrameResourceIndex = 0;

		RenderPassConstants m_renderPassCB;

		DirectX::BoundingFrustum m_camFrustum;
	};

}