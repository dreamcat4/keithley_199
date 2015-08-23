#include <stdio.h>
#include <diag.h>
#include <FreeRTOS.h>
#include <task.h>
#include <assert.h>
#include <system.h>
#include "tskmgr.h"

int main()
{
    /* hardware initialization. An error here is unrecoverable */
    system_init();
				/* start tasks as needed */
				tskmgr_start();
    DIAG("START: %s, %s", REPOVERSION, REPOBRANCH);
    DIAG("Starting Scheduler");
    vTaskStartScheduler();
}

void vAssertCalled(const char* fileName, unsigned long lineNo)
{
    printf("FreeRTOS: %s, %d\n", fileName, lineNo);
    while (1);
}

void vApplicationIdleHook(void)
{
    hal_sys_idle();
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, signed char *pcTaskName)
{
    assert(0);
    while (1);
}

void vApplicationMallocFailedHook()
{
    assert(0);
    while (1);
}