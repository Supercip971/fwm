#include "feather/fwm.h"
#include "feather/json_parser.h"
#include "feather/utility.h"
int main()
{
    feather::json_data j = feather::json_data();
    j.from_file("build/feather_script.json");
    printf("%s", j.get_storage()["main"]["fwm_key"]["mod"].get_value().get_as<std::string>().c_str());
    printf("starting feather window manager \n");
    feather::fwm *main_fwm = feather::fwm::the();
    main_fwm->init();
    main_fwm->run();
    main_fwm->exit();
    return 0;
}