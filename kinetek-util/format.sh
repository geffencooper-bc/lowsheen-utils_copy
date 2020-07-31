#!/bin/sh
# ============================================================================
# Copyright 2019 Brain Corporation. All rights reserved. Brain
# Corporation proprietary and confidential.
# ALL ACCESS AND USAGE OF THIS SOURCE CODE IS STRICTLY PROHIBITED
# WITHOUT EXPRESS WRITTEN APPROVAL FROM BRAIN CORPORATION.
# Portions of this Source Code and its related modules/libraries
# may be governed by one or more third party licenses, additional
# information of which can be found at:
# https://info.braincorp.com/open-source-attributions
# ============================================================================
if [ "$1" = "check_first" ]; then
    check_first=1
else
    check_first=0
fi

# directory of bash script
DIR=$( cd "$( dirname "$0" )" && pwd )

# morsa stm root directory
PROJECT_DIR=${DIR}/..
cd $PROJECT_DIR

commited_files=$(git ls-tree -r HEAD --name-only)
changed_files=$(git diff --name-only)
all_non_ext_files=$(echo "$commited_files\n$changed_files" | grep -v "ext/" | grep -v "venv/" | uniq | xargs ls -d 2>/dev/null)
source_files=$(echo "$all_non_ext_files" | egrep -e "\.c$" -e "\.cpp$" -e "\.h$")
py_format_ignore_regex='scripts/experimental/pid_tuning'
py_files=$(echo "$all_non_ext_files" | grep "\.py$" | egrep -e "scripts/" -e "src" | egrep -v $py_format_ignore_regex)

# check for DEBUG_PRINTFs without "\r\n" line endings.
lines_without_endings=$(grep -n "DEBUG_PRINTF" $source_files | grep '"' | grep -v "\\\\r" | grep -v "\/\/.*no-crlf-check")
if [ ! -z "$lines_without_endings" ]; then
    echo "Uses of DEBUG_PRINTF detected without '\\\r\\\n' at the end"
    echo "To exempt DEBUG_PRINTF statements without a '\\\r\\\n' by adding a trailing '// no-crlf-check' comment"
    echo "$lines_without_endings"
    exit 1
fi

# check for ifdefs in code because it is too easy to accidentally to use #ifdef instead of #if when checking a feature flag
# whitelisted patterns are removed by using "grep -v ..."
files_with_ifdefs=$(grep ifdef $source_files | grep -v "__cplusplus" | grep -v "__GNUC__" | grep -v NDEBUG | grep -v "stm32f4xx_hal_conf.h" | grep -v "stm32fxx_STLparam.h" | grep -v "stm32f4xx_STLsupport.cpp" | grep -v "/CMakeFiles/" | grep -v "__clang__" | grep -v "_CFFI_" | grep -v "GIT_COMMIT_HASH" | grep -v "ENABLE_SAFETY_VALUE_CORRECTION_REPORTING")
if [ ! -z "$files_with_ifdefs" ]; then
    echo "Non-whitelisted ifdefs found!"
    echo "$files_with_ifdefs"
    exit 1
fi

files_with_lint_no_comments=$(grep -rn "//lint \(-e[0-9]*\( \)*\)\+$")
if [ ! -z "$files_with_lint_no_comments" ]; then
    echo "Lint comment found with no justification comment"
    echo "$files_with_lint_no_comments"
    exit 1
fi

# determine if any files need reformating
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

# E251 unexpected spaces around keyword / parameter equals
# E225 missing whitespace around operator
# E226 missing whitespace around arithmetic operator
# E402 imports at top of file  -- needed for some files in scripts/experimental
# W504 line break after binary operator
# determine if any files need reformating
if [ $(autopep8 --ignore E251,E225,E226,E402,W504 --max-line-length 160 -d $py_files | wc -l) -gt 0 ]; then
    # reformat the files
    #autopep8 --ignore E251,E225,E226,W504 --max-line-length 160 -i src/*/*.py src/*/*/*.py
    for f in $py_files
    do
        autopep8 --ignore E251,E225,E226,E402,W504 --max-line-length 160 -i $f
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

grep -o "ESTOP_CODE_\w*" ${PROJECT_DIR}/msg/ScrubberState.msg | sort | uniq > /tmp/ScrubberState_estop_codes.txt
grep -o "ESTOP_CODE_\w*" ${PROJECT_DIR}/src/lowsheen_lib/lowsheen_utils.py | sort | uniq > /tmp/lowsheen_utils_estop_codes.txt
missing_estop_codes=$(cat /tmp/ScrubberState_estop_codes.txt /tmp/lowsheen_utils_estop_codes.txt | sort | uniq -u)
num_missing_estop_codes=$(cat /tmp/ScrubberState_estop_codes.txt /tmp/lowsheen_utils_estop_codes.txt | sort | uniq -u | wc -l)
if [ "$num_missing_estop_codes" -gt "0" ]; then
    echo "ESTOP CODES exist in ScrubberState.msg that aren't in lowsheen_utils.py"
    echo "Missing ESTOP CODES:"
    echo "$missing_estop_codes"
    exit 1
fi


flake8 $py_files --ignore E251,E225,E226,E402,W504 --max-line-length 160
if [ $? -ne 0 ]; then
    exit 1
fi

# run pylint on all python files
pylint -j 8 --rcfile=${PROJECT_DIR}/pylint_rc $py_files
if [ $? -ne 0 ]; then
    echo "Pylint checks failed (see above)"
    exit 1
fi

echo "All code checks passed."