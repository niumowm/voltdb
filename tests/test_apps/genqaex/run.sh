#!/usr/bin/env bash

APPNAME="genqa"
CLASSPATH="`ls -x ../../../voltdb/voltdb-*.jar | tr '[:space:]' ':'``ls -x ../../../lib/*.jar | tr '[:space:]' ':'`"
VOLTDB="../../../bin/voltdb"
VOLTCOMPILER="../../../bin/voltcompiler"
LICENSE="../../../voltdb/license.xml"
LEADER="localhost"

# remove build artifacts
function clean() {
    rm -rf obj debugoutput $APPNAME.jar voltdbroot plannerlog.txt voltdbroot
}

# compile the source code for procedures and the client
function srccompile() {
    mkdir -p obj
    javac -classpath $CLASSPATH -d obj \
        src/$APPNAME/*.java \
        src/$APPNAME/procedures/*.java
    # stop if compilation fails
    if [ $? != 0 ]; then exit; fi
}

# build an application catalog
function catalog() {
    srccompile
    $VOLTCOMPILER obj project.xml $APPNAME.jar
    # stop if compilation fails
    if [ $? != 0 ]; then exit; fi
}

# run the voltdb server locally
function server() {
    # if a catalog doesn't exist, build one
    if [ ! -f $APPNAME.jar ]; then catalog; fi
    # run the server
    $VOLTDB create catalog $APPNAME.jar deployment deployment.xml \
        license $LICENSE leader $LEADER
}

# run the client that drives the example
function client() {
    async-benchmark
}

# Asynchronous benchmark sample
# Use this target for argument help
function async-benchmark-help() {
    srccompile
    java -classpath obj:$CLASSPATH:obj genqa.AsyncBenchmark --help
}

function async-benchmark() {
    srccompile
    java -classpath obj:$CLASSPATH:obj genqa.AsyncBenchmark \
        --display-interval=5 \
        --duration=120 \
        --servers=localhost \
        --port=21212 \
        --procedure=JiggleSinglePartition \
        --pool-size=100000 \
        --wait=0 \
        --rate-limit=100000 \
        --auto-tune=true \
        --latency-target=10.0
}

# Multi-threaded synchronous benchmark sample
# Use this target for argument help
function sync-benchmark-help() {
    srccompile
    java -classpath obj:$CLASSPATH:obj genqa.SyncBenchmark --help
}

function sync-benchmark() {
    srccompile
    java -classpath obj:$CLASSPATH:obj genqa.SyncBenchmark \
        --threads=40 \
        --display-interval=5 \
        --duration=120 \
        --servers=localhost \
        --port=21212 \
        --procedure=JiggleSinglePartition \
        --pool-size=100000 \
        --wait=0
}

# JDBC benchmark sample
# Use this target for argument help
function jdbc-benchmark-help() {
    srccompile
    java -classpath obj:$CLASSPATH:obj genqa.JDBCBenchmark --help
}

function jdbc-benchmark() {
    srccompile
    java -classpath obj:$CLASSPATH:obj genqa.JDBCBenchmark \
        --threads=40 \
        --display-interval=5 \
        --duration=120 \
        --servers=localhost \
        --port=21212 \
        --procedure=JiggleSinglePartition \
        --pool-size=100000 \
        --wait=0
}

function help() {
    echo "Usage: ./run.sh {clean|catalog|server|async-benchmark|async-benchmark-help|...}"
    echo "       {...|sync-benchmark|sync-benchmark-help|jdbc-benchmark|jdbc-benchmark-help}"
}

# Run the target passed as the first arg on the command line
# If no first arg, run server
if [ $# -gt 1 ]; then help; exit; fi
if [ $# = 1 ]; then $1; else server; fi