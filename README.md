# How to get it run

1. Download "Fernverkehr" and "Deutschland gesamt" from https://gtfs.de/de/feeds/de_fv/ and https://gtfs.de/de/feeds/de_full/ and unzip files

2. Compile code: Linux: g++ -O3 main.cpp -o main

3. Run code: ./main -G path/to/gtfs/folder -Q path/to/query/file

# What you should do

Implement the function getSolutionWithRaptor in raptor.h, then upload raptor.h via Ilias

# Definitions and Formats

## Query format: 

first line: number of queries

each following line: source stop target stop departure time

## Footpaths:

We ignore footpaths to get rid of some special cases

## Solution format:

Pairs of number of transits and arrival time given in seconds (where midnight is 0). Only add a pair to the solution if the arrival time at the target node improved in the current raptor iteration. If there is no journey to the target, return an empty vector.

Example: If the optimal journey from Uni Stuttgart to Ludwigsburg at 10:00 is via S1 and S5 and arrives at 11:00, then the solution pair should be (2, 11\*3600). If there was another journey with only one trip arriving at 11:15, the solution would be (1, 3600\*11 + 15\*60), (2, 11\*3600)

## Transit time:

The transit time (class variable _transit_time) is the time needed to change trains at a stop. You do not need to consider the transit time at the source stop.

# Debugging

The correct output for de_fv.queries can be found in output_de_fv.log

If _transit_time is set to 0, the correct output for de_fv.queries looks like in output_de_fv_without_transittime.log







