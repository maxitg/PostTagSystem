#!/bin/sh

copy_index=${CE_JOB_COPY:-0}

echo "Copy index: $copy_index"

wolfram-postproject $@ --outfile /local/output/output.postresult --initoffset $copy_index

cli_exit_code=$?
echo "CLI exit code: $cli_exit_code"

exit $cli_exit_code
