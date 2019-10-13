#!/bin/bash
#
# Copyright (C) 2015 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



gperffile=$1
cfile=$2	# c++ source file
header=$3	# output file name

#
# check file extension
#

suffix=${cfile##*.}

if [[ "${suffix}" != "c" ]];then
	echo -e $0: input file \"${cfile}\" is no C source file
	exit 1
fi


#
# check if the wordlist has global scope, otherwise it cannot
# be accessed in other source files
#
if [ "$(grep 'global-table' ${gperffile})" == "" ];then
	exit 0
fi


#
#	generate header file based on gperf c output
#		header will contain lookup function prototype
#		as well as macros associated with the hash table
#

# generate macro based on header file
macro=$(basename ${header} | tr a-z. A-Z_)

# print file header
printf "#ifndef %s\n" "${macro}" > ${header}
printf "#define %s\n\n\n" "${macro}" >> ${header}

# included header files
grep -e '#include' ${cfile} >> ${header}
printf "\n\n" >> ${header}

# wordlist macros
printf "/* macros */\n" >> ${header}
grep -e '#define' ${cfile} >> ${header}
printf "\n\n" >> ${header}

# wordlist declaration
wordlist_name=$(grep word-array-name ${gperffile} | cut -d ' ' -f 3)
[ "${wordlist_name}" == "" ] && wordlist_name=wordlist

wordlist_decl=$(sed -ne "s/static \(.*${wordlist_name}.*\[\]\).*/\1/p" ${cfile})

printf "/* global variables */\n" >> ${header}
printf "extern %s;\n" "${wordlist_decl}" >> ${header}
printf "\n\n" >> ${header}

# lookup function prototype
lookup_name=$(grep lookup-function-name ${gperffile} | cut -d ' ' -f 3)
[ "${lookup_name}" == "" ] && lookup_name=in_word_set

lookup_line=$(grep -ne "${lookup_name}" ${cfile} | cut -d ':' -f 1 | tail -n1)
lookup_ret_type=$(sed -ne "$(expr ${lookup_line} - 1)p" ${cfile})

printf "/* prototypes */\n" >> ${header}
printf "%s %s(register const char *str, register size_t len);\n" "${lookup_ret_type}" "${lookup_name}" >> ${header}
printf "\n\n" >> ${header}

# print footer
printf "#endif // %s" "${macro}" >> ${header}


#
#	modify c source file
#		make worldlist a global variable (remove 'static' keyword)
#

sed -i "s/static\(.*${wordlist_name}.*\)/\1/" ${cfile}
