#include <Windows.h>
#include "GameTimer.h"

using namespace DK;

GameTimer::GameTimer()
	: m_dSecondsPerCount(0.0), m_dDeltaTime(-1.0), 
	  m_llBaseTime(0), m_llPausedTime(0), m_llStopTime(0), m_llPrevTime(0), m_llCurrTime(0),
	  m_bStopped(false)
{
	_LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	m_dSecondsPerCount = 1.0 / static_cast<double>(frequency.QuadPart);
}

double GameTimer::TotalTime() const
{
	// stop
	if (m_bStopped)
	{
		return (m_llStopTime - m_llBaseTime - m_llPausedTime) * m_dSecondsPerCount;
	}
	// running
	else
	{
		return (m_llCurrTime - m_llBaseTime - m_llPausedTime) * m_dSecondsPerCount;
	}
}

// Returns the total time elapsed since Reset() was called, NOT counting any
// time when the clock is stopped.
float GameTimer::TotalTimef() const
{
	return static_cast<float>(TotalTime());
}

double GameTimer::DeltaTime() const
{
	return m_dDeltaTime;
}

float GameTimer::DeltaTimef() const
{
	return static_cast<float>(m_dDeltaTime);
}

void DK::GameTimer::Init()
{
	Reset();
}

void GameTimer::Reset()
{
	__int64 currCount = GetCurrCounter();

	m_llBaseTime = currCount;
	m_llPausedTime = 0;
	m_llStopTime = 0;
	m_llPrevTime = currCount;
	m_bStopped = false;
}

void GameTimer::Start()
{
	// Accumulate the time elapsed between stop and start pairs.
	//
	//                     |<-------d------->|
	// ----*---------------*-----------------*------------> time
	//  mBaseTime       mStopTime        startTime     

	if (m_bStopped)
	{
		__int64 startTime = GetCurrCounter();

		m_llPausedTime += (startTime - m_llStopTime);

		// reset
		m_llStopTime = 0;
		m_llPrevTime = startTime;
		m_bStopped = false;
	}
}

void GameTimer::Stop()
{
	if (!m_bStopped)
	{
		m_llStopTime = GetCurrCounter();
		m_bStopped = true;
	}
}

void GameTimer::Tick()
{
	if (m_bStopped)
	{
		m_dDeltaTime = 0.0;
		return;
	}

	m_llCurrTime = GetCurrCounter();

	// Time difference between this frame and the previous.
	m_dDeltaTime = (m_llCurrTime - m_llPrevTime) * m_dSecondsPerCount;

	// Prepare for next frame.
	m_llPrevTime = m_llCurrTime;

	// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
	// processor goes into a power save mode or we get shuffled to another
	// processor, then mDeltaTime can be negative.
	if (m_dDeltaTime < 0)
	{
		m_dDeltaTime = 0;
	}
}

__int64 GameTimer::GetCurrCounter()
{
	LARGE_INTEGER currTickCount;
	QueryPerformanceCounter(&currTickCount);
	return currTickCount.QuadPart;
}
