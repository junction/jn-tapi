
##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual -ansi

noinst_PROGRAMS = jingleaudiortp_test

jingleaudiortp_test_SOURCES = jingleaudiortp_test.cpp
jingleaudiortp_test_LDADD = ../../stanza.o ../../jid.o ../../tag.o ../../prep.o \
 		../../gloox.o ../../stanzaextensionfactory.o ../../jingledescription.o \
		../../iq.o ../../util.o ../../sha.o ../../base64.o
jingleaudiortp_test_CFLAGS = $(CPPFLAGS)
