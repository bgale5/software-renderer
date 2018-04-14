#include <cstdlib>
#include <cstdio>
#include <string>
#include "engine.hpp"
#include "viewer.hpp"


//============= GLOBAL VARIABLES ===========================//
bool z_buffer_test_switch = false;
bool shade_test_switch = false;
bool clip_test_switch = false;
bool polygon_test_switch = false;

// ============= VIEWER FUNCTIONS ==========================//

void init_viewer(int argc, char **argv)
{
    if (argc != 2) {
        printf("Viewer takes single argument: vjs path or demo name\n");
        exit(0);
    }
    std::string vjs_path = argv[1];
    if (vjs_path.compare("clip") == 0)
        clip_test();
    else if (vjs_path.compare("smoothshading") == 0)
        shade_test();
    else if(vjs_path.compare("z_buffer") == 0)
        z_buffer_test();
    else if (vjs_path.compare("polygon") == 0)
        polygon_test();
    // TODO: check for demo cases
    else {
        load_compass();
        load_user_object(vjs_path);
    }
}

void load_compass()
{
    std::string compass_path = "compass.vjs";
    Object compass;
    Object_attribs properties;
    properties.scale = 0.5; // arbitrary compass size
    properties.centre.x = 50;
    properties.centre.y = 50;
    properties.centre.z = 0;
    properties.fixed_location = true;
    properties.fixed_orientation = false;
    load_vjs(compass_path, compass, properties);
    world_objects.push_back(compass); // Compass should always be index 0; call this first
}

void load_user_object(std::string vjs_path)
{
    Object model;
    Object_attribs properties;
    properties.centre.x = FRAME_WIDE / 2;
    properties.centre.y = FRAME_HIGH / 2;
    properties.centre.z = 0;
    properties.scale = 1.0;
    properties.fixed_location = false;
    properties.fixed_orientation = false;
    load_vjs(vjs_path, model, properties);
    world_objects.push_back(model);
}

void clip_test()
{
    clip_test_switch = true;
}

void shade_test()
{
    shade_test_switch = true;
    Object triangle;
    object_attribs properties;
    properties.centre.x = FRAME_WIDE / 2;
    properties.centre.y = FRAME_HIGH / 2;
    properties.centre.z = 0;
    properties.scale = 1;
    properties.visible = true;
    properties.fixed_location = true;
    load_vjs("triangle.vjs", triangle, properties);
    world_objects.push_back(triangle); 
}

void z_buffer_test()
{
    z_buffer_test_switch = true;
    Object static_cube;
    Object free_cube;
    Object_attribs props;
    load_vjs("cube.vjs", static_cube, props);
    load_vjs("cube.vjs", free_cube, props);
    static_cube.properties.centre.x = FRAME_WIDE / 2 - 300;
    static_cube.properties.centre.y = FRAME_HIGH / 2;
    static_cube.properties.centre.z = 500;
    static_cube.properties.fixed_location = true;
    static_cube.properties.fixed_orientation = true;
    static_cube.properties.visible = true;
    static_cube.properties.scale = 1;
    free_cube.properties.centre.x = FRAME_WIDE / 2 - 300;
    free_cube.properties.centre.y = FRAME_HIGH / 2;
    free_cube.properties.centre.z = -100;
    free_cube.properties.fixed_location = false;
    free_cube.properties.fixed_orientation = false;
    free_cube.properties.visible = true;
    free_cube.properties.scale = 1;
    world_objects.push_back(static_cube);
    world_objects.push_back(free_cube);
}

void polygon_test()
{
    polygon_test_switch = true;
}