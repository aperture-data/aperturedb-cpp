#!/bin/bash

set -e

read_version() {
  MAJOR_V=$(awk '/VDMS_VERSION_MAJOR/{print $NF}' src/version.h)
  MINOR_V=$(awk '/VDMS_VERSION_MINOR/{print $NF}' src/version.h)
  MICRO_V=$(awk '/VDMS_VERSION_MICRO/{print $NF}' src/version.h)
  # Verify all the fields in version are set
  if [ $MAJOR_V != '' ] && [ $MINOR_V != '' ] && [ $MICRO_V != '' ]
  then
    BUILD_VERSION="$MAJOR_V.$MINOR_V.$MICRO_V"
  fi
}

# Trigger read version
read_version
echo "Build version: $BUILD_VERSION"

git config --local user.name "github-actions[bot]"
git config --local user.email "41898282+github-actions[bot]@users.noreply.github.com"
git tag "v$BUILD_VERSION" $TAG_BASE
git push origin "v$BUILD_VERSION"
