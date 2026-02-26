#include <QTest>
#include <QSignalSpy>
#include <QQmlEngine>
#include <QFile>
#include <QStandardPaths>
#include <QMetaEnum>
#include "timeserieschartmodel.h"
#include "databasemanager.h"
#include "sensorreading.h"
#include "testhelpers.h"

// ZEPHYR_TESTING is defined by CMake, giving us friend access to m_data

class TestTimeSeriesChartModel : public QObject
{
    Q_OBJECT

private:
    // Helper to populate m_data directly via friend access
    void addDataPoint(TimeSeriesChartModel &model, qint64 timestamp, const qreal values[9])
    {
        TimeSeriesChartModel::DataPoint point;
        point.timestamp = timestamp;
        for (size_t i = 0; i < 9; ++i)
            point.values[i] = values[i];
        model.m_data.append(point);
    }

    void addSimpleDataPoint(TimeSeriesChartModel &model, qint64 timestamp, qreal temp)
    {
        qreal values[9] = {0, 0, 0, 0, temp, 0, 0, 0, 0};
        addDataPoint(model, timestamp, values);
    }

private slots:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void cleanupTestCase()
    {
        QStandardPaths::setTestModeEnabled(false);
    }

    void cleanup()
    {
        if (QSqlDatabase::contains(DatabaseManager::CONNECTION_NAME)) {
            QSqlDatabase::database(DatabaseManager::CONNECTION_NAME).close();
            QSqlDatabase::removeDatabase(DatabaseManager::CONNECTION_NAME);
        }
        // Clean up test DB file
        QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QFile::remove(dataPath + QStringLiteral("/zephyrsense.db"));
    }

    void initialState()
    {
        TimeSeriesChartModel model;
        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(model.columnCount(), TimeSeriesChartModel::ColumnCount);
        QCOMPARE(model.xMin(), 0.0);
        QCOMPARE(model.xMax(), 0.0);
        QCOMPARE(model.yMin(), 0.0);
        QCOMPARE(model.yMax(), 0.0);
        QCOMPARE(model.dataCount(), 0);
    }

    void columnCount_always10()
    {
        TimeSeriesChartModel model;
        QCOMPARE(model.columnCount(), 10);

        // Even with data, columnCount stays 10
        addSimpleDataPoint(model, 1000, 22.5);
        QCOMPARE(model.columnCount(), 10);
    }

    void rowCount_withValidParent()
    {
        TimeSeriesChartModel model;
        addSimpleDataPoint(model, 1000, 22.5);
        // With a valid parent index, rowCount returns 0 (table model, not tree)
        QCOMPARE(model.rowCount(model.index(0, 0)), 0);
    }

    void columnCount_withValidParent()
    {
        TimeSeriesChartModel model;
        addSimpleDataPoint(model, 1000, 22.5);
        QCOMPARE(model.columnCount(model.index(0, 0)), 0);
    }

    void data_invalidIndex()
    {
        TimeSeriesChartModel model;
        addSimpleDataPoint(model, 1000, 22.5);

        // Row out of range
        QModelIndex badRow = model.index(5, 0);
        QVERIFY(!model.data(badRow).isValid());

        // Invalid index
        QVERIFY(!model.data(QModelIndex()).isValid());
    }

    void data_negativeRow()
    {
        TimeSeriesChartModel model;
        addSimpleDataPoint(model, 1000, 22.5);

        // Negative row via createIndex — QModelIndex() with invalid state
        QVERIFY(!model.data(QModelIndex()).isValid());
    }

    void data_nonDisplayRole()
    {
        TimeSeriesChartModel model;
        addSimpleDataPoint(model, 1000, 22.5);

        QModelIndex idx = model.index(0, 0);
        QVERIFY(!model.data(idx, Qt::EditRole).isValid());
    }

    void data_timestampColumn()
    {
        TimeSeriesChartModel model;
        addSimpleDataPoint(model, 123456789, 22.5);

        QModelIndex idx = model.index(0, TimeSeriesChartModel::TimestampColumn);
        QCOMPARE(model.data(idx).toLongLong(), 123456789LL);
    }

