#pragma once

#include <cmath>

/// Coordinates with both latitude and longitude within this tolerance of 0
/// are treated as invalid "null island" defaults from uninitialised GPS hardware.
/// 1e-5 degrees ≈ 1.1 metres at the equator (per coordinate).
static constexpr double NullIslandEpsilon = 1e-5;

/// Returns true when \a latitude and \a longitude represent a plausible GPS fix.
/// Rejects out-of-range values and the (0, 0) "null island" default.
inline bool isValidCoordinate(double latitude, double longitude)
{
    if (latitude < -90.0 || latitude > 90.0)
        return false;
    if (longitude < -180.0 || longitude > 180.0)
        return false;
    if (std::abs(latitude) <= NullIslandEpsilon && std::abs(longitude) <= NullIslandEpsilon)
        return false;
    return true;
}

/// \overload for float coordinates — delegates to the double overload.
/// float→double promotion is exact for all values in [-180, 180].
inline bool isValidCoordinate(float latitude, float longitude)
{
    return isValidCoordinate(static_cast<double>(latitude),
                             static_cast<double>(longitude));
}
