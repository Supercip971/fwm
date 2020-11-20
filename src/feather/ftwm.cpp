#include "ftwm.h"
#include "fwm.h"
#include <X11/X.h>
#include <X11/Xlib.h>
namespace feather
{
    void feather_tiling_manager::launch_tile_process(std::unordered_map<Window, fwm_winfo> *list)
    {

        this->list = list;

        process = std::thread(&feather_tiling_manager::main_process, this);
    }

    void windows_tiling_element::add_window(Window w)
    {
        window_list.push_back(w);
    }
    void windows_tiling_element::remove_window(Window w)
    {
        for (int i = 0; i < window_list.size(); i++)
        {
            if (window_list[i] == w)
            {
                window_list.erase(window_list.begin()+i);
                return;
            }
        }
    }
    void windows_tiling_element::update()
    {
        for (int i = 0; i < sub_elements.size(); i++)
        {
            sub_elements[i]->update();
        }
        for (int i = 0; i < window_list.size(); i++)
        {
            fwm_winfo *d = &dad->list->at(window_list[i]);
            if (d->has_changed)
            {
                update(window_list[i], d);
                d->has_changed = false;
            }
        }
    }
    void windows_tiling_element::add(windows_tiling_element *y)
    {
        sub_elements.push_back(y);
    }

    windows_tiling_element::windows_tiling_element()
    {
    }
    windows_tiling_element::windows_tiling_element(feather_tiling_manager *parent, window_tiling_elements_type t)
    {
        dad = parent;
        type = (int)t;
    }
    void windows_tiling_element::update(Window t, fwm_winfo *window)
    {
        if (window->full_screen == true)
        {
            // don't update
        }
        else
        {
            XMoveWindow(window->w_display, window->frame, window->next_x, window->next_y);
            XResizeWindow(window->w_display, window->frame, window->next_width, window->next_height);
            XResizeWindow(window->w_display, t, window->next_width, window->next_height);
        }
    }

    void feather_tiling_manager::update(fwm_winfo *window, const Window w)
    {

        if (window->full_screen == true)
        {

            XMoveWindow(window->w_display, window->frame, gaps, gaps);
            XMoveWindow(window->w_display, w, gaps, gaps);
            XResizeWindow(window->w_display, window->frame, XDisplayWidth(window->w_display, 0) - gaps * 2, XDisplayHeight(window->w_display, 0) - gaps * 2);
            XResizeWindow(window->w_display, w, XDisplayWidth(window->w_display, 0) - gaps * 2, XDisplayHeight(window->w_display, 0) - gaps * 2);
        }
        else
        {
            XMoveWindow(window->w_display, window->frame, window->next_x, window->next_y);
            XResizeWindow(window->w_display, window->frame, window->next_width, window->next_height);
            XResizeWindow(window->w_display, w, window->next_width, window->next_height);
        }
    }
    void feather_tiling_manager::main_process()
    {

        feather::main_mutex.lock();
        main_telement = new windows_tiling_element(this, WINDOW_WIDTH);
        feather::main_mutex.unlock();
        while (true)
        {
            for (std::pair<const Window, fwm_winfo> &d : *list)
            {
                feather::main_mutex.lock();
                if (d.second.created)
                {
                    main_telement->add_window(d.first);
                    d.second.created = false;
                }
                main_telement->update();
                
                feather::main_mutex.unlock();
            }

            usleep(10);
        }
    }

} // namespace feather