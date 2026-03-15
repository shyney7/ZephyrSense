#pragma once

#include <QtGlobal>

/// Returns true when \a latitude and \a longitude represent a plausible GPS fix.
/// Rejects out-of-range values and the (0, 0) "null island" default.
inline bool isValidCoordinate(float latitude, float longitude)
{
    if (latitude < -90.0f || latitude > 90.0f)
        return false;
    if (longitude < -180.0f || longitude > 180.0f)
        return false;
    if (qFuzzyIsNull(latitude) && qFuzzyIsNull(longitude))
        return false;
    return true;
}

/// \overload for double-precision coordinates (e.g. from database queries).
inline bool isValidCoordinate(double latitude, double longitude)
{
    if (latitude < -90.0 || latitude > 90.0)
        return false;
    if (longitude < -180.0 || longitude > 180.0)
        return false;
    if (qFuzzyIsNull(latitude) && qFuzzyIsNull(longitude))
        return false;
    return true;
}
