#pragma once
#include <X11/Xutil.h>
#include <mutex>
#include <unistd.h>
#include <unordered_map>
namespace feather
{

    struct fwm_winfo
    {
        bool created;
        unsigned int depth;
        bool full_screen;
        bool has_changed;
        bool can_be_moved;
        bool can_be_resized;
        int on_dscreen;
        Display *w_display;
        Window frame;

        unsigned int next_x = 0;
        unsigned int next_y = 0;
        unsigned int next_width = 0;
        unsigned int next_height = 0;

        int add_pos = 0;

        int add_size = 0;
    };
} // namespace feather