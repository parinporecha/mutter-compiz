/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2007 William Jon McCann <mccann@jhu.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#include <stdlib.h>
#include <unistd.h>
#include <libintl.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>

#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include <X11/Xlib.h>
#include <X11/extensions/sync.h>

#include "main.h"
#include "meta-idle-monitor-private.h"
#include "monitor-private.h"

#define GNOME_SESSION_DBUS_NAME       "org.gnome.SessionManager"
#define GNOME_SESSION_DBUS_PATH       "/org/gnome/SessionManager"
#define GNOME_SESSION_DBUS_INTERFACE  "org.gnome.SessionManager"
#define GNOME_SESSION_CLIENT_PRIVATE_DBUS_INTERFACE "org.gnome.SessionManager.ClientPrivate"

static gboolean   debug        = FALSE;
static gboolean   replace      = FALSE;

static GMainLoop *mainloop;
static int        term_signal_pipe_fds[2];

static GDBusProxy         *sm_proxy = NULL;

static GOptionEntry entries[] = {
        {"debug", 0, 0, G_OPTION_ARG_NONE, &debug, N_("Enable debugging code"), NULL },
        {"replace", 'r', 0, G_OPTION_ARG_NONE, &replace, N_("Replace existing daemon"), NULL },
        {NULL}
};

typedef struct
{
        /* X11 implementation */
        Display     *display;
        int          sync_event_base;
        int          sync_error_base;
        unsigned int have_xsync : 1;

}MetaXSync;

static MetaXSync *xsync;

void
mainloop_quit(void)
{
        g_debug ("Shutting down");
        g_main_loop_quit (mainloop);
}

GdkFilterReturn
xevent_filter (GdkXEvent *xevent,
               GdkEvent *event,
               MetaIdleMonitor *monitor)
{
  XEvent *ev;

  ev = xevent;
  if (ev->xany.type != xsync->sync_event_base + XSyncAlarmNotify) {
    return GDK_FILTER_CONTINUE;
  }

  meta_idle_monitor_handle_xevent_all (ev);

  return GDK_FILTER_CONTINUE;
}

static void
on_session_over (GDBusProxy *proxy,
                 gchar      *sender_name,
                 gchar      *signal_name,
                 GVariant   *parameters,
                 gpointer    user_data)
{
        if (g_strcmp0 (signal_name, "SessionOver") == 0) {
                g_debug ("Got a SessionOver signal - stopping");
                g_main_loop_quit (mainloop);
        }
}

static gboolean
session_manager_connect (void)
{

        sm_proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION, 0, NULL,
                                              GNOME_SESSION_DBUS_NAME,
                                              GNOME_SESSION_DBUS_PATH,
                                              GNOME_SESSION_DBUS_INTERFACE, NULL, NULL);

        g_signal_connect (G_OBJECT (sm_proxy), "g-signal",
                          G_CALLBACK (on_session_over), NULL);

        return (sm_proxy != NULL);
}

static void
respond_to_end_session (GDBusProxy *proxy)
{
        /* we must answer with "EndSessionResponse" */
        g_dbus_proxy_call (proxy, "EndSessionResponse",
                           g_variant_new ("(bs)",
                                          TRUE, ""),
                           G_DBUS_CALL_FLAGS_NONE,
                           -1, NULL, NULL, NULL);

        g_main_loop_quit (mainloop);
}

static void
client_proxy_signal_cb (GDBusProxy *proxy,
                        gchar *sender_name,
                        gchar *signal_name,
                        GVariant *parameters,
                        gpointer user_data)
{
        if (g_strcmp0 (signal_name, "QueryEndSession") == 0) {
                g_debug ("Got QueryEndSession signal");
                respond_to_end_session (proxy);
        } else if (g_strcmp0 (signal_name, "EndSession") == 0) {
                g_debug ("Got EndSession signal");
                respond_to_end_session (proxy);
        } else if (g_strcmp0 (signal_name, "Stop") == 0) {
                g_debug ("Got Stop signal");
                g_main_loop_quit (mainloop);
        }
}

static void
got_client_proxy (GObject *object,
                  GAsyncResult *res,
                  gpointer user_data)
{
        GDBusProxy *client_proxy;
        GError *error = NULL;

        client_proxy = g_dbus_proxy_new_for_bus_finish (res, &error);

        if (error != NULL) {
                g_debug ("Unable to get the session client proxy: %s", error->message);
                g_error_free (error);
                return;
        }

        g_signal_connect (client_proxy, "g-signal",
                          G_CALLBACK (client_proxy_signal_cb), NULL);

}


static void
on_client_registered (GObject             *source_object,
                      GAsyncResult        *res,
                      gpointer             user_data)
{
        GVariant *variant;
        GError *error = NULL;
        gchar *object_path = NULL;

        variant = g_dbus_proxy_call_finish (G_DBUS_PROXY (source_object), res, &error);
        if (error != NULL) {
                g_warning ("Unable to register client: %s", error->message);
                g_error_free (error);
        } else {
                g_variant_get (variant, "(o)", &object_path);

                g_debug ("Registered client at path %s", object_path);

                g_dbus_proxy_new_for_bus (G_BUS_TYPE_SESSION, 0, NULL,
                                          GNOME_SESSION_DBUS_NAME,
                                          object_path,
                                          GNOME_SESSION_CLIENT_PRIVATE_DBUS_INTERFACE,
                                          NULL,
                                          got_client_proxy,
                                          NULL);

                g_free (object_path);
                g_variant_unref (variant);
        }
}

