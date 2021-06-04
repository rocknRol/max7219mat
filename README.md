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

![](https://raw.githubusercontent.com/rocknRol/max7219mat/master/img/matrix2.png)

Unfortunately this product is made to work like a simple scrolling text viewer. 
Technically it works like a big scrolling register where the bits go through from one chip Maxim 7219 to the other adjacent.
On Internet you can find thousands projects to do this with Arduino or Raspberry using dedicated libraries.

However to form a decent display I used four lines, but to drive every display 8x8 individually I have decided to hack them.
The modification consists in interrupting the connection Data I/O and chip selection CS signal between the various Maxim 7219 chips.

![](https://raw.githubusercontent.com/rocknRol/max7219mat/master/img/matrix3.png)

So I assembled everything and connected the cables:

![](https://raw.githubusercontent.com/rocknRol/max7219mat/master/img/matrix4.png)

![](https://raw.githubusercontent.com/rocknRol/max7219mat/master/img/matrix5.png)

![](https://raw.githubusercontent.com/rocknRol/max7219mat/master/img/matrix6.png)

Now to connect the display to the Raspberry PI I designed an interface with the following  two requirements:

- To implement a mechanism to select the correct module 8x8 to send the data
- To interface in safe the level signals of Raspberry PI at 3.3V with the 5V level of display

I used the MC74HC154N a 4-line to 16-line decoder/demultiplexer to send data at correct module 8x8 . Like the MAXIM7219 also for this chip the supply voltage is 5V because it's from the TTL family.
The voltage level conversion is build around at Mosfet 2N7000 and it is also bi-directional, but here is used only from 3.3V to 5V


