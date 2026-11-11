#!/bin/bash
set -euo pipefail
export SWGB_GAME_DIR="${SWGB_GAME_DIR:-$HOME/Library/Application Support/Galactic Genie/Game-CloneCampaigns}"
exec "$(dirname "$0")/script/build_and_run.sh" "$@"