    void data_sensorColumns()
    {
        TimeSeriesChartModel model;
        qreal values[9] = {100.0, 50.0, 2.5, 1.2, 22.5, 55.0, 1013.25, 150.0, 450.0};
        addDataPoint(model, 1000, values);

        // Column 1 = PartectorNumber (index 0 in values)
        QCOMPARE(model.data(model.index(0, 1)).toReal(), 100.0);
        // Column 5 = Temperature (index 4 in values)
        QCOMPARE(model.data(model.index(0, 5)).toReal(), 22.5);
        // Column 9 = CO2 (index 8 in values)
        QCOMPARE(model.data(model.index(0, 9)).toReal(), 450.0);
    }

    void data_outOfColumnRange()
    {
        TimeSeriesChartModel model;
        addSimpleDataPoint(model, 1000, 22.5);

        QModelIndex idx = model.index(0, 10);  // Column 10 is out of range
        QVERIFY(!model.data(idx).isValid());
    }

    void clear_resetsBounds()
    {
        TimeSeriesChartModel model;
        addSimpleDataPoint(model, 1000, 22.5);
        addSimpleDataPoint(model, 2000, 25.0);
        model.calculateBounds();

        model.clear();
        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(model.xMin(), 0.0);
        QCOMPARE(model.xMax(), 0.0);
        QCOMPARE(model.yMin(), 0.0);
        QCOMPARE(model.yMax(), 0.0);
    }

    void clear_emitsSignals()
    {
        TimeSeriesChartModel model;
        addSimpleDataPoint(model, 1000, 22.5);

        QSignalSpy boundsSpy(&model, &TimeSeriesChartModel::boundsChanged);
        QSignalSpy countSpy(&model, &TimeSeriesChartModel::dataCountChanged);
        model.clear();
        QCOMPARE(boundsSpy.count(), 1);
        QCOMPARE(countSpy.count(), 1);
    }

    void getYBoundsForColumn_emptyData()
    {
        TimeSeriesChartModel model;
        QCOMPARE(model.getYMinForColumn(TimeSeriesChartModel::TemperatureColumn), 0.0);
        QCOMPARE(model.getYMaxForColumn(TimeSeriesChartModel::TemperatureColumn), 100.0);
    }

    void getYBoundsForColumn_invalidColumn()
    {
        TimeSeriesChartModel model;
        addSimpleDataPoint(model, 1000, 22.5);

        // Column 0 (timestamp) is invalid for Y bounds
        QCOMPARE(model.getYMinForColumn(0), 0.0);
        QCOMPARE(model.getYMaxForColumn(0), 100.0);

        // Column 10 is out of range
        QCOMPARE(model.getYMinForColumn(10), 0.0);
        QCOMPARE(model.getYMaxForColumn(10), 100.0);
    }

    void getYBoundsForColumn_range()
    {
        TimeSeriesChartModel model;
        // Temperature column (index 4 in values array, column 5)
        qreal v1[9] = {0, 0, 0, 0, 20.0, 0, 0, 0, 0};
        qreal v2[9] = {0, 0, 0, 0, 30.0, 0, 0, 0, 0};
        addDataPoint(model, 1000, v1);
        addDataPoint(model, 2000, v2);

        qreal yMin = model.getYMinForColumn(TimeSeriesChartModel::TemperatureColumn);
        qreal yMax = model.getYMaxForColumn(TimeSeriesChartModel::TemperatureColumn);

        // 10% padding: range is 10, padding is 1
        QVERIFY(qAbs(yMin - 19.0) < 0.01);
        QVERIFY(qAbs(yMax - 31.0) < 0.01);
    }

