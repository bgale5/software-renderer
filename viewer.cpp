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
    std::string compass_path = "compass_point.vjs";
    Object compass;
    Object_attribs properties;
    properties.scale = 0.1; // arbitrary compass size
    properties.centre.x = FRAME_WIDE - 30;
    properties.centre.y = FRAME_HIGH - 30;
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
}

void z_buffer_test()
{
    z_buffer_test_switch = true;
}

void polygon_test()
{
    polygon_test_switch = true;
}