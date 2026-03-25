#pragma once

#include <QLatin1StringView>

/// Single source of truth for the readings-table DDL.
/// Each caller executes these via its own QSqlQuery with its own error handling.

inline constexpr QLatin1StringView kReadingsTableSql{R"(
    CREATE TABLE IF NOT EXISTS readings (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        timestamp INTEGER NOT NULL,
        partectorNumber INTEGER,
        partectorDiam INTEGER,
        partectorMass REAL,
        grimmValue REAL,
        temperature REAL,
        humidity REAL,
        pressure REAL,
        altitude REAL,
        latitude REAL,
        longitude REAL,
        co2 INTEGER
    )
)"};

inline constexpr QLatin1StringView kReadingsIndexSql{R"(
    CREATE INDEX IF NOT EXISTS idx_timestamp ON readings(timestamp)
)"};
