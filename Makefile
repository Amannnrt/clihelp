NAME    := clihelp
VERSION ?= 1.0.0
PREFIX  ?= /usr
DESTDIR ?=
CC      ?= gcc
CFLAGS  ?= -O2 -Wall -Wextra

DATADIR := $(PREFIX)/share/$(NAME)/data
ARCH    := $(shell dpkg --print-architecture 2>/dev/null || echo amd64)
DEB_DIR := build/$(NAME)_$(VERSION)_$(ARCH)

.PHONY: all run install uninstall deb clean

all: $(NAME)

$(NAME): clihelp.c
	$(CC) $(CFLAGS) -DVERSION='"$(VERSION)"' -DDATADIR='"$(DATADIR)"' -o $@ $<

# run from the source folder without installing
run: $(NAME)
	CLIHELP_DATA=data ./$(NAME)

install: $(NAME)
	install -Dm755 $(NAME) $(DESTDIR)$(PREFIX)/bin/$(NAME)
	install -d $(DESTDIR)$(DATADIR)
	install -m644 data/*.txt $(DESTDIR)$(DATADIR)/

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(NAME)
	rm -rf $(DESTDIR)$(PREFIX)/share/$(NAME)

# build a .deb into build/
deb: clean all
	$(MAKE) install DESTDIR=$(DEB_DIR)
	install -d $(DEB_DIR)/DEBIAN
	sed -e 's/@VERSION@/$(VERSION)/' -e 's/@ARCH@/$(ARCH)/' \
	    packaging/control.in > $(DEB_DIR)/DEBIAN/control
	dpkg-deb --root-owner-group -Zxz --build $(DEB_DIR)
	@echo "Built: $(DEB_DIR).deb"

clean:
	rm -rf $(NAME) build
