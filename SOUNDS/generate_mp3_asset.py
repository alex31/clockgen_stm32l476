#!/usr/bin/env python3

import argparse
import array
import pathlib
import re
import subprocess
import sys
import tempfile
import wave


def run(cmd):
    subprocess.run(cmd, check=True)


def ffprobe_value(path, entries):
    cmd = [
        "ffprobe",
        "-v",
        "error",
        "-select_streams",
        "a:0",
        "-show_entries",
        f"stream={entries}",
        "-of",
        "default=noprint_wrappers=1:nokey=1",
        str(path),
    ]
    result = subprocess.run(cmd, check=True, capture_output=True, text=True)
    return [line.strip() for line in result.stdout.splitlines() if line.strip()]


def decode_to_wav(input_path, wav_path, sample_rate, duration_seconds):
    cmd = [
        "ffmpeg",
        "-y",
        "-i",
        str(input_path),
    ]
    if duration_seconds is not None:
        cmd.extend(["-t", f"{duration_seconds}"])
    cmd.extend([
        "-vn",
        "-ac",
        "1",
        "-ar",
        str(sample_rate),
        "-c:a",
        "pcm_s16le",
        str(wav_path),
    ])
    run(cmd)


def encode_mp3(wav_path, mp3_path, bitrate):
    run([
        "ffmpeg",
        "-y",
        "-i",
        str(wav_path),
        "-ac",
        "1",
        "-c:a",
        "libmp3lame",
        "-b:a",
        bitrate,
        str(mp3_path),
    ])


def read_wav_samples(path):
    with wave.open(str(path), "rb") as wav_file:
        if wav_file.getnchannels() != 1 or wav_file.getsampwidth() != 2:
            raise RuntimeError(f"{path} is not mono 16-bit PCM")

        pcm = array.array("h")
        pcm.frombytes(wav_file.readframes(wav_file.getnframes()))
        if sys.byteorder != "little":
            pcm.byteswap()
        return pcm


def write_wav_samples(path, sample_rate, samples):
    out = array.array("h", samples)
    if sys.byteorder != "little":
        out.byteswap()

    with wave.open(str(path), "wb") as wav_file:
        wav_file.setnchannels(1)
        wav_file.setsampwidth(2)
        wav_file.setframerate(sample_rate)
        wav_file.writeframes(out.tobytes())


def choose_loop_end(samples, match_samples, search_samples):
    total = len(samples)
    if total <= (match_samples * 2):
        return total

    search_span = min(search_samples, max(match_samples * 2, total // 8))
    first_candidate = max(match_samples + 1, total - search_span)
    head = samples[:match_samples]
    head_first = samples[0]
    head_next = samples[1]

    best_end = total
    best_score = None
    for end in range(first_candidate, total + 1):
        tail_start = end - match_samples
        score = 0
        for i in range(match_samples):
            delta = samples[tail_start + i] - head[i]
            score += delta * delta

        tail_last = samples[end - 1]
        tail_prev = samples[end - 2]
        edge_delta = tail_last - head_first
        slope_delta = (tail_last - tail_prev) - (head_next - head_first)
        score += 32 * edge_delta * edge_delta
        score += 8 * slope_delta * slope_delta

        if best_score is None or score < best_score:
            best_score = score
            best_end = end

    return best_end


def smooth_loop_tail(samples, play_samples, smooth_samples):
    smooth = min(smooth_samples, play_samples // 4)
    if smooth < 2:
        return samples[:play_samples]

    tuned = array.array("h", samples[:play_samples])
    tail_start = play_samples - smooth
    for i in range(smooth):
        alpha_num = i + 1
        alpha_den = smooth + 1
        head = tuned[i]
        tail = tuned[tail_start + i]
        blended = ((alpha_den - alpha_num) * tail + alpha_num * head) // alpha_den
        tuned[tail_start + i] = blended

    tuned[play_samples - 1] = tuned[0]
    return tuned


def sanitize_identifier(name):
    return re.sub(r"[^0-9A-Za-z_]", "_", name)


def write_metadata(meta_path, output_base, skip_samples, play_samples):
    identifier = sanitize_identifier(output_base)
    meta_path.write_text(
        "\n".join([
            f"constexpr AudioLoopMetadata {identifier}_meta = {{",
            f"  .skip_samples = {skip_samples}U,",
            f"  .play_samples = {play_samples}U,",
            "};",
            "",
        ]),
        encoding="ascii",
    )


def main():
    parser = argparse.ArgumentParser(
        description="Generate a mono MP3 asset and associated gapless loop metadata."
    )
    parser.add_argument("input_audio", type=pathlib.Path)
    parser.add_argument("output_base")
    parser.add_argument("--bitrate", default="64k")
    parser.add_argument("--sample-rate", type=int, default=32000)
    parser.add_argument("--duration-seconds", type=float, default=None)
    parser.add_argument("--metadata-only", action="store_true")
    parser.add_argument("--match-samples", type=int, default=192)
    parser.add_argument("--search-samples", type=int, default=2048)
    parser.add_argument("--smooth-samples", type=int, default=96)
    args = parser.parse_args()

    sounds_dir = pathlib.Path(__file__).resolve().parent
    mp3_dir = sounds_dir / "MP3"
    mp3_dir.mkdir(exist_ok=True)
    output_mp3 = mp3_dir / f"{args.output_base}.mp3"
    output_meta = mp3_dir / f"{args.output_base}.meta.hpp"

    with tempfile.TemporaryDirectory() as temp_dir_name:
        temp_dir = pathlib.Path(temp_dir_name)
        decoded_wav = temp_dir / "decoded.wav"
        processed_wav = temp_dir / "processed.wav"
        probe_target = output_mp3 if args.metadata_only else output_mp3

        if args.metadata_only:
            decode_to_wav(args.input_audio, decoded_wav, args.sample_rate, args.duration_seconds)
        else:
            decode_to_wav(args.input_audio, decoded_wav, args.sample_rate, args.duration_seconds)
            decoded_samples = read_wav_samples(decoded_wav)
            play_samples = choose_loop_end(decoded_samples, args.match_samples, args.search_samples)
            processed_samples = smooth_loop_tail(decoded_samples, play_samples, args.smooth_samples)
            write_wav_samples(processed_wav, args.sample_rate, processed_samples)
            encode_mp3(processed_wav, output_mp3, args.bitrate)
            probe_target = output_mp3

        if args.metadata_only:
            if args.input_audio.suffix.lower() != ".mp3":
                raise RuntimeError("--metadata-only currently expects an MP3 input")
            probe_target = args.input_audio
            if output_mp3 != args.input_audio.resolve():
                output_mp3.write_bytes(args.input_audio.read_bytes())
            decoded_samples = read_wav_samples(decoded_wav)
            play_samples = choose_loop_end(decoded_samples, args.match_samples, args.search_samples)
        else:
            # Keep metadata in sync with the processed loop PCM that was encoded.
            decoded_samples = processed_samples
            play_samples = len(processed_samples)

        sample_rate_str, start_time_str = ffprobe_value(probe_target, "sample_rate,start_time")
        skip_samples = int(round(float(start_time_str) * float(sample_rate_str)))
        write_metadata(output_meta, args.output_base, skip_samples, play_samples)

    print(f"generated {output_mp3}")
    print(f"generated {output_meta}")


if __name__ == "__main__":
    main()
