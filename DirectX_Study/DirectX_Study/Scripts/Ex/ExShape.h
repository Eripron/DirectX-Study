#pragma once

#include "../Engine/GraphicEngine.h"

#include "../Engine/Utils/D3DUtils.h"
#include "../Engine/Utils/MathUtils.h"
#include "../Engine/Utils/GeometryGenerator.h"

#include "../Engine/Data/DataTypes.h"
#include "../Engine/Resource/FrameResource.h"
#include "../Engine/Component/GameObject.h"
#include "../Engine/Common/Wave.h"

namespace DK
{
	const int g_NumFrameResources = 3;

	struct RenderObject
	{
		RenderObject() = default;

		int NumFramesDirty = g_NumFrameResources;
		GameObject* pGameObject = nullptr;
		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	};

	class ExShape : public GraphicEngine
	{
	public:
		ExShape(HWND hWnd);
		~ExShape();


	protected:
		virtual void Init() override;
		virtual bool OnResize(int width, int height, bool force) override;

		virtual bool Update() override;
		virtual bool Render() override;

		void UpdateWave(const GameTimer& gt);
		void UpdateObjectCBs(const GameTimer& gt);
		void UpdateRenderPassCB(const GameTimer& gt);
		void UpdateMaterialCB(const GameTimer& gt);

		void CreateGeometry();
		void CreateMaterials();
		void BuildRenderObject();
		void BuildShadersAndInputLayout();
		void BuildFrameResources();
		void BuildRootSignature();
		void BuildPSOs();

		void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderObject>& renderObjects);

		DirectX::XMFLOAT3 GetHillNormal(float x, float z);

	private:

		std::unique_ptr<Wave> m_waves;

		std::unordered_map<std::string, std::unique_ptr<MeshBuffer>> m_meshBuffers;
		std::unordered_map<std::string, std::unique_ptr<Material>> m_materials;

		GameObject* m_pGOWave;
		std::vector<std::unique_ptr<GameObject>> m_gameObjects;
		std::vector<RenderObject> m_vecRenderObject;

		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> m_shaderByteCode;
		std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;

		std::vector<std::unique_ptr<FrameResource>> m_frameResources;
		FrameResource* m_pCurrFrameResource = nullptr;
		int m_nCurrFrameResourceIndex = 0;

		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;

		RenderPassConstants m_renderPassCB;
	};
}