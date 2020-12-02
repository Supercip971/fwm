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
    fwm_winfo* windows_tiling_element::get_next_fullscreen_window(int id){
        id++;
        while(window_list.size() > (size_t)id){
            fwm_winfo *d = &dad->list->at(window_list[id]);
            if(d->full_screen){
                return d;
            }
            id++;
        }
        return nullptr;
    }
    void windows_tiling_element::update(Window t, fwm_winfo *window, int id)
    {

        if (window->full_screen == true)
        {
            int count = sub_elements.size();
            for(size_t i = 0; i < window_list.size(); i++){
                if(dad->list->at(window_list[i]).full_screen){
                    count++;
                }
            }
            if(count == 0){
                count = 1;
            }
            //   XMoveWindow(window->w_display, window->frame, window->next_x, window->next_y);
            int last_x = this->x;
            int last_y = this->y;
            for (int i = 0; i < id; i++)
            {

                fwm_winfo *d = &dad->list->at(window_list[i]);
                if(d->full_screen){
                    last_x += this->width / count;
                    last_x += d->add_size;
                    last_y += this->height / count;
                    last_y += d->add_size;

                }
            }
            if (type == WINDOW_HEIGHT)
            {
                unsigned int fheight = (this->height / count) + window->add_size;
                if (window->next_height != fheight)
                {
                    if (window_list.size() > (size_t)id + 1)
                    {

                        window->add_size = window->next_height - (this->height / count);
                        fheight = (this->height / count) + window->add_size;
                        fwm_winfo* nfo = get_next_fullscreen_window(id);
                        if(nfo != nullptr){
                            nfo->add_size = -window->add_size;
                        }
                    }
                }
                const unsigned int fy = last_y;
                const unsigned int fx = this->x;
                const unsigned int fwidth = this->width;
                XMoveWindow(window->w_display, window->frame, fx, fy);
                XResizeWindow(window->w_display, window->frame, fwidth, fheight);
                XResizeWindow(window->w_display, t, fwidth, fheight);
            }
            else if (type == WINDOW_WIDTH)
            {
                unsigned int fwidth = (this->width / count) + window->add_size;
                if (window->next_width != fwidth)
                {
                    if (window_list.size() > (size_t)id + 1)
                    {

                        window->add_size = window->next_width - (this->width / count);
                        fwidth = (this->width / count) + window->add_size;
                        fwm_winfo* nfo = get_next_fullscreen_window(id);
                        if(nfo != nullptr){
                            nfo->add_size = -window->add_size;
                        }
                    }
                }

                const unsigned int fx = last_x;
                const unsigned int fy = this->y;
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
