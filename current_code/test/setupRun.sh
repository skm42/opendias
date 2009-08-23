
usage() {
  echo "Usage: [-h] [-r] [-m] [-s] [-c] [<tests>]"
  echo "  -h       help (show this page)"
  echo "  -r       record results files (will distroy current results files of tests that are about to be run)"
  echo "  -m       don't do memory checking"
  echo "  -s       don't use any memory checking suppressions (exclusive to -m flag)"
  echo "  -c       don't do coverage checking"
#  echo "  -b       don't rebuild app (may override -c)"
  echo "  <tests>  The tests you wish to run. Default to all tests."
  echo "             eg: 1*"
  echo "                 2_1 2_2 2_3"
  echo "                 2_1 3_4*"
}

#
# Parse off all the parameters
#
RECORD=""
SKIPMEMORY=""
SKIPCOVERAGE=""
while getopts ":hrmcs" flag 
do 
  case $flag in 
    h)
      usage;
      exit 1;
    ;;
    r)
      RECORD="-r";
      echo Will record results.
    ;;
    m)
      SKIPMEMORY="-m";
      echo Will skip analysing memory usage.
    ;;
    c)
      SKIPCOVERAGE="-c";
      echo Will skip analysing code coverage.
    ;;
    s)
      NOSUPPRESSIONS="-s";
    ;;
  esac 
done 
if [ "$NOSUPPRESSIONS" != "" ]; then
  if [ "$SKIPMEMORY" == "" ]; then
    echo No memory suppression clauses will be used.
  else
    echo eek - you asked for no memory checking - and - use no memory supressions. This does not seam right.
    usage;
    exit 1;
  fi
fi
shift $((OPTIND-1))

#
# Cleanup
#
rm -rf src/*.gcda src/*.gcno test/valgrind.out test/lastCoverage test/lastRegression
mkdir test/lastCoverage/
mkdir test/lastRegression/


#
# Compile, so that 'code coverage' analysis reports can be generated.
#
echo Cleaning ...
make clean > test/lastRegression/buildLog.out
# if the file is here, then last time configure was run was in this script
# so no need to re-do it.
if [ ! -f dontBotherReCompilingMe-testScript ] || [ "$SKIPCOVERAGE" == "-c" ]; then
  echo Configuring ...
  if [ "$SKIPCOVERAGE" == "-c" ]; then
    ./configure CFLAGS=' -g ' &> test/lastRegression/buildLog2.out
  else
    ./configure CFLAGS='--coverage' LIBS='-lgcov' &> test/lastRegression/buildLog2.out
  fi
  # unfortunatly bash cannot support "&>>" - yet!
  cat test/lastRegression/buildLog2.out >> test/lastRegression/buildLog.out
  rm test/lastRegression/buildLog2.out
fi
# Will have to re-make, incase anything was changed in source.
echo Making ...
make &> test/lastRegression/buildLog2.out
# unfortunatly bash cannot support "&>>" - yet!
cat test/lastRegression/buildLog2.out >> test/lastRegression/buildLog.out
rm test/lastRegression/buildLog2.out
touch dontBotherReCompilingMe-testScript

#
# Generate baseline (coverage) information
#
if [ "$SKIPCOVERAGE" == "" ]; then
  echo Getting baseline coverage information ...
  lcov -c -i -d src -o test/lastCoverage/app_base.info >> test/lastRegression/buildLog.out
fi


#
# Start the app with memory management reporting enabled.
#
echo Creating startup scripts ...
if [ "$SKIPMEMORY" == "" ]; then
  CODENAME=`lsb_release -c | cut -f2`
  SUPPRESS=""
  if [ "$NOSUPPRESSIONS" == "" ]; then
    for SUPP in `ls test/suppressions/$CODENAME/*`; do
      if [ -f $SUPP ]; then
        SUPPRESS="$SUPPRESS --suppressions=$SUPP"
      fi
    done
  fi
  VALGRINDOPTS="--leak-check=full --leak-resolution=high --error-limit=no --tool=memcheck --num-callers=50 --log-file=test/lastRegression/valgrind.out "
  GENSUPP="--gen-suppressions=all "
  VALGRIND="G_SLICE=always-malloc G_DEBUG=gc-friendly valgrind "
fi
echo touch runningTest > test/runMe
echo $VALGRIND $SUPPRESS $VALGRINDOPTS $GENSUPP src/opendias \&\> test/lastRegression/appLog.out >> test/runMe
echo rm runningTest >> test/runMe
chmod 755 test/runMe


#
# Backup the data area
#
DTE=`date +%Y%m%d%H%M%S`
if [ -d ~/.openDIAS ]; then
  echo Backing up personal data ...
  tar -czf ~/openDias_bkp.$DTE.tar.gz ~/.openDIAS
  rm -rf ~/.openDIAS
fi


#######################################
# Run automated tests
echo Starting test harness ...
test/harness.sh $RECORD $SKIPMEMORY $@
#######################################



#
# Recover data area
#
if [ -f /openDias_bkp.$DTE.tar.gz ]; then
  echo Restoring personal data ...
  rm -rf ~/.openDIAS
  OP=`pwd`
  cd /
  tar -xf ~/openDias_bkp.$DTE.tar.gz
  cd $OP
fi


#
# Collect process and build coverage report
#
if [ "$SKIPCOVERAGE" == "" ]; then
  echo Creating run coverage information ...
  lcov -c -d src -o test/lastCoverage/app_test.info >> test/lastRegression/buildLog.out
  lcov -a test/lastCoverage/app_base.info -a test/lastCoverage/app_test.info -o test/lastCoverage/app_total.info >> test/lastRegression/buildLog.out
  genhtml -o test/lastCoverage test/lastCoverage/app_total.info >> test/lastRegression/buildLog.out
fi




