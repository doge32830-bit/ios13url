# palera1n for usbliter8
#
# The upstream fork assumed that every dependency was installed in a local
# build tree and invoked `brew` unconditionally.  That made a normal Intel
# macOS checkout fail before the compiler was even run.  Keep the build
# self-contained, but use Homebrew/pkg-config when they are available.

SHELL := /bin/sh
SRC := $(CURDIR)
DEP ?= $(SRC)/dep_root
CC ?= cc
STRIP ?= strip
PKG_CONFIG ?= pkg-config
TARGET_OS ?= $(shell uname -s)
UNAME := $(shell uname -s)
HOST_ARCH := $(shell uname -m)

# Homebrew is /usr/local on Intel and /opt/homebrew on Apple Silicon.  Do not
# call brew unless it exists: command substitution errors used to obscure the
# real compiler error on machines without Homebrew on PATH.
BREW_PREFIX ?= $(shell if command -v brew >/dev/null 2>&1; then brew --prefix; fi)
BREW_CFLAGS := $(if $(BREW_PREFIX),-I$(BREW_PREFIX)/include,)
BREW_LDFLAGS := $(if $(BREW_PREFIX),-L$(BREW_PREFIX)/lib,)
BREW_FORMULAE := libusb libimobiledevice libirecovery libusbmuxd libimobiledevice-glue libplist mbedtls readline
BREW_PKG_CONFIG_PATH := $(shell \
	if command -v brew >/dev/null 2>&1; then \
		for formula in $(BREW_FORMULAE); do \
			prefix=$$(brew --prefix "$$formula" 2>/dev/null) || continue; \
			printf '%s/lib/pkgconfig:%s/share/pkgconfig:' "$$prefix" "$$prefix"; \
		done; \
	fi)
export PKG_CONFIG_PATH := $(BREW_PKG_CONFIG_PATH)$(PKG_CONFIG_PATH)

# libusb is needed by the usbliter8 DFU transport even when the normal
# palera1n USB backend uses macOS IOKit.
LIBUSB_CFLAGS := $(shell if command -v "$(PKG_CONFIG)" >/dev/null 2>&1; then "$(PKG_CONFIG)" --cflags libusb-1.0 2>/dev/null; fi)
LIBUSB_LIBS := $(shell if command -v "$(PKG_CONFIG)" >/dev/null 2>&1; then "$(PKG_CONFIG)" --libs libusb-1.0 2>/dev/null; fi)

CFLAGS += -isystem $(DEP)/include -I$(DEP)/include/libusb-1.0 -I$(SRC)/include -I$(SRC) -I$(SRC)/src
CFLAGS += $(BREW_CFLAGS) $(LIBUSB_CFLAGS)
CFLAGS += -D_XOPEN_SOURCE=700 -D_POSIX_C_SOURCE=200809L -D_C99_SOURCE
CFLAGS += -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-strict-prototypes
CFLAGS += -std=c99 -pedantic-errors -DHAVE_LIBIMOBILEDEVICE

LDFLAGS += $(BREW_LDFLAGS)
LIBS += $(LIBUSB_LIBS)
LIBS += -limobiledevice-1.0 -lirecovery-1.0 -lusbmuxd-2.0
LIBS += -limobiledevice-glue-1.0 -lplist-2.0
LIBS += -lmbedtls -lmbedcrypto -lmbedx509 -lreadline
LIBS += -pthread -lm

ifeq ($(TARGET_OS),Darwin)
CFLAGS += -Wno-nullability-extension
ifeq (,$(findstring version-min=, $(CFLAGS)))
CFLAGS += -mmacosx-version-min=10.8
endif
LDFLAGS += -Wl,-dead_strip
LIBS += -framework CoreFoundation -framework IOKit
else
CFLAGS += -fdata-sections -ffunction-sections
LDFLAGS += -Wl,--gc-sections
endif

ifeq ($(TUI),1)
ifeq ($(TARGET_OS),Linux)
LIBS += $(DEP)/lib/libgpm.a
endif
endif

ifeq ($(DEV_BUILD),1)
CFLAGS += -O0 -g -DDEV_BUILD -fno-omit-frame-pointer
ifeq ($(ASAN),1)
BUILD_STYLE = ASAN
CFLAGS += -fsanitize=address,undefined -fsanitize-address-use-after-return=runtime
else ifeq ($(TSAN),1)
BUILD_STYLE = TSAN
CFLAGS += -fsanitize=thread,undefined
else
BUILD_STYLE = DEVELOPMENT
endif
else
CFLAGS += -Os -g
BUILD_STYLE = RELEASE
endif

ifeq ($(TARGET_OS),Linux)
ifneq ($(shell echo '$(BUILD_STYLE)' | grep -q '[A-Z]\+SAN' && echo 1),1)
LDFLAGS += -static -no-pie
endif
endif

ifneq ($(BAKERAIN_DEVELOPE_R),)
CFLAGS += -DBAKERAIN_DEVELOPE_R="\"$(BAKERAIN_DEVELOPE_R)\""
endif

