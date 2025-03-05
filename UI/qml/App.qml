import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import FluentUI 1.0
import "global"

FluLauncher {
    id: app
    // connection: solt<->emit
    Connections {
        target: FluTheme

        function onDarkModeChanged() {
            SettingsHelper.saveDarkMode(FluTheme.darkMode)
        }
    }
    Connections {
        target: FluApp

        function onUseSystemAppBarChanged() {
            SettingsHelper.saveUseSystemAppBar(FluApp.useSystemAppBar)
        }
    }
    Connections {
        target: TranslateHelper

        function onCurrentChanged() {
            SettingsHelper.saveLanguage(TranslateHelper.current)
        }
    }
    Connections {
        target: FluTheme

        function onAccentColorChanged() {
            SettingsHelper.saveAccentColorNormal(FluTheme.accentColor.normal)
            SettingsHelper.saveAccentColorDark(FluTheme.accentColor.dark)
            SettingsHelper.saveAccentColorDarker(FluTheme.accentColor.darker)
            SettingsHelper.saveAccentColorDarkest(FluTheme.accentColor.darkest)
            SettingsHelper.saveAccentColorLight(FluTheme.accentColor.light)
            SettingsHelper.saveAccentColorLighter(FluTheme.accentColor.lighter)
            SettingsHelper.saveAccentColorLightest(FluTheme.accentColor.lightest)
        }
    }
    Connections {
        target: FluTheme

        function onBlurBehindWindowEnabledChanged() {
            SettingsHelper.saveBlurWindow(FluTheme.blurBehindWindowEnabled)
        }
    }
    Connections {
        target: FluTheme

        function onNativeTextChanged() {
            SettingsHelper.saveNativeText(FluTheme.nativeText)
        }
    }
    Connections {
        target: FluTheme

        function onAnimationEnabledChanged() {
            SettingsHelper.saveAnimation(FluTheme.animationEnabled)
        }
    }
    Component.onCompleted: {
        FluApp.init(app, Qt.locale(TranslateHelper.current))
        FluApp.windowIcon = "qrc:/main/res/image/favicon.ico"
        FluApp.useSystemAppBar = SettingsHelper.getUseSystemAppBar()
        FluTheme.darkMode = SettingsHelper.getDarkMode()
        let tmp = FluColors.createAccentColor(SettingsHelper.getAccentColorNormal())
        tmp.normal = SettingsHelper.getAccentColorNormal()
        tmp.dark = SettingsHelper.getAccentColorDark()
        tmp.darker = SettingsHelper.getAccentColorDarker()
        tmp.darkest = SettingsHelper.getAccentColorDarkest()
        tmp.light = SettingsHelper.getAccentColorLight()
        tmp.lighter = SettingsHelper.getAccentColorLighter()
        tmp.lightest = SettingsHelper.getAccentColorLightest()
        FluTheme.accentColor = tmp
        FluTheme.blurBehindWindowEnabled = SettingsHelper.getBlurWindow()
        FluTheme.nativeText = SettingsHelper.getNativeText()
        FluTheme.animationEnabled = SettingsHelper.getAnimation()
        FluRouter.routes = {
            "/": "qrc:/main/qml/window/MainWindow.qml"
        }
        var args = Qt.application.arguments
        if (args.length >= 2 && args[1].startsWith("-crashed=")) {
            FluRouter.navigate("/")
        } else {
            FluRouter.navigate("/")
        }
    }
}
