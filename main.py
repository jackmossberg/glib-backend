
"""

IF YOU CAN :
take a peek at the graphics.py file to learn some of the raw api calls
as most of the code here is abstracted away from the glib library for simplicity sake
PS - if you get any weird looking non-python errors please lmk, its probably not your fault

"""

# import glib
import glib
# a file that contains some abstractions from the raw glib (OpenGL) api
import graphics as g

# os for getting the file path
import os

def main():
    # get the global file path
    fpath : str = os.path.abspath(__file__).rstrip("main.py")   

    # spawn in the app object
    app = glib.create_app(1000, 900, "window", True, 0.6, 0.2, 0.29)

    # spawn in a 3D camera
    main_camera = g.Camera()
    # initalize the camera (fov, screen width, screen height) / set position
    main_camera.init(45.0, 1000, 900)
    main_camera.position = [0.0, 0.0, -5.0]

    # CubePrimitive is a container for the cubes vertex data (i abstracted it to the graphics file because its long and disracting as shit)
    cube = g.CubePrimitive()

    # spawn in a mesh object (this is just a container for the vertex data that is digestable for gapi)
    test_mesh = g.Mesh()
    test_mesh.init(cube.positions, cube.indices, cube.uvs, cube.normals)

    # spawn in the model object (requires a mesh, working shader files, and a reference to the app object)
    test_model = g.Model()  
    test_model.init(
        # replace these with the correct file paths
        # or just use 'import os' to automate this bc its like 2:00 AM and i don't feel like doing it myself
        fpath+"default.vert", 
        fpath+"default.frag",
        fpath+"/example_assets/DOOR_1E.PNG",
        test_mesh,
        app
    )

    model_2 = g.Model()
    model_2.init(
        fpath+"default.vert", 
        fpath+"default.frag",
        fpath+"/example_assets/DOOR_1A.PNG",
        test_mesh,
        app
    )

    model_2.position = [1.0, 0.0, 0.0]

    glib.depth_test(True)

    # the main update loop
    while not glib.should_app_close(app):
        # spin the model around
        test_model.rotation[0] += 0.15
        test_model.rotation[1] -= 0.25
        test_model.rotation[2] += 0.35

        # bind an app that we want to render to (in this case we only have one)
        glib.bind_app(app)
        # call the render function attached to the model object
        test_model.render(app, main_camera)
        model_2.render(app, main_camera)
        # unbind the app object from rendering
        glib.unbind_app(app)

    # preform deallocation operations for the app (ALWAYS do this before exiting the program)
    glib.destroy_app(app)

if __name__ == "__main__":
    main()