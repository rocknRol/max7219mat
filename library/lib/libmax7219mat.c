/*
 * Copyright (C) 2021 Rolando Spennato (@rocknRol)
 *
 * This driver is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define PIXEL_SCREEN_WIDTH      32
#define PIXEL_SCREEN_HEIGHT     32
#define NUMBER_OF_MODULES       16
#define NUMBER_OF_LINES         8
#define BRIGHTNESS_VALUE_MIN    1
#define BRIGHTNESS_VALUE_MAX    15
#define DEVICE_NODE             "/dev/max7219mat"

int g_matrix[PIXEL_SCREEN_HEIGHT][PIXEL_SCREEN_WIDTH];

int g_library_initialized = 0;

int g_screen_clipping_enable; /* disable to default */
int g_auto_render_enable; /* enable to default */

int g_device_descriptor;

int check_library_initialization()
{
    if(!g_library_initialized) {
        printf("Library libmax7219mat is not initialized\n");
        return -1;
    }

    return 0;
}

int selftest_display()
{
    unsigned short buf1, buf2;
    int i;

    if(check_library_initialization())
      return -1;

    buf1 = 0x0F01; /* display test mode on */
    buf2 = 0x0F00; /* display test mode off */
    for(i = 1; i <= NUMBER_OF_MODULES; i++) {
        ioctl(g_device_descriptor, 1, i);
        write(g_device_descriptor, &buf1, 2);
        usleep(500000);
        write(g_device_descriptor, &buf2, 2);
    }

    return 0;
}

int check_display_module(int module, int toggle_on_off)
{
    unsigned short buf;

    if(check_library_initialization())
        return -1;

    if(module < 1 || module > NUMBER_OF_MODULES) {
        printf("Module value is invalid\n");
        return -1;
    }

    if(toggle_on_off != 0 && toggle_on_off != 1) {
        printf("toggle_on_off value is invalid\n");
        return -1;
    }

    ioctl(g_device_descriptor, 1, module);
    if(toggle_on_off)
        buf = 0x0F01; /* display test mode on */
    else
        buf = 0x0F00; /* display test mode off */

    write(g_device_descriptor, &buf, 2);

    return 0;
}

int set_brightness(int brightness, int module)
{
    unsigned short buf;
    int i;

    if(check_library_initialization())
        return -1;

    if(brightness < BRIGHTNESS_VALUE_MIN || brightness > BRIGHTNESS_VALUE_MAX) {
        printf("Brightness value is invalid\n");
        return -1;
    }

    if(module < 1 || module > NUMBER_OF_MODULES) {
        printf("Module value is invalid\n");
        return -1;
    }

    buf = 0x0A00 | brightness; /* intensity register */

    if(module == 0) {
        for(i = 1; i <= NUMBER_OF_MODULES; i++) {
            ioctl(g_device_descriptor, 1, i);
            write(g_device_descriptor, &buf, 2);
        }
    } else {
        ioctl(g_device_descriptor, 1, module);
        write(g_device_descriptor, &buf, 2);
    }

    return 0;
}

int render_line(int module, int line)
{
    unsigned short buf = 0x0;
    int i, x_mat, y_mat;

    if(check_library_initialization())
        return -1;

    if(module < 1 || module > NUMBER_OF_MODULES) {
        printf("Module value is invalid\n");
        return -1;
    }

    if(line < 0 || line > NUMBER_OF_LINES - 1) {
        printf("Line value is invalid\n");
        return -1;
    }

    ioctl(g_device_descriptor, 1, module);

    buf |= ((8-line) << 8) & 0xFFFF;

    for(i = 0; i < 8; i++) {
        x_mat = ((module - 1) % 4) * 8 + i;
        y_mat = module % 4 == 0 ? (((module - 1) / 4) * 8) + line: ((module / 4) * 8) + line;

        if(g_matrix[x_mat][y_mat] == 1)
            buf |= (0x0001 << i) & 0xFFFF;
        else
            buf &= ~(0x0001 << i) & 0xFFFF;
    }

    write(g_device_descriptor, &buf, 2);

    return 0;
}

