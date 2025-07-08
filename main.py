import glib
import graphics as gapi
import os
from datetime import datetime, timedelta
from dataclasses import dataclass, field
import numpy as np

def generate_icosphere(vert_fpath: str, frag_fpath: str, texture_fpath: str, texture_scale: float, scale: float, resolution: int, app: gapi.App) -> gapi.Model:
    t = (1.0 + np.sqrt(5.0)) / 2.0
    positions = [
        [-1, t, 0], [1, t, 0], [-1, -t, 0], [1, -t, 0],
        [0, -1, t], [0, 1, t], [0, -1, -t], [0, 1, -t],
        [t, 0, -1], [t, 0, 1], [-t, 0, -1], [-t, 0, 1]
    ]
    
    positions = [p / np.linalg.norm(p) * scale for p in positions]
    
    indices = [
        0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11,
        1, 5, 9, 5, 11, 4, 11, 10, 2, 10, 7, 6, 7, 1, 8,
        3, 9, 4, 3, 4, 2, 3, 2, 6, 3, 6, 8, 3, 8, 9,
        4, 9, 5, 2, 4, 11, 6, 2, 10, 8, 6, 7, 9, 8, 1
    ]
    
    if resolution > 0:
        edge_midpoints = {}
        new_indices = []
        
        def get_midpoint(v1, v2):
            key = tuple(sorted([v1, v2]))
            if key not in edge_midpoints:
                p1, p2 = positions[v1], positions[v2]
                mid = [(p1[i] + p2[i]) / 2 for i in range(3)]
                mid = mid / np.linalg.norm(mid) * scale
                edge_midpoints[key] = len(positions)
                positions.append(mid)
            return edge_midpoints[key]
        
        for i in range(0, len(indices), 3):
            v1, v2, v3 = indices[i:i+3]
            
            if resolution == 1:
                m1 = get_midpoint(v1, v2)
                m2 = get_midpoint(v2, v3)
                m3 = get_midpoint(v3, v1)
                
                new_indices.extend([
                    v1, m1, m3,
                    v2, m2, m1,
                    v3, m3, m2,
                    m1, m2, m3
                ])
            else:
                new_indices.extend([v1, v2, v3])
        indices = new_indices

        for _ in range(resolution - 1):
            edge_midpoints = {}
            new_indices = []
            for i in range(0, len(indices), 3):
                v1, v2, v3 = indices[i:i+3]
                m1 = get_midpoint(v1, v2)
                m2 = get_midpoint(v2, v3)
                m3 = get_midpoint(v3, v1)
                
                new_indices.extend([
                    v1, m1, m3,
                    v2, m2, m1,
                    v3, m3, m2,
                    m1, m2, m3
                ])
            indices = new_indices
    
    uvs = []
    for pos in positions:
        u = 0.5 + np.arctan2(pos[2], pos[0]) / (2 * np.pi)
        v = 0.5 - np.arcsin(pos[1] / scale) / np.pi
        uvs.append([u, v])
    
    normals = [p / np.linalg.norm(p) for p in positions]
    
    positions = np.array(positions, dtype=np.float32).flatten()
    indices = np.array(indices, dtype=np.uint32)
    uvs = np.array(uvs, dtype=np.float32).flatten()
    normals = np.array(normals, dtype=np.float32).flatten()
    
    mesh = gapi.Mesh()
    mesh.init(positions, indices, uvs, normals)
    
    model = gapi.Model()
    model.init(vert_fpath, frag_fpath, texture_fpath, texture_scale, mesh, app)
    
    return model

def generate_plane(x : int, y : int, subdev : int, fpath : str, tfpath : str, tscale : float, app) -> gapi.Model:
    step_x = x / subdev
    step_y = y / subdev
    
    positions = []
    uvs = []
    normals = []
    indices = []

    for i in range(subdev + 1):
        for j in range(subdev + 1):
            pos_x = -x/2 + i * step_x
            pos_y = -y/2 + j * step_y
            positions.extend([pos_x, 0, pos_y])

            u = i / subdev
            v = j / subdev
            uvs.extend([u, v])

            normals.extend([0.0, 1.0, 0.0])
    for i in range(subdev):
        for j in range(subdev):
            t_left = i * (subdev + 1) + j
            t_right = t_left + 1
            b_left = (i + 1) * (subdev + 1) + j
            b_right = b_left + 1

            indices.extend([t_left, b_left, t_right])
            indices.extend([t_right, b_left, b_right])

    positions = np.array(positions, dtype=np.float32)
    indices = np.array(indices, dtype=np.uint32)
    uvs = np.array(uvs, dtype=np.float32)
    normals = np.array(normals, dtype=np.float32)

    mesh : gapi.Mesh = gapi.Mesh()
    mesh.init(positions=positions, indices=indices, uvs=uvs, normals=normals)

    model : gapi.Model = gapi.Model()
    model.init(
        fpath+"default.vert", 
        fpath+"default.frag",
        fpath+tfpath,
        tscale,
        mesh,
        app
    )

    return model

