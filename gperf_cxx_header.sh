#!/bin/bash
#
# Copyright (C) 2015 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



cfile=$1	# c++ source file
header=$2	# output file name

suffix=${cfile##*.}

if [[ "${suffix}" != "cc" && "${suffix}" != "cpp" ]];then
	echo -e $0: input file \"${cfile}\" is no C++ source file
	exit 1
fi


#
#	generate header file based on gerf c++ output
#		header will contain class definition extracted
#		from c++ source and extended by the macros
#		defined in the source file
#


# generate macro based on header file
macro=$(basename ${header} | tr a-z. A-Z_)

# print header
printf "#ifndef %s\n" "${macro}" > ${header}
printf "#define %s\n\n\n" "${macro}" >> ${header}

# extract included header files
grep -e '#include' ${cfile} >> ${header}
printf "\n\n" >> ${header}

# extract class, removing the last line, i.e. the closing "};"
class_def=$(sed -e '/class/,/}/!d' ${cfile} | sed '$d')
class_name=$(echo ${class_def} | cut -d ' ' -f 2)

printf "%s\n\n" "${class_def}" >> ${header}

# extract macro values and place them as const members into the class
printf "public:\n" >> ${header}

while read line
do
	name=$(echo ${line} | cut -d ' ' -f 2)
	value=$(echo ${line} | cut -d ' ' -f 3)

	printf "  static const unsigned int %s = %s;\n" "${name}" "${value}" >> ${header}
done <<< "$(grep -e '#define' ${cfile})"

printf "\n" >> ${header}

# extract wordlist definition and declaration
wordlist_decl=$(grep -e  "wordlist\[\]" ${cfile} | cut -d '=' -f 1 | sed "s/static//")
wordlist_def=$(sed -e '/wordlist\[\]/,/};/!d' ${cfile} | cut -d '=' -f 2)

# add wordlist declaration to class
printf "  static %s;\n" "${wordlist_decl}" >> ${header}

# print class closing brace
printf "};\n" >> ${header}

# print footer
printf "\n\n#endif // %s" "${macro}" >> ${header}


#
#	modify c++ source file
#		replace class definition with generated header file
#		and add initialisation of class member wordlist
#


# add header to beginning of file
sed -i -e "1s:^:#include \"$(basename ${header})\"\n:" ${cfile}

# remove class declaration
sed -i -e '/class/,/}/d' ${cfile}

# add initialisation of class member wordlist
printf "%s = " "$(echo ${wordlist_decl} | sed "s/wordlist/${class_name}::wordlist/")" >> ${cfile}
printf "%s" "${wordlist_def}" >> ${cfile}


exit 0
