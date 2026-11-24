#---------------------------------------------------------------------------------
# WiiPython - Top-Level Build
#
#   make            -> baut alle drei Artefakte (= make all)
#   make app        -> build/boot.dol            (Bootloader, libogc1, kein Python)
#   make loader1    -> build/python-libogc.dol   (Python-Runtime, libogc1)
#   make loader2    -> build/python-libogc2.dol  (Python-Runtime, libogc2)
#   make clean      -> loescht build/
#
# Die Artefakte landen alle im Ordner build/.
#
# Abhaengigkeiten werden aus den INSTALLIERTEN portlibs gezogen
# ($(DEVKITPRO)/portlibs/ppc). Deshalb muss wii-cpython vorher installiert sein:
#
#   # im cpython-Ordner (3.15.0a7):
#   sudo make install LIBOGC=1   # libpython3.15 / libbitmap / libcurl / libwiidl ...
#   sudo make install LIBOGC=2   # libpython3.152 / libbitmap2 / libcurl2 ... (fuer loader2)
#
# Die libogc-abhaengigen .a bekommen bei LIBOGC=2 ein "2"-Suffix und koennen so
# parallel zur libogc1-Variante in denselben portlibs liegen (siehe README des
# cpython-Ports).
#---------------------------------------------------------------------------------

BUILD_ROOT := build

#=================================================================================
# EBENE 1 - Dispatcher (kein TARGET_KIND gesetzt)
#=================================================================================
ifeq ($(TARGET_KIND),)

.PHONY: all app loader1 loader2 loadertest clean
.DEFAULT_GOAL := all

all: folder

