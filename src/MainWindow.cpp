#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QtCharts/QChart>
#include <QtCharts/QPieSlice>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDebug>
#include <QProcess>
#include <QTimer>
#include <QCheckBox>

// Custom QTableWidgetItem that sorts numerically using UserRole data
class NumericTableWidgetItem : public QTableWidgetItem {
public:
    bool operator<(const QTableWidgetItem &other) const override {
        return data(Qt::UserRole).toDouble() < other.data(Qt::UserRole).toDouble();
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_processTable(nullptr)
    , m_chartView(nullptr)
    , m_pieSeries(nullptr)
    , m_refreshTimer(nullptr)
    , m_monitor(nullptr)
    , m_workerThread(nullptr)
    , m_refreshInterval(5)
    , m_chartProcessCount(25)
    , m_isPaused(false)
{
    setupUI();

    // Create worker thread and monitor
    m_workerThread = new QThread(this);
    m_monitor = new SystemMonitor();
    m_monitor->moveToThread(m_workerThread);

    // Connect signals
    connect(m_workerThread, &QThread::started, m_monitor, &SystemMonitor::collectData);
    connect(m_monitor, &SystemMonitor::dataReady, this, &MainWindow::updateUI);
    connect(m_monitor, &SystemMonitor::errorOccurred, this, &MainWindow::handleError);

    // Start the worker thread
    m_workerThread->start();

    // Setup auto-refresh timer
    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, m_monitor, &SystemMonitor::collectData);
    m_refreshTimer->start(m_refreshInterval * 1000);

    // Initial data collection
    QMetaObject::invokeMethod(m_monitor, &SystemMonitor::collectData, Qt::QueuedConnection);
}

MainWindow::~MainWindow() {
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
    }
    delete m_monitor;
}

void MainWindow::setupUI() {
    setWindowTitle("Memory Monitor");
    setMinimumSize(1000, 700);

    // Set window icon (for Dock)
    setWindowIcon(QIcon(":/MemoryMonitor.icns"));

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Setup controls widget
    QWidget *controlsWidget = new QWidget(this);
    QHBoxLayout *controlsLayout = new QHBoxLayout(controlsWidget);

    // Refresh interval control
    QLabel *intervalLabel = new QLabel("Refresh Interval (seconds):", this);
    QSpinBox *intervalSpinBox = new QSpinBox(this);
    intervalSpinBox->setRange(1, 60);
    intervalSpinBox->setValue(m_refreshInterval);
    connect(intervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onRefreshIntervalChanged);

    // Manual refresh button
    QPushButton *refreshButton = new QPushButton("Refresh Now", this);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::onManualRefresh);

    // Pause/Resume button
    QPushButton *pauseButton = new QPushButton("Pause", this);
    pauseButton->setCheckable(true);
    connect(pauseButton, &QPushButton::clicked, this, &MainWindow::onPauseResume);

    // Purge inactive memory button
    QPushButton *purgeButton = new QPushButton("Purge Inactive Memory", this);
    connect(purgeButton, &QPushButton::clicked, this, &MainWindow::onPurgeMemory);

    // Always on top checkbox
    QCheckBox *alwaysOnTopCheckbox = new QCheckBox("Always on Top", this);
    connect(alwaysOnTopCheckbox, &QCheckBox::toggled, this, &MainWindow::onAlwaysOnTopChanged);

    // Auto-refresh toggle checkbox
    QCheckBox *autoRefreshCheckbox = new QCheckBox("Auto Refresh", this);
    autoRefreshCheckbox->setChecked(true);  // Default is on
    connect(autoRefreshCheckbox, &QCheckBox::toggled, this, &MainWindow::onAutoRefreshToggled);

    controlsLayout->addWidget(intervalLabel);
    controlsLayout->addWidget(intervalSpinBox);
    controlsLayout->addWidget(refreshButton);
    controlsLayout->addWidget(pauseButton);
    controlsLayout->addWidget(purgeButton);
    controlsLayout->addWidget(alwaysOnTopCheckbox);
    controlsLayout->addWidget(autoRefreshCheckbox);
    controlsLayout->addStretch();

    mainLayout->addWidget(controlsWidget);

    setupTable();

    // Table takes full width now (no pie chart)
    mainLayout->addWidget(m_processTable);

    setCentralWidget(centralWidget);
    setupMenuBar();
    setupStatusBar();
}

