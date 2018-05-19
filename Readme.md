# Bare Metal STM32F103

This repo contains code example that shows how to use UART and timer peripherals without any libraries and headers on stm32f103c8t6.

Initially based on [This stackoverflow answer](https://stackoverflow.com/questions/43059223/programming-nvic-on-stm32-without-libraries/43060033#43060033), slightly modified for stm32f103 MCU using UART2 and TIM2.
`nvic_enable` function code is taken from [this github repo with project examples](https://github.com/trebisky/stm32f103).

[How to set up and run openocd on linux](https://github.com/rogerclarkmelbourne/Arduino_STM32/wiki/Programming-an-STM32F103XXX-with-a-generic-%22ST-Link-V2%22-programmer-from-Linux).

UART2 (PA2:TX, PA3:RX) is configured with speed 115200, TIM2 is configured to trigger the interrupt every second on 8 MHz clock frequency.
