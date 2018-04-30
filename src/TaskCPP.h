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
 *
 *
 *
 * Modifed by GitModu
 *
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

enum TaskPriority {
	TaskPrio_Idle = 0,                                                    ///< Non-Real Time operations. tasks that don't block
	TaskPrio_Low = ((configMAX_PRIORITIES) > 1),                             ///< Non-Critical operations
	TaskPrio_HMI = (TaskPrio_Low + ((configMAX_PRIORITIES) > 5)),            ///< Normal User Interface Level
	TaskPrio_Mid = ((configMAX_PRIORITIES) / 2),                            ///< Semi-Critical, have deadlines, not a lot of processing
	TaskPrio_High = ((configMAX_PRIORITIES)-1 - ((configMAX_PRIORITIES) > 4)), ///< Urgent tasks, short deadlines, not much processing
	TaskPrio_Highest = ((configMAX_PRIORITIES)-1)                         ///< Critical Tasks, Do NOW, must be quick (Used by FreeRTOS)
};


class TaskCPP
{
private:
private:
	enum TaskStateEnum
	{
		Dead,
		Alive,
		Suspended
	} TaskState;

	const char * TaskName;
	UBaseType_t Priority = tskIDLE_PRIORITY;
	TaskHandle_t TaskHandle = NULL;

public:
	TaskCPP(UBaseType_t priority)
	{
		TaskName = "X";
		TaskHandle = NULL;
	}

	TaskCPP(const char * const taskName, UBaseType_t priority)
	{
		TaskName = taskName;
		Priority = priority;
		TaskHandle = NULL;
	}

	UBaseType_t GetPriority()
	{
		return Priority;
	}

	const char * GetTaskName()
	{
		return pcTaskGetTaskName(TaskHandle);
	}

	uint16_t GetStackSize()
	{
#if INCLUDE_uxTaskGetStackHighWaterMark
		return uxTaskGetStackHighWaterMark(TaskHandle);
#else
		return 0;
#endif		
	}

	void Run()
	{
		OnRun();
		// If we get here, task has returned, delete ourselves or block indefinitely.
		Stop();
	}

	void Delay(const uint32_t period)
	{
		vTaskDelay(period * portTICK_PERIOD_MS);
	}

	bool IsAlive()
	{
		return TaskState == TaskStateEnum::Alive || TaskState == TaskStateEnum::Suspended;
	}

	void Debug(Stream * serial)
	{
		serial->print("TaskState: ");
		switch (TaskState)
		{
		case TaskStateEnum::Dead:
			serial->println("Dead");
			break;
		case TaskStateEnum::Alive:
			serial->println("Alive");
			break;
		case TaskStateEnum::Suspended:
			serial->println("Suspended");
			break;
		default:
			serial->println("Unknown");
			break;
		}
	}

	bool IsSuspended()
	{
		return TaskState == TaskStateEnum::Suspended;
	}

	void Start()
	{
		switch (TaskState)
		{
		case TaskStateEnum::Dead:
			TaskState = TaskStateEnum::Alive;
			OnStart();
#if INCLUDE_vTaskDelete
			xTaskCreate(StaticTask, TaskName,
				TASK_CPP_STACK_DEPTH,
				this, Priority,
				&TaskHandle);
#else
			if (TaskHandle == NULL)
			{
				xTaskCreate(StaticTask, TaskName,
					TASK_CPP_STACK_DEPTH,
					this, Priority,
					&TaskHandle);				
			}
			else
			{
				//If have no task delete and we still have a TaskHandle, that means the task is locked in sleep.
			}
#endif			
			break;
		case TaskStateEnum::Suspended:
			TaskState = TaskStateEnum::Alive;
			OnResume();
			vTaskResume(TaskHandle);
			break;
		case TaskStateEnum::Alive:
		default:
			break;
		}
	}

	void Pause()
	{
		switch (TaskState)
		{
		case TaskStateEnum::Alive:
			TaskState = TaskStateEnum::Suspended;
			OnSuspend();
			vTaskSuspend(TaskHandle);
			break;
		case TaskStateEnum::Suspended:
		case TaskStateEnum::Dead:
		default:
			break;
		}
	}

	void AbortDelay()
	{
#if INCLUDE_xTaskAbortDelay
		xTaskAbortDelay(TaskHandle);
#endif
	}

	void Resume()
	{
		switch (TaskState)
		{
		case TaskStateEnum::Dead:
			Start();
		case TaskStateEnum::Suspended:
			TaskState = TaskStateEnum::Alive;
			OnResume();
			vTaskResume(TaskHandle);
			break;
		case TaskStateEnum::Alive:
		default:
			break;
		}
	}

	void Stop()
	{
		switch (TaskState)
		{
		case TaskStateEnum::Suspended:
			vTaskResume(TaskHandle);
		case TaskStateEnum::Alive:
			TaskState = TaskStateEnum::Dead;
			OnDestroy();
#if INCLUDE_vTaskDelete
			vTaskDelete(TaskHandle);
#else
			while (TaskHandle != NULL)
				vTaskDelay(portMAX_DELAY);
#endif
			break;
		case TaskStateEnum::Dead:
		default:
			break;
		}
	}

protected:
	virtual void OnStart() {}
	virtual void OnRun() {}
	virtual void OnResume() {}
	virtual void OnDestroy() {}
	virtual void OnSuspend() {}

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

