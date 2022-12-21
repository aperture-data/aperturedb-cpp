#! /bin/bash

source $(dirname "$0")/version.sh

# Set default version to develop
BUILD_VERSION=develop

# Trigger read version
read_version
echo "Build version: $BUILD_VERSION"

create_release() {
    user="aperture-data"
    repo="aperturedb-cpp"
    token=$TOKEN
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

upload_custom_release_file() {
    mkdir /tmp/aperturedb-cpp
    mkdir -p /tmp/aperturedb-cpp/lib
    mkdir -p /tmp/aperturedb-cpp/include

    docker create -ti --name aperturedb aperturedb-client-testing bash
    docker start aperturedb
    docker exec aperturedb bash -c "mkdir -p /x64; cp /usr/local/lib/libprotobuf.so.* /x64"
    docker exec aperturedb bash -c "cp /aperturedb-client/lib/* /x64"
    docker exec aperturedb bash -c "mkdir -p /comm; cp /aperturedb-client/include/comm/* /comm"
    docker exec aperturedb bash -c "mkdir -p /aperturedb; cp /aperturedb-client/include/aperturedb/* /aperturedb"
    docker cp aperturedb:/x64/ /tmp/aperturedb-cpp/lib/
    docker cp aperturedb:/comm/ /tmp/aperturedb-cpp/include/
    docker cp aperturedb:/aperturedb/ /tmp/aperturedb-cpp/include/

    tar -cvzf libs_64.tgz -C /tmp aperturedb-cpp
    docker stop aperturedb
    docker rm -f aperturedb

    token=$TOKEN
    file="libs_64.tgz"
    name="libs_64.tgz"

    url=`jq -r .upload_url release.json | cut -d{ -f'1'`
    command="\
      curl -s -o upload.json -w '%{http_code}' \
           --request POST \
           --header 'Accept: application/vnd.github.v3+json' \
           --header 'Authorization: token ${token}' \
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
upload_custom_release_file
