#include <stdio.h>

#include "adc.h"
#include "hal_uart.h"
#include "hal_i2c.h"
#include "HardwareProfile.h"
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <sys.h>
#include <math.h>

static void sysMgmTask(void *pvParameters);
extern void dmmTaskMain(void* pvParameters);
TaskHandle_t measTaskHandle;
void _mon_putc(char c){
    hal_uart_send_byte(3,c);
}
int main()
{
    /* hardware initialization. An error here is unrecoverable */
    hal_sys_init();
    hal_uart_open(3,115200,HAL_UART_PARITY_NONE, HAL_UART_STOP_BITS_1);
    hal_i2c_init();
    while(1){
        hal_i2c_start(HAL_EEPROM_ADDRESS, HAL_I2C_WRITE);
        hal_i2c_write(0x00);
        hal_i2c_write(0x01);
        hal_i2c_write(0x55);
        hal_i2c_stop();
        hal_i2c_start_wait(HAL_EEPROM_ADDRESS, HAL_I2C_WRITE);
        hal_i2c_write(0x00);
        hal_i2c_write(0x01);
        hal_i2c_rep_start(HAL_EEPROM_ADDRESS, HAL_I2C_READ);
        char a = hal_i2c_read(0)
        hal_i2c_stop();
        if(a == 0x55)
            printf("youp!\n");
    }
    xTaskCreate(dmmTaskMain,"T1",configMINIMAL_STACK_SIZE,NULL,3,&measTaskHandle);
    /* suspend measurement task until we load dmm state on management task. */
    if(measTaskHandle)
        vTaskSuspend(measTaskHandle);
    /* system management task. */
    xTaskCreate(sysMgmTask,"T2",configMINIMAL_STACK_SIZE,NULL,3,NULL);
    //xTaskCreate(doCalTask,"T3",configMINIMAL_STACK_SIZE, NULL, 3, NULL);
    vTaskStartScheduler();
}

static void sysMgmTask(void *pvParameters){
    /* Initialize system state */
    hal_disp_init();
    dmm_error err = sys_dmm_init();
    /* The default configuration cannot be loaded. Other could work. The user
     * can leave this state pressing a button. The button 1 is mem1, 2 mem2 etc.
     * if the user press zero, default settings are loaded again. */
    if(err == DMM_ERR_EEPROM){
        hal_disp_puts("MEM 0-9?");
        switch(hal_disp_wait_for_key()){
            case KEY_VOLTS: sys_dmm_default_to_factory_settings(); break;
        }
    }
    bool shift_key = false;
    vTaskResume(measTaskHandle);

    while(1){
        switch(hal_disp_wait_for_key())
        {
            case KEY_RANGE_UP: sys_dmm_scale_up(); continue;
            case KEY_RANGE_DOWN: sys_dmm_scale_down(); continue;
            case KEY_AC: sys_dmm_toggle_acdc(); continue;
            case KEY_OHMS:  sys_dmm_set_mode(ADC_INPUT_RESISTANCE_2W); continue;
            case KEY_VOLTS: sys_dmm_restore_voltage_mode(); continue;
            case KEY_AMPS:  sys_dmm_restore_current_mode(); continue;
            case KEY_ZERO: continue;
            case KEY_LOCAL: continue;
            case KEY_AUTO: continue;
            case KEY_SCANNER: continue;
            case KEY_TRIGGER: continue;
            case KEY_NEXT: shift_key = true; continue;
            case KEY_CAL:
                vTaskSuspend(measTaskHandle);
                //vTaskStart(doCalTaskHandle);
                continue;
            default:
                Nop();
        }
        vTaskDelay(10);
    }
}

void vAssertCalled( const char* fileName, unsigned long lineNo )
{
    printf("%s, %d\n", fileName, lineNo);
    while(1);  
} 

void vApplicationIdleHook( void ){
    hal_sys_idle();
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask, signed char *pcTaskName )
{
    while(1);
}

void vApplicationMallocFailedHook()
{
    while(1);
}