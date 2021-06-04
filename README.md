# Matrix display 32x32 pixel based on Maxim 7219 chip controller and Linux platform driver for Raspberry PI


This project was born with an educational intent, because I have used it to learn how to write an platform driver with DTS for Linux embedded system.
In the follow we will see how to use Raspberry PI 3B to drive a 32x32 led matrix display. 
We will starting from design electronic circuit until to develop animations in Python...

"max7219mat" is composed by: the electronic board, a platform device driver, a minimal library in C language and a Cython wrapper to making callable the C library from Python code.

Now I describe first the electronic board and after the device driver for Linux.

## Electronic board and display

The display board is made up of 16 modules types 8x8 matrix led. 
Each module is managed of Maxim 7219 chip.

This is a simple module with which I have started my experiments:

![](https://raw.githubusercontent.com/rocknRol/max7219mat/master/img/matrix1.png)

But to build a bigger display I have decided to use a line with four modules like this:


