# Makefile for ZLIB

CC = ccpsx
CFLAGS = -I. -I.. -G0 -O2 -nostdlib

TARGET = zlib.lib

LIB_SRC = adler32.c crc32.c infback.c inffast.c inflate.c inftrees.c\
	trees.c zutil.c
#deflate.c gzwrite.c compress.c uncompr.c
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
