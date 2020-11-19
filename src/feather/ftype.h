#pragma once
#include <X11/Xutil.h>
#include <mutex>
#include <unistd.h>
#include <unordered_map>
namespace feather
{

    struct fwm_winfo
    {
        unsigned int depth;
        bool full_screen;
        bool has_changed;
        bool can_be_moved;
        bool can_be_resized;
        int on_dscreen;
        Display *w_display;
        Window frame;
    };
} // namespace feather