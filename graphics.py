import numpy as np
import glib
from dataclasses import dataclass, field

@dataclass
class CubePrimitive:
    positions: list = field(default_factory=lambda: np.array([
        -0.5, -0.5,  0.5,
        0.5, -0.5,  0.5,
        0.5,  0.5,  0.5,
        -0.5,  0.5,  0.5,
        -0.5, -0.5, -0.5,
        0.5, -0.5, -0.5,
        0.5,  0.5, -0.5,
        -0.5,  0.5, -0.5,
        -0.5, -0.5, -0.5,
        -0.5, -0.5,  0.5,
        -0.5,  0.5,  0.5,
        -0.5,  0.5, -0.5,
        0.5, -0.5,  0.5,
        0.5, -0.5, -0.5,
        0.5,  0.5, -0.5,
        0.5,  0.5,  0.5,
        -0.5,  0.5,  0.5,
        0.5,  0.5,  0.5,
        0.5,  0.5, -0.5,
        -0.5,  0.5, -0.5,
        -0.5, -0.5, -0.5,
        0.5, -0.5, -0.5,
        0.5, -0.5,  0.5,
        -0.5, -0.5,  0.5], 
        dtype=np.float32)
    )
    indices: list = field(default_factory=lambda: np.array([ 
        0,  1,  2,  0,  2,  3,
        4,  6,  5,  4,  7,  6,
        8,  9, 10,  8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23],
        dtype=np.uint32)
    )
    uvs: list = field(default_factory=lambda: np.array([
        0.0, 0.0,  1.0, 0.0,  1.0, 1.0,  0.0, 1.0,
        0.0, 0.0,  1.0, 0.0,  1.0, 1.0,  0.0, 1.0,
        0.0, 0.0,  1.0, 0.0,  1.0, 1.0,  0.0, 1.0,
        0.0, 0.0,  1.0, 0.0,  1.0, 1.0,  0.0, 1.0,
        0.0, 0.0,  1.0, 0.0,  1.0, 1.0,  0.0, 1.0,
        0.0, 0.0,  1.0, 0.0,  1.0, 1.0,  0.0, 1.0], 
        dtype=np.float32)
    )
    normals: list = field(default_factory=lambda: np.array([
        0.0, 0.0, 1.0,  0.0, 0.0, 1.0,  0.0, 0.0, 1.0,  0.0, 0.0, 1.0,
        0.0, 0.0, -1.0,  0.0, 0.0, -1.0,  0.0, 0.0, -1.0,  0.0, 0.0, -1.0,
        -1.0, 0.0, 0.0,  -1.0, 0.0, 0.0,  -1.0, 0.0, 0.0,  -1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,  1.0, 0.0, 0.0,  1.0, 0.0, 0.0,  1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,  0.0, 1.0, 0.0,  0.0, 1.0, 0.0,  0.0, 1.0, 0.0,
        0.0, -1.0, 0.0,  0.0, -1.0, 0.0,  0.0, -1.0, 0.0,  0.0, -1.0, 0.0],
        dtype=np.float32)
    )

@dataclass
class Mesh:
    positions: list = field(default_factory=list)
    indices: list = field(default_factory=list)
    uvs: list = field(default_factory=list)
    normals: list = field(default_factory=list)

    def init(self, positions, indices, uvs, normals):
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

    def init(self, fovy: float, window_width: float, window_height: float):
        self.fovy = fovy
        self.window_width = window_width
        self.window_height = window_height

@dataclass
class Model:
    position: list = field(default_factory=lambda: [0.0, 0.0, 0.0])
    rotation: list = field(default_factory=lambda: [0.0, 0.0, 0.0])
    scale: list = field(default_factory=lambda: [1.0, 1.0, 1.0])
    vao: int = 0
    shader: int = 0
    texture: int = 0
    vmesh: Mesh = field(default_factory=Mesh)

    def render(self, camera: Camera):
        projection_matrix = glib.projection_matrix4x4(camera.fovy, camera.window_width, camera.window_height)
        view_matrix = glib.view_matrix4x4(np.array(camera.position, dtype=np.float32), 
                                         np.array(camera.rotation, dtype=np.float32))
        transform_matrix = glib.transform_matrix4x4(np.array(self.position, dtype=np.float32), 
                                                  np.array(self.rotation, dtype=np.float32), 
                                                  np.array(self.scale, dtype=np.float32))

        glib.bind_shader(self.shader)
        glib.push_texture2D_to_shader("tex0", self.texture, self.shader)
        glib.push_matrix4x4_to_shader("transform_matrix", transform_matrix.flatten().tolist(), self.shader)
        glib.push_matrix4x4_to_shader("view_matrix", view_matrix.flatten().tolist(), self.shader)
        glib.push_matrix4x4_to_shader("projection_matrix", projection_matrix.flatten().tolist(), self.shader)
        glib.bind_vertex_buffer_object(self.vao)
        glib.draw_vertex_buffer_object(len(self.vmesh.indices)*4)
        glib.unbind_vertex_buffer_object()
        glib.unbind_shader()

    def init(self, vert_fpath: str, frag_fpath: str, texture_fpath: str, vmesh: Mesh, app):
        self.texture = glib.gen_texture_from_fpath(app, texture_fpath)
        self.vmesh = vmesh
        self.shader = glib.gen_shader_program_s(app, vert_fpath, frag_fpath)
        self.vao = glib.gen_vertex_buffer_object(app, vmesh.positions, vmesh.indices, vmesh.uvs, vmesh.normals)