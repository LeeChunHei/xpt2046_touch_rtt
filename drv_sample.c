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

#include <rtdevice.h>
#include "drv_xpt2046.h"
#include "drv_soft_spi.h"

#ifdef PKG_XPT2046_USING_SAMPLE

#define TOUCH_DEVICE_NAME "xpt0"
#define TFTLCD_DEVICE_NAME "lcd"

void xpt2046_entry(void *parameter)
{
    /* Find the touch device */
    rt_device_t touch = rt_device_find(TOUCH_DEVICE_NAME);
    if (touch == RT_NULL)
    {
        rt_kprintf("can't find touch device:%s\n", TOUCH_DEVICE_NAME);
        return;
    }
    if (rt_device_open(touch, RT_DEVICE_FLAG_INT_RX) != RT_EOK)
    {
        rt_kprintf("can't open touch device:%s\n", TOUCH_DEVICE_NAME);
        return;
    }
    /* Mount the spi device to the spi bus, here you need to call
    the corresponding api according to the board you are using */
    rt_uint8_t dev_num = 0;
    char dev_name[RT_NAME_MAX];
    do
    {
        rt_sprintf(dev_name, "%s%d", PKG_XPT2046_SPI_BUS, dev_num++);
        if (dev_num == 255)
        {
            return;
        }
    } while (rt_device_find(dev_name));
    rt_hw_soft_spi_device_attach(PKG_XPT2046_SPI_BUS, dev_name, PKG_XPT2046_CS_PIN);

    /* configure spi device */
    rt_xpt2046_t tc = (rt_xpt2046_t)touch;
    tc->spi = (struct rt_spi_device *)rt_device_find(dev_name);
    struct rt_spi_configuration spi_config;
    spi_config.data_width = 8;
    spi_config.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    /* Max freq of XPT2046 is 2MHz */
    spi_config.max_hz = 2000000;
    rt_spi_configure(tc->spi, &spi_config);
	
	/* Calibrate touch */
    struct calibrate_args cali_args;
    cali_args.lcd_name = TFTLCD_DEVICE_NAME;
    cali_args.touch_name = TOUCH_DEVICE_NAME;
    if (rt_device_control(touch, RT_TOUCH_CALIBRATION, (void *)&cali_args) != RT_EOK)
    {
        rt_kprintf("xpt2046 calibration failed!\n");
        return;
    }

    /* Find the TFTLCD device */
    rt_device_t lcd = rt_device_find(TFTLCD_DEVICE_NAME);
    if (lcd == RT_NULL)
    {
        rt_kprintf("can't find display device:%s\n", TFTLCD_DEVICE_NAME);
        return;
    }
    if (rt_device_open(lcd, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("can't open display device:%s\n", TFTLCD_DEVICE_NAME);
        return;
    }
    while (1)
    {
        /* Prepare variable to read out the touch data */
        struct rt_touch_data read_data;
        rt_memset(&read_data, 0, sizeof(struct rt_touch_data));
        if (rt_device_read(touch, 0, &read_data, 1) == 1)
        {
            const rt_uint32_t black = 0x0;
            rt_graphix_ops(lcd)->set_pixel((const char *)(&black),
                                           read_data.x_coordinate,
                                           read_data.y_coordinate);
        }
        rt_thread_mdelay(1);
    }
}

static int xpt2046_sample(void)
{
    rt_thread_t tid = rt_thread_create("xpt2046", xpt2046_entry, RT_NULL, 1024, 8, 20);
    RT_ASSERT(tid != RT_NULL);
    rt_thread_startup(tid);
    return RT_EOK;
}
INIT_APP_EXPORT(xpt2046_sample);

#endif
