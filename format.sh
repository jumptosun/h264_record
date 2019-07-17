#!/bin/bash

source_file=""

get_source_file() {
    source_file=$(ls $1 | grep "\.h\>")
    source_file+=" "

    source_file+=$(ls $1 | grep "\.hpp\>") 
    source_file+=" "

    source_file+=$(ls $1 | grep "\.c\>") 
    source_file+=" "

    source_file+=$(ls $1 | grep "\.cc\>") 
    source_file+=" "

    source_file+=$(ls $1 | grep "\.cpp\>") 
    source_file+=" "
}

format_folder() {
    for file in $(ls $1); do
        if [[ -d $file ]] 
        then
            format_folder $file
        fi
    done 

    get_source_file $1
    for file in $source_file; do
        clang-format -i "$1/$file" -style WebKit
    done
}

main() {
    format_folder $(pwd)
} 
main
