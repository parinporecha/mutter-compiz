
#include <stdlib.h>

#include <glib.h>


#include <gtk/gtk.h>

#ifdef GDK_WINDOWING_X11
#include <X11/Xlib.h>
#endif

#include "monitor.c"

int main (int argc, char ** argv) {
        GMainLoop *loop = g_main_loop_new(NULL, FALSE);

        meta_monitor_manager_initialize();

        g_main_loop_run(loop);
        return 0;
}

