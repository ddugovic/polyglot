#AM_CPPFLAGS=-DDEBUG
#AM_CXXFLAGS=-g

bin_PROGRAMS = polyglot
polyglot_SOURCES = mainloop.c attack.c board.c book.c book_make.c book_merge.c colour.c  engine.c epd.c fen.c gui.c game.c hash.c ini.c io.c line.c list.c main.c move.c move_do.c move_gen.c move_legal.c option.c parse.c pgn.c piece.c pipex_posix.c pipex_win32.c random.c san.c search.c square.c uci.c uci2uci.c util.c xboard2uci.c mainloop.h colour.h hash.h ini.h move_gen.h piece.h uci2uci.h attack.h config.h gui.h io.h move.h pipex.h uci.h board.h engine.h line.h move_legal.h random.h util.h book.h epd.h list.h option.h san.h book_make.h fen.h main.h parse.h search.h book_merge.h game.h move_do.h pgn.h square.h xboard2uci.h

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

EXTRA_DIST=makefile.gcc makefile.ms polyglot.man polyglot.pod polyglot.spec debian/changelog debian/control debian/docs debian/README debian/compat debian/copyright debian/files debian/polyglot.substvars debian/rules

