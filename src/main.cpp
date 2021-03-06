#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QTextStream>

// global environment
#include "global.h"
GlobalEnvironment gEnv;
#include "deviceconfig.h"

// Get the default Qt message handler.
static const QtMessageHandler QT_DEFAULT_MESSAGE_HANDLER = qInstallMessageHandler(0);

// define QT_NO_DEBUG_OUTPUT - no debug info
void CustomMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // mutex?
    if (gEnv.pDebugWindow != nullptr) {
        // для мультипотока, хз правильно ли, но работает // не уверен насчёт ссылки, мб надо копию передавать с мультипотоком
        QMetaObject::invokeMethod(gEnv.pDebugWindow, "printMsg", Qt::QueuedConnection, Q_ARG(const QString, msg));
    }

    // Call the default handler.
    (*QT_DEFAULT_MESSAGE_HANDLER)(type, context, msg);
}

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    //qputenv("QT_SCALE_FACTOR", "0.8");

    QElapsedTimer time;
    time.start();

    QApplication a(argc, argv);

    // global
    QSettings app_settings("FreeJoySettings.conf", QSettings::IniFormat);
    DeviceConfig device_config;

    gEnv.pAppSettings = &app_settings;
    gEnv.pDeviceConfig = &device_config;

    qInstallMessageHandler(CustomMessageHandler);

    gEnv.pApp_start_time = &time;

    // set font size
    gEnv.pAppSettings->beginGroup("FontSettings");
    QApplication::setFont(QFont("MS Shell Dlg 2", gEnv.pAppSettings->value("FontSize", "8").toInt()));
    gEnv.pAppSettings->endGroup();

    // set styleSheet
    gEnv.pAppSettings->beginGroup("StyleSettings");
    QString style = gEnv.pAppSettings->value("StyleSheet", "default").toString();
    gEnv.pAppSettings->endGroup();

    MainWindow w;

    // slightly increased startup(+30-50ms) but
    // w.setStyleSheet(ts.readAll());  --  x7 faster style switch. details in advancedsettings.cpp AdvancedSettings::SetStyle()
    if (style == "default") // ?
    {
        QFile f(":/styles/default.qss");
        if (!f.exists()) {
            qDebug() << "Unable to set stylesheet, file not found\n";
        } else {
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&f);
            w.setStyleSheet(ts.readAll());
        }
    } else if (style == "white") {
        QFile f(":qss/qss.qss");
        if (!f.exists()) {
            qDebug() << "Unable to set stylesheet, file not found\n";
        } else {
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&f);
            w.setStyleSheet(ts.readAll());
        }
    } else if (style == "dark") {
        QFile f(":qdarkstyle/style.qss");
        if (!f.exists()) {
            qDebug() << "Unable to set stylesheet, file not found\n";
        } else {
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&f);
            w.setStyleSheet(ts.readAll());
        }
    }

    qDebug() << "Application startup time =" << gEnv.pApp_start_time->elapsed() << "ms";
    w.show();

    return a.exec();
}
