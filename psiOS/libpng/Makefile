# Makefile for LIBPNG

CC = ccpsx
CFLAGS = -I. -I.. -I../zlib/ -G2 -O2 -nostdlib

TARGET = libpng.lib

LIB_SRC = png.c pngerror.c pngget.c pngmem.c pngpread.c pngread.c\
	pngrio.c pngrtran.c pngrutil.c pngset.c pngtrans.c
#pngwio.c	pngwrite.c pngwtran.c pngwutil.c
LIB_OBJ = $(patsubst %.c, $(RELMODE)/%.obj, ${LIB_SRC})

$(RELMODE)/%.obj : %.c
	$(CC) $(CFLAGS) -c $< -o $@

all : $(TARGET)

$(TARGET) : $(LIB_OBJ)
	psylib /a ../$(TARGET) $(LIB_OBJ)
	
clean:
	rm -f $(RELMODE)/*.*
	rm -f ../$(TARGET)

rebuild: clean all
