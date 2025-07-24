#include "Wave.h"

#include <ppl.h>
#include <algorithm>
#include <vector>
#include <cassert>
#include <assert.h>

namespace DK
{
	Wave::Wave(int row, int col, float dx, float dt, float speed, float damping)
	{
		mRowCount = row;
		mColCount = col;

		mVertexCount = row * col;
		mTriangleCount = (row - 1) * (col - 1) * 2;

		mTimeStep = dt;
		mSpatialStep = dx;

		float d = damping * dt + 2.0f;
		float e = (speed * speed) * (dt * dt) / (dx * dx);
		mK1 = (damping * dt - 2.0f) / d;
		mK2 = (4.0f - 8.0f * e) / d;
		mK3 = (2.0f * e) / d;

		mPrevSolution.resize(row * col);
		mCurrSolution.resize(row * col);
		mNormals.resize(row * col);
		mTangentX.resize(row * col);

		// Generate grid vertices in system memory.

		float halfWidth = (row - 1) * dx * 0.5f;
		float halfDepth = (col - 1) * dx * 0.5f;
		for (int i = 0; i < row; ++i)
		{
			float z = halfDepth - i * dx;

			for (int j = 0; j < col; ++j)
			{
				float x = -halfWidth + j * dx;

				int curIndex = i * col + j;

				mPrevSolution[curIndex] = DirectX::XMFLOAT3(x, 0.0f, z);
				mCurrSolution[curIndex] = DirectX::XMFLOAT3(x, 0.0f, z);
				mNormals[curIndex] = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
				mTangentX[curIndex] = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
			}
		}
	}

	Wave::~Wave()
	{
	}

	void Wave::Update(float dt)
	{
		static float t = 0;

		// Accumulate time.
		t += dt;

		// Only update the simulation at the specified time step.
		if (t >= mTimeStep)
		{
			// Only update interior points; we use zero boundary conditions.
			concurrency::parallel_for(1, mRowCount - 1, [this](int i)
				//for(int i = 1; i < mNumRows-1; ++i)
				{
					for (int j = 1; j < mColCount - 1; ++j)
					{
						// After this update we will be discarding the old previous
						// buffer, so overwrite that buffer with the new update.
						// Note how we can do this inplace (read/write to same element) 
						// because we won't need prev_ij again and the assignment happens last.

						// Note j indexes x and i indexes z: h(x_j, z_i, t_k)
						// Moreover, our +z axis goes "down"; this is just to 
						// keep consistent with our row indices going down.

						mPrevSolution[i * mColCount + j].y =
							mK1 * mPrevSolution[i * mColCount + j].y +
							mK2 * mCurrSolution[i * mColCount + j].y +
							mK3 * (mCurrSolution[(i + 1) * mColCount + j].y +
								mCurrSolution[(i - 1) * mColCount + j].y +
								mCurrSolution[i * mColCount + j + 1].y +
								mCurrSolution[i * mColCount + j - 1].y);
					}
				});

			// We just overwrote the previous buffer with the new data, so
			// this data needs to become the current solution and the old
			// current solution becomes the new previous solution.
			std::swap(mPrevSolution, mCurrSolution);

			t = 0.0f; // reset time

			//
			// Compute normals using finite difference scheme.
			//
			concurrency::parallel_for(1, mRowCount - 1, [this](int i)
				//for(int i = 1; i < mNumRows - 1; ++i)
				{
					for (int j = 1; j < mColCount - 1; ++j)
					{
						float l = mCurrSolution[i * mColCount + j - 1].y;
						float r = mCurrSolution[i * mColCount + j + 1].y;
						float t = mCurrSolution[(i - 1) * mColCount + j].y;
						float b = mCurrSolution[(i + 1) * mColCount + j].y;
						mNormals[i * mColCount + j].x = -r + l;
						mNormals[i * mColCount + j].y = 2.0f * mSpatialStep;
						mNormals[i * mColCount + j].z = b - t;

						DirectX::XMVECTOR n = DirectX::XMVector3Normalize(XMLoadFloat3(&mNormals[i * mColCount + j]));
						XMStoreFloat3(&mNormals[i * mColCount + j], n);

						mTangentX[i * mColCount + j] = DirectX::XMFLOAT3(2.0f * mSpatialStep, r - l, 0.0f);
						DirectX::XMVECTOR T = DirectX::XMVector3Normalize(XMLoadFloat3(&mTangentX[i * mColCount + j]));
						XMStoreFloat3(&mTangentX[i * mColCount + j], T);
					}
				});
		}
	}

	int Wave::RowCount() const
	{
		return mRowCount;
	}

	int Wave::ColumnCount() const
	{
		return mColCount;
	}

	int Wave::VertexCount() const
	{
		return mVertexCount;
	}

	int Wave::TriangleCount() const
	{
		return mTriangleCount;
	}

	float Wave::Width() const
	{
		return mColCount * mSpatialStep;
	}

	float Wave::Depth() const
	{
		return mRowCount * mSpatialStep;
	}

	void Wave::Disturb(int i, int j, float magnitude)
	{
		// Don't disturb boundaries.
		/*assert(i > 1 && i < mNumRows - 2);
		assert(j > 1 && j < mNumCols - 2);*/

		float halfMag = 0.5f * magnitude;

		// Disturb the ijth vertex height and its neighbors.
		mCurrSolution[i * mColCount + j].y += magnitude;
		mCurrSolution[i * mColCount + j + 1].y += halfMag;
		mCurrSolution[i * mColCount + j - 1].y += halfMag;
		mCurrSolution[(i + 1) * mColCount + j].y += halfMag;
		mCurrSolution[(i - 1) * mColCount + j].y += halfMag;
	}
}