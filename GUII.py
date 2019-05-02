#############################
# Module:   GUI.py
# Author:
# Date:
# Version:
#
# Description:
#
###############################

# -*- coding: utf-8 -*-
import pygame, sys, os
import random
from random import random
import matplotlib.pyplot as plt
from matplotlib.figure import Figure
from matplotlib import pylab
from pylab import *
import numpy as np
import time
import matplotlib.animation as animation
from pygame.locals import *
from PIL import Image

# ----------------------------------------------
# Constantes
# ----------------------------------------------

green = (32, 178, 100)
lime = (0, 255, 0)
red = (200, 0, 0)
black = (0, 0, 0)
white = (255, 255, 255)
bright_red = (255, 0, 0)
bright_green = (173, 255, 47)
light_blue = (79, 198, 255)
blue = (0, 172, 192)
window_Width = 1000
window_Height = 700

# y labels for the graphics
v_axis = ["NA1", "NA2", "Altitude (m)", "Pressure (Pa)", "Temperature (C)", "Voltage (V)", "GPS Time (s)",
          "GPS Latitude", "GPS Longitude", "GPS Altitude", "GPS Sats", "Pitch", "Roll", "Blade Spin Rate (rpm)"]


# ----------------------------------------------
# Clases y Funciones utilizadas
# ----------------------------------------------


# Effect: Create a csv file
# Required: variables's vector (v_variables)
# Modified: csv file
def create_csv(v_variables):

    v_variables[17] = open("Flight_3666.csv", "w")
    v_variables[17].write("MISSION_TIME;PACKET_COUNT;ALTITUDE;PRESSURE;TEMP;VOLTAGE;GPS_TIME;GPS_LATITUDE;"
                          "GPS_LONGITUDE;GPS_ALTITUDE;GPS_SATS;PITCH;ROLL;BLADE_SPIN_RATE;SOFTWARE_STATE\n")


# Effect: Add a row of data to the csv file
# Required: variables's vector (v_variables)
# Modified: csv file
def add_data_csv(v_variables):
    # v_variables[17] = file entrance
    for x in range(0, 14):
        v = "%d" % (v_variables[x])
        v_variables[17].write(v)
        if x < 14:
            v_variables[17].write(";")
    v_variables[17].write(v_variables[14])
    v_variables[17].write("\n")


# Effect: load an image from the current directory and return it
# Required: image's name, current directory
# Modified: display screen
def load_image(nombre, dir_imagen, alpha=False):
    # Encontramos la ruta completa de la imagen
    ruta = os.path.join(dir_imagen, nombre)
    try:
        image = pygame.image.load(ruta)
    except:
        print("Error, no se puede cargar la imagen: " + ruta)
        sys.exit(1)

        # Comprobar si la imagen tiene "canal alpha" (como los png)
    if alpha is True:
        image = image.convert_alpha()
    else:
        image = image.convert()

    return image


# Effect: Create a text object and return it
# Required: text, font type
def text_objects(text, font):
    textSurface = font.render(text, True, black)

    return textSurface, textSurface.get_rect()


# Effect: display a message with an specified size and position
# Required: screen, text, size, x and y axis (for position)
# Modified: display screen
def message_display(screen, text, size, x, y):
    font = pygame.font.Font('freesansbold.ttf', size)
    text_surf, text_rect = text_objects(text, font)
    text_rect.center = (x, y)
    screen.blit(text_surf, text_rect)


