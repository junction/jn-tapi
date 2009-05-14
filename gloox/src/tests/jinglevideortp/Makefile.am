
##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual -ansi

noinst_PROGRAMS = jinglevideortp_test

jinglevideortp_test_SOURCES = jinglevideortp_test.cpp
jinglevideortp_test_LDADD = ../../stanza.o ../../jid.o ../../tag.o ../../prep.o \
 		../../gloox.o ../../stanzaextensionfactory.o ../../jingledescription.o \
		../../iq.o ../../util.o ../../sha.o ../../base64.o
jinglevideortp_test_CFLAGS = $(CPPFLAGS)