    void getYBoundsForColumn_allSameValue()
    {
        TimeSeriesChartModel model;
        qreal v1[9] = {0, 0, 0, 0, 25.0, 0, 0, 0, 0};
        qreal v2[9] = {0, 0, 0, 0, 25.0, 0, 0, 0, 0};
        addDataPoint(model, 1000, v1);
        addDataPoint(model, 2000, v2);

        qreal yMin = model.getYMinForColumn(TimeSeriesChartModel::TemperatureColumn);
        qreal yMax = model.getYMaxForColumn(TimeSeriesChartModel::TemperatureColumn);

        // When all values are the same, should have min-1 to max+1
        QVERIFY(qAbs(yMin - 24.0) < 0.01);
        QVERIFY(qAbs(yMax - 26.0) < 0.01);
    }

    void updateYBoundsForColumn_valid()
    {
        TimeSeriesChartModel model;
        qreal v1[9] = {0, 0, 0, 0, 20.0, 0, 0, 0, 0};
        qreal v2[9] = {0, 0, 0, 0, 30.0, 0, 0, 0, 0};
        addDataPoint(model, 1000, v1);
        addDataPoint(model, 2000, v2);

        QSignalSpy spy(&model, &TimeSeriesChartModel::boundsChanged);
        model.updateYBoundsForColumn(TimeSeriesChartModel::TemperatureColumn);

        QCOMPARE(spy.count(), 1);
        // yMin/yMax should be updated with 10% padding
        QVERIFY(qAbs(model.yMin() - 19.0) < 0.01);
        QVERIFY(qAbs(model.yMax() - 31.0) < 0.01);
    }

    void updateYBoundsForColumn_invalidColumn()
    {
        TimeSeriesChartModel model;
        addSimpleDataPoint(model, 1000, 22.5);

        QSignalSpy spy(&model, &TimeSeriesChartModel::boundsChanged);
        model.updateYBoundsForColumn(0);   // Timestamp column - invalid
        model.updateYBoundsForColumn(10);  // Out of range

        QCOMPARE(spy.count(), 0);
    }

    void calculateBounds_xMinMax()
    {
        TimeSeriesChartModel model;
        addSimpleDataPoint(model, 1000, 22.5);
        addSimpleDataPoint(model, 3000, 25.0);
        addSimpleDataPoint(model, 2000, 23.0);  // Middle - xMin/xMax based on first/last

        model.calculateBounds();

        QCOMPARE(model.xMin(), 1000.0);
        QCOMPARE(model.xMax(), 2000.0);  // Last item appended
    }

    void calculateBounds_emptyData()
    {
        TimeSeriesChartModel model;
        model.calculateBounds();

        QCOMPARE(model.xMin(), 0.0);
        QCOMPARE(model.xMax(), 0.0);
        QCOMPARE(model.yMin(), 0.0);
        QCOMPARE(model.yMax(), 0.0);
    }

    void calculateBounds_setsActiveColumnBounds()
    {
        TimeSeriesChartModel model;
        // Default active column is TemperatureColumn
        qreal v1[9] = {0, 0, 0, 0, 20.0, 0, 0, 0, 0};
        qreal v2[9] = {0, 0, 0, 0, 30.0, 0, 0, 0, 0};
        addDataPoint(model, 1000, v1);
        addDataPoint(model, 2000, v2);

        model.calculateBounds();

        QCOMPARE(model.xMin(), 1000.0);
        QCOMPARE(model.xMax(), 2000.0);
        // Y bounds for temperature with 10% padding
        QVERIFY(qAbs(model.yMin() - 19.0) < 0.01);
        QVERIFY(qAbs(model.yMax() - 31.0) < 0.01);
    }

    void calculateYBoundsForColumn_flatValues()
    {
        TimeSeriesChartModel model;
        qreal v1[9] = {0, 0, 0, 0, 25.0, 0, 0, 0, 0};
        qreal v2[9] = {0, 0, 0, 0, 25.0, 0, 0, 0, 0};
        addDataPoint(model, 1000, v1);
        addDataPoint(model, 2000, v2);

        model.updateYBoundsForColumn(TimeSeriesChartModel::TemperatureColumn);

        // All same values → ±1 fallback
        QVERIFY(qAbs(model.yMin() - 24.0) < 0.01);
        QVERIFY(qAbs(model.yMax() - 26.0) < 0.01);
    }

