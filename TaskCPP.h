/**
 * @file TaskCPP.h
 * @brief FreeRTOS Task Wrapper
 *
 * This file contains a set of lightweight wrappers for tasks using FreeRTOS
 *
 * @copyright (c) 2007-2015 Richard Damon
 * @author Richard Damon <richard.damon@gmail.com>
 * @parblock
 * MIT License:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * It is requested (but not required by license) that any bugs found or
 * improvements made be shared, preferably to the author.
 * @endparblock
 *
 * @ingroup FreeRTOSCpp
 *
 * @defgroup FreeRTOSCpp Free RTOS C++ Wrapper
 */
#ifndef TaskCPP_H
#define TaskCPP_H


#ifndef TASK_CPP_STACK_DEPTH
#define TASK_CPP_STACK_DEPTH configMINIMAL_STACK_SIZE
#endif // !TASK_CPP_STACK_DEPTH

#include <MapleFreeRTOS900.h>

 /*
 *| configMAX_PRIORITIES:	| 1 | 2 | 3 | 4 | 5 | 6 | N>6	| Use |
 *| --------------------:	| - | - | - | - | - | - | - : -	| : ------------------------------------------------- : |
 *| TaskPrio_Idle			| 0 | 0 | 0 | 0 | 0 | 0 |	0	| Non - Real Time operations, Tasks that don't block	|
 *| TaskPrio_Low			| 0 | 1 | 1 | 1 | 1 | 1 |	1	| Non - Critical operations								|
 *| TaskPrio_HMI			| 0 | 1 | 1 | 1 | 1 | 2 |	2	| Normal User Interface									|
 *| TaskPrio_Mid			| 0 | 1 | 1 | 2 | 2 | 3 | N / 2 | Semi - Critical, Deadlines, not much processing		|
 *| TaskPrio_High			| 0 | 1 | 2 | 3 | 3 | 4 | N - 2 | Urgent, Short Deadlines, not much processing			|
 *| TaskPrio_Highest		| 0 | 1 | 2 | 3 | 4 | 5 | N - 1 | Critical, do NOW, must be quick(Used by FreeRTOS)		|
 */

class TaskCPP
{
private:
	bool Alive = false;
	bool Suspended = false;
	const char * TaskName;
	UBaseType_t Priority = tskIDLE_PRIORITY;
	TaskHandle_t TaskHandle;

public:
	TaskCPP(UBaseType_t priority)
	{
		TaskName = "X";
	}

	TaskCPP(const char * const taskName, UBaseType_t priority)
	{
		TaskName = taskName;
		Priority = priority;
	}

	UBaseType_t GetPriority()
	{
		return Priority;
	}

	const char * GetTaskName()
	{
		return pcTaskGetTaskName(TaskHandle);
	}

	void Run()
	{
		OnRun();
		OnDestroy();
		// If we get here, task has returned, delete ourselves or block indefinitely.
#if INCLUDE_vTaskDelete
		TaskHandle = 0;
		vTaskDelete(TaskHandle);
#else
		while (1)
			vTaskDelay(portMAX_DELAY);
#endif
		
	}

	void Delay(const uint32_t period)
	{
		vTaskDelay(period * portTICK_PERIOD_MS);
	}

	bool IsAlive()
	{
		return Alive;
	}

	void Start()
	{
		if (!Alive)
		{
			Alive = true;
			xTaskCreate(StaticTask, TaskName,
				TASK_CPP_STACK_DEPTH,
				this, Priority,
				&TaskHandle);
		}
	}

	void Pause()
	{
		if (IsAlive() && !Suspended)
		{
			Suspended = true;
			vTaskSuspend(TaskHandle);
		}
	}

	void Resume()
	{
		if (IsAlive() && Suspended)
		{
			OnResume();
			vTaskResume(TaskHandle);
			Suspended = false;
		}
	}

	void Stop()
	{
		if (Alive)
		{
			OnStop();
		}
		Alive = false;
	}

protected:
	virtual void OnRun() {}
	virtual void OnResume() {}
	virtual void OnStop() {}
	virtual void OnDestroy() {}

private:
	//Static trap escape method. 
	static void StaticTask(void* pvParameters)
	{
		//pvParameters is the task object instance, as passed in xTaskCreate.
		TaskCPP *internalTask = static_cast<TaskCPP *>(pvParameters);

		//Now that we have the class object pointer, let's call the Run method.
		internalTask->Run();
	}
};
#endif

