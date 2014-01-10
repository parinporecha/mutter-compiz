NULL =

INCLUDES = $(MUTTER_CFLAGS)								\
	-I$(top_srcdir)

libexec_PROGRAMS = display-config-daemon

display_config_service_built_sources = $(dbus_xrandr_built_sources) 			\
					$(dbus_idle_built_sources)

display_config_daemon_SOURCES = \
	boxes.c   		\
	edid-parse.c 		\
	main.c    		\
	meta-idle-monitor.c 	\
	monitor.c		\
	monitor-config.c 	\
	monitor-xrandr.c 	\
	meta-dbus-xrandr.c 	\
	meta-dbus-idle-monitor.c

display_config_daemon_LDADD = $(MUTTER_LIBS)

CLEANFILES = $(dbus_xrandr_built_sources) $(dbus_idle_built_sources)

EXTRA_DIST = xrandr.xml idle-monitor.xml

BUILT_SOURCES = $(dbus_xrandr_built_sources) $(dbus_idle_built_sources)

dbus_xrandr_built_sources = meta-dbus-xrandr.c meta-dbus-xrandr.h

$(dbus_xrandr_built_sources) : Makefile.am xrandr.xml
	$(AM_V_GEN)gdbus-codegen							\
		--interface-prefix org.gnome.Mutter					\
		--c-namespace MetaDBus							\
		--generate-c-code meta-dbus-xrandr					\
		$(srcdir)/xrandr.xml

dbus_idle_built_sources = meta-dbus-idle-monitor.c meta-dbus-idle-monitor.h

$(dbus_idle_built_sources) : Makefile.am idle-monitor.xml
	$(AM_V_GEN)gdbus-codegen							\
		--interface-prefix org.gnome.Mutter					\
		--c-namespace MetaDBus							\
		--generate-c-code meta-dbus-idle-monitor				\
		--c-generate-object-manager						\
		$(srcdir)/idle-monitor.xml
