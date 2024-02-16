# stm32_st7735_demo
## Overview
This project serves as a demo program to interface an ST7735 based LCD with a STM32L476 microcontroller. <br>
It is done so without the use of ST's HAL libraries, only CMSIS files. <br>
It also uses a lightweight version of `printf`, whose source code you can find [here](https://www.menie.org/georges/embedded/small_printf_source_code.html) <br>
This program only provides a simple interface : it is up to you to write drawing functions or any other higher-level stuff.

## Pin description
| MCU pin / Function | Display pin / Function |
|--------------------|------------------------|
|PA5 / SPI SCK       | SCL / Serial clock     |
|PA7 / SPI MOSI      | SDA / Serial data      |
|PA4 / SPI CS        | *CS / Chip select      |
|PA9 / GPIO          | DC / Data-command      |
|PA10 / GPIO         | *RST / Reset           |
|PA11 / GPIO         | BLK / Backlight        |

For SPI in half-duplex communication, the reference manual recommands adding a 1k resistor between PA7 and SDA for safety.

## Details
The communication between the MCU and the display is done through hardware SPI. <br>
In this case the SPI1 peripheral is used. <br>
The peripheral clock is set to 64MHz and the SCK frequency is set to 250kHz (for easier troubleshooting), which is the lowest achievable with a 64MHz input clock. <br>
To improve data transmission speed, it is easily possible to change the baud rate up to 32MHz (though I haven't tested the program at this speed) by modifying the `SPI_BR` bits in the `CR1` register. <br>

Furthermore, the USART2 peripheral is also initialized to send debug infos at 57600 bauds.