import glib
from dataclasses import dataclass, field
from datetime import datetime, timedelta
import os
import math

gconst = 6.674

# F = G * (m₁ * m₂) / r²
def tick_gravity(app, v1: list, m1: float, v2: list, m2: float) -> list:
    result_v = [0.0, 0.0, 0.0]
    direction = [v1[i] - v2[i] for i in range(3)]
    dist_sq = sum(d * d for d in direction)
    if dist_sq > 0:
        force = gconst * (m1 * m2) / dist_sq
        mag = math.sqrt(dist_sq)
        result_v = [d * force / mag for d in direction]
    return result_v

def sphere_to_sphere_collision(app, p1: list, r1: float, p2: list, r2: float) -> bool:
    dist_sq = sum((p1[i] - p2[i]) ** 2 for i in range(3))
    return dist_sq <= (r1 + r2) ** 2

pointlights = []

@dataclass
class CubePrimitive:
    positions = [
        -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, 0.5,
        -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5,
        -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5,
        0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5,
        -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5,
        -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, -0.5, -0.5, 0.5
    ]
    indices = [
        0, 1, 2, 0, 2, 3,
        4, 6, 5, 4, 7, 6,
        8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23
    ]
    uvs = [
        0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0,
        0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0,
        0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0,
        0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0,
        0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0,
        0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0
    ]
    normals = [
        0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0,
        0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0,
        -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0,
        0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0
    ]

@dataclass
class Mesh:
    positions: list = field(default_factory=list)
    indices: list = field(default_factory=list)
    uvs: list = field(default_factory=list)
    normals: list = field(default_factory=list)

    def init(self, positions, indices, uvs, normals) -> None:
        self.positions = positions
        self.indices = indices
        self.uvs = uvs
        self.normals = normals

@dataclass
class Camera:
    position: list = field(default_factory=lambda: [0.0, 0.0, 0.0])
    rotation: list = field(default_factory=lambda: [0.0, 0.0, 0.0])
    fovy: float = 0.0
    window_width: float = 0.0
    window_height: float = 0.0

    def init(self, fovy: float, window_width: float, window_height: float) -> None:
        self.fovy = fovy
        self.window_width = window_width
        self.window_height = window_height

render_i = 0

@dataclass
class Model:
    position: list = field(default_factory=lambda: [0.0, 0.0, 0.0])
    rotation: list = field(default_factory=lambda: [0.0, 0.0, 0.0])
    scale: list = field(default_factory=lambda: [1.0, 1.0, 1.0])
    vao: int = 0
    shader: int = 0
    texture: int = 0
    texture_scale: float = 1.0
    vmesh: Mesh = field(default_factory=Mesh)

    def render(self, app, camera: Camera) -> None:
        global render_i
        projection_matrix = glib.projection_matrix4x4(camera.fovy, glib.get_window_width(app.pointer), glib.get_window_height(app.pointer))
        view_matrix = glib.view_matrix4x4(camera.position, camera.rotation)
        transform_matrix = glib.transform_matrix4x4(self.position, self.rotation, self.scale)
        
        glib.bind_shader(self.shader)
        glib.push_float_to_shader("tex0_scale", self.texture_scale, self.shader)
        glib.push_texture2D_to_shader("tex0", self.texture, self.shader)
        glib.push_matrix4x4_to_shader("transform_matrix", transform_matrix, self.shader)
        glib.push_matrix4x4_to_shader("view_matrix", view_matrix, self.shader)
        glib.push_matrix4x4_to_shader("projection_matrix", projection_matrix, self.shader)
        glib.push_float_to_shader("ambient_light_pow", 0.2, self.shader)

        for idx, i in enumerate(pointlights):
            glib.push_vec3_to_shader(f"point_position[{idx}]", i.position, self.shader)
            glib.push_vec3_to_shader(f"point_color[{idx}]", i.color, self.shader)
            glib.push_float_to_shader(f"point_strength[{idx}]", i.intensity, self.shader)
            glib.push_float_to_shader(f"point_atten[{idx}]", i.attenu, self.shader)
            glib.bind_vertex_buffer_object(self.vao)
            glib.draw_vertex_buffer_object(len(self.vmesh.indices)*4)

        glib.unbind_vertex_buffer_object()
        glib.unbind_shader()

    def init(self, vert_fpath: str, frag_fpath: str, texture_fpath, texture_scale: float, vmesh: Mesh, app) -> None:
        self.texture_scale = texture_scale
        if isinstance(texture_fpath, str):
            self.texture = glib.gen_texture_from_fpath(app.pointer, texture_fpath)
        else:
            self.texture = texture_fpath
        self.vmesh = vmesh
        self.shader = glib.gen_shader_program_s(app.pointer, vert_fpath, frag_fpath)
        self.vao = glib.gen_vertex_buffer_object(app.pointer, vmesh.positions, vmesh.indices, vmesh.uvs, vmesh.normals)

@dataclass
class PointLight:
    position: list = field(default_factory=lambda: [0.0, 0.0, 0.0])
    color: list = field(default_factory=lambda: [1.0, 1.0, 1.0])
    intensity: float = 1.0
    attenu: float = 0.5

    def init(self, intensity: float, attenuation: float) -> None:
        self.intensity = intensity
        self.attenu = attenuation
        pointlights.append(self)
    
    def destroy(self) -> None:
        pointlights.remove(self)

@dataclass
class App:
    pointer: id = 0
    deltatime: float = 0.0
    frame_start_time: float = 0.0
    frame_end_time: float = 0.0

    def init(self, width: int, height: int, title: str, resizable: bool, r: float, g: float, b: float) -> None:
        self.pointer = glib.create_app(width, height, title, resizable, r, g, b)
        light = PointLight()
        light.init(0.0, 0.0)
        
    def bind(self) -> None:
        self.frame_start_time = datetime.now()
        glib.bind_app(self.pointer)
        
    def unbind(self) -> None:
        glib.unbind_app(self.pointer)
        self.frame_end_time = datetime.now()
        self.deltatime = float(self.frame_end_time.timestamp() - self.frame_start_time.timestamp())

    def destroy(self) -> None:
        glib.destroy_app(self.pointer)