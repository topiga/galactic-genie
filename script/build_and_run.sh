#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
: "${SWGB_GAME_DIR:?Set SWGB_GAME_DIR to the original Game directory}"
export DYLD_LIBRARY_PATH="/opt/homebrew/opt/sdl3/lib${DYLD_LIBRARY_PATH:+:$DYLD_LIBRARY_PATH}"
python3 script/build_desktop.py
if [[ "${1:-}" == "--build" ]]; then exit 0; fi
if [[ "${1:-}" == "--smoke" ]]; then
  export SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy
else
  pkill -x swgb-desktop 2>/dev/null || true
fi
cd "${SWGB_DESKTOP_BUILD:-$HOME/.local/share/swgb/galactic-genie-build}"
if [[ "${1:-}" == "--smoke" ]]; then
  exec ./swgb-desktop "$SWGB_GAME_DIR" --smoke
fi
exec open -W -n --stdout "$PWD/desktop-live.log" --stderr "$PWD/desktop-live.log" "Galactic Genie Skirmish.app" --args "$SWGB_GAME_DIR"
