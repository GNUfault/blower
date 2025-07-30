SU = sudo
CC = gcc
PREFIX = /usr
BINPREFIX = /usr/local
LIBEXEC = $(PREFIX)/libexec/blower
ICONDIR = $(PREFIX)/share/icons/hicolor/scalable/apps
DESKTOPDIR = $(PREFIX)/share/applications
SRC = src
BUILD = build
CFLAGS = `pkg-config --cflags --libs gtk4 libadwaita-1`

all: $(BUILD)/blower $(BUILD)/program install

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/blower: $(SRC)/blower.c | $(BUILD)
	$(CC) $< -o $@

$(BUILD)/program: $(SRC)/main.c | $(BUILD)
	$(CC) $< -o $@ $(CFLAGS)
	
install: install-program install-blower install-desktop install-icon

install-program:
	$(SU) install -d $(LIBEXEC)
	$(SU) install $(BUILD)/program $(LIBEXEC)/

install-blower:
	$(SU) install -d $(BINPREFIX)/bin
	$(SU) install $(BUILD)/blower $(BINPREFIX)/bin/blower

install-desktop:
	$(SU) install -d $(DESKTOPDIR)
	$(SU) install $(SRC)/assets/com.bluMATRIKZ.blower.desktop $(DESKTOPDIR)/

install-icon:
	$(SU) install -d $(ICONDIR)
	$(SU) install $(SRC)/assets/com.bluMATRIKZ.blower.svg $(ICONDIR)/

remove:
	$(SU) rm -f $(BINPREFIX)/bin/blower \
	$(ICONDIR)/com.bluMATRIKZ.blower.svg \
	$(DESKTOPDIR)/com.bluMATRIKZ.blower.desktop
	$(SU) rm -rf $(LIBEXEC)

clean:
	rm -f $(BUILD)/blower $(BUILD)/program

