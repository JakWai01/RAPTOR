#include "raptor.h"
#include <chrono>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <random>

using namespace std;

string getTime() {
  auto time1 = std::chrono::system_clock::now();
  time_t time2 = chrono::system_clock::to_time_t(time1);
  return ctime(&time2);
}

int main(int argc, char *argv[]) {

  string path_to_gtfs, path_to_query_file;
  int num_queries = 100;
  while (1) {
    int result = getopt(argc, argv, "G:Q:n:");
    if (result == -1)
      break; /* end of list */
    switch (result) {
    case '?': /* unknown parameter */
      break;
    case ':': /* missing argument of a parameter */
      fprintf(stderr, "missing argument.\n");
      break;
    case 'G':
      path_to_gtfs = optarg;
      break;
    case 'Q':
      path_to_query_file = optarg;
      break;
    case 'n':
      num_queries = stoi(optarg);
    default: /* unknown */
      break;
    }
  }

  cout << "Usage: " << argv[0] << " -G <path to gtfs> -Q <query file>" << endl;
  RAPTOR raptor;
  raptor.readInGTFS(path_to_gtfs);
  /*{
      vector<Query> queries = raptor.generateQueries(num_queries);
      ofstream writer(path_to_query_file);
      writer << queries.size() << "\n";
      for (const Query &query : queries)
      {
          writer << query.source << " " << query.target << " " <<
  query.departure_time << "\n";
      }
      writer.close();
  }*/

  ifstream reader(path_to_query_file);
  vector<Query> queries;
  reader >> num_queries;
  for (int i = 0; i < num_queries; i++) {
    StopID source, target;
    int departure_time;
    reader >> source >> target >> departure_time;
    queries.push_back(Query(source, target, departure_time));
  }
  reader.close();
  vector<Solution> solutions(num_queries);
  std::chrono::steady_clock::time_point start(std::chrono::steady_clock::now());
  for (int i = 0; i < queries.size(); i++) {
    solutions.at(i) = raptor.getSolutionWithRaptor(queries.at(i));
  }
  std::chrono::steady_clock::time_point end(std::chrono::steady_clock::now());
  int counter_no_journey_found = 0;
  long checksum = 0;
  for (int i = 0; i < num_queries; i++) {
    cout << "Query " << i + 1 << ":";
    if (solutions.at(i).pairs_of_num_trips_and_arrival_time.size() == 0) {
      cout << " no journey found";
      counter_no_journey_found++;
    } else {
      const Solution &solution = solutions.at(i);
      for (pair<int, int> sol : solution.pairs_of_num_trips_and_arrival_time) {
        cout << " (" << sol.first << ", " << sol.second << ")";
        checksum += sol.first + sol.second;
      }
    }
    cout << endl;
  }
  checksum += counter_no_journey_found;
  cout << "Queries took "
       << std::chrono::duration_cast<std::chrono::duration<double>>(end - start)
                  .count() /
              queries.size()
       << " seconds on average" << endl;
  cout << "Counter no journeys: " << counter_no_journey_found << endl;
  cout << "Checksum: " << checksum << endl;
  return 0;
}