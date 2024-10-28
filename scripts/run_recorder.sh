#!/bin/bash

current_date=$(date -d "+0 day" +"%Y-%m-%d")
date_file_format=$(date -d "+0 day" +"%d%m%y")
script_dir=$(dirname "$(realpath "$0")")
root_dir="${script_dir}/.."
"$root_dir/build/recorder" --symbol all --date "$current_date" --configfile "$root_dir/config/test_config.ini" > "$root_dir/log/${date_file_format}_recorder.log" 2>&1
