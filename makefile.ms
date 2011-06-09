PROJ = polyglot
EXE = $(PROJ).exe

OBJS = attack.obj board.obj book.obj book_make.obj book_merge.obj colour.obj\
 engine.obj epd.obj fen.obj game.obj gui.obj hash.obj io.obj line.obj list.obj\
 main.obj mainloop.obj move.obj move_do.obj move_gen.obj move_legal.obj\
 option.obj parse.obj pgn.obj piece.obj pipex_win32.obj pipex_posix.obj\
 random.obj san.obj search.obj square.obj uci.obj uci2uci.obj util.obj\
 xboard2uci.obj

LIBS = oldnames.lib


VCVER=6
DEBUG=0


# VC++ version
!if $(VCVER) >= 8
CLIB = libcmt
LIBF = -MT
DEPRECATE=-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_USE_32BIT_TIME_T
!else
VSO = -Gf -Og
CLIB = msvcrt
LIBF = -MD
!endif


# Enable debugging?
!if $(DEBUG) == 0
CFDEBUG = -DNDEBUG
OPTS = -Os -Oy -Gs -GA -Zl $(VSO) $(LIBF)
LFDEBUG = $(CLIB).lib
!else
CFDEBUG = -D_DEBUG -DDEBUG
OPTS = -Od -Zi -Fd$(PROJ).pdb $(LIBF)d
LFDEBUG = -DEBUG -PDB:$(PROJ).pdb -MAP:$(PROJ).map $(CLIB)d.lib
!endif


CC = @cl
LINK = link
#WARN = -W3
DEFS = $(CFDEBUG) $(DEPRECATE)
CFLAGS = -c -nologo $(WARN) $(DEFS) $(OPTS)
LFLAGS = -nologo $(LFDEBUG)

all: $(EXE)

clean:
	-erase $(OBJS) $(EXE)

$(EXE): $(OBJS)
	$(LINK) $(LFLAGS) $(OBJS) $(LIBS) -out:$(EXE)

.c.obj:
	$(CC) $(CFLAGS) -c $<
