#!/bin/bash -ex
#
# easy_build.sh
#
# A hopefully simple script to download all dependencies for Agilent Tools
# and then compile automatically, for those who are unfamiliar with CMake.
#
# Requires:
#
# 1. CMake version 3.2 or greater
# 2. A C++11 compliant compiler e.g. GCC 4.8 or higher
#

WD=$PWD
# First download Eigen
EXT_DIR="External"
mkdir -p $EXT_DIR
cd $EXT_DIR

# Eigen
EIGEN_VER="3.2.4"
EIGEN_DIR="eigen${EIGEN_VER}"
EIGEN_URL="http://bitbucket.org/eigen/eigen/get/${EIGEN_VER}.tar.gz"
curl --location $EIGEN_URL > ${EIGEN_DIR}.tar.gz
mkdir -p $EIGEN_DIR
tar --extract --file=${EIGEN_DIR}.tar.gz --strip-components=1 --directory=${EIGEN_DIR}

cd $WD
# Now build Agilent Tools
BLD_DIR="Build"
BLD_OPTS="-DCMAKE_BUILD_TYPE=Release -DEIGEN3_INCLUDE_DIR=${WD}/${EXT_DIR}/${EIGEN_DIR}"
mkdir -p $BLD_DIR
cd $BLD_DIR
cmake $WD ${BLD_OPTS}
make -j
