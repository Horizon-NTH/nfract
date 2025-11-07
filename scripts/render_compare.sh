#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat <<'EOF'
Usage: scripts/render_compare.sh [-o OUTPUT_DIR]

Builds the Newton fractal renderer twice (ISPC + CPU), renders every color
palette with both binaries using heavier settings, and prints precise elapsed
seconds so you can compare performance. By default images are written to
./renders (and that folder is removed afterwards); pass -o to keep them
somewhere else.
EOF
}

DEFAULT_OUTPUT_DIR="renders"
OUTPUT_DIR=""
OUTPUT_DIR_SPECIFIED=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        -o|--output-dir)
            [[ $# -lt 2 ]] && { echo "Missing argument for $1" >&2; exit 1; }
            OUTPUT_DIR="$2"
            OUTPUT_DIR_SPECIFIED=true
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown argument: $1" >&2
            usage
            exit 1
            ;;
    esac
done

if [[ -z "$OUTPUT_DIR" ]]; then
    OUTPUT_DIR="$DEFAULT_OUTPUT_DIR"
fi

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_ISPC="$REPO_ROOT/build_ispc"
BUILD_CPU="$REPO_ROOT/build_cpu"

mkdir -p "$OUTPUT_DIR"
OUTPUT_DIR="$(cd "$OUTPUT_DIR" && pwd)"

cleanup() {
    rm -rf "$BUILD_ISPC" "$BUILD_CPU"
    if [[ "$OUTPUT_DIR_SPECIFIED" == false ]]; then
        rm -rf "$OUTPUT_DIR"
    fi
}
trap cleanup EXIT

COLOR_NAMES=("classic" "neon" "jewelry")
declare -a ISPC_TIMES
declare -a CPU_TIMES

RENDER_ARGS=(--width 2560 --height 1440 --max-iter 400)

color_flag() {
    case "$1" in
        classic) ;;
        neon) printf -- "--neon" ;;
        jewelry) printf -- "--jewelry" ;;
        *)
            echo "Unknown color $1" >&2
            exit 1
            ;;
    esac
}

configure_and_build() {
    local build_dir=$1
    local run_on_cpu=$2
    local label=$3

    echo ">>> Configuring ${label} build (${build_dir})"
    cmake -S "$REPO_ROOT" -B "$build_dir" -DRUN_ON_CPU="$run_on_cpu" -DCMAKE_BUILD_TYPE=Release
    cmake --build "$build_dir" --target nfract --config Release --parallel
}

locate_binary() {
    local build_dir=$1
    if [[ -x "$build_dir/nfract" ]]; then
        echo "$build_dir/nfract"
    elif [[ -x "$build_dir/Release/nfract" ]]; then
        echo "$build_dir/Release/nfract"
    elif [[ -x "$build_dir/Debug/nfract" ]]; then
        echo "$build_dir/Debug/nfract"
    else
        echo "Could not find nfract binary in $build_dir" >&2
        exit 1
    fi
}

measure_command_seconds() {
    python3 - "$@" <<'PY'
import sys
import time
import subprocess

cmd = sys.argv[1:]
start = time.perf_counter()
subprocess.run(cmd, check=True)
elapsed = time.perf_counter() - start
print(f"{elapsed:.3f}")
PY
}

configure_and_build "$BUILD_ISPC" OFF "ISPC"
configure_and_build "$BUILD_CPU" ON "CPU"

ISPC_BIN="$(locate_binary "$BUILD_ISPC")"
CPU_BIN="$(locate_binary "$BUILD_CPU")"

for idx in "${!COLOR_NAMES[@]}"; do
    color="${COLOR_NAMES[$idx]}"
    flag="$(color_flag "$color")"

    outfile="${OUTPUT_DIR}/nfract_${color}_ispc.png"
    cmd=("$ISPC_BIN" --out "$outfile" "${RENDER_ARGS[@]}")
    if [[ -n "$flag" ]]; then
        cmd+=("$flag")
    fi
    seconds=$(measure_command_seconds "${cmd[@]}")
    if [[ "$OUTPUT_DIR_SPECIFIED" == true ]]; then
        printf "Rendered %-7s via %-4s in %5ss -> %s\n" "$color" "ispc" "$seconds" "$outfile"
    else
        printf "Rendered %-7s via %-4s in %5ss\n" "$color" "ispc" "$seconds"
    fi
    ISPC_TIMES[$idx]="$seconds"

    outfile="${OUTPUT_DIR}/nfract_${color}_cpu.png"
    cmd=("$CPU_BIN" --out "$outfile" "${RENDER_ARGS[@]}")
    if [[ -n "$flag" ]]; then
        cmd+=("$flag")
    fi
    seconds=$(measure_command_seconds "${cmd[@]}")
    if [[ "$OUTPUT_DIR_SPECIFIED" == true ]]; then
        printf "Rendered %-7s via %-4s in %5ss -> %s\n" "$color" "cpu" "$seconds" "$outfile"
    else
        printf "Rendered %-7s via %-4s in %5ss\n" "$color" "cpu" "$seconds"
    fi
    CPU_TIMES[$idx]="$seconds"
done

echo ""
printf "%-8s %-6s %s\n" "Color" "Mode" "Seconds"
printf "%-8s %-6s %s\n" "------" "------" "-------"
for idx in "${!COLOR_NAMES[@]}"; do
    color="${COLOR_NAMES[$idx]}"
    printf "%-8s %-6s %s\n" "$color" "ispc" "${ISPC_TIMES[$idx]}"
    printf "%-8s %-6s %s\n" "$color" "cpu" "${CPU_TIMES[$idx]}"
done

echo ""
if [[ "$OUTPUT_DIR_SPECIFIED" == true ]]; then
    echo "Images saved under: $OUTPUT_DIR"
else
    echo "Rendered images were removed during cleanup (no output directory specified)."
fi
