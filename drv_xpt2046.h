/*
 * Copyright (c) 2021 Lee Chun Hei, Leslie
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __DRV_XPT2046_H__
#define __DRV_XPT2046_H__

#include <rtthread.h>
#include <rtdevice.h>

#include "../../../components/drivers/touch/touch.h"

/* cmd for Calibrate touch */
#define RT_TOUCH_CALIBRATION (RT_DEVICE_CTRL_BASE(Touch) + 20)

struct rt_xpt2046
{
    struct rt_touch_device parent;
    struct rt_spi_device *spi; 
    rt_uint16_t min_raw_x;
    rt_uint16_t min_raw_y;
    rt_uint16_t max_raw_x;
    rt_uint16_t max_raw_y;
};
typedef struct rt_xpt2046 *rt_xpt2046_t;

struct calibrate_args
{
    char *lcd_name;
    char *touch_name;
};


#endif

