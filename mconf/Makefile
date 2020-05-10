#
# Copyright (C) 2015 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



hostbin-y := mconfig confheader

$(loc_build_tree)/zconf.tab.host.o: $(loc_build_tree)/zconf.lex.c $(loc_build_tree)/zconf.hash.c $(loc_build_tree)/zconf.tab.c


mconfig-y := zconf.tab.o mconf.o lxdialog/

mconfig-hostldlibs := -lncurses
mconfig-hostcppflags := -I"$(loc_src_tree)" -I"$(loc_build_tree)" -DYYDEBUG -DCURSES_LOC="<ncurses.h>"
mconfig-hostcflags := -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -Wno-missing-field-initializers -Wno-sign-compare -Wno-implicit-fallthrough -Wno-shadow
mconfig-yaccflags := --name-prefix=zconf
mconfig-lexflags := --prefix=zconf
mconfig-gperfflags := -a -C -E -g -k '1,3,$$$$' -p -t

confheader-y := confheader.o zconf.tab.o

confheader-hostcppflags := -I"$(loc_src_tree)" -I"$(loc_build_tree)" -DYYDEBUG
confheader-hostcflags := -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -Wno-shadow
confheader-yaccflags := --name-prefix=zconf
confheader-lexflags := --prefix=zconf
confheader-gperfflags := -a -C -E -g -k '1,3,$$$$' -p -t
