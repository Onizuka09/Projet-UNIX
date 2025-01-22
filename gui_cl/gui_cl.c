#include <stdio.h>
#include <Gooey/gooey.h>
#include "ui.h"

extern GooeyWindow win;
int main()
{
    set_logging_enabled(true);
    set_minimum_log_level(DEBUG_LEVEL_INFO);

    Gooey_Init(GLFW);

    win = GooeyWindow_Create("GUI client", 600, 900, 1);
    // Create_authentification_layout();
    
     create_services_layout();

    GooeyWindow_Run(1, &win);

    GooeyWindow_Cleanup(1, &win);

    return 0;
}
