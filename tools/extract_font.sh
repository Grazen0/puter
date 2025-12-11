#!/usr/bin/env bash

input_file="$1"

head -n 256 "$input_file" | while IFS=: read -r _idx hex; do
    [ -z "$hex" ] && continue
    printf "%s\n" "$hex" | fold -w2
done
