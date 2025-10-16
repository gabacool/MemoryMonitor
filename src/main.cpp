#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set application metadata
    QCoreApplication::setOrganizationName("MemoryMonitor");
    QCoreApplication::setApplicationName("Memory Monitor");
    QCoreApplication::setApplicationVersion("1.0.0");

    // Create and show main window
    MainWindow window;
    window.show();

    return app.exec();
}
