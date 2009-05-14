##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -g3 -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual -ansi

noinst_PROGRAMS = stanzaextensionfactory_test

stanzaextensionfactory_test_SOURCES = stanzaextensionfactory_test.cpp
stanzaextensionfactory_test_LDADD = ../../tag.o ../../stanza.o ../../jid.o ../../prep.o \
                                    ../../stanzaextensionfactory.o ../../gloox.o ../../util.o ../../sha.o \
                                    ../../base64.o ../../iq.o
stanzaextensionfactory_test_CFLAGS = $(CPPFLAGS)
