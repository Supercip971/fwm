#pragma once

#include "ftwm.h"
#include "ftype.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <mutex>
#include <unistd.h>
#include <unordered_map>
namespace feather
{

    extern std::mutex main_mutex;
    struct pos
    {
        int x;
        int y;
    };

    class fwm
    {
    public:
        std::unordered_map<Window, fwm_winfo> frame_list;
        static fwm *the();
        void setup_window_key(Window wid);
        void frame_window(Window wid);
        void init_top_window();
        void init();
        void run();
        void exit();
        int on_error_x(Display *d, XErrorEvent *event);
        int on_wm_detected(Display *d, XErrorEvent *event);

        bool create_event(const XConfigureRequestEvent the_event);
        bool map_request_event(const XMapRequestEvent event);
        bool unmap_request_event(const XUnmapEvent &event);
        bool on_motion_event(const XMotionEvent &event);
        bool on_button_event(const XButtonEvent &event);
        bool interpret_event(XEvent the_event);
        bool on_key_press(const XKeyEvent &event);

    private:
        unsigned int mod_code = Mod4Mask;              // windows
        unsigned int move_button = Button1;            // left click
        unsigned int move_button_mask = Button1Mask;   // left click
        unsigned int resize_button = Button3;          // right click
        unsigned int resize_button_mask = Button3Mask; // left click
        unsigned int kill_window_key = XK_F4;          // f4
        unsigned int switch_window = XK_Tab;
        unsigned int window_full_screen = XK_space;
        KeyCode window_full_screen_raw;
        Display *current_display;
        Window main_window;
        pos last_mouse_click;
        XWindowAttributes last_focused_window;
        feather_tiling_manager ftm;
    };

    inline int son_error_x(Display *d, XErrorEvent *event)
    {
        return fwm::the()->on_error_x(d, event);
    };
    inline int son_wm_detected(Display *d, XErrorEvent *event)
    {
        return fwm::the()->on_wm_detected(d, event);
    };

} // namespace feather