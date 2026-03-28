#!/usr/bin/env python3
import socket
import json
import pygame
import itertools

PANEL_SIZE = 8
SCREEN_SIZE = 24
PIXEL_SIZE = 20
PIXEL_RADIUS_RATIO = 0.5

pygame.init()

screen = pygame.display.set_mode((PIXEL_SIZE*SCREEN_SIZE,PIXEL_SIZE*SCREEN_SIZE))
clock = pygame.time.Clock()

sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
sock.bind(("0.0.0.0",1337))

def is_drawable(r,c):
    panel=(r//8,c//8)
    return panel in ((0,1),(1,0),(1,1),(1,2),(2,1))

while True:

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            exit()

    data,_ = sock.recvfrom(65535)
    frame=json.loads(data.decode())

    screen.fill((0,0,0))

    for r,c in itertools.product(range(24),repeat=2):

        if is_drawable(r,c):

            val=frame[r][c]/7
            led=(30,210*val,30)

            pygame.draw.circle(
                screen,
                led,
                (PIXEL_SIZE*(c+0.5),PIXEL_SIZE*(r+0.5)),
                PIXEL_SIZE*PIXEL_RADIUS_RATIO/2
            )

    pygame.display.flip()
    clock.tick(60)
