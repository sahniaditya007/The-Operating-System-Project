#!/bin/bash

set -e

# Base QEMU args; -debugcon stdio will show port 0xE9 output in the terminal
QEMU_ARGS='-debugcon stdio -m 32'

if [ "$#" -le 1 ]; then
    echo "Usage: ./run.sh <image_type> <image>"
    echo "       image_type: floppy | disk"
    echo "       image: path to image file (relative or absolute)"
    exit 1
fi

# Normalize image path to absolute to avoid cwd issues
IMG_PATH="$2"
case "$IMG_PATH" in
  /*) ;; # already absolute
  *) IMG_PATH="$PWD/$IMG_PATH" ;;
esac

case "$1" in
    "floppy")   QEMU_ARGS="${QEMU_ARGS} -fda \"$IMG_PATH\""
    ;;
    "disk")     QEMU_ARGS="${QEMU_ARGS} -hda \"$IMG_PATH\""
    ;;
    *)          echo "Unknown image type $1."
                exit 2
esac

# Choose display mode:
# - If NBOS_QEMU_DISPLAY is set (e.g., gtk/sdl/curses/none), use it.
# - Else, if no GUI is available (no DISPLAY and no WAYLAND_DISPLAY), fall back to curses
#   so you can see VGA text output in the terminal (useful in headless WSL).
DISPLAY_MODE="${NBOS_QEMU_DISPLAY:-}"
if [ -z "$DISPLAY_MODE" ]; then
    if [ -z "${DISPLAY:-}" ] && [ -z "${WAYLAND_DISPLAY:-}" ]; then
        DISPLAY_MODE="curses"
    fi
fi

if [ -n "$DISPLAY_MODE" ]; then
    QEMU_ARGS="${QEMU_ARGS} -display $DISPLAY_MODE"
fi

eval qemu-system-i386 $QEMU_ARGS