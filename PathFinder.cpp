#include <iostream>
#include <vector> 
#include <string>
#include <cctype>
#include <algorithm>
#include <unordered_map>
#include <utility>
#include <queue>
#include <thread>
#include <chrono>
#define N 7
using namespace std;
using namespace std::chrono;

vector<string> cities = {
    "cairo",       // 0
    "giza",        // 1
    "alexandria",  // 2
    "aswan",       // 3
    "luxor",       // 4
    "suez",        // 5
    "port said"    // 6
};

const int INF = 1e9;

vector<vector<int>> graph = {
    // C    G    A    As   L    S    P
    { 0,   15,  220, INF, INF, 130, INF }, // Cairo
    { 15,   0,  210, INF, INF, 140, INF }, // Giza
    { 220, 210,  0,  INF, INF, INF, 190 }, // Alexandria
    { INF, INF, INF,  0,   60, INF, INF }, // Aswan
    { INF, INF, INF,  60,   0, INF, INF }, // Luxor
    { 130, 140, INF, INF, INF,  0,   150 }, // Suez
    { INF, INF, 190, INF, INF, 150,   0  }  // Port Said
};

vector<vector<int>> dist(N, vector<int>(N, -1));
vector<vector<int>> nextCity(N, vector<int>(N, -1));

void floydWarshall(){
    for (int i = 0; i < N; ++i)
       for (int j = 0; j < N; ++j)
          if (graph[i][j] != INF)
              nextCity[i][j] = j;

    for (int k = 0 ; k < N ; k++){
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++){
                dist[i][j] = min (graph[i][j], graph[i][k] + graph[k][j]);
                nextCity[i][j] = nextCity[i][k];
            }
        }
}

unordered_map<string, int> cityIndex;

void initializeCityIndex() {
    for (int i = 0; i < N; ++i) {
        cityIndex[cities[i]] = i;
    }
}

const double AVERAGE_SPEED_KMH = 80.0;
const double TIME_SCALE = 0.05; // كل دقيقه حقيقيه تساوى 50 ميللي ثانيه (للتجربه)

pair<int, int> convertKmToTime(int distanceKm) {
    double timeHours = distanceKm / AVERAGE_SPEED_KMH;  // الوقت بالساعات (كعدد عشري)
    int hours = static_cast<int>(timeHours);            // الجزء الصحيح = الساعات
    int minutes = static_cast<int>((timeHours - hours) * 60); // الجزء العشري * 60 = الدقايق
    return {hours, minutes};
}

vector<string> getPath(int u, int v) {
    if (nextCity[u][v] == -1) return {}; // مفيش طريق
    vector<string> path = { cities[u] };
    while (u != v) {
        u = nextCity[u][v];
        path.push_back(cities[u]);
    }
    return path;
}

void simulatTravel (string start, string end){
    int f = cityIndex[start];
    int t = cityIndex[end];
    const vector<string>& path = getPath(f, t);
    for (size_t i = 0; i < path.size() - 1; i++){
        string from = path[i];
        string to = path[i+1];
        int fromIdx = cityIndex[from];
        int toIdx = cityIndex[to];
        int destination = dist[fromIdx][toIdx];

        auto [hrs, mins] = convertKmToTime (destination);
        int totalTime = (hrs * 60 + mins);

        cout << "Heading from " << from << " to " << to << " (Estimated time: "
        << hrs << "h " << mins << "m)\n";
        
        for (int minute = 1; minute <= totalTime; minute++){
            this_thread::sleep_for(milliseconds(static_cast<int>(TIME_SCALE * 1000)));
            cout << "  >> Minute " << minute << "/" << totalTime << "...\r" << flush;
        }
        cout << "\nArrived at " << to << "\n\n_";
    }
    cout << "You Arrived to your destination.\n";
}

vector<pair<string, int>> getNearestCities(int sourceIndex, int num) {
    if (num >= N) {
        cout << "Requested number exceeds or equals number of cities." << endl;
        return {};
    }

    // Min-Heap: {distance, cityIndex}
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> pq;

    for (int i = 0; i < N; i++) {
        if (i != sourceIndex && dist[sourceIndex][i] != INF) {
            pq.push({ dist[sourceIndex][i], i });
        }
    }

    vector<pair<string, int>> nearest;

    while (!pq.empty() && nearest.size() < num) {
        auto [distance, cityIdx] = pq.top(); pq.pop();
        nearest.push_back({ cities[cityIdx], distance });
    }

    return nearest;
}

unordered_map<string, vector<string>> facilitiesInCity = {
    { "cairo",       {"fuel station", "hospital", "police station"} },
    { "giza",        {"hospital", "hotel"} },
    { "alexandria",  {"fuel station", "hotel", "airport"} },
    { "aswan",       {"hospital"} },
    { "luxor",       {"police station"} },
    { "suez",        {"fuel station", "hospital"} },
    { "port said",   {"fuel station", "hotel", "police station"} }
};

bool hasFacility(const string& city, const string& facilityType) {
    const auto& facilities = facilitiesInCity[city];
    return find(facilities.begin(), facilities.end(), facilityType) != facilities.end();
}


