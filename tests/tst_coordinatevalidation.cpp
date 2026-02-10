#include <QTest>
#include "coordinatevalidator.h"

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
