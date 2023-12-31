COMPILER = gcc
LANGUAGE = c

SOURCES = 	main \
			vpn/tunnel vpn/server vpn/client vpn/config vpn/vpndef vpn/args \
			utils/hashmap utils/json utils/system \
			crypto/AES crypto/RSA
LIBS = -lcrypto
SUBDIRS = vpn utils crypto
MKSUBDIRS = $(addprefix $(OBJPATH)/, $(SUBDIRS))

SRCPATH = ./src
BINPATH = ./bin
OBJPATH = ./bin/obj
DBGSUFF = _DBG.o
OPTSUFF = _OPT.o
DBGFLGS = -Wall -g
OPTFLGS = -Ofast
DBGBINP = $(BINPATH)/debug
OPTBINP = $(BINPATH)/optimized

DEPENDS = $(addprefix $(SRCPATH)/, $(addsuffix $(LANGUAGE), $(SOURCES)) $(addsuffix .h, $(SOURCES)))
DBGOBJ = $(addprefix $(OBJPATH)/, $(addsuffix $(DBGSUFF), $(SOURCES)))
OPTOBJ = $(addprefix $(OBJPATH)/, $(addsuffix $(OPTSUFF), $(SOURCES)))

.DEFAULT_GOAL := debug
.PHONY: debug, run, clean, debug-compile, optimized-compile

$(OBJPATH)/%$(DBGSUFF): $(SRCPATH)/%.$(LANGUAGE) $(SRCPATH)/%.h
	$(COMPILER) $(DBGFLGS) $(LIBS) -c -o $@ $<

$(OBJPATH)/%$(OPTSUFF): $(SRCPATH)/%.$(LANGUAGE) $(SRCPATH)/%.h
	$(COMPILER) $(OPTFLGS) $(LIBS) -c -o $@ $<

debug-compile: $(DBGOBJ)
	$(COMPILER) $(DBGFLGS) -o $(DBGBINP) $^ $(LIBS)

optimized-compile: $(OPTOBJ)
	$(COMPILER) $(OPTFLGS) -o $(OPTBINP) $^ $(LIBS)

debug: debug-compile
	$(DBGBINP)

run: optimized-compile
	$(OPTBINP)

clean:
	rm -r ./bin
	mkdir $(BINPATH) $(OBJPATH) $(MKSUBDIRS)