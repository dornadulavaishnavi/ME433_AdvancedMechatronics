import board
import busio # for the uart functions
import digitalio

from adafruit_ov7670 import (
    OV7670,
    OV7670_SIZE_DIV16,
    OV7670_COLOR_YUV,
    OV7670_TEST_PATTERN_COLOR_BAR,
)

import sys
import time
import digitalio
import busio
from ulab import numpy as np

with digitalio.DigitalInOut(board.GP10) as reset:
    reset.switch_to_output(False)
    time.sleep(0.001)
    bus = busio.I2C(scl=board.GP5, sda=board.GP4)

uart = busio.UART(tx=board.GP0, rx=board.GP1, baudrate=115200,  timeout= 1) # open the uart

threshold_path = 160
motor_straight = 127

cam = OV7670(
    bus,
    data_pins=[
        board.GP12,
        board.GP13,
        board.GP14,
        board.GP15,
        board.GP16,
        board.GP17,
        board.GP18,
        board.GP19,
    ],
    clock=board.GP11,
    vsync=board.GP7,
    href=board.GP21,
    mclk=board.GP20,
    shutdown=board.GP22,
    reset=board.GP10,
)

pid = cam.product_id
ver = cam.product_version
#print(f"Detected pid={pid:x} ver={ver:x}")

cam.size = OV7670_SIZE_DIV16
#print(cam.width)    #40
#print(cam.height)   #30

#cam.test_pattern = OV7670_TEST_PATTERN_COLOR_BAR

cam.colorspace = OV7670_COLOR_YUV
cam.flip_y = True

buf = bytearray(2 * cam.width * cam.height) # where all the raw data is stored

# store the converted pixel data
red = np.linspace(1,1,cam.width * cam.height, dtype=np.float)
green = np.linspace(0,0,cam.width * cam.height, dtype=np.float)
blue = np.linspace(0,0,cam.width * cam.height, dtype=np.float)
ind = 0

while True:
    sys.stdin.readline() # wait for a newline before taking an image
    cam.capture(buf) # get the image

    # process the raw data into color pixels
    ind = 0
    row_sum = 0
    count_path = 0
    loop_count = 1
    row_number = 1
    threshold_counter = 0
    for d in range(0,2 * cam.width * cam.height,4):
        u = buf[d+1] - 128
        v = buf[d+3] - 128

        red[ind] = buf[d]
        # if ind > (40*7) and ind < (40*8) or ind > (40*14) and ind < (40*15) or ind > (40*21) and ind < (40*22):
#             red[ind] = buf[d] + 1.370705 * v
        if red[ind] > 255:
            red[ind] = 255
        if red[ind] < 0:
            red[ind] = 0

        green[ind] = buf[d]
        # if ind > (40*7) and ind < (40*8) or ind > (40*14) and ind < (40*15) or ind > (40*21) and ind < (40*22):
#             green[ind] = buf[d] - 0.337633 * u - 0.698001 * v
        if green[ind] > 255:
            green[ind] = 255
        if green[ind] < 0:
            green[ind] = 0

        blue[ind] = buf[d]
        # if ind > (40*7) and ind < (40*8) or ind > (40*14) and ind < (40*15) or ind > (40*21) and ind < (40*22):
#             blue[ind] = buf[d] + 1.732446 * u
        if blue[ind] > 255:
            blue[ind] = 255
        if blue[ind] < 0:
            blue[ind] = 0

        avg_color = (red[ind]+green[ind]+blue[ind])/3
        if avg_color < threshold_path:  #for each pixel, check if dark
            threshold_counter += 1
        # if (loop_count%40) == 0: #for each row
#             if threshold_counter > 29: #if 3/4 of image is dark, then take that as the path
#                 row_sum += row_number
#                 count_path += 1
#             print(threshold_counter, row_number)
#             threshold_counter = 0
#             row_number += 1
        loop_count += 1

        ind = ind+1
        red[ind] = buf[d+2]
        # if ind > (40*7) and ind < (40*8) or ind > (40*14) and ind < (40*15) or ind > (40*21) and ind < (40*22):
#             red[ind] = buf[d+2] + 1.370705 * v
        if red[ind] > 255:
            red[ind] = 255
        if red[ind] < 0:
            red[ind] = 0

        green[ind] = buf[d+2]
        # if ind > (40*7) and ind < (40*8) or ind > (40*14) and ind < (40*15) or ind > (40*21) and ind < (40*22):
#             green[ind] = buf[d+2] - 0.337633 * u - 0.698001 * v
        if green[ind] > 255:
            green[ind] = 255
        if green[ind] < 0:
            green[ind] = 0

        blue[ind] = buf[d+2]
        # if ind > (40*7) and ind < (40*8) or ind > (40*14) and ind < (40*15) or ind > (40*21) and ind < (40*22):
#             blue[ind] = buf[d+2] + 1.732446 * u
        if blue[ind] > 255:
            blue[ind] = 255
        if blue[ind] < 0:
            blue[ind] = 0

        avg_color = (red[ind]+green[ind]+blue[ind])/3
        if avg_color < threshold_path:  #for each pixel, check if dark (pitch dark is about 30)
            threshold_counter += 1
        if (loop_count%40) == 0: #for each row
            if threshold_counter > 20: #if 3/4 of image is dark, then take that as the path
                if row_number > 1:
                    row_sum += row_number
                    count_path += 1
#             print(threshold_counter, row_number)
            threshold_counter = 0
            row_number += 1

        ind=ind+1
        loop_count += 1

    #calculate center row of dark area

    # send the color data as index red green blue
    # for c in range(red.size):
#         print(str(c)+" "+str(int(red[c]))+" "+str(int(green[c]))+" "+str(int(blue[c])))

    center_row =  row_sum/count_path

    p_error = 15-center_row

    if p_error < -2:
        print(f"right " ,center_row)    #turn right wheel
    elif p_error > 2:
        print(f"left " ,center_row)
    else:
        print(f"straight " ,center_row)

    i = 0
    s = str(p_error) + "\r\n" # make a string to print
    i = i + 1
    uart.write(bytearray(s)) # uart prints are of type byte
    data = uart.readline() # read until newline or timeout
    print(data) # print the data received

    if data is not None:
        print(str(data,'utf-8')) # print the data as a string