BUILD_NUMBER := $(shell git rev-list --count HEAD 2>/dev/null || echo 0)
BUILD_TAG := $(shell git describe --dirty --tags --abbrev=7 2>/dev/null || echo unknown)
BUILD_BRANCH := $(shell git rev-parse --abbrev-ref HEAD 2>/dev/null || echo unknown)
BUILD_COMMIT := $(shell git rev-parse HEAD 2>/dev/null || echo unknown)

CFLAGS += -DBUILD_STYLE="\"$(BUILD_STYLE)\"" -DBUILD_TAG="\"$(BUILD_TAG)\""
CFLAGS += -DBUILD_NUMBER="\"$(BUILD_NUMBER)\"" -DBUILD_BRANCH="\"$(BUILD_BRANCH)\""
CFLAGS += -DBUILD_COMMIT="\"$(BUILD_COMMIT)\""

CPATH =
LIBRARY_PATH =
export SRC DEP UNAME CC STRIP CFLAGS LDFLAGS LIBS SHELL TARGET_OS DEV_BUILD BUILD_DATE BUILD_TAG BUILD_WHOAMI BUILD_STYLE BUILD_NUMBER BUILD_BRANCH CC_FOR_BUILD CFLAGS_FOR_BUILD

# Only download the checkra1n payload for the host platform.  The old target
# fetched four unused Linux images on an Intel Mac before compiling anything.
ifeq ($(TARGET_OS),Darwin)
DOWNLOAD_CHECKRA1N = checkra1n-macos
else ifeq ($(HOST_ARCH),x86_64)
DOWNLOAD_CHECKRA1N = checkra1n-linux-x86_64
else ifneq (,$(findstring arm64,$(HOST_ARCH)))
DOWNLOAD_CHECKRA1N = checkra1n-linux-arm64
else
DOWNLOAD_CHECKRA1N = checkra1n-linux-x86
endif

DOWNLOAD_RESOURCES = $(DOWNLOAD_CHECKRA1N) checkra1n-kpf-pongo ramdisk.dmg binpack.dmg Pongo.bin
ifeq ($(NO_CHECKRAIN),1)
DOWNLOAD_RESOURCES := $(filter-out $(DOWNLOAD_CHECKRA1N),$(DOWNLOAD_RESOURCES))
endif
ifeq ($(NO_KPF),1)
DOWNLOAD_RESOURCES := $(filter-out checkra1n-kpf-pongo,$(DOWNLOAD_RESOURCES))
endif
ifeq ($(NO_RAMDISK),1)
DOWNLOAD_RESOURCES := $(filter-out ramdisk.dmg,$(DOWNLOAD_RESOURCES))
endif
ifeq ($(NO_OVERLAY),1)
DOWNLOAD_RESOURCES := $(filter-out binpack.dmg,$(DOWNLOAD_RESOURCES))
endif
ifeq ($(NO_CUSTOM_PONGO),1)
DOWNLOAD_RESOURCES := $(filter-out Pongo.bin,$(DOWNLOAD_RESOURCES))
endif

.PHONY: all palera1n clean docs distclean check-deps install uninstall test

all: palera1n

test:
	@tmp_bin=$$(mktemp "$${TMPDIR:-/tmp}/usbliter8-cpids.XXXXXX"); \
	$(CC) -std=c99 -Wall -Wextra -pedantic -I$(SRC)/include tests/test_usbliter8_cpids.c -o "$$tmp_bin" && "$$tmp_bin"; \
	status=$$?; rm -f "$$tmp_bin"; exit $$status

palera1n: check-deps download-deps
	$(MAKE) -C src

check-deps:
	@missing=""; \
	if ! command -v curl >/dev/null 2>&1; then missing="$$missing curl"; fi; \
	if ! command -v xxd >/dev/null 2>&1; then missing="$$missing xxd"; fi; \
	if ! command -v "$(PKG_CONFIG)" >/dev/null 2>&1; then missing="$$missing pkg-config"; \
	else \
		for package in libusb-1.0 libimobiledevice-1.0 libirecovery-1.0 libusbmuxd-2.0 libimobiledevice-glue-1.0 libplist-2.0 mbedtls mbedcrypto mbedx509 readline; do \
			if ! "$(PKG_CONFIG)" --exists "$$package" >/dev/null 2>&1; then missing="$$missing $$package"; fi; \
		done; \
	fi; \
	if [ -n "$$missing" ]; then \
		echo "error: missing build dependencies:$$missing" >&2; \
		if [ "$(TARGET_OS)" = "Darwin" ]; then echo "Install them with: brew install make pkg-config libusb libimobiledevice libirecovery libusbmuxd libimobiledevice-glue libplist mbedtls readline xz vim" >&2; fi; \
		exit 1; \
	fi

# The resource rules live in src/Makefile.
download-deps:
	$(MAKE) -C src $(addprefix resources/,$(DOWNLOAD_RESOURCES))

clean:
	$(MAKE) -C src clean
	$(MAKE) -C docs clean

docs:
	$(MAKE) -C docs

distclean: clean
	$(MAKE) -C src distclean

PREFIX ?= /usr/local/bin

install: palera1n
	install -m755 src/palera1n $(PREFIX)/palera1n-for-usbliter8
	ln -sf $(PREFIX)/palera1n-for-usbliter8 $(PREFIX)/palera1n

uninstall:
	rm -f $(PREFIX)/palera1n
	rm -f $(PREFIX)/palera1n-for-usbliter8
