#include "ftwm.h"
#include "fwm.h"
#include <X11/X.h>
#include <X11/Xlib.h>
namespace feather {
    void feather_tiling_manager::launch_tile_process( std::unordered_map<Window, fwm_winfo>* list){

        this->list = list;

        process = std::thread(&feather_tiling_manager::main_process, this);

    }

    void feather_tiling_manager::update(fwm_winfo* window, const Window w){


        if(window->full_screen == true){
            
            XMoveWindow(window->w_display, window->frame, gaps,gaps);
            XMoveWindow(window->w_display,  w, gaps,gaps);
            XResizeWindow(window->w_display, window->frame, XDisplayWidth(window->w_display, 0) - gaps*2, XDisplayHeight(window->w_display, 0) - gaps*2);
            XResizeWindow(window->w_display, w, XDisplayWidth(window->w_display, 0) - gaps*2, XDisplayHeight(window->w_display, 0) - gaps*2);

        }else{

        }
    }
    void feather_tiling_manager::main_process(){
        
        
        while(true){
            for(std::pair<const Window, fwm_winfo>& d : *list){
                feather::main_mutex.lock();
                if(d.second.has_changed){
                    update(&d.second, d.first);

                    d.second.has_changed = false;
                }
                feather::main_mutex.unlock();

            }
            
            usleep(10);
        }
    }

}