void MainWindow::setupTable() {
    m_processTable = new QTableWidget(this);
    m_processTable->setColumnCount(5);
    m_processTable->setHorizontalHeaderLabels(
        {"Process Name", "Path", "RAM Usage", "% of Total", "Cumulative %"});

    m_processTable->setSortingEnabled(true);
    m_processTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_processTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_processTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Adjust column widths
    m_processTable->horizontalHeader()->setStretchLastSection(false);
    m_processTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_processTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_processTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_processTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_processTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    // Connect table click signal
    connect(m_processTable, &QTableWidget::cellClicked,
            this, &MainWindow::onTableRowClicked);

    // Connect to sort indicator change to recalculate cumulative percentage AFTER sort
    connect(m_processTable->horizontalHeader(), &QHeaderView::sortIndicatorChanged,
            this, [this](int, Qt::SortOrder) {
                // Use QTimer to defer recalculation until after sort completes
                QTimer::singleShot(0, this, &MainWindow::recalculateCumulativePercentage);
            });
}

void MainWindow::setupChart() {
    // Chart removed - no longer needed
}

void MainWindow::setupControls() {
    // This function is now integrated into setupUI()
}

void MainWindow::setupMenuBar() {
    QMenuBar *menuBar = new QMenuBar(this);

    QMenu *fileMenu = menuBar->addMenu("&File");
    QAction *quitAction = fileMenu->addAction("&Quit");
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &MainWindow::close);

    QMenu *viewMenu = menuBar->addMenu("&View");
    QAction *refreshAction = viewMenu->addAction("&Refresh");
    refreshAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    connect(refreshAction, &QAction::triggered, this, &MainWindow::onManualRefresh);

    QAction *pauseAction = viewMenu->addAction("&Pause/Resume");
    pauseAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_P));
    connect(pauseAction, &QAction::triggered, this, &MainWindow::onPauseResume);

    setMenuBar(menuBar);
}

void MainWindow::setupStatusBar() {
    QStatusBar *status = new QStatusBar(this);
    setStatusBar(status);
}

void MainWindow::updateUI() {
    updateTable();
    updateStatusBar();
}

void MainWindow::updateTable() {
    if (!m_monitor) return;

    const auto& processes = m_monitor->getProcesses();
    uint64_t totalRAM = m_monitor->getTotalPhysicalRAM();

    m_processTable->setSortingEnabled(false);
    m_processTable->setRowCount(0);

    // Show ALL processes (not just top 100)
    size_t numToShow = processes.size();
    m_processTable->setRowCount(numToShow);

    for (size_t i = 0; i < numToShow; ++i) {
        const ProcessInfo& proc = processes[i];
        double percentage = proc.getMemoryPercentage(totalRAM);

        QTableWidgetItem *nameItem = new QTableWidgetItem(QString::fromStdString(proc.getName()));
        QTableWidgetItem *pathItem = new QTableWidgetItem(QString::fromStdString(proc.getPath()));
        NumericTableWidgetItem *sizeItem = new NumericTableWidgetItem();
        sizeItem->setText(formatMemorySize(proc.getResidentSize()));
        sizeItem->setData(Qt::UserRole, QVariant::fromValue(proc.getResidentSize()));

        NumericTableWidgetItem *percentItem = new NumericTableWidgetItem();
        percentItem->setText(formatPercentage(percentage));
        percentItem->setData(Qt::UserRole, QVariant::fromValue(percentage));

        QTableWidgetItem *cumulativeItem = new QTableWidgetItem("");  // Will be calculated after sort

        m_processTable->setItem(i, 0, nameItem);
        m_processTable->setItem(i, 1, pathItem);
        m_processTable->setItem(i, 2, sizeItem);
        m_processTable->setItem(i, 3, percentItem);
        m_processTable->setItem(i, 4, cumulativeItem);
    }

    m_processTable->setSortingEnabled(true);

    // Force sort using column 2 (RAM Usage) in descending order
    m_processTable->horizontalHeader()->setSortIndicator(2, Qt::DescendingOrder);

    // Calculate cumulative percentage after initial sort
    recalculateCumulativePercentage();
}

