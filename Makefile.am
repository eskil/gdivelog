## Process this file with automake to produce Makefile.in

@INTLTOOL_DESKTOP_RULE@

SUBDIRS = src po

EXTRA_DIST = \
       gdivelog.spec.in \
       gdivelog.desktop.in.in \
	autogen.sh \
	gdivelog.glade \
	gdivelog.gladep

gnomemenudir = $(datadir)/applications
gnomemenu_premerge_file = gdivelog.desktop.in
gnomemenu_DATA = $(gnomemenu_premerge_file:.desktop.in=.desktop)

install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/pixmaps; then \
	  $(mkinstalldirs) $(DESTDIR)$(datadir)/pixmaps/$(PACKAGE); \
	  for pixmap in $(srcdir)/pixmaps/*; do \
	    if test -f $$pixmap; then \
	      $(INSTALL_DATA) $$pixmap $(DESTDIR)$(datadir)/pixmaps/$(PACKAGE); \
	    fi \
	  done \
	fi

dist-hook:
	if test -d pixmaps; then \
	  mkdir $(distdir)/pixmaps; \
	  for pixmap in pixmaps/*; do \
	    if test -f $$pixmap; then \
	      cp -p $$pixmap $(distdir)/pixmaps; \
	    fi \
	  done \
	fi

uninstall-local:
	cd pixmaps
	for i in `ls *` ; do \
	  rm -rf $(DESTDIR)$(datadir)/pixmaps/$(PACKAGE)/$$i ; done
	cd ..
	rmdir --ignore-fail-on-non-empty $(DESTDIR)$(datadir)/pixmaps/$(PACKAGE)

CLEANFILES = \
	$(gnomemenu_DATA) 

DISTCLEANFILES = \
	intltool-extract \
	intltool-merge \
	intltool-update \
	$(gnomemenu_DATA) 

