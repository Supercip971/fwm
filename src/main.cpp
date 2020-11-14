#include "feather/fwm.h"
#include "feather/utility.h"

int main()
{
    printf("starting feather window manager \n");
    feather::fwm *main_fwm = feather::fwm::the();
    main_fwm->init();
    main_fwm->run();
    main_fwm->exit();
    return 0;
}