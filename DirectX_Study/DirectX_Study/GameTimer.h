#pragma once

namespace DK
{
	class GameTimer
	{
	public:
		GameTimer();

		double TotalTime() const;
		float TotalTimef() const;
		double DeltaTime() const;
		float DeltaTimef() const;

		void Reset();
		void Start();
		void Stop();
		void Tick();

	private:
		__int64 GetCurrCounter();

	private:
		double mSecondsPerCount;	// 1tick time
		double mDeltaTime;

		__int64 mBaseTime;
		__int64 mPausedTime;
		__int64 mStopTime;
		__int64 mPrevTime;
		__int64 mCurrTime;

		bool mStopped;
	};

}