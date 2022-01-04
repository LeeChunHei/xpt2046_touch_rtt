# ST7735R TFT LCD Driver

[Chinese](README_ZH.md) | English

## 1. Introduction

This software package is a driver for ST7735R TFT LCD. This driver provide access through IO device model.

### 1.1 License

ST7735R TFT LCD driver package is licensed with MIT license.

### 1.2 Dependency

- RT-Thread 4.0+
- SPI device driver
- PIN device driver

## 2. How to use ST7735R TFT LCD Driver

To use ST7735R TFT LCD driver package, you need to select it in the package manager of RT-Thread. The specific path is as follows:

```
RT-Thread online packages
    peripheral libraries and drivers --->
        [*] st7735r tft lcd driver package --->
            [*] Setup st7735r tft in menuconfig --->
                (spi0)  SPI bus connected to the tft lcd
                ()      GPIO port number for the chip select pin
                ()      GPIO pin number for the chip select pin
                ()      GPIO port number for the backlight pin
                ()      GPIO pin number for the backlight pin
                ()      GPIO port number for the dc pin
                ()      GPIO pin number for the dc pin
                ()      GPIO port number for the reset pin
                ()      GPIO pin number for the reset pin
                (128)   Width of the tft lcd
                (160)   Height of the tft lcd
```

The detailed description of the package options is as follows:

| Option | Description |
|-|-|
| Setup st7735r tft in menuconfig | Whether the ST7735R LCD device initalized when rt-thread boot up |
| SPI bus connected to the tft lcd | The SPI bus name used to connect to the TFT LCD |
| GPIO port number for the chip select pin | The cs pin is (pin number) of pin in the (port number) of GPIO port |
| GPIO pin number for the chip select pin | The cs pin is (pin number) of pin in the (port number) of GPIO port |
| GPIO port number for the backlight pin | The bl pin is (pin number) of pin in the (port number) of GPIO port |
| GPIO pin number for the backlight pin | The bl pin is (pin number) of pin in the (port number) of GPIO port |
| GPIO port number for the dc pin | The dc pin is (pin number) of pin in the (port number) of GPIO port |
| GPIO pin number for the dc pin | The dc pin is (pin number) of pin in the (port number) of GPIO port |
| GPIO port number for the reset pin | The res pin is (pin number) of pin in the (port number) of GPIO port |
| GPIO pin number for the reset pin | The res pin is (pin number) of pin in the (port number) of GPIO port |
| Width of the tft lcd | Width of the TFT LCD, it should be 128 |
| Height of the tft lcd | Height of the TFT LCD, it should be 160 |

After selecting the options you need, use RT-Thread's package manager to automatically update, or use the `pkgs --update` command to update the package to the BSP.

## 3. Use ST7735R TFT LCD Driver

After opening the ST7735R TFT LCD driver package and selecting the corresponding function option, it will be added to the BSP project for compilation when the BSP is compiled.
Burn the program to the target development board, and the user can use the following method to control the display of the TFT LCD:

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