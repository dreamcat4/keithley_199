/* 
 * Copyright (C) September 26, 2014 Diego F. Asanza
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * File:   dmmTask.c
 * Author: Diego F. Asanza <f.asanza[at]gmail.com>
 *
 * Created on September 26, 2014, 11:35 PM
 */

//#include <math.h>
//#include <string.h>
//#include <stdio.h>
//#include <sys.h>
//#include <dispkyb.h>
//
//#include <tmp275.h>
//#include <sysstate.h>
//#include "disfmt.h"
//
//void dmmTaskMain(void* pvParameters)
//{
//    char buff[10];
//    tmp245_init();
//    sys_error err = sys_state_init();
//    /* The default configuration cannot be loaded. Other could work. The user
//     * can leave this state pressing a button. The button 1 is mem1, 2 mem2 etc.
//     * if the user press zero, default settings are loaded again. */
//    if(err != DMM_ERROR_NONE){
//        display_puts("MEM 0-9?");
//        switch(display_wait_for_key()){
//            case KEY_0: sys_state_restore_factory_settings(); break;
//        }
//    }
//    while(1)
//    {
//          hal_disp_adci_toggle();
//          double value = adc_read_value(0); /* read value in channel 0 */
//          double temp = tmp245_read_temp_double();
//          fmt_format_string(buff,sys_state_get_scale(),value);
//          fmt_append_scale(buff, sys_state_get_mode());
//          display_clear();
//          display_setmode(fmt_get_disp_mode(adc_get_input()));
//          display_puts(buff);
//          printf("%f,%f\n",value, temp);
//    }
//}
