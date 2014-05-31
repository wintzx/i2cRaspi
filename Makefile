# -------------------------------------------------------------------
#
# -------------------------------------------------------------------

# Choix du compilateur :
CC := g++
# Options
CPPFLAGS := -W -Wall -ansi -pedantic -O3 -std=c++0x
LDFLAGS := -L/usr/local/lib -lwiringPi -lwiringPiDev
SRC := i2cTest.cpp \
		LcdDisplay/LcdDisplay.cpp \
		Ds1621/Ds1621.cpp
VPATH := $(dir $(SRC))
BINDIR := bin
OBJDIR := obj
ARCH := $(shell arch)
OBJS := $(patsubst %.cpp, $(OBJDIR)/$(ARCH)/%.o, $(notdir $(SRC)))
EXEC=i2cTest

all: create_folder $(BINDIR)

# regle pour fabriquer les dossiers;
create_folder: dirobj dirbin
	@echo
	@echo "dossiers crees"
	@echo

# the "common" object files
$(OBJDIR)/$(ARCH)/%.o : %.cpp Makefile
	@echo creating $@ ...
	$(CC) $(CPPFLAGS) -c -o $@ $<

# This will make the cbsdk shared library
$(BINDIR): $(OBJS)
	@echo building output ...
	$(CC) -o $(BINDIR)/$(EXEC) $(OBJS) $(LDFLAGS)


# -------------------------------------------------------------------
#  regles de creation des dossiers
# -------------------------------------------------------------------
.PHONY: dirobj dirbin
# creation du dossier $(OBJDIR) si besoin :
ifeq ($(strip $( $(wildcard $(OBJDIR)) ) ), )
dirobj:
	mkdir -p $(OBJDIR)/$(ARCH)
else
dirobj:
endif

# creation du dossier $(BINDIR) si besoin :
ifeq ($(strip $( $(wildcard $(BINDIR)) ) ), )
dirbin:
	mkdir -p $(BINDIR)
else
dirbin:
endif

# -------------------------------------------------------------------
#  regles de nettoyage
# -------------------------------------------------------------------
.PHONY: clean mrproper

clean:
	@rm -rf $(OBJDIR)

mrproper: clean
	@rm -rf $(BINDIR)

# ------------------------------------------------
# $@ 		Le nom de la cible
# $<		Le nom de la premiere dependance
# $^		La liste des dependances
# shell		pour executer une commande
# wildecard	equivalent de * dans le terminal
# basename	prend le nom sans le .c ou .o ...
# notdir	supprime le chemin devant un fichier
# strip		supprime les blancs => ifeq( $(strip $(VAR) ) , ) = si $(VAR) est vide
# ------------------------------------------------
