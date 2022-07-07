# Read version
read_version() {
  MAJOR_V=$(awk '/VDMS_VERSION_MAJOR/{print $NF}' src/aperturedb/version.h)
  MINOR_V=$(awk '/VDMS_VERSION_MINOR/{print $NF}' src/aperturedb/version.h)
  MICRO_V=$(awk '/VDMS_VERSION_MICRO/{print $NF}' src/aperturedb/version.h)
  # Verify all the fields in version are set
  if [ $MAJOR_V != '' ] && [ $MINOR_V != '' ] && [ $MICRO_V != '' ]
  then
    BUILD_VERSION="$MAJOR_V.$MINOR_V.$MICRO_V"
  fi
}