# Effect: display all messages and buttons on interface's general tab
# Required: screen, color1, variables's vector (v_variables)
# Modified: display screen
def display_general(screen, color1, v_variables):
    pygame.draw.rect(screen, color1, (405, 50, 190, 60))
    message_display(screen, "General", 40, 405 + 190 / 2, 50 + 60 / 2)

    message_display(screen, 'Team #: 3666', 20, 135, 210)
    message_display(screen, 'Mission Time:', 20, 140, 290)
    message_display(screen, 'Port:', 20, 95, 340)
    message_display(screen, 'Launch:', 20, 110, 390)
    message_display(screen, 'Software State:', 20, 145, 440)

    message_display(screen, 'Altitude:', 20, 795, 70)
    message_display(screen, 'Pressure:', 20, 795, 110)
    message_display(screen, 'Temperature:', 20, 795, 150)
    message_display(screen, 'Voltage:', 20, 795, 190)

    message_display(screen, 'GPS Time:', 20, 795, 270)
    message_display(screen, 'GPS Latitude:', 20, 795, 310)
    message_display(screen, 'GPS Longitude:', 20, 795, 350)
    message_display(screen, 'GPS Altitude:', 20, 795, 390)
    message_display(screen, 'GPS Sats:', 20, 795, 430)

    message_display(screen, 'Pitch:', 20, 795, 510)
    message_display(screen, 'Roll:', 20, 795, 550)
    message_display(screen, 'Blade Spin Rate:', 20, 795, 590)

    # v_variables[15] = bool launch enable
    if v_variables[15]:
        pygame.draw.rect(screen, green, (182, 378, 20, 20))
    else:
        pygame.draw.rect(screen, red, (182, 378, 20, 20))

    message_display(screen, v_variables[14], 20, 250, 440)

    # display mission time
    t = " %d seconds" % (v_variables[0])
    message_display(screen, t, 20, 265, 290)

#  display all data in general's tab
#  t is a local variable
    if v_variables[16]:
        t = " %d" % (v_variables[2])  # Altitude
        message_display(screen, t, 20, 920, 70)
        t = " %d" % (v_variables[3])  # Pressure
        message_display(screen, t, 20, 920, 110)
        t = " %d" % (v_variables[4])  # Temperature
        message_display(screen, t, 20, 920, 150)
        t = " %d" % (v_variables[5])  # Voltage
        message_display(screen, t, 20, 920, 190)

        t = " %d" % (v_variables[6])  # Gps Time
        message_display(screen, t, 20, 920, 270)
        t = " %d" % (v_variables[7])  # Gps latitude
        message_display(screen, t, 20, 920, 310)
        t = " %d" % (v_variables[8])  # Gps longitude
        message_display(screen, t, 20, 920, 350)
        t = " %d" % (v_variables[9])  # Gps altitude
        message_display(screen, t, 20, 920, 390)
        t = " %d" % (v_variables[10])  # Gps sats
        message_display(screen, t, 20, 920, 430)

        t = " %d" % (v_variables[11])  # pitch
        message_display(screen, t, 20, 920, 510)
        t = " %d" % (v_variables[12])  # roll
        message_display(screen, t, 20, 920, 550)
        t = " %d" % (v_variables[13])  # blade spin rate
        message_display(screen, t, 20, 920, 590)


# Effect: Update the data in each program iteration
# Required: variables's vector (v_variables)
# Modified: v_variables
def update_variables(v_variables):
    if v_variables[16] and v_variables[18] < v_variables[0]:  # v_time
        for x in range(2, 14):
            v_variables[x] = randint(1, 30) + v_variables[0]
        add_data_csv(v_variables)
        v_variables[18] += 1  # v_skip


class Graph(pygame.sprite.Sprite):
    def __init__(self, text, num):
        self.text = text
        self.num = num
        # graph box color: (200, 135, 620, 450)

    # Effect: Display the current graph on the screen
    # Required: screen, color1, variables's vector(v_variables), num(number to labels's vector)
    # Modified: display screen
    def display_graph(self, screen, color1, v_variables, v_axis):
        pygame.draw.rect(screen, color1, (343, 50, 338, 60))
        # Temporary Values
        xaxis = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
        yaxis = [300, 295, 287, 285, 279, 275, 262, 260, 253, 249, 243]
        plt.plot(xaxis, yaxis)
        plt.ylabel(v_axis)
        plt.xlabel('Time (s)')
        plt.savefig('testplot.png')
        # Load of graphic as an image to add to the existing window
        img = pygame.image.load('testplot.png')
        screen.blit(img, (200, 135))
        # pygame.display.flip()

        # pygame.draw.rect(screen, (235, 235, 235), (200, 135, 620, 450))
        message_display(screen, self.text, 40, 343 + 338 / 2, 50 + 60 / 2)

        if v_variables[16]:
            c = "%s = %d seconds" % (0, 0)
            # message_display(screen, c, 20, 515, 540)

            c = "%s = %d " % (self.num, self.num)
            # message_display(screen, c, 20, 270, 350)

    def update(self):
        self.text = ""


