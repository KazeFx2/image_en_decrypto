#include <QtQml/qqmlextensionplugin.h>

#include <QApplication>
#include <QDir>
#include <QLoggingCategory>
#include <QProcess>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>

#ifdef FLUENTUI_BUILD_STATIC_LIB
#  if (QT_VERSION > QT_VERSION_CHECK(6, 2, 0))
Q_IMPORT_QML_PLUGIN(FluentUIPlugin)
#  endif
#  include <FluentUI.h>
#endif

#include "AppInfo.h"
#include "Version.h"
#include "helper/Log.h"
#include "component/CircularReveal.h"
#include "component/FpsItem.h"
#include "component/KeyKeeper.h"
#include "helper/SettingsHelper.h"
#include "helper/TranslateHelper.h"

#ifdef WIN32
#  include "app_dmp.h"
#endif

#include "AppInfo.h"
#include "component/Crypto.h"
#include "component/ImageProvider.h"
#include "component/VideoProvider.h"
#include "component/AudioProvider.h"

int main(int argc, char* argv[])
{
    initRandom();
#ifdef WIN32
    ::SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
    qputenv("QT_QPA_PLATFORM", "windows:darkmode=2");
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
#else
    qputenv("QT_QUICK_CONTROLS_STYLE", "Default");
#endif
#ifdef Q_OS_LINUX
    // fix bug UOSv20 does not print logs
    qputenv("QT_LOGGING_RULES", "");
    // fix bug UOSv20 v-sync does not work
    qputenv("QSG_RENDER_LOOP", "basic");
    qputenv("QT_PLUGIN_PATH", "../plugins");
#endif
    QApplication::setOrganizationName("KazeFx");
    QApplication::setOrganizationDomain("https://kazefx.top");
    QApplication::setApplicationName(APP_NAME);
    QApplication::setApplicationDisplayName(APP_NAME);
    QApplication::setApplicationVersion(APPLICATION_VERSION);
    QApplication::setQuitOnLastWindowClosed(false);
    SettingsHelper::getInstance()->init(argv);
    Log::setup(argv, APP_NAME);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
#endif
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#  if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#  endif
#endif
    QApplication app(argc, argv);
    //@uri example
    qmlRegisterType<CircularReveal>(QML_URL, 1, 0, "CircularReveal");
    qmlRegisterType<FpsItem>(QML_URL, 1, 0, "FpsItem");

    QQmlApplicationEngine engine;
#ifdef __linux__
    engine.addImportPath("../qml");
#endif
    TranslateHelper::getInstance()->init(&engine);
    engine.rootContext()->setContextProperty("AppInfo", AppInfo::getInstance());
    engine.rootContext()->setContextProperty("SettingsHelper", SettingsHelper::getInstance());
    engine.rootContext()->setContextProperty("TranslateHelper", TranslateHelper::getInstance());
    engine.rootContext()->setContextProperty("KeyKeeper", KeyKeeper::getInstance());
    engine.rootContext()->setContextProperty("Crypto", Crypto::getInstance());
    engine.rootContext()->setContextProperty("AudioProvider", Audio::getInstance());
    memImage = new MemoryImage();
    engine.addImageProvider("MemoryImage", memImage);
    engine.rootContext()->setContextProperty("MemImage", memImage);
    VideoProvider = new Video();
    engine.addImageProvider("Video", VideoProvider);
    engine.rootContext()->setContextProperty("VideoProvider", VideoProvider);
    qmlRegisterUncreatableMetaObject(Video::staticMetaObject, QML_URL, 1, 0, "DecodeType", "");
#ifdef FLUENTUI_BUILD_STATIC_LIB
    FluentUI::getInstance()->registerTypes(&engine);
#endif
    const QUrl url(QStringLiteral("qrc:/main/qml/App.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject* obj, const QUrl& objUrl)
        {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);
    const int exec = QApplication::exec();
    if (exec == 931)
    {
        QProcess::startDetached(qApp->applicationFilePath(), qApp->arguments());
    }
    return exec;
}
