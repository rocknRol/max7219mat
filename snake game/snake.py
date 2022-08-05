"""
 Copyright (C) 2022 Rolando Spennato (@rocknRol)
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 """

import pymax7219lib
import curses
import time
import random

def getMilliseconds():
    return int(round(time.time() * 1000))

def init(screen):
    global snake, direction, oldDirection, inGameLoop, speed, msInit
    direction = oldDirection = 1
    snake = [[16,16]]
    speed = 50 # in milliseconds
    msInit = getMilliseconds()
    inGameLoop = True
    pymax7219lib.initializeLibrary()
    screen.nodelay(1)

def putFood():
    global xFood, yFood
    xFood = random.randint(0,31)
    yFood = random.randint(0,31)
    pymax7219lib.setPixel(xFood, yFood, 1)
    
def processInput(screen):
    global direction, oldDirection, inGameLoop
    key = screen.getch()
    if key != -1:
        if key == 27:
            inGameLoop = False
        elif key == 261 and oldDirection != 2: #right
            direction = 1
        elif key == 260 and oldDirection != 1: #left
            direction = 2
        elif key == 259 and oldDirection != 4: #up
            direction = 3
        elif key == 258 and oldDirection != 3: #down
            direction = 4
        oldDirection = direction
    
def sync():
    global speed, msInit
    if getMilliseconds() - msInit > speed:
        msInit = getMilliseconds()
        return True
    else:
        return False

def gameOver():
    global snake
    x = snake[0][0] - 2
    y = snake[0][1] - 2
    for i in range(5):
        for j in range(5):
            pymax7219lib.setPixel(x + i, y + j, 1)
    time.sleep(1)

def updateState():
    global snake, direction, inGameLoop
    if direction == 1:
        tile = [abs((snake[0][0] + 1) % 32), abs(snake[0][1] % 32)]
    elif direction == 2:
        tile = [abs((snake[0][0] - 1) % 32), abs(snake[0][1] % 32)]
    elif direction == 3:
        tile = [abs(snake[0][0] % 32), abs((snake[0][1] - 1) % 32)]
    elif direction == 4:
        tile = [abs(snake[0][0] % 32), abs((snake[0][1] + 1) % 32)]
    if pymax7219lib.getPixel(tile[0], tile[1]) == 0:
        snake.insert(0, tile)
        snake.pop()
    else:
        if tile[0] == xFood and tile[1] == yFood:
            snake.insert(0, tile)
            putFood()
        else:
            gameOver()
            inGameLoop = False

def drawScreen():
    global snake
    pymax7219lib.clearScreen()
    pymax7219lib.setPixel(xFood, yFood, 1)
    for tile in snake:
        pymax7219lib.setPixel(tile[0], tile[1], 1)

def quit():
    pymax7219lib.quitLibrary()

def main(screen):
    init(screen)
    putFood()
    while inGameLoop: 
        processInput(screen)
        if sync():
            updateState()
            drawScreen()
    quit()

curses.wrapper(main)