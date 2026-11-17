#include "gnss_processor.h"

#include <cmath>
#include <cstdio>
#include <iostream>

namespace {

    constexpr float    kMaxHdop = 5.0f;   // maximum hdop allowed
    constexpr int      kMinSat = 4;      // minimum satellites
    constexpr uint64_t kPeriodMs = 100;    // sensor required to run at 10hz
    constexpr uint64_t kMaxGapMs = 1500;   // maximum time period allowed

        bool reject(const GnssFrame& f, const char* why) {
            std::cerr << "[REJECT] t=" << f.timestamp_ms << " " << why << "\n";
            return false;
        }

}

GnssProcessor::GnssProcessor() : have_last_(false), last_ts_(0) {}

bool GnssProcessor::process(const GnssFrame& f) {
    //Timestamp and time gap checks
    uint64_t dt = 0;  //initializing a time variable for first transmissions
    if (have_last_) {
        if (f.timestamp_ms <= last_ts_) return reject(f, "timestamp is not increasing");
        dt = f.timestamp_ms - last_ts_; //used dt var to store the time gap between two timestamps
    }

    have_last_ = true;
    last_ts_ = f.timestamp_ms; //update the last timestamp for continuity

    if (dt > kMaxGapMs) return reject(f, "Gap longer than 1500 ms"); //Condition for not allowing large time gaps

    //Main content checks
    if (f.fix_type !=1 && f.fix_type !=2) return reject(f, "No fix"); //Condition for fix type

    if (f.satellites_used < kMinSat) return reject(f, "Very few Satellites Used"); //Condition used for checking satellites

    if (!std::isfinite(f.hdop) ||f.hdop <=0 || f.hdop > kMaxHdop) return reject(f, "Invalid HDOP detected");  //Condition used for eliminating high HDOP readings

    if (!std::isfinite(f.lat) || !std::isfinite(f.lon) || f.lat < -90.0 || f.lat > 90.0 || f.lon < -180.0 || f.lon > 180.0) return reject(f, "Latitude and Longitude coordinates are out of accepted range");

    //Unique Test Cases Start --->

    if (f.satellites_used > 50) return reject(f, "Unrealistic satellite count"); //This satellite count indicates faulty reading
    
    if (f.hdop < 0.5f) return reject(f, "Suspiciously low HDOP value");

    //if (f.lat < 18.0 || f.lat > 19.0 || f.lon < 73.0 || f.lon > 74.0) return reject(f, "Coordinates outside expected Pune region"); //Condition added for a unique test case where robot works in a specific area (in this case: Pune)

    if (f.lat == 0.0 && f.lon == 0.0) return reject(f, "Impossible Lat and Long Values");

    //Unique Test Cases End

    //Late Valid Frame check
    if (dt > 2 * kPeriodMs) {  
        std::cerr << "[WARN] t=" << f.timestamp_ms << " late frame: " << dt << " ms since previous\n";
    }
    std::printf("[POS] t=%llu lat=%.7f lon=%.7f\n",
                static_cast<unsigned long long>(f.timestamp_ms), f.lat, f.lon);
    return true;
}
