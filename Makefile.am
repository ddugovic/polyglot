#AM_CPPFLAGS=-DDEBUG
#AM_CXXFLAGS=-g

bin_PROGRAMS = polyglot
polyglot_SOURCES = adapter.cpp attack.cpp board.cpp book.cpp book_make.cpp book_merge.cpp colour.cpp  engine.cpp epd.cpp fen.cpp gui.cpp game.cpp hash.cpp io.cpp line.cpp list.cpp main.cpp move.cpp move_do.cpp move_gen.cpp move_legal.cpp option.cpp parse.cpp pgn.cpp piece.cpp pipe.cpp posix.cpp random.cpp san.cpp search.cpp square.cpp uci.cpp uci2uci.cpp util.cpp adapter.h colour.h hash.h move_gen.h piece.h uci2uci.h attack.h config.h gui.h io.h move.h pipe.h posix.h uci.h board.h engine.h line.h move_legal.h random.h util.h book.h epd.h list.h option.h san.h book_make.h fen.h main.h parse.h search.h book_merge.h game.h move_do.h pgn.h square.h

dist_doc_DATA = README README1.3 README1.4 README1.4w README1.4w10UCI book_format.html

man6_MANS = polyglot.man

polyglot.man: polyglot.pod
	pod2man -c "" -r "" -s 6 polyglot.pod > polyglot.man

README: polyglot.man
	groff -t -e -mandoc -Tascii polyglot.man| col -bx > README		

deb:
	dpkg-buildpackage

rpm:
	make dist
	rpmbuild -ta polyglot-@VERSION@.tar.gz

EXTRA_DIST=makefile.gcc polyglot.man polyglot.pod polyglot.spec debian/changelog debian/control debian/docs debian/README debian/compat debian/copyright debian/files debian/polyglot.substvars debian/rules

