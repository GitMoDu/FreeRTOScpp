// Process.h

#ifndef _PROCESS_h
#define _PROCESS_h

#include <Arduino.h>
#include <TaskCPP.h>

enum ProcPriority
{
	HIGHEST_PRIORITY = TaskPrio_Highest,
	HIGH_PRIORITY = TaskPrio_High,
	MEDIUM_PRIORITY = TaskPrio_Mid,
	LOW_PRIORITY = TaskPrio_Low,
	IDLE_PRIORITY = TaskPrio_Idle,
	NUM_PRIORITY_LEVELS
};

class Process : TaskCPP
{
private:
	enum RequestAction
	{
		None,
		Enable,
		Disable,
		Run
	} PendingAction = None;

	uint32_t Period = 0;

	ProcPriority Priority = IDLE_PRIORITY;

public:
	Process(ProcPriority priority, uint32_t period)
		:TaskCPP((UBaseType_t)priority)
	{
		Priority = priority;
		setPeriod(period);
		PendingAction = RequestAction::Disable;
	}

	Process(const char * const taskName, UBaseType_t priority, uint32_t period)
		:TaskCPP(taskName, (UBaseType_t)priority)
	{
		setPeriod(period);
		PendingAction = RequestAction::Disable;
	}

	inline ProcPriority getPriority()
	{
		return Priority;
	}

	void OnRun()
	{
		if (PendingAction == RequestAction::Disable)
		{
			Pause();
		}
		setup();

		while (true)
		{
			switch (PendingAction)
			{
			case RequestAction::Enable:
				onEnable();
				PendingAction = RequestAction::Run;
				break;
			case RequestAction::Disable:
				onDisable();
				PendingAction = RequestAction::None;
				Pause();
				break;
			case RequestAction::Run:
				service();
				if (Period > 0)
				{
					Delay(Period);
				}
				break;
			case RequestAction::None:
			default:
				Stop();
				return;
			}
		}
	}

	void setPeriod(const uint32_t period)
	{
		Period = period;
	}

	uint32_t getPeriod()
	{
		return Period;
	}

	void disable()
	{
		PendingAction = RequestAction::Disable;
	}

	void restart()
	{
		Stop();
		PendingAction = RequestAction::Enable;
		Start();
	}

	bool isEnabled()
	{
		return IsAlive() &&
			(PendingAction != RequestAction::Disable && PendingAction != RequestAction::None);
	}

	bool isNotDestroyed() { return IsAlive(); }

	void add(bool enableIfNot = true)
	{
		Start();
		if (enableIfNot)
		{
			PendingAction = RequestAction::Enable;
		}
		else
		{
			PendingAction = RequestAction::Disable;
			Pause();
		}
		
	}

	void enable()
	{
		Resume();
	}

	void destroy()
	{
		PendingAction = Process::None;
	}

protected:
	void OnDestroy()
	{
		if (PendingAction == Process::Disable)
		{
			onDisable();
		}
		cleanup();
	}

	void OnResume()
	{
		PendingAction = Process::Enable;
	}

	void OnStop()
	{
		PendingAction = Process::Disable;
	}

	void PrintName(Stream * serial)
	{
		serial->print(GetTaskName());
	}
protected:

	virtual void setup() {}
	virtual void cleanup() {}
	virtual void onEnable() {}
	virtual void onDisable() {}
	virtual void service() {}
};
#endif

