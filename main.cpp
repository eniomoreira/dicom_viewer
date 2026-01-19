#include <QApplication>
#include <QScreen>

#include "MainWindow.h"

#include <dcmtk/dcmjpeg/djdecode.h>
#include <dcmtk/dcmjpeg/djencode.h>
#include <dcmtk/dcmdata/dcrledrg.h>

int main(int argc, char *argv[])
{
    DJDecoderRegistration::registerCodecs();
    DJEncoderRegistration::registerCodecs();
    DcmRLEDecoderRegistration::registerCodecs();

    QApplication app(argc, argv);
    MainWindow w;

    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        w.setGeometry(screenGeometry);
    }

    w.show();

    int ret = app.exec();

    DJDecoderRegistration::cleanup();
    DJEncoderRegistration::cleanup();
    DcmRLEDecoderRegistration::cleanup();

    return ret;
}
