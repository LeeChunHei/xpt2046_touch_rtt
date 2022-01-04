# XPT2046 Touch Driver

[Chinese](README_ZH.md) | English

## 1. Introduction

This software package is a driver for XPT2046 Touch LCD. This driver provide access through IO device model.

### 1.1 License

XPT2046 Touch driver package is licensed with MIT license.

### 1.2 Dependency

- RT-Thread 4.0+
- SPI device driver
- PIN device driver

## 2. How to use XPT2046 Touch Driver

To use XPT2046 Touch driver package, you need to select it in the package manager of RT-Thread. The specific path is as follows:

```
RT-Thread online packages
    peripheral libraries and drivers --->
        [*] xpt2046 touch driver package --->
            [*] Setup xpt2046 touch in menuconfig --->
                (spi0)  SPI bus connected to the xpt2046
                ()      GPIO port number for the chip select pin
                ()      GPIO pin number for the chip select pin
                ()      GPIO port number for the pen pin
                ()      GPIO pin number for the pen pin
                (128)   Width of the touch lcd
                (160)   Height of the touch lcd
                (181)   Minimum raw x axis value
                (189)   Minimum raw y axis value
                (1871)  Maximum raw x axis value
                (2048)  Maximum raw y axis value
```

The detailed description of the package options is as follows:

| Option | Description |
|-|-|
| Setup xpt2046 touch in menuconfig | Whether the xpt2046 touch device initalized when rt-thread boot up |
| SPI bus connected to the xpt2046 | The SPI bus name used to connect to the xpt2046 |
| GPIO port number for the chip select pin | The cs pin is (pin number) of pin in the (port number) of GPIO port |
| GPIO pin number for the chip select pin | The cs pin is (pin number) of pin in the (port number) of GPIO port |
| GPIO port number for the pen pin | The pen pin is (pin number) of pin in the (port number) of GPIO port |
| GPIO pin number for the pen pin | The pen pin is (pin number) of pin in the (port number) of GPIO port |
| Width of the touch lcd | Width of the touch lcd, it should be the lcd width pixel count |
| Height of the touch lcd | Height of the touch lcd, it should be the lcd height pixel count |
| Minimum raw x axis value | |
| Minimum raw y axis value | |
| Maximum raw x axis value | |
| Maximum raw y axis value | |

After selecting the options you need, use RT-Thread's package manager to automatically update, or use the `pkgs --update` command to update the package to the BSP.

## 3. Use XPT2046 Touch Driver

After opening the XPT2046 Touch driver package and selecting the corresponding function option, it will be added to the BSP project for compilation when the BSP is compiled.
Burn the program to the target development board, and the user can use the following method to read the touch driver to read the touch point:

| Function | Parameter | Action |
|---|---|---|
| `rt_err_t rt_device_control(rt_device_t dev, int cmd, void *arg)` | cmd: RT_ST7735R_SET_RECT, arg: rect | Set the active rect on the TFT LCD, any write action after that will fill inside that region |
| `rt_size_t rt_device_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t   size)` | pos: RT_ST7735R_WRITE_COLOR_PIXEL or RT_ST7735R_WRITE_GRAYSCALE_PIXEL | Fill the TFT LCD rect region with buffer's pixel data, one byte per pixel in grayscale pixel mode and two byte per pixel(rgb565) in color pixel mode |

## 4. Example
```
#include <rtdevice.h>
#include "drv_st7735r.h"

int main(void)
{
    /* Find the TFT LCD device */
    rt_device_t lcd = rt_device_find("lcd0");
    rt_device_open(lcd, 0);

    for (rt_uint8_t y = 0; y < 120; ++y)
    {
        /* Set the TFT LCD rect region */
        struct rt_st7735r_rect rect =
        {
            .x = 0,
            .y = y,
            .width = 120,
            .height = 1,
        };
        rt_device_control(lcd, RT_ST7735R_SET_RECT, &rect);
        /* Fill grayscale pxiels */
        rt_uint8_t pixels[120];
        rt_memset(pixels, y, 120);
        rt_device_write(lcd, RT_ST7735R_WRITE_GRAYSCALE_PIXEL, pixels, 120);
    }

    while (1)
    {
        rt_thread_mdelay(100);
    }
}
```