int render_module(int module)
{
    int i;

    if(check_library_initialization())
        return -1;

    if(module < 1 || module > NUMBER_OF_MODULES) {
        printf("Module value is invalid\n");
        return -1;
    }

    for(i = 0; i < 8; i++)
        render_line(module, i);

    return 0;
}

int render_screen()
{
    int i;

    if(check_library_initialization())
        return -1;

    for(i = 1; i <= NUMBER_OF_MODULES; i++)
        render_module(i);

    return 0;
}

int auto_render_enable(int value)
{
    if(check_library_initialization())
        return -1;

    if(value != 0 && value != 1) {
        printf("Render value incorrect. Use 1 to enable render and 0 to disable render\n");
        return -1;
    }

    g_auto_render_enable = value;

    return 0;
}

int screen_clipping_enable(int value) 
{

    if(check_library_initialization())
        return -1;

    if(value != 0 && value != 1) {
        printf("Screen Clipping value incorrect. Use 1 to enable render and 0 to disable render\n");
        return -1;
    }

    g_screen_clipping_enable = value;
}

int set_pixel(int x, int y, int value)
{
    int mod_x, mod_y;
    int module, line;

    if(check_library_initialization())
        return -1;

    mod_x = x % PIXEL_SCREEN_WIDTH;
    mod_y = y % PIXEL_SCREEN_HEIGHT;

    module = (mod_x / 8) + 1 + (mod_y / 8) * 4;
    line = mod_y % 8;

    if(g_screen_clipping_enable) {
        if(x >= 0 && x <= PIXEL_SCREEN_WIDTH - 1 && y >= 0 && y <= PIXEL_SCREEN_HEIGHT - 1) {
            g_matrix[x][y] = value;
            if (g_auto_render_enable)
                render_line(module, line);
        }
    } else {
        g_matrix[mod_x][mod_y] = value;
        if (g_auto_render_enable)
            render_line(module, line);
    }

    return 0;
}

int get_pixel(int x, int y)
{    
    int mod_x, mod_y;

    if(check_library_initialization())
        return -1;

    mod_x = x % PIXEL_SCREEN_WIDTH;
    mod_y = y % PIXEL_SCREEN_HEIGHT;

    return g_matrix[mod_x][mod_y];      
}

int clear_screen()
{
    int i, j;

    if(check_library_initialization())
        return -1;

    for(i = 0; i < PIXEL_SCREEN_HEIGHT; i++)
        for(j = 0; j < PIXEL_SCREEN_WIDTH; j++)
            g_matrix[i][j] = 0;

    render_screen();

    return 0;
}

int initialize_library()
{
    unsigned short buf;
    int i;

    g_device_descriptor = open(DEVICE_NODE, O_RDWR);

    if (g_device_descriptor < 0) {
        printf("Failed to open device node: %s\n", DEVICE_NODE);
        return -1;
    }

    for(i = 1; i <= NUMBER_OF_MODULES; i++) {
        ioctl(g_device_descriptor, 1, i);

        buf = 0x0B07; /* enable all columns via the scan limit register */
        write(g_device_descriptor, &buf, 2);
		buf = 0x0900; /* decode mode: no decode */
        write(g_device_descriptor, &buf, 2);
        buf = 0x0F00; /* display test mode off */
        write(g_device_descriptor, &buf, 2);
        buf = 0x0A01; /* set Brightness at minimum value */
        write(g_device_descriptor, &buf, 2);

		/* clear registers */
	    for(buf=0x0100; buf<=0x0800; buf=buf+0x0100)
	        write(g_device_descriptor, &buf, 2);

		buf = 0x0C01; /* disable shutdown mode */
        write(g_device_descriptor, &buf, 2);
    }

    g_auto_render_enable = 1;
    g_screen_clipping_enable = 0;
    g_library_initialized = 1;

    clear_screen();

    return 0;
}

int quit_library()
{
    unsigned short buf;
    int i;

    if(check_library_initialization())
        return -1;    

    for(i = 1; i <= NUMBER_OF_MODULES; i++) {
        ioctl(g_device_descriptor, 1, i);

        buf = 0x0C00; /* enable shutdown mode */
        write(g_device_descriptor, &buf, 2);
    }

    g_library_initialized = 0;
    
    close(g_device_descriptor);

    return 0;
}