void MainWindow::recalculateCumulativePercentage() {
    if (!m_processTable) return;

    // Block signals to prevent recursive calls
    m_processTable->blockSignals(true);

    double cumulativePercent = 0.0;
    int rowCount = m_processTable->rowCount();

    for (int visualRow = 0; visualRow < rowCount; ++visualRow) {
        // Get the % of Total value from column 3
        QTableWidgetItem *percentItem = m_processTable->item(visualRow, 3);
        if (!percentItem) continue;

        // Extract percentage value (stored in UserRole)
        double percentage = percentItem->data(Qt::UserRole).toDouble();
        cumulativePercent += percentage;

        // Update cumulative column
        QTableWidgetItem *cumulativeItem = m_processTable->item(visualRow, 4);
        if (cumulativeItem) {
            cumulativeItem->setText(formatPercentage(cumulativePercent));
            cumulativeItem->setData(Qt::UserRole, QVariant::fromValue(cumulativePercent));

            // Color code cumulative percentage with better contrast
            if (cumulativePercent > 75.0) {
                cumulativeItem->setBackground(QColor(255, 180, 180));  // Light red
                cumulativeItem->setForeground(QColor(0, 0, 0));  // Black text
            } else if (cumulativePercent > 50.0) {
                cumulativeItem->setBackground(QColor(255, 255, 180));  // Light yellow
                cumulativeItem->setForeground(QColor(0, 0, 0));  // Black text
            } else if (cumulativePercent > 25.0) {
                cumulativeItem->setBackground(QColor(180, 255, 180));  // Light green
                cumulativeItem->setForeground(QColor(0, 0, 0));  // Black text
            } else {
                cumulativeItem->setBackground(QColor(240, 240, 240));  // Light gray
                cumulativeItem->setForeground(QColor(0, 0, 0));  // Black text
            }
        }
    }

    // Unblock signals
    m_processTable->blockSignals(false);
}

void MainWindow::updateChart() {
    if (!m_monitor) return;

    // Clear existing slices
    m_pieSeries->clear();

    // Get top N processes based on user setting
    auto topProcesses = m_monitor->getTopProcessesByMemory(m_chartProcessCount);
    uint64_t totalRAM = m_monitor->getTotalPhysicalRAM();
    const auto& allProcesses = m_monitor->getProcesses();

    // Calculate "others" percentage
    double topPercentageSum = 0.0;
    for (const auto& proc : topProcesses) {
        topPercentageSum += proc.getMemoryPercentage(totalRAM);
    }

    // Add slices for top processes
    for (const auto& proc : topProcesses) {
        double percentage = proc.getMemoryPercentage(totalRAM);
        QPieSlice *slice = m_pieSeries->append(
            QString::fromStdString(proc.getName()),
            percentage);
        slice->setLabelVisible(percentage > 1.0);  // Only show label if > 1%
        slice->setLabel(QString("%1: %2%")
            .arg(QString::fromStdString(proc.getName()))
            .arg(percentage, 0, 'f', 1));
    }

    // Add "Others" slice if there's remaining memory
    double othersPercentage = 100.0 - topPercentageSum;
    if (othersPercentage > 0.1) {
        int othersCount = allProcesses.size() - topProcesses.size();
        QPieSlice *othersSlice = m_pieSeries->append("Others", othersPercentage);
        othersSlice->setLabelVisible(true);

        // Create detailed tooltip for "Others"
        QString othersTooltip = QString("Others: %1% (%2 processes)\n\nTop processes in 'Others':")
            .arg(othersPercentage, 0, 'f', 1)
            .arg(othersCount);

        // Add next 10 processes to tooltip
        size_t tooltipCount = std::min(size_t(10), allProcesses.size() - topProcesses.size());
        for (size_t i = topProcesses.size(); i < topProcesses.size() + tooltipCount; ++i) {
            othersTooltip += QString("\n- %1: %2")
                .arg(QString::fromStdString(allProcesses[i].getName()))
                .arg(formatMemorySize(allProcesses[i].getResidentSize()));
        }

        if (othersCount > 10) {
            othersTooltip += QString("\n... and %1 more").arg(othersCount - 10);
        }

        othersSlice->setLabel(QString("Others: %1% (%2)")
            .arg(othersPercentage, 0, 'f', 1)
            .arg(othersCount));
    }
}

