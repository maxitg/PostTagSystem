#!/bin/sh

copy_index=${CE_JOB_COPY:-0}
outfile_name="child-$copy_index.postresult"

echo "Copy index: $copy_index"

wolfram-postproject $@ --outfile /local/output/$outfile_name --initoffset $copy_index

cli_exit_code=$?
echo "CLI exit code: $cli_exit_code"

exit $cli_exit_code
