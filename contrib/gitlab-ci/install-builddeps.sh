#!/bin/bash

OLD_PWD=$PWD
TMP=/tmp

apt-get update
apt-get dist-upgrade -y
apt-get install -y \
    automake \
    autotools-dev \
    bsdmainutils \
    build-essential \
    libboost-chrono-dev \
    libboost-filesystem-dev \
    libboost-program-options-dev \
    libboost-system-dev \
    libboost-test-dev \
    libboost-thread-dev \
    libevent-dev \
    libminiupnpc-dev \
    libprotobuf-dev \
    libqrencode-dev \
    libqt5core5a \
    libqt5dbus5 \
    libqt5gui5 \
    libssl-dev \
    libtool \
    pkg-config \
    protobuf-compiler \
    qttools5-dev \
    qttools5-dev-tools \
    wget

# Build libdb4.8

if [[ -d $BDB_PREFIX ]]
then
    echo "$BDB_PREFIX exists -- asuming cached"
else
    cd $TMP
    mkdir -p $BDB_PREFIX
    wget 'http://download.oracle.com/berkeley-db/db-4.8.30.NC.tar.gz'
    echo '12edc0df75bf9abd7f82f821795bcee50f42cb2e5f76a6a281b85732798364ef  db-4.8.30.NC.tar.gz' | sha256sum -c
    tar -xzvf db-4.8.30.NC.tar.gz
    cd db-4.8.30.NC/build_unix/
    ../dist/configure --enable-cxx --disable-shared --with-pic --prefix=$BDB_PREFIX
    make install
fi

# Build libboost
if [[ -d $BOOST_PREFIX ]]
then
    echo "$BOOST_PREFIX exists -- assuming cached"
else
    cd $TMP
    mkdir -p $BOOST_PREFIX
    wget 'http://dl.bintray.com/boostorg/release/1.65.0/source/boost_1_65_0.tar.gz'
    echo '8a142d33ab4b4ed0de3abea3280ae3b2ce91c48c09478518c73e5dd2ba8f20aa  boost_1_65_0.tar.gz' | sha256sum -c
    tar -xzvf boost_1_65_0.tar.gz
    cd boost_1_65_0
    ./bootstrap.sh --prefix=$BOOST_PREFIX
    ./b2 install
fi

# Move back to where the script was called from
cd $OLD_PWD
