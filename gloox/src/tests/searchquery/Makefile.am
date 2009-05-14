##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual -ansi

noinst_PROGRAMS = searchquery_test

searchquery_test_SOURCES = searchquery_test.cpp
searchquery_test_LDADD = ../../stanza.o ../../jid.o ../../dataform.o ../../dataformfieldcontainer.o \
 		../../dataformreported.o ../../dataformitem.o ../../dataformfield.o ../../tag.o ../../prep.o \
 		../../gloox.o ../../stanzaextensionfactory.o \
		../../iq.o ../../util.o ../../sha.o ../../base64.o \
		../../error.o
searchquery_test_CFLAGS = $(CPPFLAGS)
