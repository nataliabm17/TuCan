import pygame
from pygame.locals import *

green = (32,178,100)
lime = (0, 255, 0)
red = (200, 0 ,0)
black = (0,0,0)
white = (255,255,255)
bright_red = (255,0,0)
bright_green = (173,255,47)
light_blue = (200,255,255)
window_Width = 1200
window_Height = 900
background_colour = light_blue


# create text objects
def text_objects(text, font):
    textSurface = font.render(text, True, black)
    return textSurface, textSurface.get_rect()

# display text
def message_display(text, x, y, size):
    largeText = pygame.font.Font('freesansbold.ttf', size)
    TextSurf, TextRect = text_objects(text, largeText)
    TextRect.center = (x,y)
    window.blit(TextSurf, TextRect)

# display general tab
def display_general(color1, text):
#    pygame.draw.rect(screen, light_blue,(120,230,800,650))
    pygame.draw.rect(screen, color1,(430,70,350,80))            # create rectangle for the title
    message_display(text, 600, 110, 40)                         # display title
    message_display('Team #: 0000', 197, 260, 25)               # display text
    message_display('Mission Time:', 202, 380 , 25)
    message_display('Port:', 148, 440, 25)
    message_display('Launch:', 170, 500, 25)
    pygame.draw.rect(screen, red,(280,485,35,35))               # create rectangle that changes to green when launch occurs
    pygame.draw.rect(screen, lime,(475,775,250,50))             # create rectangle for the save and store button
    pygame.draw.rect(screen, red,(250,775,100,50))              # create rectangle for the reset button
    message_display('Save and export', 600, 800, 25)
    message_display('Reset', 300, 800, 25)
    message_display('Tilt:', 595, 380, 25)
    message_display('Altitude:', 621, 440, 25)
    message_display('Voltage:', 624, 500, 25)
    message_display('Pressure:', 635, 560, 25)
    message_display('Temperature:', 657, 620, 25)
    message_display('Acceleration:', 655, 680, 25)

# create button
def button(x, y, height, width, color1, color2, text, text_size, position):
    click = pygame.mouse.get_pressed()
    mouse = pygame.mouse.get_pos()
    pygame.display.update()
    if (x+width > mouse[0] > x and (y*position)+height > mouse[1] > y*position):        # in case the mouse is on the button's position
        pygame.draw.rect(screen, color2,(x,y*position,width,height))                    # change button's color
        message_display(text, (x + width/2), (y*position + height/2), text_size)        # display text
        if (click[0]==1):                           # in case the button is selected
            if (position==1):                       # in case the first button is selected
                display_general(color1, text)       # diplay general tab
            else:                                   # in case is any other button
                pygame.draw.rect(screen, light_blue,(120,230,800,650))  # erase last tab
                pygame.draw.rect(screen, color1,(430,70,350,80))        # create rectangle for the title
                message_display(text, 600, 110, 40)                     # display text
    else:
        pygame.draw.rect(screen, color1,(x,y*position,width,height))       # create rectangle for the button
        message_display(text, (x + width/2), (y*position + height/2), text_size)    # display text

# create all the buttons for the different tags
def createButtons():
    button(window_Width/1.2, window_Height/8, 50, 130, green, bright_green, 'General', 20, 1)
    button(window_Width/1.2, window_Height/8, 50, 130, green, bright_green, 'Altitude', 20, 2)
    button(window_Width/1.2, window_Height/8, 50, 130, green, bright_green, 'Voltage', 20, 3)
    button(window_Width/1.2, window_Height/8, 50, 130, green, bright_green, 'Pressure', 20, 4)
    button(window_Width/1.2, window_Height/8, 50, 130, green, bright_green, 'Temperature', 20, 5)
    button(window_Width/1.2, window_Height/8, 50, 130, green, bright_green, 'Acceleration', 20, 6)
    button(window_Width/1.2, window_Height/8, 50, 130, green, bright_green, 'Location', 20, 7)
#    pygame.display.update()

def main_loop():
    pygame.display.set_caption('TuCan CANSAT UCR')         # set caption for the window
    screen.fill(background_colour)                         # background color
    crashed = False
    while not crashed:
        createButtons()
        for event in pygame.event.get():
    		if event.type == pygame.QUIT:
    			crashed = True

pygame.init()                                                                   # initialize pygame
window = pygame.display.set_mode((window_Width, window_Height), DOUBLEBUF)      # display window
screen = pygame.display.get_surface()                                           # display surface
main_loop()                                                                     # start loop
