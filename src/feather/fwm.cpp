#include "fwm.h"
#include "utility.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#include <X11/keysym.h>

#include <memory>
#include <mutex>
#include <stdlib.h>

namespace feather
{

    std::mutex main_mutex;
    const char *event_string_list[] = {
        "null",
        "null",
        "key press",
        "key release",
        "button press",
        "button release",
        "enter notify",
        "leave notify",
        "focus in",
        "focus out",
        "keymap notify",
        "expose",
        "graphics expose",
        "no expose",
        "visibility notify",
        "create notify",
        "destroy notify",
        "unmap notify",
        "map notify",
        "map request",
        "reparent notify",
        "configure notify",
        "configure request",
        "null",
        "null",
        "null",
        "null",
        "null",
        "null",
        "null",
        "null",
        "null",
        "null",
        "null",
        "null",
        "null",
        "null",
        "null",
        "null",
        "null",
    };
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
        ftm.display = current_display;
        ftm.launch_tile_process(&frame_list);
        window_full_screen_raw = XKeysymToKeycode(current_display, window_full_screen);
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
        XGrabKey(
            current_display,
            XKeysymToKeycode(current_display, window_full_screen),
            mod_code,
            wid,
            false,
            GrabModeAsync,
            GrabModeAsync);
    }
    void fwm::frame_window(Window wid)
    {
        int b_width = 2;
        if (frame_list.count(wid))
        {
            return; // already framed
        }
        const unsigned long BORDER_COLOR = 0x000000;
        const unsigned long BG_COLOR = 0x000000;
        XWindowAttributes x_attributes;
        XGetWindowAttributes(current_display, wid, &x_attributes);

        Window on_top = XCreateSimpleWindow(current_display, main_window, x_attributes.x, x_attributes.y, x_attributes.width, x_attributes.height, b_width, BORDER_COLOR, BG_COLOR);

        XSelectInput(current_display, on_top, SubstructureRedirectMask | SubstructureNotifyMask);
        XAddToSaveSet(current_display, wid);
        XReparentWindow(current_display, wid, on_top, 0, 0);
        XMapWindow(current_display, on_top);
        frame_list[wid].frame = on_top;
        frame_list[wid].can_be_moved = true;
        frame_list[wid].can_be_resized = true;
        frame_list[wid].has_changed = true;
        frame_list[wid].w_display = current_display;
        frame_list[wid].created = true;

        frame_list[wid].next_x = x_attributes.x;
        frame_list[wid].next_y = x_attributes.y;
        frame_list[wid].next_width = x_attributes.width;
        frame_list[wid].next_height = x_attributes.height;
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
        Window frame = frame_list[event.window].frame;
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
            XConfigureWindow(current_display, frame_list[the_event.window].frame, the_event.value_mask, &end_window);
        }
        XConfigureWindow(current_display, the_event.window, the_event.value_mask, &end_window);
        return true;
    }

    bool fwm::on_key_press(const XKeyEvent &event)
    {
        if (event.keycode == window_full_screen_raw)
        {
            context.log("event %x \n", event.keycode);
            if (frame_list[event.window].full_screen)
            {
                frame_list[event.window].full_screen = false;
                frame_list[event.window].can_be_moved = true;
            }
            else
            {
                frame_list[event.window].full_screen = true;
                frame_list[event.window].can_be_moved = false;
            }

            frame_list[event.window].has_changed = true;
        }
        else
        {

            context.log("nevent %x \n", event.keycode);
            context.log("nstate %x \n", event.state);
            context.log("should %x \n", window_full_screen_raw);
        }
        return true;
    }
    bool fwm::on_motion_event(const XMotionEvent &event)
    {
        if (!frame_list.count(event.window))
        {
            return true;
        }
        frame_list[event.window].has_changed = true;
        const Window frame = frame_list[event.window].frame;
        pos final = {event.x_root, event.y_root};
        final.x -= last_mouse_click.x;
        final.y -= last_mouse_click.y;
        if (event.state & move_button_mask)
        {

            if (!frame_list[event.window].can_be_moved)
            {
                return true;
            }
            final.x += last_focused_window.x;
            final.y += last_focused_window.y;
            frame_list[event.window].next_x = final.x;
            frame_list[event.window].next_y = final.y;
            //     XMoveWindow(current_display, frame, final.x, final.y);
        }
        else if (event.state & resize_button_mask)
        {

            if (!frame_list[event.window].can_be_resized)
            {
                return true;
            }
            pos resize_change = {
                std::max(final.x + last_focused_window.width, 1),
                std::max(final.y + last_focused_window.height, 1)};
            pos dest_new_size = {resize_change.x, resize_change.y};

            frame_list[event.window].next_width = resize_change.x;
            frame_list[event.window].next_height = resize_change.y;
            //       XResizeWindow(current_display, event.window, dest_new_size.x, dest_new_size.y);
            //       XResizeWindow(current_display, frame, dest_new_size.x, dest_new_size.y);
        }
        else
        {
            context.generate_error("not valid event motion received :^( %i", event.state);
        }
        return true;
    }

    bool fwm::on_button_event(const XButtonEvent &event)
    {
        if (!frame_list.count(event.window))
        {
            return true;
        }

        const Window targeted = frame_list[event.window].frame;
        last_mouse_click = {event.x_root, event.y_root};
        XRaiseWindow(current_display, targeted);
        XGetWindowAttributes(current_display, targeted, &last_focused_window);
        return true;
    }
    bool fwm::interpret_event(XEvent the_event)
    {
        bool result = false;
        switch (the_event.type)
        {
        case KeyPress:
            result = on_key_press(the_event.xkey);
            break;
        case ConfigureRequest:
            result = create_event(the_event.xconfigurerequest);
            break;
        case MapRequest:
            result = map_request_event(the_event.xmaprequest);
            break;
        case UnmapNotify:
            result = unmap_request_event(the_event.xunmap);
            break;
        case ButtonRelease:
            result = true;
            break;
        case ConfigureNotify:
            result = true;
            break;
        case MotionNotify:
            while (XCheckTypedWindowEvent(current_display, the_event.xmotion.window, MotionNotify, &the_event))
            {
                // skip pending operation
            }
            result = on_motion_event(the_event.xmotion);
            break;
        case ButtonPress:
            result = on_button_event(the_event.xbutton);
            break;
        default:
            break;
        }

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

            main_mutex.lock();
            XEvent current_event;
            XNextEvent(current_display, &current_event);

            context.log("receive event : %i ", current_event.type);
            if (!interpret_event(current_event))
            {
                exit();
            }
            main_mutex.unlock();
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
            context.generate_error("unknown error while loading %i\n", event->error_code);
            exit();
        }
        return 0;
    }
}; // namespace feather
