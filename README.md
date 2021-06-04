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

![](https://raw.githubusercontent.com/rocknRol/max7219mat/master/img/sch1.png)

Below is the interface "Raspberry PI to Display" circuit diagram that you can see better in KiCad file available in this repository

![](https://raw.githubusercontent.com/rocknRol/max7219mat/master/img/sch2.png)

In the pictures below You can see the prototype on breadboards:

![](https://raw.githubusercontent.com/rocknRol/max7219mat/master/img/prt1.png)

and finally the display is assembled on prototype PCB board. I have left a little free space for future developments.


![](https://raw.githubusercontent.com/rocknRol/max7219mat/master/img/prt3.png)


![](https://raw.githubusercontent.com/rocknRol/max7219mat/master/img/prt2.png)


## Platform driver and DTS

Our hardware is directly connected at the Raspberry PI GPI and cannot recognized from system. On the other hand the Raspberry PI no mode to discoverable it and it is not enumerable. So this hardware is a platform device.
The solution is use Device Tree technique. 
Indeed the primary purpose of Device Tree in Linux is to provide a way to describe to the system a non-discoverable hardware.

In the Device Tree Source (DTS) file we describe hardware and next we compile it to obtain a Device Tree Binary (DTB) file.
The DTB is loaded on boot time.

Follow an extract from DTS file:

```
&gpio {
    ....
    ....
    ....

	max7219mat_pins: max7219mat_pins {
		brcm,pins = <21 22 23 24 25 26 27>; /* GPIO numbers */
		brcm,function = <1>;	/* output */
		brcm,pull = <1 1 1 1 1 1 1>;	/* pull up */
	};
};

&soc {
    ....
    ....
    ....

	max7219mat {
		compatible = "rocknrol,max7219mat";
		label = "max7219mat";
		pinctrl-0 = <&max7219mat_pins>;
	};
};
```

The driver generates all signals needed to MAX7219. This signals are shown in the following Timing Diagram (extraxted from MAXIM MAX7219 Datasheet):

![](https://raw.githubusercontent.com/rocknRol/max7219mat/master/img/dgr1.png)

In userland the software can use this "ioctl" to select the module from 1 to 16 and "write" function to send data.


## Graphics library in C language

The graphics library "libmax7219mat" allows you to manage the display hardware and turning pixels on/off.

Following all functions exposed from library:

- **int initialize_library(void);**

	*Desc: initialize display hardware*

	*Param: none*

	*Ret:* 
		
		*0: ok*
		
		*-1: error status* 


- **int selftest_display(void);**
	*Desc: execute the self-test on display*
	*Param: none*
	*Ret:* 
		*0: ok*
		*-1: error status* 






