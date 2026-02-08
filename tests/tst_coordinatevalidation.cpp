#include <QTest>

// Mirror SensorReadingModel::isValidCoordinate() logic for standalone tests.
// The original is a private method, so we keep this in sync with its behavior.
static bool isValidCoordinate(float latitude, float longitude)
{
    if (latitude < -90.0f || latitude > 90.0f)
        return false;
    if (longitude < -180.0f || longitude > 180.0f)
        return false;
    // Reject null island (0, 0) as likely invalid default (exact float comparison)
    if (latitude == 0.0f && longitude == 0.0f)
        return false;
    return true;
}

class TestCoordinateValidation : public QObject
{
    Q_OBJECT

private slots:
    void validCoordinate()
    {
        QVERIFY(isValidCoordinate(48.12f, 11.56f));
    }

    void nullIsland_rejected()
    {
        QVERIFY(!isValidCoordinate(0.0f, 0.0f));
    }

    void outOfRange_latitude()
    {
        QVERIFY(!isValidCoordinate(91.0f, 0.0f));
        QVERIFY(!isValidCoordinate(-91.0f, 0.0f));
    }

    void outOfRange_longitude()
    {
        QVERIFY(!isValidCoordinate(0.0f, 181.0f));
    }

    void boundary_valid()
    {
        QVERIFY(isValidCoordinate(90.0f, 180.0f));
        QVERIFY(isValidCoordinate(-90.0f, -180.0f));
    }
};

QTEST_GUILESS_MAIN(TestCoordinateValidation)
#include "tst_coordinatevalidation.moc"
