#include "feather/fwm.h"
#include "feather/utility.h"
#include <libgen.h>
#include "feather/feather_wren_integration.h"
int main(int argc, char* argv[])
{dirname(argv[0]);
    feather::context.set_current_path(argv[0]);
    printf("running from %s \n", argv[0]);
    fwren_init_system();
    printf("starting feather window manager \n");
    std::cout << main_settings;
    feather::fwm *main_fwm = feather::fwm::the();
    main_fwm->init();
    main_fwm->run();
    main_fwm->exit();
    return 0;
}
