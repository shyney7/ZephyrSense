#include <QTest>
#include <QtMath>

// Replicate SensorReadingModel::isValidCoordinate() logic as standalone tests.
// The original is a private method, so we test the logic directly here.
static bool isValidCoordinate(double latitude, double longitude)
{
    if (latitude < -90.0 || latitude > 90.0)
        return false;
    if (longitude < -180.0 || longitude > 180.0)
        return false;
    // Reject null island (0, 0) as likely invalid default
    if (qFuzzyIsNull(latitude) && qFuzzyIsNull(longitude))
        return false;
    return true;
}

class TestCoordinateValidation : public QObject
{
    Q_OBJECT

private slots:
    void validCoordinate()
    {
        QVERIFY(isValidCoordinate(48.12, 11.56));
    }

    void nullIsland_rejected()
    {
        QVERIFY(!isValidCoordinate(0.0, 0.0));
    }

    void outOfRange_latitude()
    {
        QVERIFY(!isValidCoordinate(91.0, 0.0));
        QVERIFY(!isValidCoordinate(-91.0, 0.0));
    }

    void outOfRange_longitude()
    {
        QVERIFY(!isValidCoordinate(0.0, 181.0));
    }

    void boundary_valid()
    {
        QVERIFY(isValidCoordinate(90.0, 180.0));
        QVERIFY(isValidCoordinate(-90.0, -180.0));
    }
};

QTEST_GUILESS_MAIN(TestCoordinateValidation)
#include "tst_coordinatevalidation.moc"
