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
        for (size_t i = 0; i < window_list.size(); i++)
        {
            if (window_list[i] == w)
            {
                window_list.erase(window_list.begin() + i);
                return;
            }
        }
    }
    void windows_tiling_element::update()
    {
        for (size_t i = 0; i < sub_elements.size(); i++)
        {
            sub_elements[i]->update();
        }
        bool has_one_changed = false;
        for (size_t i = 0; i < window_list.size(); i++)
        {
            fwm_winfo *d = &dad->list->at(window_list[i]);
            if (d->has_changed)
            {
                has_one_changed = true;
                break;
            }
        }
        if (!has_one_changed)
        {
            return;
        }
        for (size_t i = 0; i < window_list.size(); i++)
        {
            fwm_winfo *d = &dad->list->at(window_list[i]);
            update(window_list[i], d, i);
            d->has_changed = false;
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
        x = 0;
        y = 0;
        width = XDisplayWidth(dad->display, 0);
        height = XDisplayHeight(dad->display, 0);
    }
    windows_tiling_element::windows_tiling_element(feather_tiling_manager *parent, window_tiling_elements_type t, int ix, int iy, int iwidth, int iheight)
    {
        x = ix;
        y = iy;
        width = iwidth;
        height = iheight;
        dad = parent;
        type = (int)t;
    }
    void windows_tiling_element::update(Window t, fwm_winfo *window, int id)
    {
        if (window->full_screen == true)
        {
            //   XMoveWindow(window->w_display, window->frame, window->next_x, window->next_y);
            const int count = window_list.size() + sub_elements.size();

            if (type == WINDOW_HEIGHT)
            {
                const unsigned int fy = (this->height / count) * id;
                const unsigned int fx = this->x;
                const unsigned int fwidth = this->width;
                const unsigned int fheight = this->height / count;
                XMoveWindow(window->w_display, window->frame, fx, fy);
                XResizeWindow(window->w_display, window->frame, fwidth, fheight);
                XResizeWindow(window->w_display, t, fwidth, fheight);
            }
            else if (type == WINDOW_WIDTH)
            {
                const unsigned int fx = (this->width / count) * id;
                const unsigned int fy = this->y;
                const unsigned int fwidth = this->width / count;
                const unsigned int fheight = this->height;
                XMoveWindow(window->w_display, window->frame, fx, fy);
                XResizeWindow(window->w_display, window->frame, fwidth, fheight);
                XResizeWindow(window->w_display, t, fwidth, fheight);
            }
        }
        else
        {
            XMoveWindow(window->w_display, window->frame, window->next_x, window->next_y);
            XResizeWindow(window->w_display, window->frame, window->next_width, window->next_height);
            XResizeWindow(window->w_display, t, window->next_width, window->next_height);
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
                if (d.second.created)
                {
                    main_telement->add_window(d.first);
                    d.second.created = false;
                }
            }

            usleep(10);

            feather::main_mutex.lock();
            main_telement->update();
            feather::main_mutex.unlock();
        }
    }

} // namespace feather