#pragma once

#include <qcolordialog.h>
#include <QtCore/qobject.h>
#include <QtQml/qqml.h>
#include <QSettings>
#include <QScopedPointer>
#include <QDir>
#include "singleton.h"
#include "FluAccentColor.h"

class SettingsHelper : public QObject
{
    Q_OBJECT

private:
    explicit SettingsHelper(QObject* parent = nullptr);

public:
    SINGLETON(SettingsHelper)
    ~SettingsHelper() override;
    void init(char* argv[]);
    Q_INVOKABLE void saveDarkMode(int darkModel)
    {
        save("darkMode", darkModel);
    }

    Q_INVOKABLE int getDarkMode()
    {
        return get("darkMode", QVariant(0)).toInt();
    }

    Q_INVOKABLE void saveUseSystemAppBar(bool useSystemAppBar)
    {
        save("useSystemAppBar", useSystemAppBar);
    }

    Q_INVOKABLE bool getUseSystemAppBar()
    {
        return get("useSystemAppBar", QVariant(false)).toBool();
    }

    Q_INVOKABLE void saveLanguage(const QString& language)
    {
        save("language", language);
    }

    Q_INVOKABLE QString getLanguage()
    {
        return get("language", QVariant("en_US")).toString();
    }

    Q_INVOKABLE void saveAppBarWindows(bool appBarWindows)
    {
        save("appBarWindows", appBarWindows);
    }

    Q_INVOKABLE bool getAppBarWindows()
    {
        return get("appBarWindows", QVariant(false)).toBool();
    }

    Q_INVOKABLE void saveAccentColor(FluAccentColor* accentColor)
    {
        save("accentColorDarkest", accentColor->darkest().rgba());
        save("accentColorDarker", accentColor->darker().rgba());
        save("accentColorDark", accentColor->dark().rgba());
        save("accentColorNormal", accentColor->normal().rgba());
        save("accentColorLight", accentColor->light().rgba());
        save("accentColorLighter", accentColor->lighter().rgba());
        save("accentColorLightest", accentColor->lightest().rgba());
    }

    Q_INVOKABLE FluAccentColor* getAccentColor()
    {
        auto accentColor = new FluAccentColor(this);
        accentColor->normal(QColor::fromRgba(get("accentColorNormal", QColor(0, 120, 212).rgba()).toUInt()));
        accentColor->dark(QColor::fromRgba(get("accentColorDark", QColor(0, 102, 180).rgba()).toUInt()));
        accentColor->darker(QColor::fromRgba(get("accentColorDarker", QColor(0, 84, 148).rgba()).toUInt()));
        accentColor->darkest(QColor::fromRgba(get("accentColorDarkest", QColor(0, 74, 131).rgba()).toUInt()));
        accentColor->light(QColor::fromRgba(get("accentColorLight", QColor(38, 140, 220).rgba()).toUInt()));
        accentColor->lighter(QColor::fromRgba(get("accentColorLighter", QColor(76, 160, 224).rgba()).toUInt()));
        accentColor->lightest(QColor::fromRgba(get("accentColorLightest", QColor(96, 171, 228).rgba()).toUInt()));
        return accentColor;
    }

    Q_INVOKABLE void saveBlurWindow(bool blurWindow)
    {
        save("blurWindow", blurWindow);
    }

    Q_INVOKABLE bool getBlurWindow()
    {
        return get("blurWindow", QVariant(false)).toBool();
    }

    Q_INVOKABLE void saveWindowOpacity(double windowOpacity)
    {
        save("windowOpacity", windowOpacity);
    }

    Q_INVOKABLE double getWindowOpacity()
    {
        return get("windowOpacity", 0.5).toDouble();
    }

    Q_INVOKABLE void saveBlurLevel(int blurLevel)
    {
        save("blurLevel", blurLevel);
    }

    Q_INVOKABLE int getBlurLevel()
    {
        return get("blurLevel", QVariant(50)).toInt();
    }

    Q_INVOKABLE void saveNativeText(bool nativeText)
    {
        save("nativeText", nativeText);
    }

    Q_INVOKABLE bool getNativeText()
    {
        return get("nativeText", QVariant(false)).toBool();
    }

    Q_INVOKABLE void saveDisplayMode(int displayMode)
    {
        save("displayMode", displayMode);
    }

    Q_INVOKABLE int getDisplayMode()
    {
        return get("displayMode", QVariant(3)).toInt();
    }

    Q_INVOKABLE void saveAnimation(bool animation)
    {
        save("animation", animation);
    }

    Q_INVOKABLE bool getAnimation()
    {
        return get("animation", QVariant(true)).toBool();
    }

    Q_INVOKABLE void saveShowFPS(bool showFPS)
    {
        save("showFPS", showFPS);
    }

    Q_INVOKABLE bool getShowFPS()
    {
        return get("showFPS", QVariant(true)).toBool();
    }

private:
    void save(const QString& key, QVariant val);
    QVariant get(const QString& key, QVariant def = {});
    QScopedPointer<QSettings> m_settings;
};
