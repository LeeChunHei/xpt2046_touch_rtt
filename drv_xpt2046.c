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

#include <drv_xpt2046.h>

#ifdef PKG_USING_XPT2046_TOUCH

#include <drv_gpio.h>

#define LOG_TAG             "drv.xpt2046"
#include <drv_log.h>

#ifdef PKG_XPT2046_USING_KCONFIG
    #define PKG_XPT2046_CS      GET_PIN(PKG_XPT2046_CS_GPIO, PKG_XPT2046_CS_PIN)
    #define PKG_XPT2046_IRQ     GET_PIN(PKG_XPT2046_IRQ_GPIO, PKG_XPT2046_IRQ_PIN)
#endif

static const rt_uint8_t xpt2046_tx_data[21] = {0xD0, 0, 0xD0, 0, 0xD0, 0, 0xD0, 0, 0xD0, 0, 0x90, 0, 0x90, 0, 0x90, 0, 0x90, 0, 0x90, 0, 0};

static rt_size_t xpt2046_touch_readpoint(struct rt_touch_device *touch, void *buf, rt_size_t touch_num)
{
    if (touch_num != 0)
    {
        struct rt_touch_data *result = (rt_uint16_t *)buf;
        rt_xpt2046_t dev = (rt_xpt2046_t)touch;
#ifdef RT_TOUCH_PIN_IRQ
        if (rt_pin_read(dev->parent.config.irq_pin.pin))
        {
            result->event = RT_TOUCH_EVENT_NONE;
            return 0;
        }
#endif
        rt_uint8_t rx_data[21];
        rt_spi_transfer(dev->spi, xpt2046_tx_data, rx_data, 21);
        rt_uint8_t x_count = 0;
        rt_uint8_t y_count = 0;
        rt_uint32_t x_cum = 0;
        rt_uint32_t y_cum = 0;
        for (rt_uint8_t i = 1; i < 11; i += 2)
        {
            rt_uint16_t temp = (rx_data[i] << 8 | rx_data[i + 1]) >> 4;
            // rt_kprintf("x: %x ", temp);
            if (temp >= dev->min_raw_x && temp <= dev->max_raw_x)
            {
                ++x_count;
                x_cum += temp;
            }
            temp = (rx_data[i + 10] << 8 | rx_data[i + 11]) >> 4;
            // rt_kprintf("y: %x\r\n", temp);
            if (temp >= dev->min_raw_y && temp <= dev->max_raw_y)
            {
                ++y_count;
                y_cum += temp;
            }
        }
        if (!x_count || !y_count)
        {
            result->event = RT_TOUCH_EVENT_NONE;
            return 0;
        }
        result->event = RT_TOUCH_EVENT_DOWN;
        result->x_coordinate = ((float)x_cum / x_count - dev->min_raw_x) / (dev->max_raw_x - dev->min_raw_x) * dev->parent.info.range_x;
        result->y_coordinate = ((float)y_cum / y_count - dev->min_raw_y) / (dev->max_raw_y - dev->min_raw_y) * dev->parent.info.range_y;
        return touch_num;
    }
    else
    {
        return 0;
    }
}

static rt_err_t xpt2046_touch_control(struct rt_touch_device *touch, int cmd, void *arg)
{

}

static struct rt_touch_ops xpt2046_ops =
{
    .touch_readpoint = xpt2046_touch_readpoint,
    .touch_control = xpt2046_touch_control,
};

