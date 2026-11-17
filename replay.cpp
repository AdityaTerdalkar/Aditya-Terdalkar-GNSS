#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "gnss_processor.h"

int main() {
    std::ifstream in("gnss.csv");
    if (!in) { std::cerr << "Cannot open gnss.csv\n"; return 1; }

    GnssProcessor processor;
    int accepted = 0, rejected = 0;
    std::string line;
    std::getline(in, line);                    // Used to skip header row

    while (std::getline(in, line)) {
        for (char& c : line) if (c == ',') c = ' ';   // CSV File
        std::istringstream row(line);
        GnssFrame f;
        if (!(row >> f.timestamp_ms >> f.lat >> f.lon >> f.hdop >> f.satellites_used >> f.fix_type)) continue;

        if (processor.process(f)) ++accepted;
        else ++rejected;
    }

    std::cout << "Accepted: " << accepted << "\nRejected: " << rejected << "\n";
}
