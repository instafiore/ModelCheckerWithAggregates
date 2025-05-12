#!/usr/bin/bash
today=$(date "+%Y-%m-%d_%H:%M")
first_file=1
outputfile=results/joined_excels
while [[ $# -gt 0 ]]; do
    arg="$1"
    files=$(ls $arg)
    for file in $files; do
        abs_path="$arg"/"$file"
        if [[ $first_file -eq 1 ]]; then
            rm_header="1"
            first_file=0
        else
            rm_header="2"
        fi
        if ./extract.pl "$abs_path" > "tmp" 2>/dev/null; then
            echo "extracting $abs_path"
            if tail -n +"$rm_header" tmp >> "$outputfile/${today}"; then
                rm tmp
            else
                echo "Error: Failed to append to extracted file."
                rm tmp
                exit 2
            fi
        else
            echo "extracting $file"
            $(./extract.pl $file > "tmp" ; tail -n +$rm_header tmp >> $outputfile/$today""; rm tmp)
            rm tmp 2>/dev/null 
        fi
    done
    echo "$1 extracted"
    shift
done