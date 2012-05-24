#!/bin/bash

usage() {
  echo "Usage: [-h] [-r] [-m] [-s] [-c] [<tests>]"
  echo "  -h       help (show this page)"
  echo "  -r       record results files (will distroy current results files of tests that are about to be run)"
  echo "  -m       don't do memory checking"
  echo "  -s       don't use any memory checking suppressions (exclusive to -m flag)"
  echo "  -c       don't do coverage checking"
  echo "  -b       don't rebuild app (may override -c)"
  echo "  -g       show a visual debugger for the webclient"
  echo "  <tests>  The tests you wish to run. Default to all tests."
  echo "             eg: 5*"
  echo "                 01* 021* 022*"
  echo "                 006_scan"
}

#check env
if [ -z $OPENDIAS_SRC ] ; then 
	echo "OPENDIAS_SRC missing"
	exit 1
fi
if [ -z $OPENDIAS_LOGLOCATION ] ; then 
	echo "OPENDIAS_LOGLOCATION missing"
	exit 1
fi
if [ -z $OPENDIAS_RUNLOCATION ] ; then 
	echo "OPENDIAS_RUNLOCATION missing"
	exit 1
fi
if [ -z $OPENDIAS_BINLOCATION ] ; then 
	echo "OPENDIAS_BINLOCATION missing"
	exit 1
fi

#
# Parse off all the parameters
#

NOBUILD=""
RECORD=""
SKIPMEMORY=""
SKIPCOVERAGE=""
GRAPHICALCLIENT=""
while getopts ":hrmcsbg" flag 
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
    b)
      NOBUILD="-b";
      echo Will not \(re\)build the app.
    ;;
    g)
      GRAPHICALCLIENT="-g";
      echo The web client will have a graphical debugger frontend
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

if [ ! -f "/usr/bin/lcov" ]; then
  SKIPCOVERAGE="-c";
  echo "Will skip analysing code coverage (package not available)."
fi

# So that everything else does not have to run as root (for testing), reset back later
for W in $OPENDIAS_RUNLOCATION/opendias.pid; do # /var/log/opendias/opendias.log; do
  if [ ! -w "$W" ]; then
    echo $W is not writable. Please correct before running testing.
    exit
  fi
done

# Check we dont have a running service atm.
kill -0 `cat $OPENDIAS_RUNLOCATION/opendias.pid` 2>/dev/null
if [ "$?" -eq "0" ]; then
  echo "It looks like the opendias service is already running. Please stop before running testing."
  exit
fi
rm -f $OPENDIAS_LOGLOCATION/opendias.log

#
# Cleanup
#
#rm -rf ../src/*.gcda ../src/*.gcno results
mkdir -p results/coverage/
mkdir -p results/resultsFiles/


