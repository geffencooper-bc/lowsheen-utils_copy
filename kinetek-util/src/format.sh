#!/bin/sh
source_files=$(echo "" | egrep -e "\.c$" -e "\.cpp$" -e "\.h$")
echo ${source_files}
files_with_lint_no_comments=$(grep -rn "//lint \(-e[0-9]*\( \)*\)\+$")
if [ ! -z "$files_with_lint_no_comments" ]; then
    echo "Lint comment found with no justification comment"
    echo "$files_with_lint_no_comments"
    exit 1
fi

if [ $(clang-format-3.8 -style=file -output-replacements-xml $source_files | grep "<replacement " -c) -gt 0 ]; then
    # reformat the files
    #clang-format-3.8 -style=file -i firmware/*.cpp firmware/*.h firmware/*/*.cpp firmware/*.h
    for f in $source_files
    do
        clang-format-3.8 -style=file -i "$f"
    done
    # report back to git to indicate that the files need to commited again
    if [ "$check_first" -eq 1 ]; then
        git diff
        exit 1
    fi
fi

# check for files with tabs
files_with_tabs=$(grep -P "\t" -c $source_files $py_files | grep -v ":0" | grep -v "/\.git/")
if [ ! -z "$files_with_tabs" ]; then
    echo "files found with tabs!"
    echo "$files_with_tabs"
    exit 1
fi