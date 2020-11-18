#pragma once
#include <mutex>
#include <future>
#include "ftype.h"
namespace feather{


    class feather_tiling_manager{
        public:
        int gaps = 0;
        void launch_tile_process(  std::unordered_map<Window, fwm_winfo>* list);
        
        void main_process();
        void operator()(){
            main_process();
        }
        private:
        void update(fwm_winfo* window, const Window w);


        
        std::unordered_map<Window, fwm_winfo>* list;
        std::thread process;
    };
};