folder: app loader1 loader2
	@mkdir -p device-root/apps/WiiPython
	@mkdir -p device-root/python
	@mkdir -p device-root/python/apps
	@cp $(BUILD_ROOT)/boot.dol device-root/apps/WiiPython/boot.dol
	@cp wiipython/* device-root/apps/WiiPython/
	@cp $(BUILD_ROOT)/python-*.dol device-root/python/
	@cp $(BUILD_ROOT)/symbols*.map device-root/python/
	@cp -r scripts/* device-root/python/
	@echo "ADD LIB FOLDER!!"

app:
	@$(MAKE) --no-print-directory _kind TARGET_KIND=app     TOPDIR=$(CURDIR)

loader1:
	@$(MAKE) --no-print-directory _kind TARGET_KIND=loader1 TOPDIR=$(CURDIR)

loader2:
	@$(MAKE) --no-print-directory _kind TARGET_KIND=loader2 TOPDIR=$(CURDIR)

# Minimale Test-DOL (libogc2 + bitmap2, kein Python) zum Isolieren des Loaders.
loadertest:
	@$(MAKE) --no-print-directory _kind TARGET_KIND=loadertest TOPDIR=$(CURDIR)

clean:
	@echo clean ...
	@rm -fr $(BUILD_ROOT)
	@rm -fr device-root

else
#=================================================================================
# EBENE 2/3 - ein konkretes Ziel (TARGET_KIND gesetzt)
#=================================================================================

CRTM := $(DEVKITPPC)/powerpc-eabi/lib/crtmain.o

#--- Per-Ziel-Konfiguration ------------------------------------------------------
ifeq ($(TARGET_KIND),app)
  TARGET  := boot
  SOURCES := source-app source-app/boot
  LIBOGC  := 1
  NEED_PY := 0
  # loader_reloc.c: ohne dieses Makro erwartet es __exception_closeall aus
  # libogc - das exportiert aber weder libogc noch libogc2. Mit dem Makro wird
  # der eingebaute No-op-Stub genutzt.
  EXTRA_CFLAGS := -DBUILD_LIBOGC31
endif
# Lade-Adresse fuer alle DOLs, die von boot.dol per boot_dol() gestartet werden.
# MUSS oberhalb des Loaders (boot.dol @ ~0x80004000-0x80080000) liegen, damit
# reloc_dol sie kopiert, ohne den laufenden Loader-Code zu ueberschreiben.
# 0x80100000 (1 MB) laesst reichlich Platz; die groesste DOL (~10,5 MB) endet
# dann bei ~0x80bA0000, weit unter dem MEM1-Top (0x81800000).
# Wird NUR fuer die geladenen DOLs gesetzt (loader1/loader2/loadertest), NICHT
# fuer app/boot.dol -- der bleibt auf 0x80004000 (HBC-gueltig).

ifeq ($(TARGET_KIND),loader1)
  TARGET  := python-libogc
  SOURCES := source-loader
  LIBOGC  := 1
  NEED_PY := 1
  SYMMAP  := symbols1.map
  EXTRA_CFLAGS := -DWIIPY_SYMBOLS_MAP='"python/$(SYMMAP)"'
  PY_TEXT_ADDR := 0x80100000
endif
ifeq ($(TARGET_KIND),loader2)
  TARGET  := python-libogc2
  SOURCES := source-loader
  LIBOGC  := 2
  NEED_PY := 1
  SYMMAP  := symbols2.map
  EXTRA_CFLAGS := -DWIIPY_SYMBOLS_MAP='"python/$(SYMMAP)"'
  PY_TEXT_ADDR := 0x80100000
endif
ifeq ($(TARGET_KIND),loadertest)
  TARGET  := loadertest
  PY_TEXT_ADDR := 0x80100000
  SOURCES := source-test
  LIBOGC  := 2
  NEED_PY := 0
  EXTRA_CFLAGS :=
endif

OBJDIR := $(BUILD_ROOT)/$(TARGET_KIND)

#--- libogc-Version waehlen (wii_rules zwingt LIBOGC_LIB immer auf v1, daher
#--- hier direkt aus LIBOGC ableiten) --------------------------------------------
ifeq ($(LIBOGC),2)
  OGC_INC := $(DEVKITPRO)/libogc2/wii/include
  OGC_LIB := $(DEVKITPRO)/libogc2/wii/lib
else
  OGC_INC := $(DEVKITPRO)/libogc/include
  OGC_LIB := $(DEVKITPRO)/libogc/lib/wii
endif

#--- Includes --------------------------------------------------------------------
INCLUDE += -I$(DEVKITPRO)/portlibs/ppc/include
INCLUDE += -I$(DEVKITPRO)/portlibs/ppc/include/Python
INCLUDE += -I$(OGC_INC)
INCLUDE += -I$(OGC_INC)/ogc
INCLUDE += $(foreach dir,$(SOURCES),-I$(TOPDIR)/$(dir))

#--- Flags -----------------------------------------------------------------------
MACHDEP   := -DGEKKO -mrvl -mcpu=750 -meabi -mhard-float

CFLAGS    += $(MACHDEP) -Os -g -std=c99 -Wall -Wextra -fdata-sections -ffunction-sections -D__wii__
CFLAGS    += $(EXTRA_CFLAGS)
CFLAGS    += $(INCLUDE)
CXXFLAGS  := $(CFLAGS)
CPPFLAGS  += $(MACHDEP) -Os -g -D__wii__
CPPFLAGS  += $(INCLUDE)

LDFLAGS   += $(MACHDEP)
LDFLAGS   += -T$(DEVKITPPC)/powerpc-eabi/lib/rvl.ld -Wl,--gc-sections
LDFLAGS   += -L$(OGC_LIB)
LDFLAGS   += -L$(DEVKITPRO)/portlibs/ppc/lib

# Der Loader (app/boot.dol) bleibt auf der HBC-gueltigen Standardadresse
# 0x80004000 (hoeher lehnt der HBC ab -- 0x81330000 ist seine eigene Region).
# Stattdessen werden die geladenen python-DOLs HOCH gelinkt (siehe loader1/
# loader2: PY_TEXT_ADDR), damit reloc_dol sie OBERHALB des winzigen Loaders
# ablegt und dessen laufenden Code nicht ueberschreibt.
ifneq ($(PY_TEXT_ADDR),)
  LDFLAGS += -Wl,--section-start,.init=$(PY_TEXT_ADDR)
endif

#--- Libraries -------------------------------------------------------------------
ifeq ($(NEED_PY),1)
  # Force-retain: CPython-API-Symbole, die NUR von SD-.so-Modulen (dlopen) ueber
  # die symbols.map gezogen werden, nicht vom Hauptprogramm. Ohne -u wirft
  # --gc-sections sie raus -> der dlopen-Loader faende sie nicht ("unresolved").
  LDFLAGS += -Wl,-u,_PyOS_URandomNonblock
  LDFLAGS += -Wl,-u,_Py_c_neg
  WII_SO_SYMS := \
  	PyObject_GetTypeData PyGILState_Check \
  	_PyArg_ParseStack _Py_IncRef PyUnicode_InternInPlace \
  	PyExc_ArithmeticError \
  	PyLong_AsUInt32 PyLong_AsUnsignedLongMask PyLong_FromInt32 \
  	wii_dlclose localtime geteuid \
  	pthread_mutex_trylock \
  	pthread_mutexattr_init pthread_mutexattr_settype pthread_mutexattr_destroy
  LDFLAGS += $(foreach s,$(WII_SO_SYMS),-Wl,-u,$(s))

  ifeq ($(LIBOGC),2)
    # libogc2-Build: die libogc-abhaengigen Libs tragen das "2"-Suffix
    # (python3.152, curl2, bitmap2, wiidl2).
    LIBS := -lpython3.152 -lwiidl2 -lcurl2 -lmbedtls -lmbedx509 -lmbedcrypto -ltfpsacrypto \
            -lz -lgdbm_compat -lgdbm -llzma -lbz2 -luuid \
            -lfat -lwiiuse -lbte -lwiikeyboard -lbitmap2 -logc -lm
  else
    LIBS := -lpython3.15 -lwiidl -lcurl -lmbedtls -lmbedx509 -lmbedcrypto -ltfpsacrypto \
            -lz -lgdbm_compat -lgdbm -llzma -lbz2 -luuid \
            -lfat -lwiiuse -lbte -lwiikeyboard -lbitmap -logc -lm
  endif
else
  # source-app ist ein reiner Bootloader (laedt die python-*.dol per boot_dol),
  # bindet KEIN Python ein -> nur libogc + fat. bitmap-Lib passend zur
  # libogc-Version (bitmap2 ist gegen libogc2 gebaut).
  ifeq ($(LIBOGC),2)
    LIBS := -lfat -lwiiuse -lbte -logc -lm -lbitmap2
  else
    LIBS := -lfat -lwiiuse -lbte -logc -lm -lbitmap
  endif
endif

include $(DEVKITPPC)/wii_rules

#--------------------------------------------------------------------------------
ifndef IN_BUILD
#--- Top-Half: laeuft im Projekt-Root, rekursiert in den Objekt-Ordner -----------

export OUTPUT  := $(TOPDIR)/$(BUILD_ROOT)/$(TARGET)
export VPATH   := $(foreach dir,$(SOURCES),$(TOPDIR)/$(dir))
export DEPSDIR := $(TOPDIR)/$(OBJDIR)

# Nur die oberste Ebene jedes Quellordners wird kompiliert (kein rekursives
# Glob). Nicht benoetigte Dateien einfach in einen Unterordner legen
# (z.B. source-app/boot/disabled/) -> werden automatisch ignoriert.
CFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(TOPDIR)/$(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(TOPDIR)/$(dir)/*.cpp)))
SFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(TOPDIR)/$(dir)/*.S)))

ifeq ($(strip $(CPPFILES)),)
  export LD := $(CC)
else
  export LD := $(CXX)
endif

export OFILES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.S=.o)

.PHONY: _kind
_kind:
	@echo "=== $(TARGET_KIND): $(TARGET).dol (libogc$(LIBOGC)) ==="
	@mkdir -p $(TOPDIR)/$(OBJDIR) $(TOPDIR)/$(BUILD_ROOT)
	@$(MAKE) --no-print-directory -C $(TOPDIR)/$(OBJDIR) -f $(TOPDIR)/Makefile IN_BUILD=1 $(OUTPUT).dol
ifeq ($(NEED_PY),1)
	@$(PREFIX)nm --defined-only $(OUTPUT).elf > $(TOPDIR)/$(BUILD_ROOT)/$(SYMMAP)
	@echo "erzeugt: $(BUILD_ROOT)/$(SYMMAP) ($$(wc -l <$(TOPDIR)/$(BUILD_ROOT)/$(SYMMAP)) Symbole)"
endif

else
#--- Bottom-Half: laeuft im Objekt-Ordner (CWD = $(OBJDIR)), linkt das .elf ------

$(OUTPUT).dol: $(OUTPUT).elf

$(OUTPUT).elf: $(OFILES)
	$(LD) -Wl,--allow-multiple-definition $(LDFLAGS) $(CRTM) $(OFILES) $(LIBS) -o $@

-include $(DEPSDIR)/*.d

endif # IN_BUILD

endif # TARGET_KIND
