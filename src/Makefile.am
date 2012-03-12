## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPLUGIN_DIR=\""$(plugindir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = gdivelog
include_HEADERS = gdivelog.h

gdivelog_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
	format_fields.c format_fields.h \
	helper_functions.c helper_functions.h \
	preferences.c preferences.h \
	db_main.c db_main.h \
	db_generic_callbacks.c db_generic_callbacks.h \
	buddy_gui.c buddy_gui.h \
	buddy_db.c buddy_db.h \
	dive_gui.c dive_gui.h \
	dive_db.c dive_db.h \
	dive_tank_gui.c dive_tank_gui.h \
	dive_tank_db.c dive_tank_db.h \
	equipment_gui.c equipment_gui.h \
	equipment_db.c equipment_db.h \
	import_dive.c import_dive.h\
	plugins.c plugins.h \
	profile_gui.c profile_gui.h \
	profile_db.c profile_db.h \
	renumber_gui.c renumber_gui.h \
	renumber_db.c renumber_db.h \
	site_gui.c site_gui.h \
	site_db.c site_db.h \
	sort_funcs.c sort_funcs.h \
	statistics_gui.c statistics_gui.h \
	statistics_db.c statistics_db.h \
	tank_gui.c tank_gui.h\
	tank_db.c tank_db.h \
	type_gui.c type_gui.h \
	type_db.c type_db.h

gdivelog_LDADD = @PACKAGE_LIBS@ $(INTLLIBS)
