/*
 * disfmt.c
 *
 * Display formatter
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
 * Created on January 24, 2015, 12:10 AM
 */

#include "disfmt.h"
#include <assert.h>
#include "strutils.h"
#include <adcdefs.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <diag.h>

static double disfmt_get_range_value(double value, adc_range scale);
static double get_value(double value, adc_range scale);

void fmt_format_string(char* buff, int buffsize, adc_range scale, adc_resolution res,
    double value, system_flags_t flag)
{
    //TODO: show when filter out of window
    sprintf(buff, " -------");
    int ndigits = 0;
    switch (res) {
        case ADC_RESOLUTION_5_5: ndigits = 6;
            break;
        case ADC_RESOLUTION_6_5: ndigits = 7;
            break;
        default: assert(0);
    }
    switch (scale) {
        case ADC_RANGE_3M: value *= 1e-3;
        case ADC_RANGE_3K: value *= 1e-3;
        case ADC_RANGE_3:  value *= 1e+0;
        {
            if (flag & SYS_FLAG_OVERFLOW || flag & SYS_FLAG_UNDERFLOW) {
                sprintf(buff, " O.VERFL");
            } else {
                utils_dtofixstr(buff, ndigits, ndigits - 1, value);
            }
        }
            break;
        case ADC_RANGE_30M: value *= 1e-3;
        case ADC_RANGE_30K: value *= 1e-6;
        case ADC_RANGE_30m: value *= 1e+3;
        case ADC_RANGE_30:  value *= 1e+0;
        {
            if (flag & SYS_FLAG_OVERFLOW || flag & SYS_FLAG_UNDERFLOW) {
                sprintf(buff, " OV.ERFL");
            } else {
                utils_dtofixstr(buff, ndigits, ndigits - 2, value);
            }
        }
            break;
        case ADC_RANGE_300M: value *= 1e-3;
        case ADC_RANGE_300K: value *= 1e-6;
        case ADC_RANGE_300m: value *= 1e+3;
        case ADC_RANGE_300:  value *= 1e+0;
        {
            if (flag & SYS_FLAG_OVERFLOW || flag & SYS_FLAG_UNDERFLOW) {
                sprintf(buff, " OVE.RFL");
            } else {
                utils_dtofixstr(buff, ndigits, ndigits - 3, value);
            }
        }
            break;
        default: break;
    }
}

void fmt_append_scale(char* buffer, adc_input mode, adc_range range)
{
    switch (mode) {
        case ADC_INPUT_VOLTAGE_AC:
            *buffer = '~';
        case ADC_INPUT_VOLTAGE_DC:
        {
            if (range == ADC_RANGE_300m)
                strcat(buffer, "mV ");
            else
                strcat(buffer, "V ");
        }
            break;
        case ADC_INPUT_CURRENT_AC:
            *buffer = '~';
        case ADC_INPUT_CURRENT_DC:
        {
            if (range == ADC_RANGE_30m)
                strcat(buffer, "mA ");
            else
                strcat(buffer, "A ");
        }
            break;
        case ADC_INPUT_RESISTANCE_2W:
        case ADC_INPUT_RESISTANCE_4W:
        {
            switch (range) {
                case ADC_RANGE_300: strcat(buffer, "o ");
                    break;
                case ADC_RANGE_3K:
                case ADC_RANGE_30K:
                case ADC_RANGE_300K: strcat(buffer, "Ko ");
                    break;
                case ADC_RANGE_3M:
                case ADC_RANGE_30M:
                case ADC_RANGE_300M: strcat(buffer, "Mo ");
                    break;
                default:
                    break;
            }
        }
            break;
        case ADC_INPUT_TEMP:
            strcat(buffer, "gC");
            break;
        default:
            break;

    }
}

static void fmt_write_display(char* buffer, int size, adc_range range, adc_input mode,
    adc_resolution res, double value)
{
    fmt_format_string(buffer, NUMBER_OF_CHARACTERS, range, res, disfmt_get_range_value(value, range),0);
    fmt_append_scale(buffer, mode, range);
    display_puts(buffer);
}

static key_id fmt_get_key()
{
    key_id val = display_wait_for_key();
    if (val == KEY_NONE) val = display_wait_for_key();
    display_wait_for_key();
    return val;
}

double fmt_get_refval(double val, adc_input mode, adc_range range, adc_resolution res)
{
    double sign;
    if (val >= 0) sign = 1;
    else sign = -1;
    key_id key = KEY_NONE;
    int hld_i = 2;
    display_evt_clear();
    char buffer[NUMBER_OF_CHARACTERS];
    do {
        if (key != KEY_NONE) {
            if (hld_i == 3) {
                if (key < 3) {
                    val = sign*key;
                } else {
                    if (key == 3) val = sign * key;
                    display_highlight(hld_i--);
                }
            } else {
                if (key <= 9) {
                    int i;
                    double dec = 1;
                    for (i = 4; i <= hld_i; i++) {
                        dec = dec * 0.1;
                    }
                    val += sign * key*dec;
                }
            }
        }
        fmt_write_display(buffer, NUMBER_OF_CHARACTERS, range, mode, res, val);
        if (hld_i > 7) hld_i = 2;
        display_highlight(hld_i++);
        key = fmt_get_key();
    } while (key != KEY_CAL);
    display_highlight(0);
    buffer[0] = ' ';
    val = sign * utils_strtod(buffer);
    return get_value(val, range);
}

static double get_value(double value, adc_range scale)
{
    switch (scale) {
        case ADC_RANGE_30m:  return value*1e-3;
        case ADC_RANGE_300m: return value*1e-3;
        case ADC_RANGE_3:    return value*1e+0;
        case ADC_RANGE_30:   return value*1e+0;
        case ADC_RANGE_300:  return value*1e+0;
        case ADC_RANGE_3K:   return value*1e+3;
        case ADC_RANGE_30K:  return value*1e+3;
        case ADC_RANGE_300K: return value*1e+3;
        case ADC_RANGE_3M:   return value*1e+6;
        case ADC_RANGE_30M:  return value*1e+6;
        case ADC_RANGE_300M: return value*1e+6;
        default: break;
    }
    return 0;
}

static double disfmt_get_range_value(double value, adc_range scale)
{
    switch (scale) {
        case ADC_RANGE_30m:  return value*1e-2;
        case ADC_RANGE_300m: return value*1e-1;
        case ADC_RANGE_3:    return value*1e+0;
        case ADC_RANGE_30:   return value*1e+1;
        case ADC_RANGE_300:  return value*1e+2;
        case ADC_RANGE_3K:   return value*1e+3;
        case ADC_RANGE_30K:  return value*1e+4;
        case ADC_RANGE_300K: return value*1e+5;
        case ADC_RANGE_3M:   return value*1e+6;
        case ADC_RANGE_30M:  return value*1e+7;
        case ADC_RANGE_300M: return value*1e+8;
        default: break;
    }
    return 0;
}