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

For SPI in half-duplex communication, the reference manual recommands adding a 1k resistor between PA7 and SDA for safety. <br>
(*) = Active low pin

## Details
The communication between the MCU and the display is done through hardware SPI. <br>
In this case the SPI1 peripheral is used, in bi-directionnal mode (half-duplex communication). <br>
The peripheral clock is set to 64MHz and the SCK frequency is set to 250kHz (for easier troubleshooting), which is the lowest achievable with a 64MHz input clock. <br>
To improve data transmission speed, it is easily possible to change the data clock up to 32MHz (though I haven't tested the program at this speed) by modifying the `SPI_BR` bits in the `CR1` register. <br>
The data is sent in an RGB 6-6-6 format or 18 bits per pixel. <br>

Before writing data to the LCD controller RAM, one must tell the controller the boundaries of the image to be put, through the `RASET` and `CASET` registers. <br>
For example, if the goal is to put a 40x40 image starting at position (x,y)=(20, 20), we would write 40 and 60 to both registers.

Furthermore, the USART2 peripheral is also initialized to send debug infos at 57600 bauds. <br>

The transfer of the frame buffer from the MCU memory to the SPI peripheral can be also done by DMA, which helps unload the CPU. <br>

In the folder `./frame_gen`, there is a python script called `frame_gen.py` that can be used to convert an image to an array with RGB 6-6-6 format. The output is written to the folder `./app/data/`. <br>



## Useful documents:
[STM32L476 datasheet](https://www.st.com/resource/en/datasheet/stm32l476je.pdf) <br>
[STM32L4 series reference manual](https://www.st.com/resource/en/reference_manual/rm0351-stm32l47xxx-stm32l48xxx-stm32l49xxx-and-stm32l4axxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) <br>
[ST7735 datasheet](https://www.crystalfontz.com/controllers/Sitronix/ST7735/)