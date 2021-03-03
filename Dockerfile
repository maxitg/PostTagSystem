FROM alpine:3.12.1 as build

RUN apk add --no-cache \
  bash boost-dev boost-static git g++ make cmake \
  clang py-pip shellcheck shfmt grep npm \
  && pip install cpplint \
  && npm install -g markdownlint-cli

COPY . /working
WORKDIR /working

# lint and build
RUN ./lint.sh \
  && mkdir build \
  && cd build \
  && cmake .. -DPOST_TAG_SYSTEM_BUILD_TESTING=ON \
    -DPOST_TAG_SYSTEM_BUILD_CLI=ON \
    -DPOST_TAG_SYSTEM_CLI_STATIC_BUILD=ON \
    -DPOST_TAG_SYSTEM_ENABLE_ALLWARNINGS=ON \
    -DCMAKE_BUILD_TYPE=Release \
  && cmake --build . --config Release

FROM alpine:3.12.1

COPY --from=build /working/build/CLI/wolfram-postproject /usr/bin/wolfram-postproject

COPY ./scripts/CLIRunner-AWSBatch.sh /usr/bin/wolfram-postproject-awsbatch
RUN chmod +x /usr/bin/wolfram-postproject-awsbatch

COPY ./scripts/CLIRunner-CharityEngine.sh /usr/bin/wolfram-postproject-charityengine
RUN chmod +x /usr/bin/wolfram-postproject-charityengine
