#!/bin/sh 
#
# Utilty to build and run memhash.c large memory key generator. 
# The argument is a hexadecimal 256 bit integer, which is an input for 
# the key generation program
# it outputs a 256bit hex encoded key generated from input after $ROUNDS rounds
# 
# MEMHASHFILE is an initial state that is shuffled together with the input
# to produce the hashed output. 
#
# MEMHASHSIZE is the size of the shuffling big array. 
# Each entry is a uint32 (32bit) number, so you need a MEMHASHFILE
# 
# ROUNDS - is the number of hashing rounds. 
#
# MEMHASHLOOPS 
# - if undefined the program is run only once (one loop) in verbose mode
# - is set to a number it runs MEMHASHLOOPS x ROUNDS number of rounds, 
# printing an intermediate output line after each repeating ROUNDS
MEMHASHLOOPS=100000

# On my I5 2.9GHz laptop the large memory latency is 88ns


# Number of shuffle rounds. Each round updates an uint32 in both 
# 256bit input/output array and the large shuffling memory array
ROUNDS=100000

# Big random uint32 array size. Use smaller (less than cpu cache) values
# to see how speed increases when this runs in cache
# Note: this is the size in number of uint32 entries, the byte size quadruples
MEMHASHSIZE=49978001
MEMHASHSIZE=15485863
# MEMHASHSIZE=52428799
# MEMHASHSIZE=20971519
# ~400kbyte array:
# MEMHASHSIZE=104729

#
# A large file with random bits. By "random" it means they-re all the same
# Use /dev/zero to test a uniform, non-random initial table
MEMHASHFILE=/dev/zero

# By default is a file called "random.bin" in the same directory with 
# this script. A few lines down random.bin if not found then is created 
# from /dev/urandom
# This creation should be replaced with a standard predefined stream
MEMHASHFILE=random.bin



#
# The default input for key derivation function
HEXPASS=aaaaaaaabbbbbbbbccccccccddddddddeeeeeeeeffffffff0000000011111111
[ -z "$1" ] || HEXPASS=$1

# Creates a 200MByte random.bin file, should well exceed most CPU caches
cd `dirname $0`
[ -f "./random.bin" ] || {
    echo -n "Creating random.bin from /dev/urandom..."
    dd if=/dev/urandom of=random.bin bs=1024 count=204800
    echo "done!"
}


export MEMHASHFILE MEMHASHSIZE MEMHASHLOOPS

[ -x "memhash.bin" ] || gcc -o memhash.bin memhash.c

./memhash.bin $ROUNDS $HEXPASS
