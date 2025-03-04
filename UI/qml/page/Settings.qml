import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import FluentUI 1.0
import "../global"

FluScrollablePage{

    title: qsTr("Settings")
    property var colorData: [FluColors.Yellow,FluColors.Orange,FluColors.Red,FluColors.Magenta,FluColors.Purple,FluColors.Blue,FluColors.Teal,FluColors.Green]
    id: root

    FluText {
        text: qsTr("App Bar")
        font.pixelSize: 20
        font.bold: true
    }

    FluFrame{
        Layout.fillWidth: true
        Layout.topMargin: 5
        height: 50
        padding: 10
        FluCheckBox{
            text: qsTr("Use System AppBar")
            checked: FluApp.useSystemAppBar
            anchors.verticalCenter: parent.verticalCenter
            onClicked: {
                FluApp.useSystemAppBar = !FluApp.useSystemAppBar
                dialog_restart.open()
            }
        }
    }

    FluFrame{
        Layout.fillWidth: true
        Layout.topMargin: 20
        height: 50
        padding: 10
        FluCheckBox{
            text:qsTr("Fits AppBar Windows")
            checked: window.fitsAppBarWindows
            anchors.verticalCenter: parent.verticalCenter
            onClicked: {
                window.fitsAppBarWindows = !window.fitsAppBarWindows
                SettingsHelper.saveAppBarWindows(window.fitsAppBarWindows)
            }
        }
    }

    FluContentDialog{
        id: dialog_restart
        title: qsTr("Friendly Reminder")
        message: qsTr("This action requires a restart of the program to take effect, is it restarted?")
        buttonFlags: FluContentDialogType.NegativeButton | FluContentDialogType.PositiveButton
        negativeText: qsTr("Cancel")
        positiveText: qsTr("OK")
        onPositiveClicked: {
            FluRouter.exit(931)
        }
    }

    FluText {
        text: qsTr("Theme")
        Layout.topMargin: 5
        font.pixelSize: 20
        font.bold: true
    }

    FluFrame{
        Layout.fillWidth: true
        Layout.topMargin: 5
        height: 128
        padding: 10

        ColumnLayout{
            spacing: 5
            anchors{
                top: parent.top
                left: parent.left
            }
            FluText{
                text: qsTr("Dark Mode")
                font: FluTextStyle.BodyStrong
                Layout.bottomMargin: 4
            }
            Repeater{
                model: [{title:qsTr("System"),mode:FluThemeType.System},{title:qsTr("Light"),mode:FluThemeType.Light},{title:qsTr("Dark"),mode:FluThemeType.Dark}]
                delegate: FluRadioButton{
                    checked : FluTheme.darkMode === modelData.mode
                    text:modelData.title
                    clickListener:function(){
                        FluTheme.darkMode = modelData.mode
                    }
                }
            }
        }
    }

    FluFrame {
        Layout.fillWidth: true
        Layout.topMargin: 20
        height: 80
        padding: 10

        ColumnLayout{
            spacing:0
            anchors{
                left: parent.left
            }

            FluText{
                text: qsTr("Theme Colors")
                font: FluTextStyle.BodyStrong
                Layout.bottomMargin: 4
            }
            RowLayout{
                Layout.topMargin: 5
                Repeater{
                    model: root.colorData
                    delegate:  Rectangle{
                        width: 42
                        height: 42
                        radius: 4
                        color: mouse_item.containsMouse ? Qt.lighter(modelData.normal,1.1) : modelData.normal
                        border.color: modelData.darker
                        FluIcon {
                            anchors.centerIn: parent
                            iconSource: FluentIcons.AcceptMedium
                            iconSize: 15
                            visible: modelData.normal === FluTheme.accentColor.normal &&
                                     modelData.light === FluTheme.accentColor.light &&
                                     modelData.lighter === FluTheme.accentColor.lighter &&
                                     modelData.lightest === FluTheme.accentColor.lightest &&
                                     modelData.dark === FluTheme.accentColor.dark &&
                                     modelData.darker === FluTheme.accentColor.darker &&
                                     modelData.darkest === FluTheme.accentColor.darkest
                            color: FluTheme.dark ? Qt.rgba(0,0,0,1) : Qt.rgba(1,1,1,1)
                        }
                        MouseArea{
                            id:mouse_item
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                FluTheme.accentColor = modelData
                            }
                        }
                    }
                }
            }
            Row{
                Layout.topMargin: 10
                spacing: 10
                FluText{
                    text: qsTr("Customize the Theme Color:")
                    anchors.verticalCenter: parent.verticalCenter
                }
                FluColorPicker{
                    id:color_picker
                    current: FluTheme.accentColor.normal
                    onAccepted: {
                       FluTheme.accentColor = FluColors.createAccentColor(current)
                    }
                    FluIcon {
                        anchors.centerIn: parent
                        iconSource: FluentIcons.AcceptMedium
                        iconSize: 15
                        visible: {
                            for(var i =0 ;i< root.colorData.length; i++){
                                if(root.colorData[i] === FluTheme.accentColor){
                                    return false
                                }
                            }
                            return true
                        }
                        color: FluTheme.dark ? Qt.rgba(0,0,0,1) : Qt.rgba(1,1,1,1)
                    }
                }
            }

        }
    }

    FluFrame {
        Layout.fillWidth: true
        Layout.topMargin: 20
        height: 80
        padding: 10

        ColumnLayout{
            spacing:0
            anchors{
                left: parent.left
            }
            FluText{
                text: qsTr("Open Blur Window")
                font: FluTextStyle.BodyStrong
                Layout.bottomMargin: 4
            }
            FluToggleSwitch{
                id: toggle_blur
                Layout.topMargin: 5
                checked: FluTheme.blurBehindWindowEnabled
                onClicked: {
                    FluTheme.blurBehindWindowEnabled = !FluTheme.blurBehindWindowEnabled
                }
            }
        }
    }

    FluFrame {
        visible: FluTheme.blurBehindWindowEnabled || window.effect === qsTr("dwm-blur")
        Layout.fillWidth: true
        Layout.topMargin: 20
        height: 80
        padding: 10

        ColumnLayout{
            spacing:0
            anchors{
                left: parent.left
            }
            FluText{
                text: qsTr("window effect")
                font: FluTextStyle.BodyStrong
                Layout.bottomMargin: 4
            }
            Row{
                spacing: 10
                Repeater{
                    model: window.availableEffects
                    delegate: FluRadioButton{
                        checked: window.effect === modelData
                        text: qsTr(`${modelData}`)
                        clickListener:function(){
                            window.effect = modelData
                            if(window.effective){
                                FluTheme.blurBehindWindowEnabled = false
                                toggle_blur.checked = Qt.binding( function() {return FluTheme.blurBehindWindowEnabled})
                            }
                        }
                    }

                }
            }
            FluText{
                visible: FluTheme.blurBehindWindowEnabled || window.effect === qsTr("dwm-blur")
                text: qsTr("window tintOpacity")
                Layout.topMargin: 20
            }
            ModSlider{
                visible: FluTheme.blurBehindWindowEnabled || window.effect === qsTr("dwm-blur")
                Layout.topMargin: 5
                useInt: false
                max: 1.0
                to:100
                onValueChanged: {
                    window.tintOpacity = value / to
                    SettingsHelper.saveWindowOpacity(window.tintOpacity)
                }
                Component.onCompleted: {
                    value = window.tintOpacity * to
                }
            }
            FluText{
                visible: FluTheme.blurBehindWindowEnabled
                text: qsTr("window blurRadius")
                Layout.topMargin: 20
            }
            FluSlider{
                visible: FluTheme.blurBehindWindowEnabled
                Layout.topMargin: 5
                to:100
                stepSize:1
                onValueChanged: {
                    window.blurRadius = value
                    SettingsHelper.saveBlurLevel(window.blurRadius)
                }
                Component.onCompleted: {
                    value = window.blurRadius
                }
            }
        }
    }

    FluFrame {
        Layout.fillWidth: true
        Layout.topMargin: 20
        height: 80
        padding: 10

        ColumnLayout{
            spacing:0
            anchors{
                left: parent.left
            }
            FluText{
                text: qsTr("Native Text")
                font: FluTextStyle.BodyStrong
                Layout.bottomMargin: 4
            }
            FluToggleSwitch{
                Layout.topMargin: 5
                checked: FluTheme.nativeText
                onClicked: {
                    FluTheme.nativeText = !FluTheme.nativeText
                }
            }
        }
    }

    FluText {
        text: qsTr("Action")
        Layout.topMargin: 5
        font.pixelSize: 20
        font.bold: true
    }

    FluFrame{
        Layout.fillWidth: true
        Layout.topMargin: 5
        height: 160
        padding: 10

        ColumnLayout{
            spacing: 5
            anchors{
                top: parent.top
                left: parent.left
            }
            FluText{
                text:qsTr("Navigation View Display Mode")
                font: FluTextStyle.BodyStrong
                Layout.bottomMargin: 4
            }
            Repeater{
                model: [{title:qsTr("Open"),mode:FluNavigationViewType.Open,index:0},{title:qsTr("Compact"),mode:FluNavigationViewType.Compact,index:1},{title:qsTr("Minimal"),mode:FluNavigationViewType.Minimal,index:2},{title:qsTr("Auto"),mode:FluNavigationViewType.Auto,index:3}]
                delegate: FluRadioButton{
                    text: modelData.title
                    checked: GlobalModel.displayMode === modelData.mode
                    clickListener:function(){
                        GlobalModel.displayMode = modelData.mode
                        SettingsHelper.saveDisplayMode(modelData.index)
                    }
                }
            }
        }
    }

    FluFrame {
        Layout.fillWidth: true
        Layout.topMargin: 20
        height: 80
        padding: 10

        ColumnLayout{
            spacing:0
            anchors{
                left: parent.left
            }
            FluText{
                text: qsTr("Open Animation")
                font: FluTextStyle.BodyStrong
                Layout.bottomMargin: 4
            }
            FluToggleSwitch{
                Layout.topMargin: 5
                checked: FluTheme.animationEnabled
                onClicked: {
                    FluTheme.animationEnabled = !FluTheme.animationEnabled
                }
            }
        }
    }

    FluText {
        text: qsTr("Localization")
        Layout.topMargin: 5
        font.pixelSize: 20
        font.bold: true
    }

    ListModel{
        id:model_language
        ListElement{
            name:"en"
        }
        ListElement{
            name:"zh"
        }
    }

    FluFrame{
        Layout.fillWidth: true
        Layout.topMargin: 5
        height: 80
        padding: 10

        ColumnLayout{
            spacing: 10
            anchors{
                top: parent.top
                left: parent.left
            }
            FluText{
                text:qsTr("Language")
                font: FluTextStyle.BodyStrong
                Layout.bottomMargin: 4
            }
            Flow{
                spacing: 5
                Repeater{
                    model: TranslateHelper.languages
                    delegate: FluRadioButton{
                        checked: TranslateHelper.current === modelData
                        text: {
                            switch(modelData) {
                            case "zh_CN":
                                return qsTr("Simplified Chinese")
                            case "en_US":
                                return qsTr("English")
                            }
                        }
                        clickListener:function(){
                            TranslateHelper.current = modelData
                            dialog_restart.open()
                        }
                    }
                }
            }
        }
    }

    FluText {
        text: qsTr("Other")
        Layout.topMargin: 5
        font.pixelSize: 20
        font.bold: true
    }

    FluFrame {
        Layout.fillWidth: true
        Layout.topMargin: 5
        height: 80
        padding: 10

        ColumnLayout{
            spacing:0
            anchors{
                left: parent.left
            }
            FluText{
                text: qsTr("Show FPS")
                font: FluTextStyle.BodyStrong
                Layout.bottomMargin: 4
            }
            FluToggleSwitch{
                Layout.topMargin: 5
                checked: GlobalModel.showFPS
                onClicked: {
                    GlobalModel.showFPS = !GlobalModel.showFPS
                }
            }
        }
    }
}
