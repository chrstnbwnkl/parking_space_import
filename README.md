# Import Parking Spaces into a Valhalla Tile Set

This project contains tools to import nodes with a `amenity=parking_space` as parking graph nodes into Valhalla for use with the multimodal drive & walk algorithm.

## Motivation 

With the new multimodal routing algorithm, Valhalla enables routing scenarios where a user wants to drive their vehicle from a known start location, park their vehicle near the destination and walk to the destination. While the algorithm is there, the parking data is largely missing. This project tackles the data problem by importing nodes from OSM data that are tagged with `amenity=parking_space` and correlating them to the graph, so that they can act as mode transition points for the algorithm.

Since parking spaces mark the individual parking space, importing these in a full planet graph is not desirable, which is why this project starts out seperately from the main Valhalla project. It is recommended only for use with small geographic areas with the number of parking spaces in the hundreds.

## How it works

The process is split into a parsing and a correlation step.

### Parsing OSM data

Reads the PBF file used to build the Valhalla graph, parses out nodes marked as parking spaces, and stores them in a custom binary file for quick memory-mapped access (using `midgard::sequence<T>`). Also stores level information, assuming the parking space is on a single level;

### Correlating to the graph 

Goes through the found parking space nodes and projects them onto the nearest edge(s) in the graph until it has a candidate for each acccess mode (i.e. an edge accessible by car and by foot; they can be the same edge but don't necesssarily have to be). It then creates edges to each candidate's start and end node, re-using the shape of the candidate edge from/up to the projected point. This will create at least two edges, possibly more if pedestrian/car access have different edge candidates.


## How to use 

If you want to add parking spaces to your Valhalla graph, you can split up your graph build into two phases and insert the parking space ingestion in between the transit and bikesharing graph build stages: 


```bash 
# build the graph up to and including the transit stage
valhalla_build_tiles -c valhalla.json -e transit 

# now the graph is at a stage where it's reasonably easy to insert new graph elements
import_parking_spaces -c valhalla.json your_map.osm.pbf 

# ...run the rest of the graph build 
valhalla_built_tiles -c valhalla.json -s bss 
```