rt_xpt2046_t xpt2046_user_init(char *spi_bus_name, rt_base_t cs_pin, rt_base_t irq_pin,
                                rt_int32_t range_x, rt_int32_t range_y,
                                rt_uint16_t min_raw_x, rt_uint16_t min_raw_y,
                                rt_uint16_t max_raw_x, rt_uint16_t max_raw_y)
{
	rt_uint8_t dev_num = 0;
	char dev_name[RT_NAME_MAX];
	do
	{
		rt_sprintf(dev_name, "%s%d", spi_bus_name, dev_num++);
		if (dev_num == 255)
		{
			return RT_NULL;
		}
	} while (rt_device_find(dev_name));
	
	rt_hw_spi_device_attach(spi_bus_name, dev_name, cs_pin);

    rt_xpt2046_t dev_obj = rt_malloc(sizeof(struct rt_xpt2046));
	if (dev_obj)
	{
		rt_memset(dev_obj, 0x0, sizeof(struct rt_xpt2046));
        dev_obj->min_raw_x = min_raw_x;
        dev_obj->min_raw_y = min_raw_y;
        dev_obj->max_raw_x = max_raw_x;
        dev_obj->max_raw_y = max_raw_y;
		dev_obj->spi = rt_device_find(dev_name);
        struct rt_spi_configuration spi_config;
        spi_config.data_width = 8;
        spi_config.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
        /* Max freq of XPT2046 is 2MHz */
        spi_config.max_hz = 2000000;
        rt_spi_configure(dev_obj->spi, &spi_config);
        dev_obj->parent.info.type = RT_TOUCH_TYPE_RESISTANCE;
        dev_obj->parent.info.vendor = RT_TOUCH_VENDOR_UNKNOWN;
        dev_obj->parent.info.point_num = 1;
        dev_obj->parent.info.range_x = range_x;
        dev_obj->parent.info.range_y = range_y;
#ifdef RT_TOUCH_PIN_IRQ
        dev_obj->parent.config.irq_pin.pin = irq_pin;
        dev_obj->parent.config.irq_pin.mode = PIN_MODE_INPUT_PULLUP;
#endif
        dev_obj->parent.ops = &xpt2046_ops;
		dev_num = 0;
		do
		{
			rt_sprintf(dev_name, "xpt%d", dev_num++);
			if (dev_num == 255)
			{
				rt_device_destroy(&(dev_obj->parent));
				return RT_NULL;
			}
		} while (rt_device_find(dev_name));
        rt_hw_touch_register(&(dev_obj->parent), dev_name, RT_DEVICE_FLAG_INT_RX, RT_NULL);
		return (rt_xpt2046_t)rt_device_find(dev_name);
	}
	else
	{
		return RT_NULL;
	}
}

#ifdef PKG_XPT2046_USING_KCONFIG
static int xpt2046_hw_init(void)
{
	if (RT_NULL == xpt2046_user_init(PKG_XPT2046_SPI_BUS, PKG_XPT2046_CS, PKG_XPT2046_IRQ,
                                        PKG_XPT2046_RANGE_X, PKG_XPT2046_RANGE_Y,
                                        PKG_XPT2046_MIN_RAW_X, PKG_XPT2046_MIN_RAW_Y,
                                        PKG_XPT2046_MAX_RAW_X, PKG_XPT2046_MAX_RAW_Y))
	{
		return -RT_ERROR;
	}
    return RT_EOK;
}
INIT_DEVICE_EXPORT(xpt2046_hw_init);
#endif