class Button:
    def __init__(self, value, surface, color1, color2, text, size, width, height, x, y):
        self.value = value
        self.surface = surface
        self.color1 = color1
        self.color2 = color2
        self.text = text
        self.size = size
        self.width = width
        self.height = height
        self.x = x
        self.y = y
        self.selected = False

    # Effect: Update the current button s state in the screen
    # Required: An action over the current button
    # Modified: The current button
    def update(self):
        mouse = pygame.mouse.get_pos()
        click = pygame.mouse.get_pressed()

        if self.x < mouse[0] < self.x + self.width and self.y < mouse[1] < self.y + self.height:
            pygame.draw.rect(self.surface, self.color2, (self.x, self.y, self.width, self.height))
            message_display(self.surface, self.text, self.size, self.x + self.width / 2, self.y + self.height / 2)

            if click[0] == 1:
                self.selected = True

        else:
            pygame.draw.rect(self.surface, self.color1, (self.x, self.y, self.width, self.height))
            message_display(self.surface, self.text, self.size, self.x + self.width / 2, self.y + self.height / 2)

        # ------------------------------


# Funcion principal del juego
# ------------------------------


def main():

    #  runs pygame's module, and initializes clock's variable
    pygame.init()
    screen = pygame.display.set_mode((window_Width, window_Height))
    pygame.display.set_caption("TuCan CANSAT UCR")
    fondo = load_image("pp.jpg", "imagen", alpha=False)
    clock = pygame.time.Clock()
    menu = True # uses to determine if interface will display main screen or some button screen
    num = 0

    # VARIABLES--------------------------------------------------------------------------------------------------------

    v_time = 0  # 0 mission time
    v_packet_count = 0  # 1
    v_altitude = 0  # 2
    v_pressure = 0  # 3
    v_temp = 0  # 4
    v_voltage = 0  # 5
    v_gps_time = 0  # 6
    v_gps_latitude = 0  # 7
    v_gps_longitude = 0  # 8
    v_gps_altitude = 0  # 9
    v_gps_sats = 0  # 10
    v_pitch = 0  # 11
    v_roll = 0  # 12
    v_blade_spin_rate = 0  # 13
    v_software_state = None  # 14

    v_launch = False  # 15 uses to determine if launch has begins
    v_start = False  # 16 uses to determine if start's button has be pressed
    v_document = None  # 17 uses to access csv file
    v_skip = -1  # 18

    v_variables = [v_time, v_packet_count, v_altitude, v_pressure, v_temp, v_voltage, v_gps_time, v_gps_latitude,
                   v_gps_longitude, v_gps_altitude, v_gps_sats, v_pitch, v_roll, v_blade_spin_rate, v_software_state,
                   v_launch, v_start, v_document, v_skip]

    # END_VARIABLES----------------------------------------------------------------------------------------------------

    # BOTONES----------------------------------------------------------------------------------------------------------

    b_start = Button(0, screen, blue, light_blue, "Start", 18, 72, 32, 90, 520)
    b_reset = Button(0, screen, blue, light_blue, "Reset", 18, 72, 32, 90, 560)
    b_payload_reset = Button(0, screen, blue, light_blue, "Payload reset", 18, 130, 32, 220, 560)
    b_return = Button(0, screen, blue, light_blue, "Return", 18, 72, 32, 90, 600)

    b_general = Button(0, screen, blue, light_blue, "General", 40, 190, 70, 80, 300)

    b_altitude = Button(0, screen, blue, light_blue, "Altitude", 18, 150, 30, 780, 10)
    b_pressure = Button(0, screen, blue, light_blue, "Pressure", 18, 150, 30, 780, 60)
    b_temperature = Button(0, screen, blue, light_blue, "Temperature", 18, 150, 30, 780, 110)
    b_voltage = Button(0, screen, blue, light_blue, "Voltage", 18, 150, 30, 780, 160)

    b_gps_time = Button(0, screen, blue, light_blue, "GPS Time", 18, 150, 30, 780, 230)
    b_gps_latitude = Button(0, screen, blue, light_blue, "GPS Latitude", 18, 150, 30, 780, 280)
    b_gps_longitude = Button(0, screen, blue, light_blue, "GPS Longitude", 18, 150, 30, 780, 330)
    b_gps_altitude = Button(0, screen, blue, light_blue, "GPS Altitude", 18, 150, 30, 780, 380)
    b_gps_sats = Button(0, screen, blue, light_blue, "GPS Sats", 18, 150, 30, 780, 430)

    b_pitch = Button(0, screen, blue, light_blue, "Pitch", 18, 150, 30, 780, 500)
    b_roll = Button(0, screen, blue, light_blue, "Roll", 18, 150, 30, 780, 550)
    b_blade_spin_rate = Button(0, screen, blue, light_blue, "Blade Spin Rate", 18, 150, 30, 780, 600)

    b_desktop = [b_general, b_altitude, b_pressure, b_temperature, b_voltage, b_gps_time, b_gps_latitude,
                 b_gps_longitude, b_gps_altitude, b_gps_sats, b_pitch, b_roll, b_blade_spin_rate]

    # END_BOTONES------------------------------------------------------------------------------------------------------

    # GRAFICOS---------------------------------------------------------------------------------------------------------

    # Graph("Name", v_variable num)

    g_altitude = Graph("Altitude", 2)
    g_pressure = Graph("Pressure", 3)
    g_temperature = Graph("Temperature", 4)
    g_voltage = Graph("Voltage", 5)

    g_gps_time = Graph("GPS Time", 6)
    g_gps_latitude = Graph("GPS Latitude", 7)
    g_gps_longitude = Graph("GPS Longitude", 8)
    g_gps_altitude = Graph("GPS Altitude", 9)
    g_gps_sats = Graph("GPS Sats", 10)

    g_pitch = Graph("Pitch", 11)
    g_roll = Graph("Roll", 12)
    g_blade_spin_rate = Graph("Blade Spin Rate", 13)

    g_variables = [g_altitude, g_pressure, g_temperature, g_voltage, g_gps_time, g_gps_latitude, g_gps_longitude,
                   g_gps_altitude, g_gps_sats, g_pitch, g_roll, g_blade_spin_rate]

    # END_GRAFICOS-----------------------------------------------------------------------------------------------------

    create_csv(v_variables)

    while True:  # Program's Loop

        update_variables(v_variables)

        if v_variables[16]:  # v_start
            tick = clock.tick(60)
            if tick / 1000 < 1:
                v_variables[0] += (tick / 1000)  # v_time
            else:
                tick = 0
            v_variables[14] = "r. data"  # v_software_state
            v_variables[15] = True  # v_launch
        else:
            v_variables[0] = 0  # v_time
            v_variables[18] = -1  # v_skip
            v_variables[14] = "idle"  # v_software_state
            v_variables[15] = False  # v_launch

        for event in pygame.event.get():

            if event.type == pygame.QUIT:
                v_variables[17].close()  # v_document
                sys.exit(0)

        if menu:  # if menu = true display main screen, else display selected button screen
            screen.blit(fondo, (0, 0))

            for x in b_desktop:  # x = each button in desktop vector
                x.update()
                if x.selected:  # uses to determine if some button has be pressed
                    menu = False

            pygame.display.flip()

        else:  # menu = false
            if b_desktop[0].selected:   # b_desktop[0] = b_general. If general button has be pressed
                screen.blit(fondo, (0, 0))
                display_general(screen, blue, v_variables)

                # button's update
                b_start.update()
                b_reset.update()
                b_return.update()
                b_payload_reset.update()

                pygame.display.flip()

                # If return, start or reset button has be pressed
                if b_return.selected:
                    menu = True
                    b_return.selected = False
                    b_general.selected = False
                elif b_start.selected:
                    v_variables[16] = True  # v_variables[16] = v_start
                    b_start.selected = False
                elif b_reset.selected:
                    # v_variables[17].close()  # v_variables[17] = v_document
                    v_variables[16] = False  # v_variables[16] = v_start
                    b_reset.selected = False
                    # create_csv(v_variables)

            else:  # b_general.selected = False

                # Finds if another button has be pressed instead of the general button and takes it's number
                if num == 0:
                    num = 1
                    con = True
                    while num <= b_desktop.__len__() and con:
                        if b_desktop[num].selected:
                            con = False
                        else:
                            num += 1

                screen.blit(fondo, (0, 0))
                b_return.update()
                # g_variables[num-1] = current graph
                g_variables[num - 1].display_graph(screen, blue, v_variables, v_axis[num + 1])
                pygame.display.flip()
                if b_return.selected:

                    menu = True
                    b_return.selected = False
                    b_desktop[num].selected = False
                    num = 0


if __name__ == "__main__":
    main()
