lib_LTLIBRARIES = libgpio.la
include_HEADERS = $(top_srcdir)/include/gpio.h $(top_srcdir)/include/pwm.h
AM_CPPFLAGS = -I$(top_srcdir)/include
libgpio_la_SOURCES = gpio.c pwm.c
libgpio_la_LDFLAGS = -version-info 1:0:0 -lpthread
