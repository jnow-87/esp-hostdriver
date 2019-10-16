################
###   init   ###
################

# init build system
project_type := c
scripts_dir := scripts/build

# init config system
use_config_sys := n

# external dependencies
tool_deps :=

# include config
-include $(config)

# init source and build tree
default_build_tree := build/
src_dirs := main/

# include build system Makefile
include $(scripts_dir)/Makefile.inc

# init default flags
cflags := $(CFLAGS) $(cflags)
cxxflags := $(CXXFLAGS) $(cxxflags)
cppflags := $(CPPFLAGS) $(cppflags) -I"include/"
ldflags := $(LDFLAGS) $(ldflags)
ldlibs := $(LDLIBSFLAGS) $(ldlibs)
asflags := $(ASFLAGS) $(asflags)
archflags := $(ARCHFLAGS) $(archflags)

yaccflags := $(YACCFLAGS) $(yaccflags)
lexflags := $(LEXFLAGS) $(lexflags)
gperfflags := $(GPERFFLAGS) $(gperfflags)

###################
###   targets   ###
###################

####
## build
####
.PHONY: all
all: $(lib) $(bin)

.PHONY: debug
debug: cflags += -g
debug: cxxflags += -g
debug: asflags += -g
debug: all


####
## cleanup
####
.PHONY: clean
clean:
	$(rm) $(filter-out $(build_tree)/$(scripts_dir),$(wildcard $(build_tree)/*))

.PHONY: distclean
distclean:
	$(rm) $(config) $(build_tree)
