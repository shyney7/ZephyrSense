#pragma once

/// Returns true when \a latitude and \a longitude represent a plausible GPS fix.
/// Rejects out-of-range values and the (0, 0) "null island" default.
inline bool isValidCoordinate(float latitude, float longitude)
{
    if (latitude < -90.0f || latitude > 90.0f)
        return false;
    if (longitude < -180.0f || longitude > 180.0f)
        return false;
    if (latitude == 0.0f && longitude == 0.0f)
        return false;
    return true;
}

