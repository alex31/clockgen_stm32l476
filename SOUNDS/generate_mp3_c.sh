#!/usr/bin/env zsh

set -euo pipefail

script_dir=${0:A:h}
exec "${script_dir}/generate_mp3_asset.py" "$@"
