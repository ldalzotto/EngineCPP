#pragma once

#include "Common/Clock/clock.hpp"


template<class EngineLoopCallbacksType>
struct EngineLoop
{
	// To avoid crazy amount of update if the elapsed time from last frame is huge
	static const int MAX_UPDATE_CALL_PER_FRAME = 2;

	int timebetweenupdates_mics;
	TimeClockPrecision previousUpdateTime_mics;
	TimeClockPrecision accumulatedelapsedtime_mics;
	EngineLoopCallbacksType callbacks;

	EngineLoop() {}

	inline EngineLoop(const EngineLoopCallbacksType& p_callbacks, TimeClockPrecision p_timebetweenupdates_mics)
	{
		this->callbacks = p_callbacks;
		this->timebetweenupdates_mics = (int)p_timebetweenupdates_mics;
		this->accumulatedelapsedtime_mics = 0;
		this->previousUpdateTime_mics = clock_currenttime_mics();
	};

	inline void update()
	{
		TimeClockPrecision l_currentTime = clock_currenttime_mics();
		TimeClockPrecision l_elapsed = l_currentTime - this->previousUpdateTime_mics;

		if (l_elapsed > (TimeClockPrecision)(this->timebetweenupdates_mics) * EngineLoop::MAX_UPDATE_CALL_PER_FRAME)
		{
			l_elapsed = (TimeClockPrecision)(this->timebetweenupdates_mics) * EngineLoop::MAX_UPDATE_CALL_PER_FRAME;
		}

		this->previousUpdateTime_mics = l_currentTime;
		this->accumulatedelapsedtime_mics += l_elapsed;

		if (this->accumulatedelapsedtime_mics >= this->timebetweenupdates_mics) {

			this->update_internal(this->timebetweenupdates_mics * 0.000001f);
		}
		else
		{
			WaitForSingleObject(GetCurrentThread(), (this->timebetweenupdates_mics - this->accumulatedelapsedtime_mics) * 0.0009999);
		}
	};

	inline void update_forced_delta(float p_delta)
	{
		TimeClockPrecision l_currentTime = clock_currenttime_mics();
		this->previousUpdateTime_mics = l_currentTime;
		this->accumulatedelapsedtime_mics += (TimeClockPrecision)p_delta;

		this->update_internal(p_delta);
	};

private:
	inline void update_internal(float p_delta)
	{
		this->callbacks.newframe_callback();

		this->callbacks.update_callback(p_delta);
		this->callbacks.endupdate_callback();

		this->callbacks.render_callback();

		//TODO -> having a more precise loop (while delta < max  {} delta -= max) but preventing some piece of code to run twice.
		this->accumulatedelapsedtime_mics = 0;

		this->callbacks.endofframe_callback();
	}
};