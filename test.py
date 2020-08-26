#!/usr/bin/env python

import numpy as np
import math
import cairo

WIDTH, HEIGHT = 512, 512 

surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, WIDTH, HEIGHT)
ctx = cairo.Context(surface)

ctx.scale(WIDTH, HEIGHT)  # Normalizing the canvas

ctx.set_source_rgba(1, 1, 1)
ctx.paint()

def draw_points(points, *, near_radius=0.03,
                           thickness=0.3,
                           field_of_view=np.pi/4):

    S = 1/np.tan(0.5*field_of_view)
    proj_mat = np.array([[ S,  0,  0,  0],
                         [ 0,  S,  0,  0],
                         [ 0,  0, -1, -1],
                         [ 0,  0, -1,  0]])

    points = [ np.array(list(point)+[1]) for point in points ]
    points = [ np.dot(proj_mat, point) for point in points ]
    points = [ point/point[3] for point in points ]

    points.sort(key=lambda point: point[2], reverse=True)
    for point in points:
        ctx.set_source_rgb(0, 0, 0)
        ctx.arc((point[0]+1)/2, (point[1]+1)/2,
                near_radius/point[2], 0, 2*np.pi)
        ctx.fill()
        ctx.set_source_rgb(1, 1, 1)
        ctx.arc((point[0]+1)/2, (point[1]+1)/2,
                near_radius/point[2]*(1-thickness), 0, 2*np.pi)
        ctx.fill()

for i, theta in enumerate(np.linspace(0, 2*np.pi, 400)):
    ctx.set_source_rgb(1, 1, 1)
    ctx.paint()
    rot_mat = np.array([[ np.cos(theta), 0, np.sin(theta)],
                        [             0, 1,             0],
                        [-np.sin(theta), 0, np.cos(theta)]])
    sphere_patches = [ np.array([i, j, k])
                        for i in [-1, 0, 1]
                        for j in [-1, 0, 1]
                        for k in [-1, 0, 1]
                        if (i, j, k) != (0, 0, 0) ]
    sphere_patches = [ patch/np.linalg.norm(patch)
                        for patch in sphere_patches ]
    sphere_patches = [ np.dot(rot_mat, patch)
                        for patch in sphere_patches ]
    sphere_patches = [ 3*patch+np.array([0, 0, -10])
                        for patch in sphere_patches ]
    draw_points(sphere_patches)

    surface.write_to_png("%03d.png"%i)  # Output to PNG

exit()