void findNearestFacility(const string& fromCity, const string& facilityType) {
    if (!cityIndex.count(fromCity)) {
        cout << "Invalid source city.\n";
        return;
    }

    int from = cityIndex[fromCity];
    int minDistance = INF;
    string nearestCity = "";

    for (const auto& [city, index] : cityIndex) {
        if (city == fromCity) continue;

        if (hasFacility(city, facilityType)) {
            int distance = dist[from][index];
            if (distance < minDistance) {
                minDistance = distance;
                nearestCity = city;
            }
        }
    }

    if (nearestCity.empty()) {
        cout << "No nearby city contains a " << facilityType << ".\n";
    } else {
        auto [hrs, mins] = convertKmToTime(minDistance);
        cout << "Nearest " << facilityType << " is in " << nearestCity << ".\n";
        cout << "Distance: " << minDistance << " km\n";
        cout << "Estimated travel time: " << hrs << " hour(s) and " << mins << " minute(s)\n";
    }
    char ans;
    cout << "Start journey (y/n)? ";
    cin >> ans;
    if (ans == 'y' || ans == 'Y')
      simulatTravel(fromCity, nearestCity);
}

void queryShortestPath(const string& from, const string& to) {
    if (!cityIndex.count(from) || !cityIndex.count(to)) {
        cout << "One or both city names are invalid.\n";
        return;
    }  
    int f = cityIndex[from];
    int t = cityIndex[to];
    int shortestKMs = dist[f][t];
    if (dist[f][t] == INF) {
        cout << "No path exists between " << cities[f] << " and " << cities[t] << "." << endl;
        return;
    }  

    cout << "Shortest distance from " << cities[f] << " to " << cities[t] << " is: " << dist[f][t] << " km" << endl;
    auto [hrs, mins] = convertKmToTime(dist[f][t]);
    cout << "Estimated travel time: " << hrs << " hour(s) and " << mins << " minute(s)" << endl;

    // عرض الطريق
    const vector<string>& path = getPath(f, t);
    cout << "Route: ";
    for (int i = 0; i < path.size(); ++i) {
        cout << path[i];
        if (i != path.size() - 1) cout << " -> ";
    }
    cout << endl;
    char ans;
    cout << "Start journey (y/n)? ";
    cin >> ans;
    if (ans == 'y' || ans == 'Y')
      simulatTravel(from, to);
}

void updateEdge(string place1, string place2, int newCost) {
    if (place1 == place2) {
        cout << "Wrong Input! Cannot update route to the same city.\n";
        return;
    }

    if (!cityIndex.count(place1) || !cityIndex.count(place2)) {
        cout << "City not found.\n";
        return;
    }

    int p1 = cityIndex[place1], p2 = cityIndex[place2];
    if (graph[p1][p2] == newCost) {
        cout << "No change needed. The cost is already " << newCost << " km.\n";
        return;
    }

    graph[p1][p2] = newCost;
    graph[p2][p1] = newCost; // لو الطريق في الاتجاهين
    floydWarshall();

    cout << "Updated route between " << place1 << " and " << place2 << " to cost " << newCost << " km.\n";
}

string toLowerString(const string& input) {
    string result = input;
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

enum choiseMenue {
    Traveling = 1,
    Shortest_path,
    Find_nearest_facility,
    Update_route,
    Nearest_cities,
    Exit
};

int main(){
    initializeCityIndex();
    dist = graph;       
    floydWarshall();
    int choise, n;
    cout << "WELCOME !" << endl;
    string from, to;
    do{
        cout << "1- Traveling\n"
                "2- Shortest path\n"
                "3- Find the nearest facility\n"
                "4- Update route\n"
                "5- Nearest cities\n"
                "6- Exit\n"
                "Choise selected: ";
        cin >> choise;
        if (choise == Traveling){
            cout << "Enter the city you're traveling from: ";
            cin.ignore();
            getline(cin, from);
            cout << "Enter the city you're traveling to: ";
            cin.ignore();
            getline(cin, to);
            from = toLowerString (from);
            to = toLowerString (to);
            simulatTravel(from, to);
        }else if (choise == Shortest_path){
            cout << "Name of the first city: ";
            cin.ignore();
            getline(cin, from);
            cout << "Name of the second city: ";
            cin.ignore();
            getline(cin, to);
            from = toLowerString (from);
            to = toLowerString (to);
            queryShortestPath (from, to);
        }else if (choise == Find_nearest_facility){
            cout << "Enter the city you are in: ";
            cin.ignore();
            getline(cin, from);
            cout << "Choos facility\n"
                    "Fuel Station\n"
                    "Police Station\n"
                    "Hospital\n"
                    "Hotel\n"
                    "Airport\n"
                    ": ";
            cin.ignore();
            getline(cin, to);
            from = toLowerString (from);
            to = toLowerString (to);
            findNearestFacility (from, to);
        }else if (choise == Update_route){
            int newCost;
            cout << "How many routes you want to update? ";
            cin >> n;
            for (int i = 0; i < n; i++){
                cout << "Enter place 1: ";
                cin.ignore();
                getline(cin, from);
                cout << "Enter place 2: ";
                cin.ignore();
                getline(cin, to);
                cout << "Enter new cost: ";
                cin >> newCost;
                from = toLowerString (from);
                to = toLowerString (to);
                updateEdge (from, to, newCost);
            }
        }else if (choise == Nearest_cities){
            cout << "Enter city name: ";
            cin.ignore();
            getline(cin, from);
            cout << "How many nearest cities do you want? ";
            cin >> n;
            from = toLowerString (from);
            if (cityIndex.count(from)) {
               int sourceIdx = cityIndex[from];
               auto result = getNearestCities(sourceIdx, n);
               for (auto& [name, d] : result) {
                   cout << name << " at distance: " << d << " km" << endl;
                    auto [hrs, mins] = convertKmToTime(d);
                    cout << "Estimated travel time: " << hrs << " hour(s) and " << mins << " minute(s)" << endl;
                }
            } else {
                cout << "City not found." << endl;
            }
        } else {
            cout << "Wrong Input. Try again!\n";
        }
        
    }while (choise != Exit);
}
