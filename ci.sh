#!/bin/bash

set -e

source $(dirname "$0")/version.sh

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
    sed -i "/#define VDMS_VERSION .*/c\#define VDMS_VERSION  \"${UPDATED_APP_VERSION}\"" ./src/aperturedb/version.h
    sed -i "/#define VDMS_VERSION_MAJOR .*/c\#define VDMS_VERSION_MAJOR  ${MAJOR_V}" ./src/aperturedb/version.h
    sed -i "/#define VDMS_VERSION_MINOR .*/c\#define VDMS_VERSION_MINOR  ${MINOR_V}" ./src/aperturedb/version.h
    sed -i "/#define VDMS_VERSION_MICRO .*/c\#define VDMS_VERSION_MICRO  ${MICRO_V}" ./src/aperturedb/version.h

    # Commit and push version bump
    git config --local user.name "github-actions[bot]"
    git config --local user.email "41898282+github-actions[bot]@users.noreply.github.com"
    git add ./src/aperturedb/version.h
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