if [ "$NOBUILD" == "" ]; then
  #
  # Compile, so that 'code coverage' analysis reports can be generated.
  #
  
  which apt-rdepends > /dev/null
  if [ "$?" -ne "0" ]; then
    echo Could not determine the installed packages. If you\'re on debian based system, install apt-rdepends
  else
    echo Recording current installed package versions off all dependencies
    dpkg -l `apt-rdepends build-essential libsqlite3-dev libsane-dev libmicrohttpd-dev uuid-dev libleptonica-dev libpoppler-cpp-dev libtesseract-dev libxml2-dev libzzip-dev libarchive-dev 2> /dev/null | grep -v "^ " | sort` &> results/buildLog2.out
    # unfortunatly bash cannot support "&>>" - yet!
    cat results/buildLog2.out >> results/buildLog.out
    rm results/buildLog2.out
  fi

  echo Performing code analysis ...
  #cd ../
  cppcheck --verbose --enable=all --error-exitcode=1 $OPENDIAS_SRC/ &> results/buildLog2.out
  if [ "$?" -ne "0" ]; then
    echo "Code analysis found a problem. Check the buildLog.out for details."
    # unfortunatly bash cannot support "&>>" - yet!
    cat results/buildLog2.out >> results/buildLog.out
    rm results/buildLog2.out
    exit
  fi
  
  # unfortunatly bash cannot support "&>>" - yet!
  cat results/buildLog2.out >> results/buildLog.out
  rm results/buildLog2.out

  # if the file is here, then last time configure was run was in this script
  # so no need to re-do it.
	#check using config.log and abort meaningful
        configure_cmd=`grep '^  \$ ./configure' $OPENDIAS_SRC/config.log`
	configure_cmdProp=`echo $configure_cmd |sed 's#\\$##'`
	typeset -i rebuild=0

	echo "last configure with: $configure_cmd"
	echo $configure_cmd |grep -q '\-\-enable\-werror'
	if [ $? -ne 0 ] ; then
		echo "missing --enable-werror"
		configure_cmdProp=`echo $configure_cmdProp|sed 's#configure #configure --enable-werror#'`
		rebuild=1
	fi 


  if [ "$SKIPCOVERAGE" == "-c" ]; then
    echo " (without coverage) ..."
		echo $configure_cmd |grep -q "\-C CFLAGS= \-g \-O "
		if [ $? -ne 0 ] ; then
			echo "missing CFLAGS=' -g -O '"
			configure_cmdProp=`echo $configure_cmdProp|sed "s#\-\-enable\-werror#--enable-werror -C CFLAGS=' -g -O' #"`
			rebuild=1
		fi 
	else 
    echo " (with coverage) ..."
		echo $configure_cmd |grep -q "\-C CFLAGS= \-g \-O --coverage"
		if [ $? -ne 0 ] ; then
			echo "missing CFLAGS=' -g -O --coverage'"
			configure_cmdProp=`echo $configure_cmdProp|sed "s#\-\-enable\-werror#--enable-werror -C CFLAGS=' -g -O --coverage' LIBS='-lgcov' #"`
			rebuild=1
		fi 
	fi

	if [ $rebuild -eq 1 ] ; then
		echo "Rebuilding"
		echo "reconfiguring $OPENDIAS_SRC with $configure_cmdProp ?"
	  	read ans
		if [ $ans == "y" ] ; then
			echo Cleaning ...
  			cd $OPENDIAS_SRC || exit 1
			echo "PWD = $PWD"
  			make clean > /tmp/buildLog2.out
		
			eval $configure_cmdProp >> /tmp/buildLog2.out 2>&1 
			if [ $? -ne 0 ] ; then
				cd -
				cat /tmp/buildLog2.out >> results/buildLog.out
				echo "configure failed check results/buildLog.out" && exit 1
			fi

			echo "running make"	
			make > /tmp/buildLog2.out 2>&1 
			if [ $? -ne 0 ] ; then
				cd -
				cat /tmp/buildLog2.out >> results/buildLog.out
				echo "make failed check results/buildLog.out" && exit 1
			fi

			echo "running make install"
			make install > /tmp/buildLog2.out 2>&1
			if [ $? -ne 0 ] ; then
				cd -
				cat /tmp/buildLog2.out >> results/buildLog.out
				echo "make install failed check results/buildLog.out" && exit 1
			fi
		  

			cd -
			cat /tmp/buildLog2.out > results/buildLog.out
			echo "rebuild done"
		else
			echo "aboring"
			exit 
		fi
	fi
  #echo -n Configuring 
  #cd ../
  #if [ "$SKIPCOVERAGE" == "-c" ]; then
  #  echo " (without coverage) ..."
  #  ./configure --enable-werror -C CFLAGS=' -g -O ' &> test/results/buildLog2.out
  #else
  #  echo " (with coverage) ..."
  #  ./configure --enable-werror -C CFLAGS=' -g -O --coverage' LIBS='-lgcov' &> test/results/buildLog2.out
  #fi
  #cd test
  # unfortunatly bash cannot support "&>>" - yet!
  #cat tmp/buildLog2.out >> results/buildLog.out
  #rm results/buildLog2.out

  # Will have to re-make, incase anything was changed in source.
  #echo Making ...
  #cd ../
  #make &> test/results/buildLog2.out
  #if [ "$?" -ne "0" ]; then
  #  echo "Compile stage failed. Check the buildLog.out for details."
  #  cd test
    # unfortunatly bash cannot support "&>>" - yet!
   # cat results/buildLog2.out >> results/buildLog.out
   # rm results/buildLog2.out
   # exit
  #fi
  #cd test
  # unfortunatly bash cannot support "&>>" - yet!
  #cat results/buildLog2.out >> results/buildLog.out
  #rm results/buildLog2.out

else
  echo Skipping the build process
fi


#
# Generate baseline (coverage) information
#
if [ "$SKIPCOVERAGE" == "" ]; then
  echo Getting baseline coverage information ...
  lcov -c -i -d $OPENDIAS_SRC -o results/coverage/app_base.info >> results/buildLog.out
fi


#
# Start the app with memory management reporting enabled.
#
echo Creating startup scripts ...
if [ "$SKIPMEMORY" == "" ]; then
  SUPPRESS=""
  if [ "$NOSUPPRESSIONS" == "" ]; then
    if [ -d suppressions ]; then
      for SUPP in `ls suppressions/*`; do
        if [ -f $SUPP ]; then
          SUPPRESS="$SUPPRESS --suppressions=$SUPP"
        fi
      done
    fi
  fi
  VALGRINDOPTS="--leak-check=full --leak-resolution=high --error-limit=no --tool=memcheck --num-callers=50 --log-file=results/resultsFiles/valgrind.out --show-below-main=yes --track-origins=yes --track-fds=yes --show-reachable=yes "
  # "-v --trace-children=yes "
  GENSUPP="--gen-suppressions=all "
  VALGRIND="valgrind "
#else
#  VALGRIND="strace "
fi


#
# Use testing sane config (so testers do not have to update their environment)
#
mkdir -p /tmp/opendiassaneconfig
cp config/sane/* /tmp/opendiassaneconfig/
export SANE_CONFIG_DIR=/tmp/opendiassaneconfig/


PWD=`pwd`
echo $VALGRIND $SUPPRESS $VALGRINDOPTS $GENSUPP $OPENDIAS_BINLOCATION/opendias -c $PWD/config/testapp.conf \> results/resultsFiles/appLog.out > config/startAppCommands



#######################################
#######################################
# Run automated tests
echo Starting test harness ...
echo perl ./harness.pl $GRAPHICALCLIENT $RECORD $SKIPMEMORY $@
perl ./harness.pl $GRAPHICALCLIENT $RECORD $SKIPMEMORY $@ 2> /dev/null
#######################################
#######################################

#
# Collect process and build coverage report
#
if [ "$SKIPCOVERAGE" == "" ]; then
  echo Creating run coverage information ...
  echo Creating run coverage information ... >> results/buildLog.out
  lcov -c -d ../src -o results/coverage/app_test.info >> results/buildLog.out
  lcov -a results/coverage/app_base.info -a results/coverage/app_test.info -o results/coverage/app_total.info >> results/buildLog.out
  genhtml -o results/coverage results/coverage/app_total.info >> results/buildLog.out
fi

#rm config/startAppCommands

