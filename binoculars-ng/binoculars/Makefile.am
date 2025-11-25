# Process this file with automake to produce Makefile.in

AM_CFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/hkl \
	-DPKGDATA=\""$(pkgdatadir)"\" \
	$(CGLM_CFLAGS) \
	$(DATATYPE99_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GSL_CFLAGS) \
	$(HDF5_CFLAGS)

AM_LDFLAGS = -version-info 0:0:0 \
	$(top_builddir)/hkl/libhkl-private.la \
	$(CGLM_LIBS) \
	$(GLIB_LIBS) \
	$(GSL_LIBS) \
	$(HDF5_LIBS)

noinst_LTLIBRARIES = libhkl-binoculars.la
libhkl_binoculars_la_SOURCES = \
	hkl-binoculars.c \
	hkl-binoculars.h \
	hkl-binoculars-detectors-2d.c \
	hkl-binoculars-detectors-2d-private.h \
	hkl-binoculars-geometry.c \
	hkl-binoculars-io-cnpy.c \
	hkl-binoculars-io-hdf5.c \
	hkl-binoculars-io-img.c \
	hkl-binoculars-io-private.h \
	hkl-binoculars-private.h

# Support for GNU Flymake, in Emacs.
check-syntax: AM_CFLAGS += -fsyntax-only -pipe
check-syntax:
	test -z "$(CHK_SOURCES)" || $(COMPILE) $(CHK_SOURCES)

.PHONY: check-syntax
