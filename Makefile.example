#
# Copyright (C) 2014 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



################
###   init   ###
################

# init build system
project_type := cxx
scripts_dir := scripts

# init config system
use_config_sys := y
config_ftype := Pconfig
config := ./config
config_tree := $(scripts_dir)/config

# external dependencies
tool_deps :=

# include config
-include $(config)

# init source and build tree
default_build_tree := build/$(CONFIG_BUILD_TYPE)/
src_dirs := example

# include build system Makefile
include $(scripts_dir)/Makefile.inc

# init default flags
cflags := $(CFLAGS) $(CONFIG_CFLAGS)
cxxflags := $(CXXFLAGS) $(CONFIG_CXXFLAGS)
cppflags := $(CPPFLAGS) $(CONFIG_CPPFLAGS)
ldflags := $(LDFLAGS) $(CONFIG_LDFLAGS)
ldlibs := $(LDLIOBSFLAGS) $(CONFIG_LDLIBS)
asflags := $(ASFLAGS) $(CONFIG_ASFLAGS)
archflags := $(ARCHFLAGS) $(CONFIG_ARCHFLAGS)

hostcflags := $(HOSTCFLAGS) $(CONFIG_HOSTCFLAGS)
hostcxxflags := $(HOSTCXXFLAGS) $(CONFIG_HOSTCCCFLAGS)
hostcppflags := $(HOSTCPPFLAGS) $(CONFIG_HOSTCPPFLAGS)
hostldflags := $(HOSTLDFLAGS) $(CONFIG_HOSTLDFLAGS)
hostldlibs := $(HOSTLDLIBS) $(CONFIG_HOSTLDLIBS)
hostasflags := $(HOSTASFLAGS) $(CONFIG_HOSTASFLAGS)
hostarchflags := $(HOSTARCHFLAGS) $(CONFIG_HOSTARCHFLAGS)

yaccflags := $(YACCFLAGS) $(CONFIG_YACCFLAGS)
lexflags := $(LEXFLAGS) $(CONFIG_LEXFLAGS)
gperfflags := $(GPERFFLAGS) $(CONFIG_GPERFFLAGS)

###################
###   targets   ###
###################

####
## build
####
.PHONY: all
ifeq ($(CONFIG_BUILD_DEBUG),y)
all: cflags += -g
all: cxxflags += -g
all: asflags += -g
all: hostcflags += -g
all: hostcxxflags += -g
all: hostasflags += -g
endif

all: $(lib) $(bin) $(hostlib) $(hostbin)

####
## cleanup
####
.PHONY: clean
clean:
	$(rm) $(filter-out $(build_tree)/$(scripts_dir),$(wildcard $(build_tree)/*))

.PHONY: distclean
distclean:
	$(rm) $(config) $(build_tree)

####
## install
####
.PHONY: install-user
install-user: all

.PHONY: install-system
install-system: all

.PHONY: uninstall
uninstall:

####
## help
####

.PHONY: help
help:
