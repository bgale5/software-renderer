#ifndef VIEWER_HPP
#define VIEWER_HPP

extern bool z_buffer_test_switch;
extern bool shade_test_switch;
extern bool clip_test_switch;
extern bool polygon_test_switch;

void init_viewer(int argc, char **argv);
void load_compass();
void load_user_object(std::string vjs_path);
void clip_test();
void shade_test();
void z_buffer_test();
void polygon_test();

#endif