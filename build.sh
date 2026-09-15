#!/bin/bash

# script working directory
SWD="$(dirname "$(realpath "$0")")"

# settings - output
OUTPUT="sft-simp-draw"

# settings - directories
DIRS=("./")

# settings - path of static libraries
PATH_ADDITIONAL_LNK=()

# settings - flags all
FLAGS_ALL=(-march=x86-64-v3 -pthread -DAPP_NAME="$OUTPUT")

# settings - flags by option
FLAGS_DEBUG=("${FLAGS_ALL[@]}" -DDEBUG -O0 -g)
FLAGS_RELEASE=("${FLAGS_ALL[@]}" -O2)

FLAGS_DEFAULT=("${FLAGS_RELEASE[@]}")

# settings - libraries
LIBS=(freetype2 libavformat libavcodec libavutil libswscale)

# settings - standards
STD_C="c23"
STD_CPP="c++23"

# get args by using pkg-config
PKG_CONFIG_INCLUDE=$(pkg-config --cflags "${LIBS[@]}")
PKG_CONFIG_LIBS=$(pkg-config --libs "${LIBS[@]}")

# default
FLAGS_FINAL=("${FLAGS_DEFAULT[@]}")

FLAGS_LINKING=(-lm)

# file to store last build flags and libs linked
FLAGS_LAST_BUILD_FILENAME="flags-last-build"
LIBS_LAST_BUILD_FILENAME="libs-last-build"

# path of output file
OUTPUT_PATH="$SWD/$OUTPUT"

# check option
FindAndClean() {
    local FORMAT="$1"
    local CMD=

    while read -r -d $'\0' FILE; do
        local OBJ_FILE="${FILE%.$FORMAT}.o"
        local DEP_FILE="${FILE%.$FORMAT}.d"
        if [ -e $OBJ_FILE ]; then
            CMD=(rm "$OBJ_FILE")

            echo "${CMD[@]}"
            "${CMD[@]}"

            ((DIR_CLEANED_FILE_COUNT++))
        fi
        if [ -e $DEP_FILE ]; then
            CMD=(rm "$DEP_FILE")

            echo "${CMD[@]}"
            "${CMD[@]}"
            
            ((DIR_CLEANED_FILE_COUNT++))
        fi
    done < <(find . -maxdepth 1 -name "*.$FORMAT"  -print0)
}

for ARG in "${@:1}"; do
    case $ARG in
        # build options
        'debug')
            FLAGS_FINAL=("${FLAGS_DEBUG[@]}")
        ;;
        'release')
            FLAGS_FINAL=("${FLAGS_RELEASE[@]}")
        ;;
        # cleanup
        'clean')
            for DIR in "${DIRS[@]}"; do
                cd "$SWD"
                if [ ! -d "$DIR" ]; then
                    echo "clean: directory '$DIR' not exist, skipping."
                    continue
                fi
                cd "$DIR"

                CWD="$(pwd)"
                echo "clean: $CWD"

                DIR_CLEANED_FILE_COUNT=0

                FindAndClean "c"
                FindAndClean "cpp"

                if [ $DIR_CLEANED_FILE_COUNT -eq 0 ]; then
                    echo "nothing to clean."
                fi
            done

            if [ -e "./$FLAGS_LAST_BUILD_FILENAME" ]; then
                rm "./$FLAGS_LAST_BUILD_FILENAME"
            fi
            if [ -e "./$LIBS_LAST_BUILD_FILENAME" ]; then
                rm "./$LIBS_LAST_BUILD_FILENAME"
            fi

            echo "done."

            exit 0
        ;;
        # verbose
        '--verbose')
            FLAGS_LINKING+=(--verbose)
        ;;
        *)
            echo "unknown argument '$ARG'"
            exit 1
        ;;
    esac
done

# compile to obj
OBJ_PATH=()

# parallel
CURR_PARALLEL_COUNT=0
MAX_PARALLEL=$(nproc)

WaitForAndGetSlot() {
    while (( CURR_PARALLEL_COUNT>=MAX_PARALLEL )); do
        wait -n
        ((CURR_PARALLEL_COUNT--))
    done

    if (( CURR_PARALLEL_COUNT<0 )); then
        CURR_PARALLEL_COUNT=0
    fi

    ((CURR_PARALLEL_COUNT++))
}

# combine flags that may change together
FLAGS_THIS_BUILD=($STD_C $STD_CPP "${FLAGS_FINAL[@]}" $PKG_CONFIG_INCLUDE)

# check last compile flags
cd "$SWD"

FLAGS_MISMATCH_TO_LAST=0
if [ -e "$FLAGS_LAST_BUILD_FILENAME" ]; then
    if [ "$(cat $FLAGS_LAST_BUILD_FILENAME)" != "$(echo "${FLAGS_THIS_BUILD[@]}")" ]; then
        FLAGS_MISMATCH_TO_LAST=1
    fi
else
    # unable to determine
    FLAGS_MISMATCH_TO_LAST=1
fi

# compile
REGEX_DEP_SRC_FILES='(?<=: )(\S+($| +))+'

TOTAL_COMPILE_COUNT=0

COMPILE_PIDS=()

