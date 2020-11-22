#pragma once
#include "ftype.h"
#include <future>
#include <mutex>
#include <vector>
namespace feather
{
    enum window_tiling_elements_type
    {
        WINDOW_HEIGHT,
        WINDOW_WIDTH,
    };
    class windows_tiling_element;

    class feather_tiling_manager
    {
    public:
        Display *display;
        std::unordered_map<Window, fwm_winfo> *list;
        int gaps = 0;
        void launch_tile_process(std::unordered_map<Window, fwm_winfo> *list);

        void main_process();
        void operator()()
        {
            main_process();
        }

    private:
        windows_tiling_element *main_telement;
        void update(fwm_winfo *window, const Window w);

        std::thread process;
    };
    class windows_tiling_element
    {
        std::vector<windows_tiling_element *> sub_elements;
        std::vector<Window> window_list;
        feather_tiling_manager *dad;
        int type = WINDOW_HEIGHT;

    public:
        int x;
        int y;
        int width;
        int height;
        void add_window(Window w);
        void remove_window(Window w);
        void update();
        void update(Window t, fwm_winfo *window, int id);
        void add(windows_tiling_element *y);

        windows_tiling_element();
        windows_tiling_element(feather_tiling_manager *parent, window_tiling_elements_type t);
        windows_tiling_element(feather_tiling_manager *parent, window_tiling_elements_type t, int ix, int iy, int iwidth, int iheight);
    };
}; // namespace feather