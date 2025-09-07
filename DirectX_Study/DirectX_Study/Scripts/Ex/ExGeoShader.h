#pragma once

#include "../Engine/GraphicEngine.h"

namespace DK
{
	class ExGeoShader : public GraphicEngine
	{
	public:
		ExGeoShader(HWND hWnd);
		~ExGeoShader();

	protected:
		virtual void Init() override;
		virtual bool Update() override;
		virtual bool Render() override;

	private:
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_spRootSignature;
		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_mapPSO;

	};

}