#pragma once

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <unistd.h>
#include <unordered_map>
namespace feather
{

    class fwm
    {
    public:
        std::unordered_map<Window, Window> frame_list;
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
        bool interpret_event(const XEvent the_event);

    private:
        unsigned int mod_code = Mod1Mask;     // alt
        unsigned int move_button = Button1;   // left click
        unsigned int resize_button = Button3; // right click
        unsigned int kill_window_key = XK_F4; // f4
        unsigned int switch_window = XK_Tab;
        Display *current_display;
        Window main_window;
    };

    static int son_error_x(Display *d, XErrorEvent *event)
    {
        return fwm::the()->on_error_x(d, event);
    };
    static int son_wm_detected(Display *d, XErrorEvent *event)
    {
        return fwm::the()->on_wm_detected(d, event);
    };

} // namespace feather