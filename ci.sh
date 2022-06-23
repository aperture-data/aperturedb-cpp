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

# Check and updates version based on release branch name
update_version() {
    echo "Checking versions"
    if [[ $BRANCH_NAME != "release"* ]]; then
        echo "Not release branch - skipping version update"
        return
    fi
    IFS=. read MAJOR_V MINOR_V MICRO_V <<<"${BRANCH_NAME##release-}"
    if [ -z "$MAJOR_V" ]; then
        echo "Missing major version"
        exit 1
    fi
    if [ -z "$MINOR_V" ]; then
        echo "Missing minor version"
        exit 1
    fi
    if [ -z "$MICRO_V" ]; then
        echo "Missing micro version"
        exit 1
    fi
    VERSION_BUMP=$MAJOR_V.$MINOR_V.$MICRO_V
    if [ $BUILD_VERSION == $VERSION_BUMP ]; then
        echo "Versions match - skipping update"
        return
    fi
    echo "Updating version $BUILD_VERSION to $VERSION_BUMP"
    # Replace version in __init__.py
    printf '%s\n' "%s/__version__.*/__version__ = \"$VERSION_BUMP\"/g" 'x' | ex aperturedb/__init__.py
    printf '%s\n' "%s/version=.*/version=\"$VERSION_BUMP\",/g" 'x' | ex setup.py

    # Commit and push version bump
    git config --local user.name "github-actions[bot]"
    git config --local user.email "41898282+github-actions[bot]@users.noreply.github.com"
    git add ./aperturedb/__init__.py
    git add ./setup.py
    git commit -m "Version bump: ${BUILD_VERSION} to ${VERSION_BUMP}"
    git push --set-upstream origin $BRANCH_NAME
    BUILD_VERSION=$VERSION_BUMP
}

build_apperturedb() {
    mkdir /tmp/aperturedb-client
    cp -r * /tmp/aperturedb-client/
    cd docker/test
    mv /tmp/aperturedb-client .
    docker build -t aperturedb-client-testing .
}

# Fetch branch
if [ -z ${BRANCH_NAME+x} ]
then
    BRANCH_NAME=$(git rev-parse --abbrev-ref HEAD)
fi

echo "Branch: $BRANCH_NAME"

# Set default version to develop
BUILD_VERSION=develop

# Trigger read version
read_version
echo "Build version: $BUILD_VERSION"

# Trigger update version
update_version

build_apperturedb