#ifndef FEATHER_WREN_INTEGRATION_H
#define FEATHER_WREN_INTEGRATION_H
#include "wren.h"
#include <vector>
#include <map>
#include "json.h"

using json = nlohmann::json;

extern json main_settings;
WrenVM* init_feather_wren();
struct fwren_func{
    WrenForeignMethodFn fwren;
    const char* signature;
    const char* class_name;
    const char* module;
    bool is_static;
};

struct fwren_class{
    WrenForeignClassMethods fclass;
    const char* module;
    const char* className;

};

int fwren_load(const char* file_path, WrenVM* vm);
void fwren_init_system();


#endif // FEATHER_WREN_INTEGRATION_H
