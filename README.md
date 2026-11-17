GNSS Processor – Take-Home Assignment

The assignment asks for a C++17 class, GnssProcessor, that checks GNSS frames (timestamp, latitude, longitude, HDOP, satellites, fix type) coming from a receiver at about 10 Hz. The processor must accept clean frames, print their position, reject bad ones and log why. I used only the C++ standard library.

How I approached it:
I read the provided tests first, because they define what "bad" means, and turned each one into a rule. The thresholds are my assumptions based on the provided test_gnss_processor file.

The main few test cases are as follows:
1. Timestamp must be strictly increasing, and a gap longer than 1500 ms is rejected.
2. Fix type must be 1 or 2, and at least 4 satellites are needed.
3. HDOP must be a real number above 0 and at most 5.0.
4. Latitude and longitude must be real numbers inside the valid range (NaN is checked explicitly, because it passes ordinary comparisons).
5. My own sanity rules: more than 50 satellites, HDOP below 0.5 (a conservative choice that is easy to remove) and a position of exactly (0, 0) are rejected. A Pune-region check is written but commented out because it is deployment-specific.
6. After a rejection the processor prints nothing, keeps no bad data and accepts the next valid frame immediately. It updates the stored timestamp even when it rejects a long gap; otherwise one gap would make every later frame look late and the processor would never recover. Frames more than 200 ms late but within the limit are accepted with a warning. Positions go to stdout and logs to stderr.

Thresholds and Limits:
- What assumptions I made:
I Assumed specific operating thresholds based on the test suite,
1. Maximum acceptable HDOP: 5.0f (and a custom lower bound of 0.5f as a sanity check).
2. Minimum satellite count: 4 satellites (and a maximum cap of 50 satellites to filter faulty readings).
3. Operating frequency / period: 10 Hz (~100 ms period), with a maximum allowable time gap between frames of 1500 ms.

Fix Types: Assumed that valid GNSS fixes correspond strictly to fix type 1 or 2.
Coordinate Boundaries: Assumed standard global geographical limits (Latitude: -90 degrees to +90 degrees, Longitude: -180 degrees to +180 degrees) and treated exact coordinates of (0.0, 0.0) as impossible/invalid.

- Which Validation Rules I implemented:
1. Timestamps must be strictly increasing relative to the previous frame.
2. Time gaps between frames must not exceed 1500 ms. A warning is issued if a frame is late (greater than 200 ms / 2 x period) but within the acceptable gap limit.
3. Fix type must be either 1 or 2.
4. Satellites used must be at least 4 and no more than 50.
5. HDOP must be a finite real number, strictly greater than 0, less than or equal to 5.0, and not suspiciously low (Less than 0.5).
6. Latitude and longitude must be finite real numbers (explicitly checking for NaN and infinity using std::isfinite) within valid global ranges.
7. Rejects null island coordinates: (0.0, 0.0).

- How should the processor behave after a rejected frame, and why?
1. Behaviour:The processor drops the bad frame entirely, outputs nothing to stdout, logs the rejection reason to stderr, and still updates the stored timestamp (last_ts_). It immediately accepts the next valid incoming frame without retaining any state from the rejected frame. 

2. Why: Updating the timestamp even on a rejection is crucial for recovery. If the timestamp were not updated after rejecting a long time gap, every subsequent frame would appear to have an exponentially compounding time gap, causing the processor to lock up and reject all future frames permanently.

- What would you change before using this component on a real robot?
1. Kinematic / Speed-Plausibility Check:
Right now, the code checks each message by itself. During testing, the GPS jumped 70 km in a fraction of a second and still passed. Before putting this on a real robot, I would add a speed check to reject sudden, impossible teleportation between two points.

2. Sensor Health Monitoring & Error Counters:
I would add a counter to track how many bad messages happen in a row. If the GPS fails too many times, the robot should be alerted to stop or switch to backup sensors.

Design Decisions:
- I used constexpr for fixed thresholds such as maximum HDOP, minimum satellite count and maximum timestamp gap. This keeps the configuration readable and avoids unnecessary hardcoding throughout the code.
- I used uint64_t for timestamps because timestamps are non-negative values and require sufficient range for millisecond-based time tracking.
- I used std::isfinite() to reject NaN and infinity values, since ordinary comparison operators alone cannot reliably detect them.
- I separated rejection logging into a helper function called reject(), which avoids repeating the same logging and return logic throughout the processor.
- I used std::cerr for warnings and rejection messages and std::printf for accepted GPS positions, keeping diagnostic output separate from normal position output.

Testing and Replay:
All 7 provided tests pass, plus I made unique 4 tests of my own:
replay.cpp runs the comma-separated gnss.csv through the processor: of 500 frames, 459 are accepted and 41 rejected with 9 late-frame warnings.

The replay exposed a limitation: the 70 km position jump at 43800 ms passes every rule because each rule looks at one frame alone. A speed-plausibility check is my first proposed improvement.

Build and run:
bash
g++ -std=c++17 test_gnss_processor.cpp gnss_processor.cpp -o test_gnss && ./test_gnss

g++ -std=c++17 replay.cpp gnss_processor.cpp -o replay && ./replay

Process:
In completing this assignment I took help of Claude AI for basic syntax checks and to help me create a more efficient code to reduce memory usage and making it as robust as possible.

How I learned it:
I used a reverse engineering approach to this, 
I asked myself 3 main questions,
    1. What is the required end goal?
    2. What and which knowledge will I require to successfully complete this assignment?
    3. What should I start learning at first?

By using this top down approach I studied what GNSS is and all of the parameters included in the particular gnss.csv file and learned what realistic values are possible what do they all mean.

My replay first printed zero frames because my reader expected tabs and the CSV uses commas, so I learned to test against the real file.


