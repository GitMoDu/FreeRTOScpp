/*

*/

#include <Process.h>
#include <taskCPP.h>

class DebugProcess : public Process
{
public:
	DebugProcess(ProcPriority priority, uint32_t period)
		:Process(priority, period)
	{
	}

	DebugProcess(const char * const taskName, ProcPriority priority, uint32_t period)
		:Process(taskName, priority, period)
	{
	}

protected:

	virtual void setup()
	{
		PrintName(&Serial);
		Serial.println(": onSetup()");
	}
	virtual void cleanup()
	{
		PrintName(&Serial);
		Serial.println(": cleanup()");
	}
	virtual void onEnable()
	{
		PrintName(&Serial);
		Serial.println(": onEnable()");
	}
	virtual void onDisable()
	{
		PrintName(&Serial);
		Serial.println(": onDisable() ");
	}
	virtual void service()
	{
		PrintName(&Serial);
		Serial.println(": service() ");
	}
};

DebugProcess TestA = DebugProcess("Task A", MEDIUM_PRIORITY, 1000);
DebugProcess TestB = DebugProcess("Task B", LOW_PRIORITY, 2000);


void TestTask(void* pvParameters)
{
	bool Flip = true;
	uint32_t LastOne = millis();
	for (;;) {
		vTaskDelay(5000);
		if (Flip)
		{
			Serial.println("Disabling A");
			TestA.disable();
		}
		else
		{
			Serial.println("Enabling A");
			TestA.enable();
		}
		Flip = !Flip;

		if (millis() - LastOne > 7000)
		{
			if (TestB.isNotDestroyed())
			{
				Serial.println("Killing B");
				TestB.destroy();
			}
			else
			{
				Serial.println("Starting B");
				TestB.add();
			}
			LastOne = millis();
		}
	}
}

void setup()
{
	Serial.begin(115200);
	while (!Serial) {
		; // wait for serial port to connect. Needed for Leonardo only

	}

	delay(1000);

	Serial.println("Starting");
	TestB.add();
	TestA.add();

	xTaskCreate(TestTask,
		"TestTask",
		configMINIMAL_STACK_SIZE,
		NULL,
		tskIDLE_PRIORITY,
		NULL);

	vTaskStartScheduler();

}

//Never runs
void loop() {}
