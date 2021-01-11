#!/usr/bin/env bash
set -eo pipefail

postTagSystemRoot=$(dirname "$(cd "$(dirname "$0")" && pwd)")
cd "$postTagSystemRoot"

# Compute the source hash

sourceFiles="$(ls libPostTagSystem/*pp CMakeLists.txt cmake/* scripts/buildLibraryResources.sh)"

if command -v shasum &>/dev/null; then
  echo "Using SHA tool: $(which shasum)"
  sha="$(
    echo "$(
      for fileToHash in $sourceFiles; do
        shasum -a 256 "$fileToHash"
      done
      uname -sm
    )" | shasum -a 256 | cut -d\  -f1
  )"
elif [[ "$OSTYPE" == "msys" && $(command -v certutil) ]]; then # there is another certutil in macOS
  echo "Using SHA tool: $(which certutil)"
  echo "$(
    for fileToHash in $sourceFiles; do
      echo "$(certutil -hashfile "$fileToHash" SHA256 | findstr -v "hash")" "$fileToHash"
    done
    uname -sm
  )" >"$TEMP"/libPostTagSystemFilesToHash
  sha=$(certutil -hashfile "$TEMP"/libPostTagSystemFilesToHash SHA256 | findstr -v "hash")
else
  echo "Could not find SHA utility"
  exit 1
fi

shortSHA=$(echo "$sha" | cut -c 1-13)
echo "libPostTagSystem sources hash: $shortSHA"

# Build the library

mkdir -p build
cd build
cmake .. -DSET_REPLACE_ENABLE_ALLWARNINGS=ON -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release # Needed for multi-config generators
cd ..

# Set the platform-specific names

if [ "$(uname -sm)" = "Darwin x86_64" ]; then
  libraryResourcesDirName=MacOSX-x86-64
  libraryExtension=dylib
elif [ "$(uname -sm)" = "Linux x86_64" ]; then
  libraryResourcesDirName=Linux-x86-64
  libraryExtension=so
elif [[ "$OSTYPE" == "msys" && "$(uname -m)" == "x86_64" ]]; then # Windows
  libraryResourcesDirName=Windows-x86-64
  libraryExtension=dll
else
  echo "Operating system unsupported"
  exit 1
fi

libraryDir=LibraryResources/$libraryResourcesDirName
echo "LibraryResources directory: $postTagSystemRoot/$libraryDir"
echo "Library extension: $libraryExtension"

# Find the compiled library

compiledLibrary=build/libPostTagSystem.$libraryExtension
if [ ! -f $compiledLibrary ]; then
  compiledLibrary=build/Release/PostTagSystem.$libraryExtension
fi

if [ ! -f $compiledLibrary ]; then
  echo "Could not find compiled library"
  exit 1
fi

echo "Found compiled library at $postTagSystemRoot/$compiledLibrary"

# Copy the library to LibraryResources

mkdir -p $libraryDir
libraryDestination=$libraryDir/libPostTagSystem-$shortSHA.$libraryExtension
echo "Copying the library to $postTagSystemRoot/$libraryDestination"
cp $compiledLibrary "$libraryDestination"

metadataDestination=$libraryDir/libPostTagSystemBuildInfo.json
echo "Writing metadata to $postTagSystemRoot/$metadataDestination"
echo "\
{
  \"LibraryFileName\": \"libPostTagSystem-$shortSHA.$libraryExtension\",
  \"LibraryBuildTime\": $(date -u "+[%-Y, %-m, %-d, %-H, %-M, %-S]"),
  \"LibrarySourceHash\": \"$shortSHA\"
}" >$metadataDestination

cat $metadataDestination
echo "Build done"
