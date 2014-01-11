NULL=

service_in_files = org.gnome.Mutter.DisplayConfig.service.in org.gnome.Mutter.IdleMonitor.service.in

servicedir = $(datadir)/dbus-1/services
service_DATA = $(service_in_files:.service.in=.service)

%.service: %.service.in Makefile
	$(AM_V_GEN) [ -d $(@D) ] || $(mkdir_p) $(@D) ; \
		sed -e "s|\@libexecdir\@|$(libexecdir)|" $< > $@.tmp && mv $@.tmp $@

EXTRA_DIST = \
	$(service_in_files)  \
	$(NULL)

CLEANFILES = $(service_DATA)

