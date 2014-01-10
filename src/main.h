#ifndef MAIN_UTIL_H
#define MAIN_UTIL_H

#include "meta-idle-monitor.h"
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

void                mainloop_quit       (void);

GdkFilterReturn     xevent_filter       (GdkXEvent *xevent,
                                         GdkEvent *event,
                                         MetaIdleMonitor *monitor);
#endif