void MainWindow::updateStatusBar() {
    if (!m_monitor) return;

    uint64_t totalRAM = m_monitor->getTotalPhysicalRAM();
    uint64_t activeRAM = m_monitor->getActiveMemory();
    uint64_t wiredRAM = m_monitor->getWiredMemory();
    uint64_t inactiveRAM = m_monitor->getInactiveMemory();
    uint64_t freeRAM = m_monitor->getFreeMemory();
    uint64_t usedRAM = activeRAM + wiredRAM + inactiveRAM;

    // Calculate actual process memory sum from all processes
    uint64_t processMemSum = 0;
    for (const auto& proc : m_monitor->getProcesses()) {
        processMemSum += proc.getResidentSize();
    }

    QString statusText = QString("Total: %1 | Used: %2 | Free: %3 | Processes: %4")
        .arg(formatMemorySize(totalRAM))
        .arg(formatMemorySize(usedRAM))
        .arg(formatMemorySize(freeRAM))
        .arg(m_monitor->getProcesses().size());

    // Add detailed breakdown in second line
    QString detailText = QString("Active: %1 | Wired: %2 | Inactive: %3 | Process RAM Sum: %4")
        .arg(formatMemorySize(activeRAM))
        .arg(formatMemorySize(wiredRAM))
        .arg(formatMemorySize(inactiveRAM))
        .arg(formatMemorySize(processMemSum));

    statusBar()->showMessage(statusText + " | " + detailText);
}

void MainWindow::onTableRowClicked(int row, int column) {
    Q_UNUSED(row);
    Q_UNUSED(column);
    // No action needed - chart removed
}

void MainWindow::onPieSliceClicked(QPieSlice *slice) {
    if (!slice) return;

    QString processName = slice->label().split(':')[0].trimmed();

    // Check if "Others" was clicked
    if (processName == "Others") {
        showOthersBreakdown();
    } else {
        highlightTableRow(processName);
    }
}

void MainWindow::highlightTableRow(int row) {
    m_processTable->selectRow(row);
}

void MainWindow::highlightTableRow(const QString& processName) {
    for (int i = 0; i < m_processTable->rowCount(); ++i) {
        QTableWidgetItem *item = m_processTable->item(i, 0);
        if (item && item->text() == processName) {
            m_processTable->selectRow(i);
            m_processTable->scrollToItem(item);
            break;
        }
    }
}

void MainWindow::highlightChartSlice(const QString& processName) {
    Q_UNUSED(processName);
    // Chart removed - no action needed
}

void MainWindow::onRefreshIntervalChanged(int seconds) {
    m_refreshInterval = seconds;
    if (!m_isPaused && m_refreshTimer) {
        m_refreshTimer->setInterval(m_refreshInterval * 1000);
    }
}

void MainWindow::onChartProcessCountChanged(int count) {
    m_chartProcessCount = count;
    updateChart();  // Immediately update chart with new count
}

void MainWindow::onManualRefresh() {
    if (m_monitor) {
        QMetaObject::invokeMethod(m_monitor, &SystemMonitor::collectData, Qt::QueuedConnection);
    }
}

void MainWindow::onPauseResume() {
    m_isPaused = !m_isPaused;

    if (m_isPaused) {
        m_refreshTimer->stop();
        statusBar()->showMessage("Auto-refresh paused", 3000);
    } else {
        m_refreshTimer->start(m_refreshInterval * 1000);
        statusBar()->showMessage("Auto-refresh resumed", 3000);
    }
}

void MainWindow::handleError(const QString& error) {
    qWarning() << "Error:" << error;
    QMessageBox::warning(this, "Error", error);
}

QString MainWindow::formatMemorySize(uint64_t bytes) const {
    const double GB = 1024.0 * 1024.0 * 1024.0;
    const double MB = 1024.0 * 1024.0;

    if (bytes >= GB) {
        return QString("%1 GB").arg(bytes / GB, 0, 'f', 2);
    } else {
        return QString("%1 MB").arg(bytes / MB, 0, 'f', 1);
    }
}

QString MainWindow::formatPercentage(double percentage) const {
    return QString("%1%").arg(percentage, 0, 'f', 2);
}

