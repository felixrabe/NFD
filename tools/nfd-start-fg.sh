#!@BASH@

VERSION="@VERSION@"

case "$1" in
  -h)
    echo Usage
    echo $0
    echo "  Start NFD and RIB Management daemon and stay in the foreground"
    exit 0
    ;;
  -V)
    echo $VERSION
    exit 0
    ;;
  "") ;; # do nothing
  *)
    echo "Unrecognized option $1"
    exit 1
    ;;
esac

nfd-start || exit $?

loop=true
trap 'loop=false' INT TERM
while $loop ; do
  sleep 2
done

nfd-stop || exit $?
