#include <QtQml/qqmlextensionplugin.h>

#include <QApplication>
#include <QDir>
#include <QLoggingCategory>
#include <QNetworkProxy>
#include <QProcess>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QSslConfiguration>

#include "AppInfo.h"
#include "Version.h"
#include "helper/Log.h"
#include "component/CircularReveal.h"
#include "component/FileWatcher.h"
#include "component/FpsItem.h"
#include "component/OpenGLItem.h"
#include "helper/InitializrHelper.h"
#include "helper/Network.h"
#include "helper/SettingsHelper.h"
#include "helper/TranslateHelper.h"

#ifdef FLUENTUI_BUILD_STATIC_LIB
#  if (QT_VERSION > QT_VERSION_CHECK(6, 2, 0))
Q_IMPORT_QML_PLUGIN(FluentUIPlugin)
#  endif
#  include <FluentUI.h>
#endif

#ifdef WIN32
#  include "app_dmp.h"
#endif

#include "AppInfo.h"

int main(int argc, char* argv[])
{
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
    qputenv("QT_PLUGIN_PATH", "plugins");
#endif
    QApplication::setOrganizationName("ZhuZiChu");
    QApplication::setOrganizationDomain("https://zhuzichu520.github.io");
    QApplication::setApplicationName(APP_NAME);
    QApplication::setApplicationDisplayName("FluentUI Example");
    QApplication::setApplicationVersion(APPLICATION_VERSION);
    QApplication::setQuitOnLastWindowClosed(false);
    SettingsHelper::getInstance()->init(argv);
    Log::setup(argv, "CryptoUI");
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
    qmlRegisterType<FileWatcher>(QML_URL, 1, 0, "FileWatcher");
    qmlRegisterType<FpsItem>(QML_URL, 1, 0, "FpsItem");
    qmlRegisterType<NetworkCallable>(QML_URL, 1, 0, "NetworkCallable");
    qmlRegisterType<NetworkParams>(QML_URL, 1, 0, "NetworkParams");
    qmlRegisterType<OpenGLItem>(QML_URL, 1, 0, "OpenGLItem");
    qmlRegisterUncreatableMetaObject(NetworkType::staticMetaObject, QML_URL, 1, 0,
                                     "NetworkType", "Access to enums & flags only");

    QQmlApplicationEngine engine;
    TranslateHelper::getInstance()->init(&engine);
    engine.rootContext()->setContextProperty("AppInfo", AppInfo::getInstance());
    engine.rootContext()->setContextProperty("SettingsHelper", SettingsHelper::getInstance());
    engine.rootContext()->setContextProperty("InitializrHelper", InitializrHelper::getInstance());
    engine.rootContext()->setContextProperty("TranslateHelper", TranslateHelper::getInstance());
    engine.rootContext()->setContextProperty("Network", Network::getInstance());
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