void MainWindow::showOthersBreakdown() {
    if (!m_monitor) return;

    const auto& allProcesses = m_monitor->getProcesses();
    uint64_t totalRAM = m_monitor->getTotalPhysicalRAM();

    // Create dialog
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("'Others' Breakdown - All Remaining Processes");
    dialog->setMinimumSize(800, 600);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    // Add summary label
    int othersCount = allProcesses.size() - m_chartProcessCount;
    double othersMemory = 0;
    for (size_t i = m_chartProcessCount; i < allProcesses.size(); ++i) {
        othersMemory += allProcesses[i].getMemoryPercentage(totalRAM);
    }

    QLabel *summaryLabel = new QLabel(
        QString("'Others' contains %1 processes using %2% of total RAM")
            .arg(othersCount)
            .arg(othersMemory, 0, 'f', 2),
        dialog
    );
    summaryLabel->setStyleSheet("font-weight: bold; font-size: 14px; padding: 10px;");
    layout->addWidget(summaryLabel);

    // Create table to show all "Others" processes
    QTableWidget *othersTable = new QTableWidget(dialog);
    othersTable->setColumnCount(4);
    othersTable->setHorizontalHeaderLabels(
        {"Process Name", "Path", "RAM Usage", "Percentage"});
    othersTable->setSortingEnabled(true);
    othersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    othersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Adjust column widths
    othersTable->horizontalHeader()->setStretchLastSection(true);
    othersTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    othersTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    // Populate table with processes from "Others"
    othersTable->setSortingEnabled(false);
    othersTable->setRowCount(othersCount);

    for (int i = 0; i < othersCount; ++i) {
        const ProcessInfo& proc = allProcesses[m_chartProcessCount + i];

        QTableWidgetItem *nameItem = new QTableWidgetItem(QString::fromStdString(proc.getName()));
        QTableWidgetItem *pathItem = new QTableWidgetItem(QString::fromStdString(proc.getPath()));
        QTableWidgetItem *sizeItem = new QTableWidgetItem(formatMemorySize(proc.getResidentSize()));
        QTableWidgetItem *percentItem = new QTableWidgetItem(
            formatPercentage(proc.getMemoryPercentage(totalRAM)));

        // Make memory size sortable numerically
        sizeItem->setData(Qt::UserRole, QVariant::fromValue(proc.getResidentSize()));
        percentItem->setData(Qt::UserRole, QVariant::fromValue(proc.getMemoryPercentage(totalRAM)));

        othersTable->setItem(i, 0, nameItem);
        othersTable->setItem(i, 1, pathItem);
        othersTable->setItem(i, 2, sizeItem);
        othersTable->setItem(i, 3, percentItem);
    }

    othersTable->setSortingEnabled(true);
    othersTable->sortByColumn(2, Qt::DescendingOrder);

    layout->addWidget(othersTable);

    // Add close button
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, dialog);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::accept);
    layout->addWidget(buttonBox);

    dialog->exec();
    delete dialog;
}

void MainWindow::onPurgeMemory() {
    // Get inactive memory before purge
    uint64_t inactiveBefore = m_monitor->getInactiveMemory();

    // Show confirmation dialog with warning
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle("Purge Inactive Memory");
    msgBox.setText("This will run 'sudo purge' to free inactive memory.");
    msgBox.setInformativeText(QString("Current inactive memory: %1\n\nNote: This requires sudo password and may temporarily slow down your system.")
        .arg(formatMemorySize(inactiveBefore)));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);

    if (msgBox.exec() == QMessageBox::Yes) {
        // Execute purge command
        QProcess process;
        process.start("osascript", QStringList()
            << "-e"
            << "do shell script \"purge\" with administrator privileges");

        if (process.waitForFinished(30000)) {  // 30 second timeout
            if (process.exitCode() == 0) {
                statusBar()->showMessage("Memory purged successfully. Refreshing data...", 3000);

                // Wait a moment for system to update
                QTimer::singleShot(1000, this, [this]() {
                    // Refresh data
                    onManualRefresh();
                });
            } else {
                QString error = process.readAllStandardError();
                QMessageBox::warning(this, "Purge Failed",
                    QString("Failed to purge memory:\n%1").arg(error));
            }
        } else {
            QMessageBox::warning(this, "Purge Timeout",
                "The purge command timed out or was cancelled.");
        }
    }
}

void MainWindow::onAlwaysOnTopChanged(bool checked) {
    Qt::WindowFlags flags = windowFlags();
    if (checked) {
        // Add WindowStaysOnTopHint flag
        setWindowFlags(flags | Qt::WindowStaysOnTopHint);
        statusBar()->showMessage("Window will stay on top", 2000);
    } else {
        // Remove WindowStaysOnTopHint flag
        setWindowFlags(flags & ~Qt::WindowStaysOnTopHint);
        statusBar()->showMessage("Window will not stay on top", 2000);
    }
    // Show window again after changing flags (required on macOS)
    show();
}

void MainWindow::onAutoRefreshToggled(bool checked) {
    if (checked) {
        // Enable auto-refresh
        m_isPaused = false;
        m_refreshTimer->start(m_refreshInterval * 1000);
        statusBar()->showMessage("Auto-refresh enabled", 2000);
    } else {
        // Disable auto-refresh
        m_isPaused = true;
        m_refreshTimer->stop();
        statusBar()->showMessage("Auto-refresh disabled", 2000);
    }
}
