# XPT2046 Touch Driver

English

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
        [*] touch drivers --->
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
Burn the program to the target development board, and the user can use the touch driver protocol to read the touch point:

## 4. Example
```.c
#include <rtdevice.h>
#include "drv_xpt2046.h"

int main(void)
{
    //Find the touch device
    rt_device_t touch = rt_device_find("xpt0");

    if (touch == RT_NULL)
    {
        rt_kprintf("can't find device:%s\n", "xpt0");
        while (1);
    }
    if (rt_device_open(touch, RT_DEVICE_FLAG_INT_RX) != RT_EOK)
    {
        rt_kprintf("open device failed!");
        while (1);
    }
    while (1)
    {
        //Prepare variable to read out the touch data
        struct rt_touch_data read_data;
        rt_memset(&read_data, 0, sizeof(struct rt_touch_data));
        if (rt_device_read(touch, 0, &read_data, 1) == 1)
        {
            //Print the touch coordinate and the corresponding information
            rt_kprintf("%d %d %d %d %d\n",
                        read_data.event,
                        read_data.x_coordinate,
                        read_data.y_coordinate,
                        read_data.timestamp,
                        read_data.width);
        }
        rt_thread_mdelay(10);
    }

    while (1)
    {
        rt_thread_mdelay(100);
    }
}
```