void xpt2046_calibration(void)
{
    /* Find the TFT LCD device */
    const char* lcd_name = "lcd0";
    const char* touch_name = "xpt0";
    rt_device_t lcd = rt_device_find(lcd_name);
    if (lcd == RT_NULL)
    {
        LOG_E(LOG_TAG" cannot find lcd device named %s\n", lcd_name);
        return;
    }
    if (rt_device_open(lcd, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        LOG_E(LOG_TAG" cannot open lcd device named %s\n", lcd_name);
        return;
    }

    rt_xpt2046_t touch = (rt_xpt2046_t)rt_device_find(touch_name);
    if (touch == RT_NULL)
    {
        LOG_E(LOG_TAG" cannot find touch device named %s\n", touch_name);
        return;
    }
    if (rt_device_open(touch, RT_DEVICE_FLAG_INT_RX) != RT_EOK)
    {
        LOG_E(LOG_TAG" cannot open touch device named %s\n", touch_name);
        return;
    }
    struct rt_device_graphic_info lcd_info;
    rt_device_control(lcd, RTGRAPHIC_CTRL_GET_INFO, &lcd_info);
    for (rt_uint32_t y = 0; y < lcd_info.height; ++y)
    {
        const uint32_t white = 0xFFFFFFFF;
        rt_graphix_ops(lcd)->draw_hline((const char *)(&white), 0, lcd_info.width, y);
    }
    rt_uint32_t cross_size = (lcd_info.width > lcd_info.height ? lcd_info.height : lcd_info.width) / 10;
    rt_uint32_t x0 = cross_size;
    rt_uint32_t y0 = cross_size;
    rt_uint32_t x1 = lcd_info.width - cross_size;
    rt_uint32_t y1 = cross_size;
    rt_uint32_t x2 = lcd_info.width - cross_size;
    rt_uint32_t y2 = lcd_info.height - cross_size;
    rt_uint32_t x3 = cross_size;
    rt_uint32_t y3 = lcd_info.height - cross_size;
    const rt_uint32_t black = 0x0;
    // Upper left cross
    rt_graphix_ops(lcd)->draw_hline((const char *)(&black), 0, x0+cross_size, y0);
    rt_graphix_ops(lcd)->draw_vline((const char *)(&black), x0, 0, y0+cross_size);
    // Upper right cross
    rt_graphix_ops(lcd)->draw_hline((const char *)(&black), x1-cross_size, lcd_info.width, y1);
    rt_graphix_ops(lcd)->draw_vline((const char *)(&black), x1, 0, y1+cross_size);
    // lower right cross
    rt_graphix_ops(lcd)->draw_hline((const char *)(&black), x2-cross_size, lcd_info.width, y2);
    rt_graphix_ops(lcd)->draw_vline((const char *)(&black), x2, y2-cross_size, lcd_info.height);
    // lower left cross
    rt_graphix_ops(lcd)->draw_hline((const char *)(&black), 0, x3+cross_size, y3);
    rt_graphix_ops(lcd)->draw_vline((const char *)(&black), x3, y3-cross_size, lcd_info.height);

    touch->min_raw_x = 0;
    touch->min_raw_y = 0;
    touch->max_raw_x = 4096;
    touch->max_raw_y = 4096;
    touch->parent.info.range_x = 4096;
    touch->parent.info.range_y = 4096;

    rt_uint16_t x_raw[4];
    rt_uint16_t y_raw[4];
    rt_uint8_t raw_idx = 0;
    rt_memset(&x_raw, 0, sizeof(rt_uint32_t)*4);
    rt_memset(&y_raw, 0, sizeof(rt_uint32_t)*4);
    while (1)
    {
        struct rt_touch_data read_data;
        rt_memset(&read_data, 0, sizeof(struct rt_touch_data));
        if (rt_device_read(touch, 0, &read_data, 1) == 1)
        {
            rt_int32_t x_diff = (rt_in32_t)read_data.x_coordinate - (rt_in32_t)x_raw[raw_idx];
            rt_int32_t y_diff = (rt_in32_t)read_data.y_coordinate - (rt_in32_t)y_raw[raw_idx];
            if (abs(x_diff) > 10 || abs(y_diff) > 10)
            {
                ++raw_idx;
            }
            if (raw_idx >= 4)
            {
                break;
            }
            x_raw[raw_idx] = read_data.x_coordinate;
            y_raw[raw_idx] = read_data.y_coordinate;
            rt_thread_mdelay(100);
        }
        rt_thread_mdelay(10);
    }
    rt_uint32_t min_x = (x_raw[0]+x_raw[3])/2;
    rt_uint32_t max_x = (x_raw[1]+x_raw[2])/2;
    rt_uint32_t min_y = (y_raw[0]+y_raw[1])/2;
    rt_uint32_t max_y = (y_raw[2]+y_raw[3])/2;

    rt_uint32_t x_raw_cnt_per_pixel = (max_x-min_x) / (x1-x0);
    rt_uint32_t y_raw_cnt_per_pixel = (max_y-min_y) / (y2-y1);

    min_x -= cross_size * x_raw_cnt_per_pixel;
    max_x += cross_size * x_raw_cnt_per_pixel;
    min_y -= cross_size * y_raw_cnt_per_pixel;
    max_y += cross_size * y_raw_cnt_per_pixel;

    touch->min_raw_x = min_x;
    touch->min_raw_y = min_y;
    touch->max_raw_x = max_x;
    touch->max_raw_y = max_y;
    touch->parent.info.range_x = lcd_info.width;
    touch->parent.info.range_y = lcd_info.height;

    LOG_I(LOG_TAG" Calibration result, min_x:%d, min_y:%d, max_x:%d, max_y:%d", min_x, min_y, max_x, max_y);

    rt_device_close(lcd);
    rt_device_close(touch);
}
MSH_CMD_EXPORT(xpt2046_calibration, xpt2046 calibration)

#endif