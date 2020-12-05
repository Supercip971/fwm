#include "feather_wren_integration.h"
#include <stdio.h>
#include <cstring>
#include <string>
#include <libgen.h>         // dirname
#include <unistd.h>         // readlink
#include <linux/limits.h>   // PATH_MAX
#include "utility.h"
#include "wren.h"
#include "wren/vm/wren_common.h"
#include "wren/vm/wren_core.h"
#include <map>

json main_settings;
static void writeFn(WrenVM* vm, const char* text)
{
  printf("[WREN] %s", text);
}

void errorFn(WrenVM* vm, WrenErrorType errorType,
             const char* module, const int line,
             const char* msg)
{
  switch (errorType)
  {
    case WREN_ERROR_COMPILE:
    {

      printf("[WREN] [%s line %d] [Error] %s\n", module, line, msg);
    } break;
    case WREN_ERROR_STACK_TRACE:
    {
      printf("[WREN] [%s line %d] in %s\n", module, line, msg);
    } break;
    case WREN_ERROR_RUNTIME:
    {
      printf("[WREN] [Runtime Error] %s\n", msg);
    } break;
  }
}
std::vector<fwren_func> cfunc_list;
std::vector<fwren_class> cclass_list;
WrenForeignMethodFn bindForeignMethod(WrenVM* vm,const char* module,const char* className,bool isStatic, const char* signature){
    for(size_t i = 0; i < cfunc_list.size(); i++){
        if(strcmp(signature, cfunc_list[i].signature) == 0){

            if(strcmp(module, cfunc_list[i].module) == 0){
                if(isStatic && (strcmp(className, cfunc_list[i].class_name) == 0)){
                    return cfunc_list[i].fwren;

                }
            }
        }
    }
    return NULL;
}
WrenForeignClassMethods bindForeignClass(WrenVM* vm, const char* module, const char* className){

    for(size_t i = 0; i < cclass_list.size(); i++){
        if(strcmp(className, cclass_list[i].className) == 0){

            if(strcmp(module, cclass_list[i].module) == 0){
                return cclass_list[i].fclass;
            }
        }
    }
    WrenForeignClassMethods classic;
    classic.allocate = NULL;
    classic.finalize = NULL;
    return classic;
}

void log(WrenVM* vm){
    wrenEnsureSlots(vm, 1);
    if(wrenGetSlotType(vm, 1) == WREN_TYPE_STRING){
        feather::context.log("[ Wren ] %s", wrenGetSlotString(vm, 1));
        wrenSetSlotBool(vm, 0, true);

    }else{
        wrenSetSlotBool(vm, 0, false);

    }
}

// set_setting("target", value)
// set_setting("gaps", 13)
void set_setting_func(WrenVM* vm){
    wrenEnsureSlots(vm, 3);
    if(wrenGetSlotType(vm, 1) == WREN_TYPE_STRING){
        std::string target = wrenGetSlotString(vm,1);
        WrenType target_type = wrenGetSlotType(vm,2);

        if(target_type == WrenType::WREN_TYPE_BOOL ){
            main_settings[target] += wrenGetSlotBool(vm,2);
        }else if(target_type == WrenType::WREN_TYPE_NUM ){
            main_settings[target]= wrenGetSlotDouble(vm,2);
        }else if(target_type == WrenType::WREN_TYPE_STRING ){
            main_settings[target] = wrenGetSlotString(vm,2);
        }

        wrenSetSlotBool(vm, 0, true);
    }else{
        wrenSetSlotBool(vm, 0, false);

    }
}

// get_setting("name")
void get_setting_func(WrenVM* vm){
    wrenEnsureSlots(vm, 2);
    if(wrenGetSlotType(vm, 1) == WREN_TYPE_STRING){
        std::string target = wrenGetSlotString(vm,1);
        if(!main_settings.contains(target)){
            wrenSetSlotNull(vm, 0);
            return;
        }
        json::value_t type =
                main_settings[target].type();
        if(type == json::value_t::number_integer || type == json::value_t::number_unsigned){
            wrenSetSlotDouble(vm, 0, double(main_settings[target].get<int>()));
        }else if(type == json::value_t::number_float){
            wrenSetSlotDouble(vm, 0, double(main_settings[target].get<float>()));
        }else if(type == json::value_t::boolean){
            wrenSetSlotBool(vm, 0, (main_settings[target].get<bool>()));
        }else if(type == json::value_t::string){
            wrenSetSlotString(vm, 0, (main_settings[target].get<std::string>()).c_str());
        }else{

            wrenSetSlotNull(vm, 0);
        }

    }else{
        wrenSetSlotNull(vm, 0);

    }
}

void add_func(WrenForeignMethodFn func, const char* module, const char* classname, const char* signature){
    fwren_func target;
    target.fwren = func;
    target.module = module;
    target.class_name = classname;
    target.signature = signature;
    target.is_static = true;

    cfunc_list.push_back(target);
}void add_class_func(WrenForeignMethodFn func, const char* module, const char* classname, const char* signature){
    fwren_func target;
    target.fwren = func;
    target.module = module;
    target.class_name = classname;
    target.signature = signature;
    target.is_static = false;
    cfunc_list.push_back(target);
}
char* load_module(WrenVM* vm, const char* module_name){
    std::string st = (feather::context.get_current_path());
    st.append("/");
    st.append(( module_name));
    printf("loading module %s \n", st.c_str());
    std::ifstream file(st, std::ios::in | std::ios::binary);
    file.seekg(0, std::ios::end);

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    char* buffer = (char*)malloc(size);
    if (file.read(buffer, size))
    {
        return buffer;
    }else{
        return NULL;

    }

}
WrenVM* init_feather_wren(){
    WrenConfiguration config;
    wrenInitConfiguration(&config);
    config.writeFn = writeFn;
    config.errorFn = errorFn;
    config.bindForeignClassFn = bindForeignClass;
    config.bindForeignMethodFn = bindForeignMethod;
    config.loadModuleFn = load_module;
    add_func(log, "fwm.wren", "Fwm", "echo(_)");
    add_func(set_setting_func, "fwm.wren", "Fwm", "set_setting(_,_)");
    add_func(get_setting_func, "fwm.wren", "Fwm", "get_setting(_)");
    WrenVM* vm = wrenNewVM(&config);
    return vm;
}

int fwren_load(const char* file_path, WrenVM* vm){
    return wrenInterpret(vm, "main",load_module(vm, file_path));
}

void fwren_init_system(){
    main_settings = json();
    WrenVM* finit_vm = init_feather_wren();
    if(fwren_load("finit.wren",finit_vm) != WREN_RESULT_SUCCESS){
        return;
    };

    wrenEnsureSlots(finit_vm, 1);

    wrenGetVariable(finit_vm, "main", "fsystem", 0);

    WrenHandle* finit_function = wrenGetSlotHandle(finit_vm, 0);
    wrenSetSlotHandle(finit_vm, 0, finit_function);
    WrenHandle* handle = wrenMakeCallHandle(finit_vm, "init()");


    wrenCall(finit_vm,handle);

}



