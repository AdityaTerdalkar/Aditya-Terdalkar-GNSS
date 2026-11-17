#pragma once
#include <cstdint>

struct GnssFrame {
    uint64_t timestamp_ms;
    double lat;
    double lon;
    float hdop;
    int satellites_used;
    int fix_type;
};

class GnssProcessor {
public:
    GnssProcessor();

    // Returns true if the frame is valid and accepted, false if rejected (anomaly/error)
    bool process(const GnssFrame& frame);

private:
    bool have_last_;     // var to check if any timestamp has arrived previously
    uint64_t last_ts_;   // last timestamp
};