static void
register_with_gnome_session (GDBusProxy *proxy)
{
        const char *startup_id;

        g_signal_connect (G_OBJECT (proxy), "g-signal",
                          G_CALLBACK (on_session_over), NULL);
        startup_id = g_getenv ("DESKTOP_AUTOSTART_ID");
        g_dbus_proxy_call (proxy,
                           "RegisterClient",
                           g_variant_new ("(ss)", "display-config-daemon", startup_id ? startup_id : ""),
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) on_client_registered,
                           NULL);
}

static gboolean
on_term_signal_pipe_closed (GIOChannel *source,
                            GIOCondition condition,
                            gpointer data)
{
        term_signal_pipe_fds[0] = -1;

        g_debug ("Received SIGTERM - shutting down");
        /* Got SIGTERM, time to clean up and get out
         */
        g_main_loop_quit (mainloop);

        return FALSE;
}

static void
on_term_signal (int signal)
{
        /* Wake up main loop to tell it to shutdown */
        close (term_signal_pipe_fds[1]);
        term_signal_pipe_fds[1] = -1;
}

static void
watch_for_term_signal ()
{
        GIOChannel *channel;

        if (-1 == pipe (term_signal_pipe_fds) ||
            -1 == fcntl (term_signal_pipe_fds[0], F_SETFD, FD_CLOEXEC) ||
            -1 == fcntl (term_signal_pipe_fds[1], F_SETFD, FD_CLOEXEC)) {
                g_error ("Could not create pipe: %s", g_strerror (errno));
                exit (EXIT_FAILURE);
        }

        channel = g_io_channel_unix_new (term_signal_pipe_fds[0]);
        g_io_channel_set_encoding (channel, NULL, NULL);
        g_io_channel_set_buffered (channel, FALSE);
        g_io_add_watch (channel, G_IO_HUP, on_term_signal_pipe_closed, NULL);
        g_io_channel_unref (channel);

        signal (SIGTERM, on_term_signal);
}


static void
gsd_log_default_handler (const gchar   *log_domain,
                         GLogLevelFlags log_level,
                         const gchar   *message,
                         gpointer       unused_data)
{
        /* filter out DEBUG messages if debug isn't set */
        if ((log_level & G_LOG_LEVEL_MASK) == G_LOG_LEVEL_DEBUG
            && ! debug) {
                return;
        }

        g_log_default_handler (log_domain,
                               log_level,
                               message,
                               unused_data);
}

static void
parse_args (int *argc, char ***argv)
{
        GError *error;
        GOptionContext *context;

        context = g_option_context_new (" - Display Config D-Bus service");

        g_option_context_add_main_entries (context, entries, NULL);

        error = NULL;
        if (!g_option_context_parse (context, argc, argv, &error)) {
                if (error != NULL) {
                        g_warning ("%s", error->message);
                        g_error_free (error);
                } else {
                        g_warning ("Unable to initialize");
                }
                exit (EXIT_FAILURE);
        }

        g_option_context_free (context);

        if (debug)
                g_setenv ("G_MESSAGES_DEBUG", "all", FALSE);
}

static void
init_xsync (void)
{
#ifdef HAVE_XSYNC
  {
    int major, minor;

    xsync->display = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
    xsync->have_xsync = FALSE;

    xsync->sync_error_base = 0;
    xsync->sync_event_base = 0;

    /* I don't think we really have to fill these in */
    major = SYNC_MAJOR_VERSION;
    minor = SYNC_MINOR_VERSION;

    if (!XSyncQueryExtension (xsync->display,
                              &xsync->sync_event_base,
                              &xsync->sync_error_base) ||
        !XSyncInitialize (xsync->display,
                          &major, &minor))
      {
        xsync->sync_error_base = 0;
        xsync->sync_event_base = 0;
      }
    else
      {
        xsync->have_xsync = TRUE;
        XSyncSetPriority (xsync->display, None, 10);
      }

    meta_verbose ("Attempted to init Xsync, found version %d.%d error base %d event base %d\n",
                  major, minor,
                  xsync->sync_error_base,
                  xsync->sync_event_base);
  }
#else  /* HAVE_XSYNC */
  meta_verbose ("Not compiled with Xsync support\n");
#endif /* !HAVE_XSYNC */
}

int
main (int argc, char *argv[])
{

        //bindtextdomain (GETTEXT_PACKAGE, GNOME_SETTINGS_LOCALEDIR);
        //bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
        //textdomain (GETTEXT_PACKAGE);

        parse_args (&argc, &argv);

        gdk_set_allowed_backends ("x11");

        if (! gdk_init_check (NULL, NULL)) {
                g_warning ("Unable to initialize GTK+");
                exit (EXIT_FAILURE);
        }

        g_log_set_default_handler (gsd_log_default_handler, NULL);

        if (g_strcmp0(g_getenv("XDG_CURRENT_DESKTOP"), "GNOME")) {
                xsync = g_slice_new0 (MetaXSync);
                init_xsync();
                mainloop = g_main_loop_new (NULL, FALSE);

                meta_monitor_manager_initialize (replace);
                meta_idle_monitor_init_dbus (replace);

                if (!session_manager_connect ())
                        g_warning ("Unable to connect to session manager");

                register_with_gnome_session (sm_proxy);
                watch_for_term_signal ();

                g_main_loop_run (mainloop);

                g_slice_free(MetaXSync, xsync);
        }
        
        g_debug ("DisplayConfig finished");

        return 0;
}