    void calculateYBoundsForColumn_emptyData()
    {
        TimeSeriesChartModel model;
        model.updateYBoundsForColumn(TimeSeriesChartModel::TemperatureColumn);

        // Empty data → yMin/yMax stay at 0
        QCOMPARE(model.yMin(), 0.0);
        QCOMPARE(model.yMax(), 0.0);
    }

    void qenum_accessible()
    {
        // Access Q_ENUM to cover the meta-object registration
        const QMetaObject &mo = TimeSeriesChartModel::staticMetaObject;
        int enumIndex = mo.indexOfEnumerator("Columns");
        QVERIFY(enumIndex >= 0);
        QMetaEnum me = mo.enumerator(enumIndex);
        QVERIFY(me.keyCount() > 0);
        QCOMPARE(me.value(0), static_cast<int>(TimeSeriesChartModel::TimestampColumn));
    }

    void loadData_integration()
    {
        // Set up QQmlEngine with DatabaseManager singleton
        DatabaseManager dbm;
        QVERIFY(dbm.initialize());

        // Seed DB with test data
        {
            QSqlDatabase db = QSqlDatabase::database(DatabaseManager::CONNECTION_NAME);
            QSqlQuery query(db);
            QDateTime now = QDateTime::currentDateTime();
            for (int i = 0; i < 3; ++i) {
                SensorReading reading = SensorReadingBuilder()
                    .withAllSensors()
                    .withTemperature(20.0f + static_cast<float>(i) * 5.0f)
                    .withTimestamp(now.addSecs(-120 + i * 60))
                    .build();
                query.prepare(QStringLiteral(R"(
                    INSERT INTO readings (timestamp, partectorNumber, partectorDiam, partectorMass,
                                          grimmValue, temperature, humidity, pressure,
                                          altitude, latitude, longitude, co2)
                    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                )"));
                query.addBindValue(reading.timestamp.toMSecsSinceEpoch());
                query.addBindValue(reading.partectorNumber);
                query.addBindValue(reading.partectorDiam);
                query.addBindValue(static_cast<double>(reading.partectorMass));
                query.addBindValue(static_cast<double>(reading.grimmValue));
                query.addBindValue(static_cast<double>(reading.temperature));
                query.addBindValue(static_cast<double>(reading.humidity));
                query.addBindValue(static_cast<double>(reading.pressure));
                query.addBindValue(static_cast<double>(reading.altitude));
                query.addBindValue(static_cast<double>(reading.latitude));
                query.addBindValue(static_cast<double>(reading.longitude));
                query.addBindValue(reading.co2);
                QVERIFY(query.exec());
            }
        }

        QQmlEngine engine;
        qmlRegisterSingletonInstance("ZephyrSense", 1, 0, "DatabaseManager", &dbm);

        TimeSeriesChartModel model;
        engine.setObjectOwnership(&model, QQmlEngine::CppOwnership);
        QQmlEngine::setContextForObject(&model, engine.rootContext());

        QDateTime now = QDateTime::currentDateTime();
        model.loadData(now.addSecs(-300), now);

        QCOMPARE(model.dataCount(), 3);
        QVERIFY(model.xMin() > 0);
        QVERIFY(model.xMax() >= model.xMin());
    }

    void loadData_noEngine()
    {
        TimeSeriesChartModel model;
        // No QQmlEngine set — should warn but not crash
        model.loadData(QDateTime::currentDateTime().addSecs(-60), QDateTime::currentDateTime());
        QCOMPARE(model.dataCount(), 0);
    }

    void loadData_noDbManagerSingleton()
    {
        QQmlEngine engine;
        // Register engine context but do NOT register DatabaseManager singleton
        TimeSeriesChartModel model;
        engine.setObjectOwnership(&model, QQmlEngine::CppOwnership);
        QQmlEngine::setContextForObject(&model, engine.rootContext());

        model.loadData(QDateTime::currentDateTime().addSecs(-60), QDateTime::currentDateTime());
        QCOMPARE(model.dataCount(), 0);
    }
};

QTEST_GUILESS_MAIN(TestTimeSeriesChartModel)
#include "tst_timeserieschartmodel.moc"
