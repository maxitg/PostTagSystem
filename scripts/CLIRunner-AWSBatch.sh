#!/bin/sh

timestamp=$(date +%s)

job_id=$(echo "${AWS_BATCH_JOB_ID:-$timestamp}" | cut -d ':' -f 1)
child_job_index=${AWS_BATCH_JOB_ARRAY_INDEX:-0}

# requires aws-cli installed
s3_bucket=$S3_JOB_RESULT_BUCKET

outfile_base=${POST_OUTPUT_DIR:-"/tmp/"}
outfile_path="job/job-$job_id/"
outfile_name="child-$child_job_index.postresult"

infile_prefix=$POST_INPUT_PREFIX

mkdir -p "$outfile_base$outfile_path"

echo "Arguments: $@"
echo "Job ID: $job_id"
echo "Child index: $child_job_index"

if [[ "$POST_POUNCE_MODE" == 1 ]]; then
  wolfram-postproject $@ \
    --initfile "$infile_prefix""$child_job_index"".postinit" \
    --outfile "$outfile_base""$outfile_path""$outfile_name"
else
  wolfram-postproject $@ \
    --initoffset $child_job_index \
    --outfile "$outfile_base""$outfile_path""$outfile_name"
fi

cli_exit_code=$?
echo "CLI exit code: $cli_exit_code"

if [ -n "$s3_bucket" ]; then
  s3URL="s3://$s3_bucket/$outfile_path""$outfile_name"
  echo "S3 URL: $s3URL"

  # assume --outfile was left at the default
  aws s3 cp output.postresult "$s3URL"
else
  echo "S3_JOB_RESULT_BUCKET not set; skipping S3 upload"
fi

exit $cli_exit_code
