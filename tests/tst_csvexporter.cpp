#include <QTest>
#include <QSignalSpy>
#include <QUrl>
#include "csvexporter.h"
#include "sensorreading.h"

class TestCsvExporter : public QObject
{
    Q_OBJECT

private slots:
    void defaultState()
    {
        CsvExporter exporter;
        QCOMPARE(exporter.isEnabled(), false);
        QCOMPARE(exporter.filePath(), QString());
    }

    void setEnabled_emitsSignal()
    {
        CsvExporter exporter;
        QSignalSpy spy(&exporter, &CsvExporter::enabledChanged);
        exporter.setEnabled(true);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(exporter.isEnabled(), true);
    }

    void setEnabled_sameValue_noSignal()
    {
        CsvExporter exporter;
        // Default is false, set false again
        QSignalSpy spy(&exporter, &CsvExporter::enabledChanged);
        exporter.setEnabled(false);
        QCOMPARE(spy.count(), 0);
    }

    void setFilePath_emitsSignal()
    {
        CsvExporter exporter;
        QSignalSpy spy(&exporter, &CsvExporter::filePathChanged);
        exporter.setFilePath(QStringLiteral("/tmp/test.csv"));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(exporter.filePath(), QStringLiteral("/tmp/test.csv"));
    }

    void setFilePath_sameValue_noSignal()
    {
        CsvExporter exporter;
        exporter.setFilePath(QStringLiteral("/tmp/test.csv"));
        QSignalSpy spy(&exporter, &CsvExporter::filePathChanged);
        exporter.setFilePath(QStringLiteral("/tmp/test.csv"));
        QCOMPARE(spy.count(), 0);
    }

    void setFilePathFromUrl_convertsCorrectly()
    {
        CsvExporter exporter;
        QSignalSpy spy(&exporter, &CsvExporter::filePathChanged);
        QUrl url = QUrl::fromLocalFile(QStringLiteral("C:/Users/test/data.csv"));
        exporter.setFilePathFromUrl(url);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(exporter.filePath(), url.toLocalFile());
    }

    void appendReading_deprecated_noop()
    {
        CsvExporter exporter;
        QSignalSpy errorSpy(&exporter, &CsvExporter::exportError);
        SensorReading reading;
        reading.partectorNumber = 42;
        // Should not crash or emit errors
        exporter.appendReading(reading);
        QCOMPARE(errorSpy.count(), 0);
    }
};

QTEST_GUILESS_MAIN(TestCsvExporter)
#include "tst_csvexporter.moc"
