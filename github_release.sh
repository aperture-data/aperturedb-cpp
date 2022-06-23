#! /bin/bash

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

# Set default version to develop
BUILD_VERSION=develop

# Trigger read version
read_version
echo "Build version: $BUILD_VERSION"

create_release() {
    user="AdaptiveScale"
    repo="aperturedb-cpp-experiment"
    token="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"
    tag="v$BUILD_VERSION"

    command="curl -s -o release.json -w '%{http_code}' \
         --request POST \
         --header 'Accept: application/vnd.github.v3+json' \
         --header 'Authorization: token ${token}' \
         --header 'content-type: application/json' \
         --data '{\"tag_name\": \"${tag}\", \"name\": \"${tag}\", \"body\":\"Release ${tag}\"}' \
         https://api.github.com/repos/$user/$repo/releases"
    http_code=`eval $command`
    if [ $http_code == "201" ]; then
        echo "created release:"
        cat release.json
    else
        echo "create release failed with code '$http_code':"
        cat release.json
        echo "command:"
        echo $command
        return 1
    fi
}

upload_release_file() {
    token=$1
    file=$2
    name=$3

    url=`jq -r .upload_url release.json | cut -d{ -f'1'`
    command="\
      curl -s -o upload.json -w '%{http_code}' \
           --request POST \
           --header 'authorization: token ${token}' \
           --header 'Content-Type: application/octet-stream' \
           --data-binary @\"${file}\"
           ${url}?name=${name}"
    http_code=`eval $command`
    if [ $http_code == "201" ]; then
        echo "asset $name uploaded:"
        jq -r .browser_download_url upload.json
    else
        echo "upload failed with code '$http_code':"
        cat upload.json
        echo "command:"
        echo $command
        return 1
    fi
}


create_release