FindAndCompile() {
    local FORMAT="$1"
    local COMPILER="$2"
    local STD="$3"

    local CMD=

    while read -r -d $'\0' FILE; do
        if [ -e "$FILE" ]; then
            local DEP_FILE="${FILE%.$FORMAT}.d"
            local OBJ_FILE="${FILE%.$FORMAT}.o"

            # check whether need to (re)compile
            local RECOMPILE=0
            if [ $FLAGS_MISMATCH_TO_LAST -ne 0 ]; then
                # recompile if flags changed
                RECOMPILE=1
            elif [ -e "$OBJ_FILE" ]; then
                # check existance of dependency file
                if [ -e "$DEP_FILE" ]; then
                    local SRC_DEPS=$(cat "$DEP_FILE" | tr -d '\\\n' | grep -o -P "$REGEX_DEP_SRC_FILES")

                    for DEP_SRC_FILE in $SRC_DEPS; do
                        # recompile if dependency changed
                        if [ "$DEP_SRC_FILE" -nt "$OBJ_FILE" ]; then
                            RECOMPILE=1
                        fi
                    done
                else
                    # unable to determine
                    RECOMPILE=1
                fi
            else
                RECOMPILE=1
            fi

            if [ $RECOMPILE -ne 0 ]; then
                WaitForAndGetSlot

                local CMD=($COMPILER -c "$FILE" "-std=$STD" "${FLAGS_FINAL[@]}" $PKG_CONFIG_INCLUDE -MMD)

                echo "${CMD[@]}"
                "${CMD[@]}" & COMPILE_PIDS+=($!)

                ((DIR_SRC_COMPILE_COUNT++))
                ((TOTAL_COMPILE_COUNT++))
            fi

            OBJ_PATH+=("$(realpath "$OBJ_FILE")")

            ((DIR_SRC_COUNT++))
        fi
    done < <(find . -maxdepth 1 -name "*.$FORMAT" -print0)
}

for DIR in "${DIRS[@]}"; do
    cd "$SWD"
    if [ ! -d "$DIR" ]; then
        echo "compile: directory '$DIR' not exist, skipping."
        continue
    fi
    cd "$DIR"

    CWD="$(pwd)"
    echo "compile: $CWD"

    DIR_SRC_COUNT=0
    DIR_SRC_COMPILE_COUNT=0

    FindAndCompile "c" "gcc" $STD_C
    FindAndCompile "cpp" "g++" $STD_CPP

    if [ $DIR_SRC_COMPILE_COUNT -eq 0 ]; then
        echo "nothing to compile."
    fi
done

cd "$SWD"
echo "${FLAGS_THIS_BUILD[@]}" > "$FLAGS_LAST_BUILD_FILENAME"

# compilation error check
COMPILE_EXCEPTION=0

for PID in "${COMPILE_PIDS[@]}"; do
    if ! wait "$PID"; then
        COMPILE_EXCEPTION=1
    fi
done

if [ $COMPILE_EXCEPTION -ne 0 ]; then
    echo "stopping due to compilation error."
    exit 1
fi

# link
NEED_RELINK=0

FindObjAndCheckDate() {
    local FORMAT="$1"

    while read -r -d $'\0' FILE; do
        local OBJ_FILE="${FILE%.$FORMAT}.o"

        if [ -e $OBJ_FILE ]; then
            if [ -e "$OUTPUT_PATH" ]; then
                if [ $OBJ_FILE -nt "$OUTPUT_PATH" ]; then
                    echo "'$OBJ_FILE' is newer than output."
                    NEED_RELINK=1
                fi
            fi
        else
            echo "object $OBJ_FILE not found. stopping."
            exit 1
        fi
    done < <(find . -maxdepth 1 -name "*.$FORMAT"  -print0)
}

# object date check
if [ $TOTAL_COMPILE_COUNT -eq 0 ]; then
    for DIR in "${DIRS[@]}"; do
        cd "$SWD"
        if [ ! -d "$DIR" ]; then
            echo "check: directory '$DIR' not exist."
            continue
        fi
        cd "$DIR"

        echo "check: $(pwd)"

        FindObjAndCheckDate "cpp"
        FindObjAndCheckDate "c"

        if [ $NEED_RELINK -ne 0 ]; then
            break
        fi
    done
fi

cd "$SWD"

# output file existence check
if [ ! -e $OUTPUT_PATH ]; then
    NEED_RELINK=1
fi

# lib link flag check
LIB_LINK_FLAGS=("${PATH_ADDITIONAL_LNK[@]}" "$PKG_CONFIG_LIBS")

if [ -e $LIBS_LAST_BUILD_FILENAME ]; then
    if [ "$(cat $LIBS_LAST_BUILD_FILENAME)" != "$(echo "${LIB_LINK_FLAGS[@]}")" ]; then
        NEED_RELINK=1
    fi
else
    NEED_RELINK=1
fi

cd "$SWD"
echo "link: $SWD"

if [ $TOTAL_COMPILE_COUNT -ne 0 ] || [ $NEED_RELINK -ne 0 ] ; then
    CMD_LNK=(g++ "${OBJ_PATH[@]}" "${PATH_ADDITIONAL_LNK[@]}" $PKG_CONFIG_INCLUDE $PKG_CONFIG_LIBS "${FLAGS_LINKING[@]}" -o "$OUTPUT")
    
    echo "${CMD_LNK[@]}"
    if "${CMD_LNK[@]}"; then
        echo "${LIB_LINK_FLAGS[@]}" > "$LIBS_LAST_BUILD_FILENAME"
    else
        echo "stopping due to linking error."
        exit 1
    fi
else
    echo "everything is up to date."
fi

echo "done."
