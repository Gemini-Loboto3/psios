CC		= ccpsx
ASM		= asmpsx
LINK	= slink
LIBLNK	= psylib2

CCOPTIONS  = -Wunused -O$(OPTIMIZE) -G0 -nostdlib -D$(RELMODE) -I.. -I. -c
CCOOPTIONS = -O$(OPTIMIZE) -G0 -c -DOVERLAY -D$(RELMODE)
ASMOPTIONS = /l /o c+ /o an+ /zd /x
#ASMOPTS = /l /c
LINKOPTS = /psx /c /li $(RELMODE)
LINKOOPTS = /psx /p /c /li $(RELMODE)

## sources ##
CORE_SRC = main.c control.c general.c font.c cd.c cdfile.c cdthread.c graph.c gfx.c readpng.c
CORE_S = core.asm
ENCODING_SRC = encoding.c wchar.c cp874.c cp932.c cp936.c cp949.c cp950.c cp1250.c cp1251.c cp1252.c cp1253.c cp1254.c cp1255.c cp1256.c cp1257.c cp1258.c
FS_SRC = diskio.c pff.c
GUI_SRC = test.c list.c pbar.c windows.c

## objects ##
CORE_OBJ =		$(patsubst %.c, $(RELMODE)/%.obj,	${CORE_SRC})
ENCODING_OBJ =	$(patsubst %.c, $(RELMODE)/%.oen,	${ENCODING_SRC})
FS_OBJ =		$(patsubst %.c, $(RELMODE)/%.ofs,	${FS_SRC})
GUI_OBJ =		$(patsubst %.c, $(RELMODE)/%.ogu,	${GUI_SRC})
CORE_ASM =		$(patsubst %.asm, $(RELMODE)/%.obj,	${CORE_S})

ALL_SRC = $(CORE_OBJ) $(ENCODING_OBJ) $(FS_OBJ) $(GUI_OBJ)
ALL_ASM = $(CORE_ASM)

ALL_OBJ = $(ALL_SRC) $(ALL_ASM)
## ------- ##

## compile directives ##
$(RELMODE)/%.obj : core/%.c
	$(CC) $(CCOPTIONS) $< -o $@
$(RELMODE)/%.obj : core/%.asm
	$(ASM) $(ASMOPTIONS) $<,$@
$(RELMODE)/%.oen : encoding/%.c
	$(CC) $(CCOPTIONS) $< -o $@
$(RELMODE)/%.ofs : fs/%.c
	$(CC) $(CCOPTIONS) $< -o $@
$(RELMODE)/%.ogu : gui/%.c
	$(CC) $(CCOPTIONS) $< -o $@
## ----------------- ##

all: inform $(RELMODE)/${OUTFILE}.cpe

inform:
ifneq ($(RELMODE),RELEASE)
ifneq ($(RELMODE),DEBUG)
	@echo "Invalid configuration "$(RELMODE)" specified."
	@echo "You must specify a configuration when running make, e.g."
	@echo  "make RELMODE=DEBUG"
	@echo  
	@echo  "Possible choices for configuration are 'RELEASE' and 'DEBUG'"
	@exit 1
endif
endif
	@echo "Configuration "$(RELMODE)
	@echo "------------------------"

$(RELMODE)/${OUTFILE}.cpe: ${ALL_OBJ} | inform
	$(LINK) $(LINKOPTS) @$(OUTFILE).lk,$@,$(RELMODE)/${OUTFILE}.sym,${OUTFILE}.map
	@cpe2x $(RELMODE)/${OUTFILE}.cpe
	@mv -f $(RELMODE)/${OUTFILE}.exe D:/ePSXe150/pSX/psios/SLPS_99.999
	@mv -f $(RELMODE)/${OUTFILE}.cpe D:/ePSXe150/pSX/psios/SLPS_99.999.cpe
	@rm -f $(RELMODE)/$(OUTFILE).exe
	@rm -f $(RELMODE)/$(OUTFILE).cpe
	#@run.bat
	#@rm -f log.txt

clean: inform
	rm -rd $(RELMODE)/*.*
	rm -rf overlays\*.*

rebuild: clean all

# DO NOT DELETE