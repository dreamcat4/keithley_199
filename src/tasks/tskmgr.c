/*
 * tskmgr.c
 *
 * Copyright (c) 2015, Diego F. Asanza. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301  USA
 *
 * Created on August 23, 2015, 7:30 PM
 */

#include "tskmgr.h"
#include <settings.h>
#include <system.h>
#include <FreeRTOS.h>
#include <task.h>
#include <diag.h>
#include <assert.h>
#include <dispkyb.h>
#include <queue.h>

#define SYSTEM_TASK_STACK_SIZE      200
#define TASK_STACK_SIZE             200
#define SYSTEM_TASK_PRIORITY        4
#define MESSAGE_DELAY 500
#define TASK_PRIORITY               3

typedef enum {
    TASK_MULTIMETER,
    TASK_RESISTANCE_4W,
    TASK_CALIBRATION,
    TASK_COUNT
} sys_task_t;


static void system_task(void* params);
extern void task_multimeter(void* params);
extern void task_resistance_4w(void* params);
extern void task_calibration(void* params);

static TaskHandle_t running_task = NULL;
static TaskHandle_t dmm_task;

static void sys_init(void);
static void load_settings();
static void start_task(sys_task_t task);
static void stop_running_task();
static void switch_sys_function();
static void poll_key(void);

void tskmgr_start(void)
{
    xTaskCreate(system_task, "SYS", SYSTEM_TASK_STACK_SIZE, NULL,
        SYSTEM_TASK_PRIORITY, NULL);
    xTaskCreate(task_multimeter, "MUL", TASK_STACK_SIZE, NULL, TASK_PRIORITY,
        &dmm_task);
    vTaskSuspend(dmm_task);
}

static void system_task(void* pvParameters)
{
    (void*) pvParameters;
    sys_init();
    // Reload Sysstate from eeprom.
    load_settings();
    switch_sys_function();
    while (1) {
        if (running_task) {
            vTaskDelay(10);
            continue;
        }
        poll_key();
    }
}

void taskmgr_delete(void)
{
    running_task = NULL;
    start_task(TASK_MULTIMETER);
    vTaskDelete(NULL);
}

static void start_task(sys_task_t task)
{
    system_get_lock();
    DIAG("restarting task: %d", task);
    switch (task) {
        case TASK_MULTIMETER: vTaskResume(dmm_task);
            break;
        case TASK_CALIBRATION:
            assert(running_task != NULL);
            xTaskCreate(task_calibration, "MUL", TASK_STACK_SIZE, NULL, TASK_PRIORITY,
                &running_task);
            break;
        default:
            assert(0);
    }
    system_release_lock();
    display_clear();
}

static void stop_running_task()
{
    system_get_lock();
    vTaskSuspend(dmm_task);
    system_release_lock();
}

static void sys_init(void)
{
    DIAG("Initializing System");
    display_kyb_init(); /* should be started after os starts */
    int i = 0;
    char* temp = REPOVERSION;
    while (temp[i] != 0) {
        display_putc(toupper(temp[i]), i++);
    }
    vTaskDelay(MESSAGE_DELAY / portTICK_PERIOD_MS); // show firmware version at startup.
    display_clear();
}

static void load_settings()
{
    if (settings_restore(SETTINGS_0)) {
        DIAG("Bad Settings on Store. Loading defaults");
        display_puts("SETT ERROR");
        vTaskDelay(MESSAGE_DELAY / portTICK_PERIOD_MS);
        display_clear();
    }

    switch (settings_get_integration_period()) {
        case ADC_INTEGRATION_50HZ: display_puts("FREQ=50 HZ");
            break;
        case ADC_INTEGRATION_60HZ: display_puts("FREQ=60 HZ");
            break;
        default: assert(0);
    }
    vTaskDelay(MESSAGE_DELAY / portTICK_PERIOD_MS);
    display_clear();
}

static void switch_sys_function()
{
    stop_running_task();
    /* get system lock */
    system_get_lock();
    display_clear();
    if (!calibration_restore()) {
        display_puts("CAL ERROR");
        vTaskDelay(MESSAGE_DELAY / portTICK_PERIOD_MS);
        display_clear();
    }

    if (system_set_configuration(settings_get_input(), settings_get_range(),
        settings_get_integration_period(), settings_get_channel(),
        calibration_gain(), calibration_offset())) {
        display_puts("NOT IMPL");
        vTaskDelay(MESSAGE_DELAY / portTICK_PERIOD_MS);
        display_clear();
    }
    system_release_lock();
    switch (settings_get_input()) {
        case ADC_INPUT_CURRENT_DC:
        case ADC_INPUT_CURRENT_AC:
        case ADC_INPUT_RESISTANCE_2W:
        case ADC_INPUT_VOLTAGE_DC:
        case ADC_INPUT_VOLTAGE_AC:
            start_task(TASK_MULTIMETER);
            break;
        case ADC_INPUT_RESISTANCE_4W:
            //start_task(TASK_RESISTANCE_4W);
            break;
        default:
            assert(0);
    }
}

bool shift_key = false;
bool repeat_key = true;
static void poll_key(void)
{
    key_id key = display_wait_for_key();
    DIAG("Key Pressed: %d", key);
    switch (key) {
        case KEY_0:break;
        case KEY_1:break;
        case KEY_2:break;
        case KEY_3:break;
        case KEY_4:
            if(repeat_key == false) break;
            shift_key = !shift_key;
            if (shift_key) {
                display_setmode(DISP_ZERO);
            } else
                display_clearmode(DISP_ZERO);
            repeat_key = false;
            break;
        case KEY_5:
            if (shift_key) {
                settings_set_input(ADC_INPUT_RESISTANCE_4W);
            } else {
                settings_set_input(ADC_INPUT_RESISTANCE_2W);
            }
            shift_key = false;
            switch_sys_function();
            break;
        case KEY_6:
            if (shift_key) {
                settings_set_input(ADC_INPUT_VOLTAGE_AC);
                shift_key = false;
            } else
                settings_set_input(ADC_INPUT_VOLTAGE_DC);
            switch_sys_function();
            break;
        case KEY_7:
            if (shift_key) {
                settings_set_input(ADC_INPUT_CURRENT_AC);
                shift_key = false;
            } else
                settings_set_input(ADC_INPUT_CURRENT_DC);
            switch_sys_function();
            break;
        case KEY_8:break;
        case KEY_9:break; //break;
        case KEY_UP:
            settings_range_up();
            switch_sys_function();
            break;
        case KEY_DOWN:
            settings_range_down();
            switch_sys_function();
            break;
        case KEY_CAL:
            stop_running_task();
            start_task(TASK_CALIBRATION);
            break;
        case KEY_NONE:
            repeat_key = true;
            break;
        default:
            Nop();
    }
}