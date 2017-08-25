#!/bin/sh 
#
# Utilty to compile and run memhash.c key generator. 
# The argument is a hexadecimal 256 bit integer, which is an input for 
# the key generation program.
# it outputs an encoded key, with the same format, generated from input after 
# $ROUNDS rounds
# 
# The following parameters are passed to C binary via environment export
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
# - if zero the program is run only once (one loop) in verbose mode
# - is set to a value > 0 it repeats ROUNDS MEMHASHLOOPS times , 
# printing an intermediate output line after each repeating ROUNDS
MEMHASHLOOPS=0

# On my I5 1.9GHz laptop the large memory latency is 88ns


# Number of shuffle rounds. Each round updates an uint32 in both 
# 256bit input/output array and the large shuffling memory array
ROUNDS=100000000

# Big random uint32 array size. Use smaller (less than cpu cache) values
# to see how speed increases when this runs in cache
# Note: this is the size in number of uint32 entries, the byte size quadruples
# MEMHASHSIZE=49978001
# MEMHASHSIZE=15485863

# ~256kbyte array if someone wants to compare regular speed vs cpu caches:
# MEMHASHSIZE=65521

# almost 256 MB array, big enough for "desktop" hashes
MEMHASHSIZE=67108859

#
# A large file with random bits. By "random" it means they-re all the same

# By default is a file called "bigbuf.dat" in the same directory with 
# this script. A few lines down if not found, the bigbuf.dat is created 
# by compiling and running mkbigbuf.c 
MEMHASHFILE=bigbuf.dat

# Use /dev/zero to test a zero-filled, non-chaotic initial bigbuf
# outputs still look randomish.
# MEMHASHFILE=/dev/zero



#
# The default input key.
# This just for tests, should be provided in command line
HEXPASS=0000000011111111aaaaaaaabbbbbbbbccccccccddddddddeeeeeeeeffffffff
[ -z "$1" ] || HEXPASS=$1

# Creates a 256MB bigbuf.dat file, should well exceed most CPU caches
cd `dirname $0`
[ -f "./bigbuf.dat" ] || {
    echo -n "Creating chaotic bigbuf.dat ..."
    [ -x ./mkbigbuf  ] || gcc -O3 -o mkbigbuf mkbigbuf.c
    ./mkbigbuf bigbuf.dat    > genbigbuf.out
    echo "done! - see few stats in genbigbuf.out some of their\n meaning in mkbigbuf.c\n"
}


export MEMHASHFILE MEMHASHSIZE MEMHASHLOOPS

[ -x "memhash" ] || {
    echo "Compiling memhash.c .... "
    gcc -o memhash memhash.c
}

./memhash $ROUNDS $HEXPASS
