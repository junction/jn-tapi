
##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual -ansi

noinst_PROGRAMS = jinglerawudp_test

jinglerawudp_test_SOURCES = jinglerawudp_test.cpp
jinglerawudp_test_LDADD = ../../stanza.o ../../jid.o ../../tag.o ../../prep.o \
 		../../gloox.o ../../stanzaextensionfactory.o ../../jingletransport.o \
		../../iq.o ../../util.o ../../sha.o ../../base64.o
jinglerawudp_test_CFLAGS = $(CPPFLAGS)
