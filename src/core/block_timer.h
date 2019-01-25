
namespace Timer
{
	struct Timer
	{
		f32 time;
		f32 length;
	};

	inline
	Timer start(f32 length=0.0f)
	{
		return {(f32) game.clock.time, length};
	}

	inline 
	Timer stop(Timer timer)
	{
		return {-1.0f, timer.length};
	}

	inline
	bool check(Timer timer)
	{
		// Stopped
		if (timer.time == -1.0f)
			return false;
		return game.clock.time > (timer.time + timer.length);
	}

	inline
	Timer reset(Timer timer)
	{
		return {(f32) game.clock.time, timer.length};
	}

	inline
	f32 time_passed(Timer timer)
	{
		return (f32) timer.time - game.clock.time;
	}

	inline
	f32 time_procentage_passed(Timer timer)
	{
		return (f32) (timer.time - game.clock.time) / timer.length;
	}
}

