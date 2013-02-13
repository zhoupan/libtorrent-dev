# To build java bindings, do:
#
# make autogen
# make lib
# make

INCLUDE = -I /opt/local/include/ -I ../../libtorrent/include/ -I ../../libtorrent/include/libtorrent -I ./c/
LIB = -L . -L ../../libtorrent/src/.libs -L /opt/local/lib/
JNAERATOR = jnaerator-0.11-shaded.jar
FLAGS = -DBOOST_FILESYSTEM_VERSION=2 -DBOOST_ASIO_DYN_LINK -fPIC

all: libtorrent.dylib jar pack mvn

library.o: ./c/library.cpp ./c/libtorrent.h
	g++ $(INCLUDE) $(FLAGS) -c $<

../../libtorrent/src/.libs/libtorrent-rasterbar.a:
	make lib

libtorrent.dylib: library.o ../../libtorrent/src/.libs/libtorrent-rasterbar.a
	g++ $(INCLUDE) $(LIB) ../../libtorrent/src/.libs/libtorrent-rasterbar.a -l boost_system-mt \
	-l boost_filesystem-mt -l boost_thread-mt -l ssl -l crypto -l z \
	library.o $(FLAGS) -shared -o libtorrent.dylib

lib:
	make -C ../../libtorrent

autogen:
	(cd ../../libtorrent && autoreconf -i)
	(cd ../../libtorrent && ./configure)

simple_client:
	g++ $(INCLUDE) $(LIB) $(FLAGS) -l torrent ./c/simple_client.c -o simple_client

clean:
	rm -f _jnaerator*
	rm -f libtorrent.dylib
	rm -f libtorrent.jar
	rm -f simple_client
	rm -f libtorrent-rasterbar*
	rm -f out.errors.txt
	rm -f library.o

jnaerator-0.11-shaded.jar:
	wget http://jnaerator.googlecode.com/files/jnaerator-0.11-shaded.jar

jar: $(SHAREDLIBRARY) $(JNAERATOR)
	java -jar $(JNAERATOR) -noLibBundle -library torrent ./c/libtorrent.h \
	libtorrent.dylib -package com.rasterbar.libtorrent -o . -jar libtorrent.jar -v \
	-mode Jar -runtime JNA

pack:
	rm -f libtorrent-mac.jar
	(cd . && zip libtorrent-mac.jar *.so *.dylib)

mvn:
	mvn install:install-file -Dfile=libtorrent.jar \
	  -DgroupId=com.rasterbar \
      -DartifactId=libtorrent \
      -Dversion=0.16.8-SNAPSHOT \
      -Dpackaging=jar \
      -DgeneratePom=true \
      -DcreateChecksum=true

	mvn install:install-file -Dfile=libtorrent-mac.jar \
	  -DgroupId=com.rasterbar \
      -DartifactId=libtorrent \
      -Dversion=0.16.8-SNAPSHOT \
      -Dpackaging=jar \
      -Dclassifier=natives-mac \
      -DgeneratePom=true \
      -DcreateChecksum=true
