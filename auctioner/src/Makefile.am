AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = auctionManager

if ENABLE_DEBUG
  AM_CXXFLAGS = -I$(top_srcdir)/lib/getopt_long \
				-I$(top_srcdir)/lib/httpd \
				-I$(top_srcdir)/foundation/include \
				-g -fno-inline -DDEBUG -ggdb -DPROFILING
else
  AM_CXXFLAGS = -O2 -I$(top_srcdir)/lib/getopt_long \
				-I$(top_srcdir)/lib/httpd \
				-I$(top_srcdir)/foundation/include
endif

auctionManager_SOURCES =  EventSchedulerAuctioner.cpp \
						  PageRepository.cpp \
						  CtrlComm.cpp \
						  AUMProcessor.cpp \
						  Auctioner.cpp \
						  AnslpProcessor.cpp \
						  main.cpp

auctionManager_CPPFLAGS = -I$(top_srcdir)/lib/getopt_long \
						  -I$(top_srcdir)/lib/httpd \
						  -I$(top_srcdir)/auctioner/include \
						  $(LIBGIST_CFLAGS) $(LIBPROT_CFLAGS) $(LIBFASTQUEUE_CFLAGS) $(PQXX_CFLAGS) \
						  $(LIBIPAP_CFLAGS) $(LIBANSLP_MSG_CFLAGS) $(LIBANSLP_CFLAGS) 

auctionManager_LDADD = $(top_builddir)/lib/getopt_long/libgetopt_long.a \
					   $(top_builddir)/lib/httpd/libhttpd.a \
					   $(top_builddir)/foundation/src/libauctionfdtion.la \
					   @PTHREADLIB@ @DLLIB@ @SSLLIB@ @XMLLIB@ $(PQXX_LIBS)

auctionManager_LDFLAGS = -export-dynamic -module \
						 $(CPPUNIT_LIBS) -ldl -lcppunit -lssl $(top_builddir)/lib/getopt_long/libgetopt_long.a \
						 $(top_builddir)/lib/httpd/libhttpd.a \
						 $(top_builddir)/foundation/src/libauctionfdtion.la \
						 @PTHREADLIB@ @DLLIB@ @SSLLIB@ @XMLLIB@ $(LIBIPAP_LIBS) \
						 $(LIBPROT_LIBS) $(LIBFASTQUEUE_LIBS) $(LIBGIST_LIBS) \
					     $(LIBANSLP_MSG_LIBS) $(LIBANSLP_LIBS)

# what flags you want to pass to the C compiler & linker
AM_CPPFLAGS = -g -I@top_srcdir@/include
			 
LDADD = $(LIBIPAP_LIBS)

$(top_builddir)/lib/httpd/libhttpd.a:
	cd $(top_builddir)/lib/httpd ; $(MAKE)

$(top_builddir)/lib/getopt_long/libgetopt_long.a:
	cd $(top_builddir)/lib/getopt_long ; $(MAKE)

$(top_builddir)/foundation/src/libauctionfdtion.la:
	cd $(top_builddir)/foundation/src ; $(MAKE)


install-data-hook:
	$(INSTALL) -d $(DESTDIR)/$(localstatedir)/run
	$(INSTALL) -d $(DESTDIR)/$(localstatedir)/log
