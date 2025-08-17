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

all: remove enable-fan-control $(BUILD)/blower $(BUILD)/program install done-msg

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

enable-fan-control:
	@echo "Checking for thinkpad_acpi fan control config..."
	@if ! grep -q '^options thinkpad_acpi fan_control=1' /etc/modprobe.d/thinkpad_acpi.conf 2>/dev/null; then \
		echo "Adding fan_control=1 to /etc/modprobe.d/thinkpad_acpi.conf"; \
		echo 'options thinkpad_acpi fan_control=1' | $(SU) tee -a /etc/modprobe.d/thinkpad_acpi.conf; \
	else \
		echo "Fan control option already present."; \
	fi

	@echo "Checking for thinkpad_acpi in /etc/modules..."
	@if ! grep -q '^thinkpad_acpi' /etc/modules 2>/dev/null; then \
		echo "Adding thinkpad_acpi to /etc/modules"; \
		echo 'thinkpad_acpi' | $(SU) tee -a /etc/modules; \
	else \
		echo "thinkpad_acpi already listed in /etc/modules."; \
	fi

	@echo "Loading thinkpad_acpi module with fan_control=1..."
	@$(SU) modprobe thinkpad_acpi fan_control=1

remove:
	$(SU) rm -f $(BINPREFIX)/bin/blower \
	$(ICONDIR)/com.bluMATRIKZ.blower.svg \
	$(DESKTOPDIR)/com.bluMATRIKZ.blower.desktop
	$(SU) rm -rf $(LIBEXEC)

clean:
	rm -f $(BUILD)/blower $(BUILD)/program

done-msg:
	@echo
	@echo "Done! Now run "blower" to run Blower."
