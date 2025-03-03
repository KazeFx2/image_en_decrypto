#pragma once

#include <QObject>
#include <QQmlApplicationEngine>

#include "singleton.h"
#include "stdafx.h"

#define APP_NAME "ImageCrypto"

#define PROJECT_NAME "ImageEn_DecryptoUI"

#define QML_URL "main"

class AppInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY_AUTO(QString, version)

    Q_PROPERTY_AUTO(QString, name)

private:
    explicit AppInfo(QObject* parent = nullptr);

public:
    SINGLETON(AppInfo)
    [[maybe_unused]] Q_INVOKABLE void testCrash();
};
