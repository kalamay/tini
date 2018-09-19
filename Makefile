all: static dynamic include man

# The default build is probably ok for most things, but it may be customized
# a bit. This can be done by specifying definitions for the following
# configuration options. These values should either be specified on the make
# command line and/or placed in a file named "Build.mk" next to the Makefile.
#
# Build Configuration Options:
#   PREFIX: install prefix for shared libraries (default "/usr/local")
#   DESTDIR: install target directory (default "")
#
#   BUILD: either "release" (default) or "debug"
#   BUILD_ROOT: directory to build in (default "build")
#   
#   FLAGS_common: common compiler and linker flags for release and debug builds
#   CFLAGS_common: compiler flags for release and debug builds
#   CFLAGS_debug: debug only compiler flags
#   CFLAGS_release: release only compiler flags
#   CFLAGS: override for final compiler flags
#
#   LDFLAGS_common: linker flags for release and debug builds
#   LDFLAGS_debug: debug only linker flags
#   LDFLAGS_release: release only linker flags
#   LDFLAGS: override for final linker flags

-include Build.mk

NAME?=tini
PREFIX?=/usr/local
DESTDIR?=
LIBDIR?= $(DESTDIR)$(PREFIX)/lib
INCLUDEDIR?= $(DESTDIR)$(PREFIX)/include
MANDIR?= $(DESTDIR)$(PREFIX)/share/man

VERSION_MAJOR:= 0
VERSION_MINOR:= 1
VERSION_PATCH:= 0
VERSION:=$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)
VERSION_COMPAT:=$(VERSION_MAJOR).$(VERSION_MINOR)

# set default compiler and linker flags
FLAGS_common?= -march=native
CFLAGS_common?= $(FLAGS_common) \
	-DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR) -DVERSION_PATCH=$(VERSION_PATCH) \
	-std=gnu11 -fPIC -D_GNU_SOURCE
CFLAGS_debug?= $(CFLAGS_common) -g -Wall -Wextra -Wcast-align -Werror -fno-omit-frame-pointer -fsanitize=address
CFLAGS_release?= $(CFLAGS_common) -O3 -DNDEBUG 
LDFLAGS_common?= $(FLAGS_common)
LDFLAGS_debug?= $(LDFLAGS_common) -fsanitize=address
LDFLAGS_release?= $(LDFLAGS_common) -O3

# define static and dynamic library names and set platform library flags
LIB:= lib$(NAME).a
ifeq ($(shell uname),Darwin)
  SO:=lib$(NAME).$(VERSION).dylib
  SO_COMPAT:=lib$(NAME).$(VERSION_COMPAT).dylib
  SO_ANY:=lib$(NAME).dylib
  SOFLAGS:= -dynamiclib -install_name $(PREFIX)/lib/$(SO) -compatibility_version $(VERSION_COMPAT) -current_version $(VERSION)
else
  SO:=lib$(NAME).so.$(VERSION)
  SO_COMPAT:=lib$(NAME).so.$(VERSION_MAJOR)
  SO_ANY:=lib$(NAME).so
  SOFLAGS:= -shared -Wl,-rpath=$(PREFIX)/lib
endif

# set build directory variables
BUILD?= release
BUILD_ROOT?= build
BUILD_TYPE:= $(BUILD_ROOT)/$(BUILD)
BUILD_LIB:= $(BUILD_TYPE)/lib
BUILD_INCLUDE:= $(BUILD_TYPE)/include
BUILD_MAN:= $(BUILD_TYPE)/share/man
BUILD_TMP:= $(BUILD_TYPE)/tmp

# update final build flags
CFLAGS?= $(CFLAGS_$(BUILD))
CFLAGS:= $(CFLAGS) -MMD -MP -Iinclude -I$(BUILD_TMP)
LDFLAGS?= $(LDFLAGS_$(BUILD))

# list of souce files to include in lib build
LIBSRC:= src/parse.c src/node.c src/set.c src/err.c

# list of header files to include in build
INCLUDE:= tini.h

# list of manual pages
MAN:=

# list of source files for testing
TEST:= test/parse.c

