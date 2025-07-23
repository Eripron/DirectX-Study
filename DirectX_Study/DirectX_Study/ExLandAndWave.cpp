#include "ExLandAndWave.h"

namespace DK
{
	ExLandAndWave::ExLandAndWave(HWND hWnd) : GraphicEngine(hWnd)
	{
	}

	ExLandAndWave::~ExLandAndWave()
	{
	}

	bool ExLandAndWave::Init()
	{
		if (GraphicEngine::Init() == false)
			return false;

		return true;
	}

	void ExLandAndWave::OnResize(int width, int height)
	{
		GraphicEngine::OnResize(width, height);
	}

	void ExLandAndWave::Update()
	{
	}

	void ExLandAndWave::Render()
	{
	}

	void ExLandAndWave::BuildMeshData()
	{
		GeometryGenerator geoGen;
		GeometryGenerator::MeshData meshGrid = geoGen.CreateGrid(160.0f, 160.0f, 50, 50);

	}

}