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

#ifdef PKG_USING_XPT2046

#include <drv_gpio.h>

#define LOG_TAG             "drv.xpt2046"
#include <drv_log.h>

#ifdef PKG_ST7735R_USING_KCONFIG
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

#endif