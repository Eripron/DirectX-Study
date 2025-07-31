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

		void Init();
		void Reset();
		void Start();
		void Stop();
		void Tick();

	private:
		__int64 GetCurrCounter();

	private:
		double m_dSecondsPerCount;	// 1tick time
		double m_dDeltaTime;

		__int64 m_llBaseTime;
		__int64 m_llPausedTime;
		__int64 m_llStopTime;
		__int64 m_llPrevTime;
		__int64 m_llCurrTime;

		bool m_bStopped;
	};

}