# Process this file with automake to produce Makefile.in

AM_CXXFLAGS = \
	-DPKGDATA=\""$(pkgdatadir)"\" \
	-fpermissive \
	-I$(top_srcdir) \
	-I$(top_srcdir)/hkl \
	-I$(top_srcdir)/hkl3d \
	-I$(top_srcdir)/hkl3d/bullet/src\
	$(ASSIMP_CFLAGS) \
	$(BULLET_CFLAGS) \
	$(DATATYPE99_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(YAML_CFLAGS)

AM_LDFLAGS = -version-info 0:0:0 \
	$(ASSIMP_LIBS) \
	$(BULLET_LIBS) \
	$(GLIB_LIBS) \
	$(YAML_LIBS)

noinst_LTLIBRARIES = libhkl3d.la
libhkl3d_la_SOURCES = hkl3d.cpp hkl3d-config.cpp hkl3d-gl.cpp
libhkl3d_la_LIBADD = \
	$(top_builddir)/hkl/libhkl.la

noinst_HEADERS = hkl3d.h hkl3d-private.h

# Support for GNU Flymake, in Emacs.
check-syntax: AM_CXXFLAGS += -fsyntax-only -pipe
check-syntax:
	test -z "$(CHK_SOURCES)" || $(CXXCOMPILE) $(CHK_SOURCES)

.PHONY: check-syntax
