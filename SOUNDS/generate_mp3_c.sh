#!/usr/bin/env zsh

set -euo pipefail

if [[ $# -lt 2 || $# -gt 4 ]]; then
  echo "usage: $0 <input-wav> <output-base> [bitrate] [sample-rate]" >&2
  exit 1
fi

input_wav=$1
output_base=$2
bitrate=${3:-64k}
sample_rate=${4:-32000}

output_mp3="MP3/${output_base}.mp3"
ffmpeg -y -i "${input_wav}" -ac 1 -ar "${sample_rate}" \
       -c:a libmp3lame -b:a "${bitrate}" "${output_mp3}"

echo "generated ${output_mp3}"
echo "use it directly with #embed from C/C++"
