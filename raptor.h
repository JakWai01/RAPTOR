#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <math.h>
#include <queue>
#include <set>
#include <sstream>
#include <vector>

using namespace std;

typedef int StopID;
typedef int RouteID;
typedef int TripID;
typedef int RouteID;

struct Query {
  StopID source;
  StopID target;
  int departure_time;
  Query() {}
  Query(StopID source_, StopID target_, int departure_time_)
      : source(source_), target(target_), departure_time(departure_time_) {}
};

struct Solution {
  vector<pair<int, int>> pairs_of_num_trips_and_arrival_time;
};

struct Trip {
  TripID tripid;
  RouteID routeid;
  vector<int> departure_times;
  vector<int> arrival_times;
  Trip() {}
  Trip(TripID tripid_, RouteID routeid_) : tripid(tripid_), routeid(routeid_) {}
};

struct Stop {
  StopID stopid;
  StopID parentid;
  string name;
  double latitude;
  double longitude;
  Stop() {}
  Stop(StopID stopid_, string name_, double latitude_, double longitude_)
      : stopid(stopid_), name(name_), latitude(latitude_),
        longitude(longitude_) {
    parentid = -1;
  }
  Stop(StopID stopid_, StopID parentid_, string name_, double latitude_,
       double longitude_)
      : stopid(stopid_), parentid(parentid_), name(name_), latitude(latitude_),
        longitude(longitude_) {}
};

struct Route {
  RouteID routeid;
  string name;
  vector<StopID> stops;
  Route() {}
  Route(RouteID routeid_, string name_) : routeid(routeid_), name(name_) {}
};

struct StopTime {
  TripID tripid;
  StopID stopid;
  int departure_time;
  int arrival_time;
  StopTime() {}
  StopTime(TripID tripid_, StopID stopid_, int arrival_time_,
           int departure_time_)
      : tripid(tripid_), stopid(stopid_), departure_time(departure_time_),
        arrival_time(arrival_time_) {}
};

struct Connection {
  TripID tripid;
  StopID departure_stop;
  StopID arrival_stop;
  int departure_time;
  int arrival_time;
  Connection() {}
  Connection(TripID tripid_, StopID departure_stop_, StopID arrival_stop_,
             int departure_time_, int arrival_time_)
      : tripid(tripid_), departure_stop(departure_stop_),
        arrival_stop(arrival_stop_), departure_time(departure_time_),
        arrival_time(arrival_time_) {}
};

class RAPTOR {
public:
  // Compute for every k a nondominated journey
  // to a target stop pt with minimum arrival time having
  // at most k trips.
  Solution getSolutionWithRaptor(const Query &query) {
    StopID source = query.source;
    StopID target = query.target;
    int departure_time = query.departure_time;

    // Maximum number of trips
    int k = 3;

    // Initialization of our algorithm
    vector<vector<int>> departure_times_container;
    vector<int> earliest_known_arrival_time(_stops.size(),
                                            std::numeric_limits<int>::max());
    vector<bool> visited(_stops.size(), false);

    for (int i = 0; i < k; i++) {
      // Departure times for every stop with at most k trips initialized as inf
      vector<int> departure_times(_stops.size(),
                                  std::numeric_limits<int>::max());
      departure_times_container.push_back(departure_times);
    }

    vector<int> earliest_known_arrival_time(_stops.size(),
                                            std::numeric_limits<int>::max());

    departure_times_container[0][source] = departure_time;
    earliest_known_arrival_time[source] = departure_time;

    vector<bool> visited(_stops.size(), false);

    // k rounds
    for (int i = 0; i < k; i++) {
      // PQ utilizing a min heap
      // priority_queue<int, vector<int>, greater<int>> pq;
      std::set<pair<RouteID, StopID>> q;
      // for all marked stops
      for (StopID stop = 0; stop < _stops.size(); stop++) {
        if (visited[stop] == false) {
          continue;
        }

        // Loop through routes serving the stop p
        for (const auto &route_and_position :
             _routes_and_position_of_stop_per_stop.at(stop)) {

          RouteID route_id = route_and_position.first;
          int position_in_route = route_and_position.second;

          // Iterate over each trip of the route
          for (TripID trip_id :
               _trips_per_route_sorted_by_departure_time[route_id]) {
            const Trip &trip = _trips[_original_tripid_to_internal_id[trip_id]];

            // Check if the trip can be boarded at the current stop
            if (trip.departure_times[position_in_route] <
                departure_times_container[i][stop]) {
              continue;
            }

            // Update the earliest arrival times for subsequent stops in this
            // trip
            for (int j = position_in_route; j < trip.arrival_times.size();
                 j++) {
              StopID next_stop = _routes[route_id].stops[j];
              int arrival_time_at_next_stop = trip.arrival_times[j];

              if (earliest_known_arrival_time[next_stop] >
                  arrival_time_at_next_stop) {
                earliest_known_arrival_time[next_stop] =
                    arrival_time_at_next_stop;
                departure_times_container[i + 1][next_stop] =
                    arrival_time_at_next_stop +
                    _transit_time; // Account for transfer time
                q.insert({arrival_time_at_next_stop, next_stop});
              }
            }
          }
        }

        // unmark p
        visited[stop] = false;
      }
    }

    return Solution();
  }

