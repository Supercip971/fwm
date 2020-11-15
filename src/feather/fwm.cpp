#include "fwm.h"
#include "utility.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <memory>
#include <stdlib.h>

namespace feather
{
    fwm main_fwm;
    fwm *fwm::the()
    {
        return &main_fwm;
    }
    void fwm::init()
    {
        context.log("loading FWM ... \n");
        current_display = XOpenDisplay(0);

        if (current_display == nullptr)
        {
            context.generate_error("FWM can't open display ! \n");
            std::exit(-1);
        }
        else
        {
            context.log("loading display %s", XDisplayName(nullptr));
        }

        main_window = DefaultRootWindow(current_display);
    }

    void fwm::setup_window_key(Window wid)
    {
        XGrabButton(
            current_display,
            move_button,
            mod_code,
            wid,
            false,
            ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
            GrabModeAsync,
            GrabModeAsync,
            None,
            None);
        XGrabButton(
            current_display,
            resize_button,
            mod_code,
            wid,
            false,
            ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
            GrabModeAsync,
            GrabModeAsync,
            None,
            None);
        XGrabKey(
            current_display,
            XKeysymToKeycode(current_display, kill_window_key),
            mod_code,
            wid,
            false,
            GrabModeAsync,
            GrabModeAsync);
        XGrabKey(
            current_display,
            XKeysymToKeycode(current_display, switch_window),
            mod_code,
            wid,
            false,
            GrabModeAsync,
            GrabModeAsync);
    }
    void fwm::frame_window(Window wid)
    {
        if (frame_list.count(wid))
        {
            return; // already framed
        }
        const unsigned long BORDER_COLOR = 0xffffff;
        const unsigned long BG_COLOR = 0x000000;
        XWindowAttributes x_attributes;
        XGetWindowAttributes(current_display, wid, &x_attributes);

        Window on_top = XCreateSimpleWindow(current_display, main_window, x_attributes.x, x_attributes.y, x_attributes.width, x_attributes.height, b_width, BORDER_COLOR, BG_COLOR);

        XSelectInput(current_display, on_top, SubstructureRedirectMask | SubstructureNotifyMask);
        XAddToSaveSet(current_display, wid);
        XReparentWindow(current_display, wid, on_top, 0, 0);
        XMapWindow(current_display, on_top);
        frame_list[wid] = on_top;
        setup_window_key(wid);
    }
    bool fwm::map_request_event(const XMapRequestEvent event)
    {
        frame_window(event.window);

        XMapWindow(current_display, event.window);

        return true;
    }
    bool fwm::unmap_request_event(const XUnmapEvent &event)
    {
        if (!frame_list.count(event.window))
        {
            context.log("skipping unmap window request for window %i", event.window);
            return true;
        }
        if (event.event == main_window)
        {
            return true;
        }
        Window frame = frame_list[event.window];
        XUnmapWindow(current_display, frame);
        XReparentWindow(current_display, event.window, main_window, 0, 0);
        XRemoveFromSaveSet(current_display, event.window);
        XDestroyWindow(current_display, frame);
        frame_list.erase(event.window);
        context.log("unframed %i", event.window);

        return true;
    }
    bool fwm::create_event(const XConfigureRequestEvent the_event)
    {

        context.log("creating window %i", the_event.window);
        context.log("framed %i", the_event.window);
        XWindowChanges end_window;
        end_window.x = the_event.x;
        end_window.y = the_event.y;
        end_window.width = the_event.width;
        end_window.height = the_event.height;
        end_window.border_width = the_event.border_width;
        end_window.sibling = the_event.above;
        end_window.stack_mode = the_event.detail;

        if (frame_list.count(the_event.window))
        {
            XConfigureWindow(current_display, frame_list[the_event.window], the_event.value_mask, &end_window);
        }
        XConfigureWindow(current_display, the_event.window, the_event.value_mask, &end_window);
        return true;
    }
    bool fwm::interpret_event(const XEvent the_event)
    {

        switch (the_event.type)
        {
        case ConfigureRequest:
            return create_event(the_event.xconfigurerequest);
            break;
        case MapRequest:
            return map_request_event(the_event.xmaprequest);
            break;
        case UnmapNotify:
            return unmap_request_event(the_event.xunmap);
            break;
        default:
            break;
        }

        context.generate_error("unhandled event : %i ", the_event.type);
        return true;
    }
    void fwm::init_top_window()
    {
        XGrabServer(current_display);

        context.log("loading top window");
        Window returned_root, returned_parent;
        Window *top_level_windows;
        unsigned int number_of_top_windows;
        XQueryTree(current_display, main_window, &returned_root, &returned_parent, &top_level_windows, &number_of_top_windows);
        context.log("top window count %i", number_of_top_windows);

        for (int i = 0; i < number_of_top_windows; i++)
        {
            frame_window(top_level_windows[i]);
        }
        XFree(top_level_windows);

        XUngrabServer(current_display);
    }
    void fwm::run()
    {
        context.log("running FWM ... ");
        XSetErrorHandler(son_wm_detected);
        XSelectInput(current_display, main_window, SubstructureRedirectMask | SubstructureNotifyMask);
        XSync(current_display, false);
        // if there is an wm already running it will call on_error_x

        XSetErrorHandler(son_error_x);

        while (true)
        {

            XEvent current_event;
            XNextEvent(current_display, &current_event);

            context.log("receive event : %i ", current_event.type);
            if (!interpret_event(current_event))
            {
                exit();
            }
            usleep(10);
        }
    }
    void fwm::exit()
    {
        context.log("exiting FWM ... ");
        XCloseDisplay(current_display);

        context.close();
        std::exit(1);
    }
    int fwm::on_error_x(Display *d, XErrorEvent *event)
    {
        if (event->error_code == BadWindow)
        {
            context.generate_error("Bad window error %i", event->error_code);
        }
        else
        {
            context.generate_error("unknown error %i", event->error_code);
        }
        exit();
        return 0;
    }
    int fwm::on_wm_detected(Display *d, XErrorEvent *event)
    {

        if (event->error_code == BadAccess)
        {
            context.generate_error("an window manager is already running \n");
            exit();
        }
        else
        {
            context.generate_error("unknown error while loading %i(\n", event->error_code);
            exit();
        }
        return 0;
    }
}; // namespace feather
