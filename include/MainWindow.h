#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QTimer>
#include <QThread>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <memory>
#include "SystemMonitor.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateUI();
    void onTableRowClicked(int row, int column);
    void onPieSliceClicked(QPieSlice *slice);
    void onRefreshIntervalChanged(int seconds);
    void onChartProcessCountChanged(int count);
    void onManualRefresh();
    void onPauseResume();
    void handleError(const QString& error);
    void recalculateCumulativePercentage();
    void onPurgeMemory();
    void onAlwaysOnTopChanged(bool checked);
    void onAutoRefreshToggled(bool checked);

private:
    // UI Components
    QTableWidget *m_processTable;
    QChartView *m_chartView;
    QPieSeries *m_pieSeries;
    QTimer *m_refreshTimer;

    // System monitoring
    SystemMonitor *m_monitor;
    QThread *m_workerThread;

    // State
    int m_refreshInterval;  // in seconds
    int m_chartProcessCount;  // number of processes to show in chart
    bool m_isPaused;

    // UI Setup
    void setupUI();
    void setupTable();
    void setupChart();
    void setupControls();
    void setupMenuBar();
    void setupStatusBar();

    // UI Updates
    void updateTable();
    void updateChart();
    void updateStatusBar();
    void highlightTableRow(int row);
    void highlightTableRow(const QString& processName);
    void highlightChartSlice(const QString& processName);
    void showOthersBreakdown();

    // Helper methods
    QString formatMemorySize(uint64_t bytes) const;
    QString formatPercentage(double percentage) const;
};

#endif // MAINWINDOW_H
