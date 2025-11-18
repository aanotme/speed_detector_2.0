#include <cmath>
#include <emscripten/emscripten.h>

// Haversine distance between two GPS points in meters
static double haversine(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371000.0; // Earth radius in meters
    const double toRad = M_PI / 180.0;

    double dLat = (lat2 - lat1) * toRad;
    double dLon = (lon2 - lon1) * toRad;

    double a = std::sin(dLat / 2.0) * std::sin(dLat / 2.0) +
               std::cos(lat1 * toRad) * std::cos(lat2 * toRad) *
               std::sin(dLon / 2.0) * std::sin(dLon / 2.0);

    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    return R * c;
}

// Simple state so we can feed points one by one
static bool hasPrev = false;
static double prevLat = 0.0;
static double prevLon = 0.0;
static double prevTime = 0.0;

// Reset state from JS if needed
extern "C" {
EMSCRIPTEN_KEEPALIVE
void reset() {
    hasPrev = false;
    prevLat = prevLon = prevTime = 0.0;
}

/**
 * Add a GPS sample.
 * @param lat latitude in degrees
 * @param lon longitude in degrees
 * @param timestamp timestamp in seconds (e.g. performance.now()/1000)
 * @return speed in m/s (returns -1 if no previous point)
 */
EMSCRIPTEN_KEEPALIVE
double add_sample(double lat, double lon, double timestamp) {
    if (!hasPrev) {
        hasPrev = true;
        prevLat = lat;
        prevLon = lon;
        prevTime = timestamp;
        return -1.0; // not enough data yet
    }

    double dt = timestamp - prevTime;
    if (dt <= 0.0) {
        // avoid division by zero or negative time
        return -1.0;
    }

    double dist = haversine(prevLat, prevLon, lat, lon); // meters
    double speed = dist / dt; // m/s

    prevLat = lat;
    prevLon = lon;
    prevTime = timestamp;

    return speed;
}

/**
 * Convenience: convert m/s to km/h
 */
EMSCRIPTEN_KEEPALIVE
double ms_to_kmh(double ms) {
    return ms * 3.6;
}
}