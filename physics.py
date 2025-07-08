import numpy as np
import glib
from dataclasses import dataclass, field
from datetime import datetime, timedelta
import os

gconst : float = 6.674

# F = G * (m₁ * m₂) / r²
def tick_gravity(app, v1 : list, m1 : float, v2 : list, m2 : float) -> list:
    result_v : list = [0.0, 0.0, 0.0]

    direction : list = [
        v1[0] - v2[0], 
        v1[1] - v2[1], 
        v1[2] - v2[2]
    ]

    force : float = 0.0 

    result_v = [direction[0] * force, direction[1] * force, direction[2] * force]

    return result_v
    

def sphere_to_sphere_collision(app, p1 : list, r1 : float, p2 : list, r2 : float) -> bool:
    pass