  void readInGTFS(string path_to_gtfs_folder) {
    cout << "Start reading files in " << path_to_gtfs_folder << endl;
    _original_routeid_to_internal_id.clear();
    _original_stopid_to_internal_id.clear();
    _original_tripid_to_internal_id.clear();
    { // Read in stops
      _stops.clear();
      ifstream reader(path_to_gtfs_folder + "/stops.txt");
      string line;
      getline(reader, line); // ignore header
      while (getline(reader, line)) {
        stringstream linestream(line);
        string stop_name;
        string tmp;
        if (line.find('"') != string::npos) {
          getline(linestream, stop_name, '"');
          getline(linestream, stop_name, '"');
          getline(linestream, tmp, ',');
        } else {
          getline(linestream, stop_name, ',');
        }
        getline(linestream, tmp, ',');
        StopID parentid = -1;
        ;
        if (tmp.size() > 0) {
          parentid = stoi(tmp);
        }
        getline(linestream, tmp, ',');
        StopID stopid = stoi(tmp);
        if (parentid == -1) {
          parentid = stopid;
        }
        double lat, lon;
        getline(linestream, tmp, ',');
        lat = stod(tmp);
        getline(linestream, tmp, ',');
        lon = stod(tmp);
        if (_original_stopid_to_internal_id.size() <= stopid) {
          _original_stopid_to_internal_id.resize(stopid + 1);
        }
        _original_stopid_to_internal_id.at(stopid) = _stops.size();
        _stops.push_back(Stop(stopid, parentid, stop_name, lat, lon));
      }
      cout << "Stops: " << _stops.size() << endl;
      if (_stops.size() == 0) {
        cout << "Error: no stops found" << endl;
        exit(1);
      }
    }
    { // Read in routes
      _routes.clear();
      ifstream reader(path_to_gtfs_folder + "/routes.txt");
      string line;
      getline(reader, line); // ignore header
      while (getline(reader, line)) {
        stringstream linestream(line);
        string route_name;
        string tmp;
        getline(linestream, tmp, ','); // ignore int name
        getline(linestream, route_name, ',');
        getline(linestream, tmp, ','); // ignore agency
        getline(linestream, tmp, ','); // ignore type
        getline(linestream, tmp, ',');
        RouteID routeid = stoi(tmp);
        if (_original_routeid_to_internal_id.size() <= routeid) {
          _original_routeid_to_internal_id.resize(routeid + 1);
        }
        _original_routeid_to_internal_id.at(routeid) = _routes.size();
        _routes.push_back(Route(routeid, route_name));
      }
      cout << "Routes: " << _routes.size() << endl;
    }
    { // Read in trips
      _trips.clear();
      ifstream reader(path_to_gtfs_folder + "/trips.txt");
      string line;
      getline(reader, line); // ignore header
      while (getline(reader, line)) {
        stringstream linestream(line);
        string tmp;
        getline(linestream, tmp, ',');
        RouteID routeid = stoi(tmp);
        getline(linestream, tmp, ','); // ignore service id
        getline(linestream, tmp, ',');
        TripID tripid = stoi(tmp);
        if (_original_tripid_to_internal_id.size() <= tripid) {
          _original_tripid_to_internal_id.resize(tripid + 1);
        }
        _original_tripid_to_internal_id.at(tripid) = _trips.size();
        _trips.push_back(Trip(tripid, routeid));
      }
      cout << "Trips: " << _trips.size() << endl;
    }
    { // Read in stop_times
      vector<StopTime> stop_times;
      ifstream reader(path_to_gtfs_folder + "/stop_times.txt");
      string line;
      getline(reader, line); // ignore header
      while (getline(reader, line)) {
        stringstream linestream(line);
        string tmp;
        getline(linestream, tmp, ',');
        TripID tripid = stoi(tmp);
        string arrival_time_string, departure_time_string;
        getline(linestream, arrival_time_string, ',');
        getline(linestream, departure_time_string, ',');
        getline(linestream, tmp, ',');
        StopID stopid = stoi(tmp);
        int arrival_time = 0;
        {
          stringstream time_stream(arrival_time_string);
          int hours, minutes, seconds;
          getline(time_stream, tmp, ':');
          hours = stoi(tmp);
          getline(time_stream, tmp, ':');
          minutes = stoi(tmp);
          getline(time_stream, tmp);
          seconds = stoi(tmp);
          arrival_time = hours * 3600 + minutes * 60 + seconds;
        }
        int departure_time = 0;
        {
          stringstream time_stream(departure_time_string);
          int hours, minutes, seconds;
          getline(time_stream, tmp, ':');
          hours = stoi(tmp);
          getline(time_stream, tmp, ':');
          minutes = stoi(tmp);
          getline(time_stream, tmp);
          seconds = stoi(tmp);
          departure_time = hours * 3600 + minutes * 60 + seconds;
        }
        stop_times.push_back(StopTime(
            tripid,
            _stops.at(_original_stopid_to_internal_id.at(stopid)).parentid,
            arrival_time, departure_time));
      }
      cout << "Stop times: " << stop_times.size() << endl;

      { // Initialize routes, trips and connections using stop times
        sort(stop_times.begin(), stop_times.end(),
             [](const StopTime &con1, const StopTime &con2) {
               return con1.departure_time < con2.departure_time ||
                      (con1.departure_time == con2.departure_time &&
                       con1.arrival_time < con2.arrival_time);
             });
        vector<vector<StopID>> stops_per_trip(_trips.size());
        long counter_instant_connections = 0;
        for (const StopTime &stoptime : stop_times) {
          TripID internal_trip_id =
              _original_tripid_to_internal_id.at(stoptime.tripid);
          if (stops_per_trip.at(internal_trip_id).size() > 0) {
            if (_trips.at(internal_trip_id).departure_times.back() >=
                stoptime.arrival_time) {
              counter_instant_connections++;
              continue;
            }
            _connections.push_back(Connection(
                stoptime.tripid, stops_per_trip.at(internal_trip_id).back(),
                stoptime.stopid,
                _trips.at(internal_trip_id).departure_times.back(),
                stoptime.arrival_time));
          }
          _trips.at(internal_trip_id)
              .arrival_times.push_back(stoptime.arrival_time);
          _trips.at(internal_trip_id)
              .departure_times.push_back(stoptime.departure_time);
          stops_per_trip.at(internal_trip_id).push_back(stoptime.stopid);
        }
        sort(_connections.begin(), _connections.end(),
             [](const Connection &con1, const Connection &con2) {
               return con1.departure_time < con2.departure_time ||
                      (con1.departure_time == con2.departure_time &&
                       con1.arrival_time < con2.arrival_time);
             });
        cout << "Connections: " << _connections.size() << endl;
        cout << "Removed connections with no travel time: "
             << counter_instant_connections << endl;

        // Trips with different stops can share the same route, we therefore
        // have to duplicate some routes. We also have to remove empty trips
        {
          vector<vector<Route>> route_variations_per_route(_routes.size(),
                                                           vector<Route>());
          vector<Route> all_routes;
          vector<Trip> new_trips;
          new_trips.reserve(_trips.size());
          for (Trip &trip : _trips) {
            if (trip.arrival_times.size() == 0) {
              continue;
            }
            TripID internal_trip_id =
                _original_tripid_to_internal_id.at(trip.tripid);
            RouteID internal_route_id =
                _original_routeid_to_internal_id.at(trip.routeid);
            bool route_exists = false;
            for (const Route &route :
                 route_variations_per_route.at(internal_route_id)) {
              bool is_identical_to_trip =
                  route.stops.size() ==
                  stops_per_trip.at(internal_trip_id).size();
              if (is_identical_to_trip) {
                for (int i = 0; i < route.stops.size(); i++) {
                  if (route.stops.at(i) !=
                      stops_per_trip.at(internal_trip_id).at(i)) {
                    is_identical_to_trip = false;
                    break;
                  }
                }
              }
              if (is_identical_to_trip) {
                route_exists = true;
                trip.routeid = route.routeid;
                break;
              }
            }
            if (!route_exists) {
              Route new_route(all_routes.size(),
                              _routes.at(internal_route_id).name);
              trip.routeid = all_routes.size();
              new_route.stops = stops_per_trip.at(internal_trip_id);
              route_variations_per_route.at(internal_route_id)
                  .push_back(new_route);
              all_routes.push_back(new_route);
            }
            _original_tripid_to_internal_id.at(trip.tripid) = new_trips.size();
            new_trips.push_back(trip);
          }
          _routes = all_routes;
          _trips = new_trips;
        }
        cout << "Distinct routes: " << _routes.size() << endl;
        cout << "Non-empty trips: " << _trips.size() << endl;
      }
    }

    { // Initialize data structures for raptor
      _routes_and_position_of_stop_per_stop.assign(
          _stops.size(), vector<pair<RouteID, int>>());
      for (const Route &route : _routes) {
        for (int i = 0; i < route.stops.size(); i++) {
          _routes_and_position_of_stop_per_stop
              .at(_original_stopid_to_internal_id.at(route.stops.at(i)))
              .push_back(make_pair(route.routeid, i));
        }
      }
      _trips_per_route_sorted_by_departure_time.assign(_routes.size(),
                                                       vector<TripID>());
      for (const Trip &trip : _trips) {
        _trips_per_route_sorted_by_departure_time.at(trip.routeid)
            .push_back(trip.tripid);
      }
      for (RouteID rid = 0; rid < _routes.size(); rid++) {
        sort(_trips_per_route_sorted_by_departure_time.at(rid).begin(),
             _trips_per_route_sorted_by_departure_time.at(rid).end(),
             [&](TripID t1, TripID t2) {
               return _trips.at(_original_tripid_to_internal_id.at(t1))
                          .arrival_times.at(0) <
                      _trips.at(_original_tripid_to_internal_id.at(t2))
                          .arrival_times.at(0);
             });
      }
    }
    { // Remove redundant trips
      vector<bool> trip_is_redundant(_trips.size(), false);
      long counter_redundant_trips = 0;
      for (RouteID rid = 0; rid < _routes.size(); rid++) {
        for (int t = 1;
             t < _trips_per_route_sorted_by_departure_time.at(rid).size();
             t++) {
          TripID tid = _original_tripid_to_internal_id.at(
              _trips_per_route_sorted_by_departure_time.at(rid).at(t));
          for (int t2 = 0; t2 < t && !trip_is_redundant.at(tid); t2++) {
            TripID tid2 = _original_tripid_to_internal_id.at(
                _trips_per_route_sorted_by_departure_time.at(rid).at(t2));
            if (trip_is_redundant.at(tid2)) {
              continue;
            }
            for (int x = 0; x < _routes.at(rid).stops.size(); x++) {
              if (_trips.at(tid).arrival_times.at(x) <=
                  _trips.at(tid2).arrival_times.at(x)) {
                trip_is_redundant.at(tid) = true;
                counter_redundant_trips++;
                break;
              }
            }
          }
        }
      }
      if (counter_redundant_trips > 0) {
        cout << "Redundant trips: " << counter_redundant_trips << endl;
        { // Remove redundant connections
          vector<Connection> new_connections;
          new_connections.reserve(_connections.size());
          for (const Connection &con : _connections) {
            if (!trip_is_redundant.at(
                    _original_tripid_to_internal_id.at(con.tripid))) {
              new_connections.push_back(con);
            }
          }
          _connections = new_connections;
        }
        { // Fix raptor data
          for (RouteID rid = 0; rid < _routes.size(); rid++) {
            vector<TripID> new_trips;
            for (TripID tid :
                 _trips_per_route_sorted_by_departure_time.at(rid)) {
              if (!trip_is_redundant.at(
                      _original_tripid_to_internal_id.at(tid))) {
                new_trips.push_back(tid);
              }
            }
            _trips_per_route_sorted_by_departure_time.at(rid) = new_trips;
          }
        }
        { // Remove redundant trips
          vector<Trip> new_trips;
          new_trips.reserve(_trips.size());
          for (const Trip &trip : _trips) {
            if (!trip_is_redundant.at(
                    _original_tripid_to_internal_id.at(trip.tripid))) {
              _original_tripid_to_internal_id.at(trip.tripid) =
                  new_trips.size();
              new_trips.push_back(trip);
            }
          }
          _trips = new_trips;
        }
        cout << "Remaining trips: " << _trips.size() << endl;
        cout << "Remaining connections: " << _connections.size() << endl;
      }
    }
  }

  vector<Query> generateQueries(int num_queries) {
    int start_time = 36000;
    vector<Query> queries;
    for (int i = 0; i < num_queries; i++) {
      const Connection random_start_con =
          _connections.at(random() % _connections.size());
      StopID source = random_start_con.departure_stop;
      const Connection random_stop_con =
          _connections.at(random() % _connections.size());
      StopID target = random_stop_con.arrival_stop;
      queries.push_back(Query(source, target, start_time));
    }
    return queries;
  }

private:
  vector<Route> _routes;
  vector<Trip> _trips;
  vector<Stop> _stops;
  vector<Connection> _connections;
  const int _transit_time = 180; // seconds needed to change trains
  vector<StopID> _original_stopid_to_internal_id;
  vector<TripID> _original_tripid_to_internal_id;
  vector<RouteID> _original_routeid_to_internal_id;
  vector<vector<pair<RouteID, int>>> _routes_and_position_of_stop_per_stop;
  vector<vector<TripID>> _trips_per_route_sorted_by_departure_time;
};