# list of files to install
INSTALL:= \
	$(LIBDIR)/$(SO) \
	$(LIBDIR)/$(SO_COMPAT) \
	$(LIBDIR)/$(SO_ANY) \
	$(LIBDIR)/$(LIB) \
	$(INCLUDE:%=$(INCLUDEDIR)/%) \
	$(MAN:%=$(MANDIR)/%.gz)

# object files mapped from library source files
LIBOBJ:= $(LIBSRC:src/%.c=$(BUILD_TMP)/$(NAME)-%.o)
# object files mapped from test files
TESTOBJ:= $(TEST:test/%.c=$(BUILD_TMP)/$(NAME)-test-%.o)
# executable files mapped from test files
TESTBIN:= $(TEST:test/%.c=$(BUILD_TMP)/test-%)
# build header files mapped from include files
INCLUDE_OUT:=$(INCLUDE:%=$(BUILD_INCLUDE)/%)
# build man pages mapped from man source files
MAN_OUT:=$(MAN:%=$(BUILD_MAN)/%.gz)

# compile and run all tests
test: $(TESTBIN)
	@for t in $^; do ./$$t; done

# create static library
static: $(BUILD_LIB)/$(LIB)

# compile and link shared library
dynamic: $(BUILD_LIB)/$(SO) $(BUILD_LIB)/$(SO_COMPAT) $(BUILD_LIB)/$(SO_ANY)

# copy all header files into build directory
include: $(INCLUDE_OUT)

# copy all header files into build directory
man: $(MAN_OUT)

# install all build files into destination
install: $(INSTALL)

# copy file from build directory into install destination
$(DESTDIR)$(PREFIX)/%: $(BUILD_TYPE)/%
	@mkdir -p $(dir $@)
	cp -R $< $@

# remove installed files for the current version
uninstall:
	rm -f $(INSTALL)
	@if [ -d $(DESTDIR)$(PREFIX)/include/$(NAME) ]; then rmdir $(DESTDIR)$(PREFIX)/include/$(NAME); fi

# build and execute test
test-%: $(BUILD_TMP)/test-%
	./$<

# create static library archive
$(BUILD_LIB)/$(LIB): $(LIBOBJ) | $(BUILD_LIB)
	$(AR) rcus $@ $^

# link shared library
$(BUILD_LIB)/$(SO): $(LIBOBJ) | $(BUILD_LIB)
	$(CC) $(SOFLAGS) $^ -o $@ $(LDFLAGS)

# create symbolic link for shared library
$(BUILD_LIB)/$(SO_COMPAT) $(BUILD_LIB)/$(SO_ANY):
	cd $(BUILD_LIB) && ln -s $(SO) $(notdir $@)

# copy source headers into build directory
$(BUILD_INCLUDE)/%: include/%
	mkdir -p $(dir $@)
	cp $< $@

# copy source headers into build directory
$(BUILD_MAN)/%.gz: man/%
	mkdir -p $(dir $@)
	gzip < $< > $@

# link test executables
$(BUILD_TMP)/test-%: $(BUILD_TMP)/$(NAME)-test-%.o $(LIBOBJ) | $(BUILD_TMP)
	$(CC) $^ -o $@ $(LDFLAGS)

# compile ragel source files
src/%.c: src/%.rl
	ragel -G2 $< -o $@

# compile main object files
$(BUILD_TMP)/$(NAME)-%.o: src/%.c Makefile | $(BUILD_TMP)
	$(CC) $(CFLAGS) -c $< -o $@

# compile test object files
$(BUILD_TMP)/$(NAME)-test-%.o: test/%.c Makefile | $(BUILD_TMP)
	$(CC) $(CFLAGS) -c $< -o $@

# create directory paths
$(BUILD_LIB) $(BUILD_INCLUDE) $(BUILD_MAN) $(BUILD_TMP):
	mkdir -p $@

# removes the build directory
clean:
	rm -rf $(BUILD_ROOT)

.PHONY: all test static dynamic include man install uninstall show-files clean
.PRECIOUS: $(LIBOBJ) $(TESTOBJ) $(INCLUDE_OUT) $(MAN_OUT)

# include compiler-build dependency files
-include $(LIBOBJ:.o=.d)
-include $(TESTOBJ:.o=.d)

