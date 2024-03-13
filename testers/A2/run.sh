#!/bin/bash

echo "RUNNING ISOLATION DEMO: ETA ~ 30sec"
echo "demo is running in the background"

cd ./lan-isolation/
./demo-part1.sh

if [[ ! -d ../isolation_logs ]]; then
    mkdir ../isolation_logs
fi

cp ./logs/* ../isolation_logs/

echo "FINISHED ISOLATION DEMO"
echo "check contents of isolation_logs"

echo "__________________________________"

echo "RUNNING RELAY DEMO: ETA ~ 30sec"
echo "demo is running in the background"

cd ../lan-relay/ 
./demo_part2.sh

if [[ ! -d ../relay_logs ]]; then
    mkdir ../relay_logs
fi

cp ./logs/* ../relay_logs/

cd ../

echo "FINISHED RELAY DEMO"
echo "check contents of relay_logs"
