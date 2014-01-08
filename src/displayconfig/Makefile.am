NULL =

INCLUDES = $(MUTTER_CFLAGS) \
	-I$(top_srcdir)                                                     \
	-DSN_API_NOT_YET_FROZEN=1

libexec_PROGRAMS = display_config_server

display_config_server_built_sources = $(dbus_xrandr_built_sources)

display_config_server_SOURCES = \
	boxes.c   \
	main.c    \
	monitor.c \
	monitor-config.c \
	monitor-xrandr.c \
	edid-parse.c \
        meta-dbus-xrandr.c

display_config_server_LDADD = $(MUTTER_LIBS)

CLEANFILES = $(dbus_xrandr_built_sources)

EXTRA_DIST = xrandr.xml

BUILT_SOURCES = $(dbus_xrandr_built_sources)

dbus_xrandr_built_sources = meta-dbus-xrandr.c meta-dbus-xrandr.h

$(dbus_xrandr_built_sources) : Makefile.am xrandr.xml
	$(AM_V_GEN)gdbus-codegen                                                        \
		--interface-prefix org.gnome.Mutter                                        \
		--c-namespace MetaDBus                                                        \
		--generate-c-code meta-dbus-xrandr                                        \
		$(srcdir)/xrandr.xml