@dataclass
class Planet:
    model : gapi.Model = field(default_factory=gapi.Model)

    position : list = field(default_factory=lambda: [0.0, 0.0, 0.0])
    rotation : list = field(default_factory=lambda: [0.0, 0.0, 0.0])

    radius : float = 1.0
    mass : float = 1.0

    gscale : float = 1.0

    def init(self, app, radius : float, mass : float, gscale : float):
        fpath : str = os.path.abspath(__file__).rstrip("main.py")
        self.model = generate_icosphere(
            vert_fpath=fpath+"planetsurface.vert",
            frag_fpath=fpath+"planetsurface.frag",
            texture_fpath="",
            texture_scale=3.0,
            scale=radius/2,
            resolution=int(radius) - 1,
            app=app
        )
        self.mass = mass
        self.gscale = gscale
        self.radius = radius

    def render(self, app, main_camera : gapi.Camera):
        self.model.render(app=app, camera=main_camera)

    def update(self, app):
        self.rotation[1] += 10.3 / self.radius * app.deltatime

        self.model.position = self.position
        self.model.rotation = self.rotation

        self.model.scale[0] = self.radius
        self.model.scale[1] = self.radius
        self.model.scale[2] = self.radius

    def destroy(self):
        # spawn plant 'fragments'
        # de-spawn self
        
        pass

@dataclass
class Star:
    def init():
        pass
    def update():
        pass
    def destroy():
        pass

def create_skybox(fpath : str, texture_scale, tfpath, app) -> gapi.Model:
    cube = gapi.CubePrimitive()
    mesh = gapi.Mesh()
    mesh.init(cube.positions, cube.indices, cube.uvs, cube.normals)

    model = gapi.Model()
    model.init(
        fpath+"skybox_1/sky.vert", 
        fpath+"skybox_1/sky.frag",
        fpath+tfpath,
        texture_scale,
        mesh,
        app
    )

    return model

def render_skybox(app, model : gapi.Model, camera : gapi.Camera):
    model.position = [-camera.position[0], camera.position[1], -camera.position[2]]
    glib.depth_test(False)
    model.render(camera=camera, app=app)
    glib.depth_test(True)

def main():
    fpath : str = os.path.abspath(__file__).rstrip("main.py")   
    app : gapi.App = gapi.App();
    app.init(1000, 900, "build", True, 0.6, 0.2, 0.29)

    main_camera = gapi.Camera()
    main_camera.init(90.0, 1000, 900)
    main_camera.position = [0.0, 20.0, -20.0]
    main_camera.rotation = [45.0, 0.0, 0.0]

    test_light : gapi.PointLight = gapi.PointLight()
    test_light.init(50.0, 0.5)
    test_light.position = [10.0, 5.0, 10.0]
    test_light.color = [1.0, 0.7, 1.0]

    test_planet : Planet = Planet()
    test_planet.init(app=app, radius=2.6, mass=1.0, gscale=1.2)

    test_moon : Planet = Planet()
    test_moon.init(app=app, radius=1.6, mass=0.2, gscale=0.67)
    test_moon.position = [15.0, 0.0, 0.0]

    main_skybox = create_skybox(fpath=fpath, app=app, tfpath="common_assets/dev_textures/Dark/texture_02.png", texture_scale=1.0)
    #main_skybox = create_skybox(fpath=fpath, app=app, tfpath="skybox_1/front.png", texture_scale=1.0)

    while not glib.should_app_close(app.pointer):
        test_moon.update(app=app)
        test_planet.update(app=app)

        # ----------------------------------------------
        # ^^^ app pysics calculated before rendering ^^^
        app.bind()
        render_skybox(app, main_skybox, main_camera)
        # start render here...
        # --------------------

        test_moon.render(app=app, main_camera=main_camera)
        test_planet.render(app=app, main_camera=main_camera)
        
        # --------------------
        # end render here...
        app.unbind()

    app.destroy()

if __name__ == "__main__":
    main()