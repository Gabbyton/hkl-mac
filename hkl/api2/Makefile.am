AM_CPPFLAGS = -Wextra -D_DEFAULT_SOURCE \
	-I$(top_srcdir) \
	-I$(top_srcdir)/hkl \
	$(DATATYPE99_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GSL_CFLAGS)

# need access to the private part
# change the AM_LDFLAGS to use -force_load instead of --whole-archive
# AM_LDFLAGS = -Wl,--whole-archive,$(top_builddir)/hkl/.libs/libhkl.a,--no-whole-archive
AM_LDFLAGS = -Wl,-force_load,../../hkl/.libs/libhkl.a

LDADD = \
	$(GLIB_LIBS) \
	$(GSL_LIBS)

noinst_LTLIBRARIES=libhkl2.la
libhkl2_la_SOURCES = \
	hkl2.c \
	hkl2.h
