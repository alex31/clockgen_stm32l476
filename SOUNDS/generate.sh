#!/usr/bin/zsh

/bin/rm C/* RAW/*

for f in MP3/*
do
    echo $f
    mpg123 -w WAV/"$(basename -- "$f" .mp3).wav" "$f" 
done

for f in WAV/*
do
    echo $f
    sox "$f" -e unsigned-integer -b 8 -Dr 8k -c 1 RAW/"$(basename -- "$f" .wav).raw" rate -I -v channels 1 norm gain -1
done

cd RAW
mmv '*.aiff.raw' '#1.raw'
for f in *.raw
do
    echo $f
    xxd -i $f > ../C/"$(basename -- "$f" .raw).c" 
done

cd ../C
perl -i -ape 's/unsigned char/constexpr uint8_t/;' *.c
perl -i -ape 's/unsigned int/constexpr size_t/;' *.c
