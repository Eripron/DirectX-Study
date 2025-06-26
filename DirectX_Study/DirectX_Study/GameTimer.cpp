#include <Windows.h>
#include "GameTimer.h"

DK::GameTimer::GameTimer()
	: mSecondsPerCount(0.0), mDeltaTime(-1.0), 
	  mBaseTime(0), mPausedTime(0), mStopTime(0), mPrevTime(0), mCurrTime(0),
	  mStopped(false)
{
	_LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	mSecondsPerCount = 1.0 / static_cast<double>(frequency.QuadPart);
}

double DK::GameTimer::TotalTime() const
{
	// stop
	if (mStopped)
	{
		return (mStopTime - mBaseTime - mPausedTime) * mSecondsPerCount;
	}
	// running
	else
	{
		return (mCurrTime - mBaseTime - mPausedTime) * mSecondsPerCount;
	}
}

// Returns the total time elapsed since Reset() was called, NOT counting any
// time when the clock is stopped.
float DK::GameTimer::TotalTimef() const
{
	return static_cast<float>(TotalTime());
}

double DK::GameTimer::DeltaTime() const
{
	return mDeltaTime;
}

float DK::GameTimer::DeltaTimef() const
{
	return static_cast<float>(mDeltaTime);
}

void DK::GameTimer::Reset()
{
	__int64 currCount = GetCurrCounter();

	mBaseTime = currCount;
	mPausedTime = 0;
	mStopTime = 0;
	mPrevTime = currCount;
	mStopped = false;
}

void DK::GameTimer::Start()
{
	// Accumulate the time elapsed between stop and start pairs.
	//
	//                     |<-------d------->|
	// ----*---------------*-----------------*------------> time
	//  mBaseTime       mStopTime        startTime     

	if (mStopped)
	{
		__int64 startTime = GetCurrCounter();

		mPausedTime += (startTime - mStopTime);

		// reset
		mStopTime = 0;
		mPrevTime = startTime;
		mStopped = false;
	}

}

void DK::GameTimer::Stop()
{
	if (!mStopped)
	{
		mStopTime = GetCurrCounter();
		mStopped = true;
	}
}

void DK::GameTimer::Tick()
{
	if (mStopped)
	{
		mDeltaTime = 0.0;
		return;
	}

	mCurrTime = GetCurrCounter();

	// Time difference between this frame and the previous.
	mDeltaTime = (mCurrTime - mPrevTime) * mSecondsPerCount;

	// Prepare for next frame.
	mPrevTime = mCurrTime;

	// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
	// processor goes into a power save mode or we get shuffled to another
	// processor, then mDeltaTime can be negative.
	if (mDeltaTime < 0)
	{
		mDeltaTime = 0;
	}
}

__int64 DK::GameTimer::GetCurrCounter()
{
	LARGE_INTEGER currTickCount;
	QueryPerformanceCounter(&currTickCount);
	return currTickCount.QuadPart;
}
