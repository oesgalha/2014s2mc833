#!/bin/bash
for i in `seq 1 10`;
do
  echo "$i"
  (./cliente 127.0.0.1 1024 < input) &
done
