import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml 2.15
import Qt.labs.platform 1.1
import FluentUI 1.0
import main 1.0
import "../global"

FluWindow {

    id: window
    title: AppInfo.name
    width: 730
    height: 668
    minimumWidth: 730
    minimumHeight: 320
    launchMode: FluWindowType.SingleTask
    fitsAppBarWindows: SettingsHelper.getAppBarWindows()
    tintOpacity: SettingsHelper.getWindowOpacity()
    blurRadius: SettingsHelper.getBlurLevel()
    effect: {
        let idx = SettingsHelper.getWindowEffect()
        if (idx >= 0 && idx < availableEffects.length)
            return availableEffects[idx]
        return effect
    }
    appBar: FluAppBar {
        height: 30
        showDark: true
        darkClickListener: (button) => handleDarkChanged(button)
        closeClickListener: () => {
            dialog_close.open()
        }
        z: 7
    }

    onLazyLoad: {
    }

    Component.onCompleted: {
    }

    Component.onDestruction: {
        FluRouter.exit()
    }

    Connections {
        target: Crypto
        function onSaveImageFinished(idx) {
            if (idx === -1) {
                GlobalVar.save_all_btn_progrs = 1.0
                showError(qsTr("Save Cancelled"))
                return
            }
            GlobalVar.save_all_btn_progrs = 1.0 * (idx + 1) / GlobalVar.out_model.length
            if (GlobalVar.save_all_btn_progrs === 1.0) showSuccess(qsTr("Image Saving Complete!"))
        }
    }
    Connections {
        target: Crypto
        function onCryptoFinished(idx, url) {
            if (idx === -1) {
                GlobalVar.cvt_btn_progrs = 1.0
                showError(qsTr("Conversion Cancelled"))
                return
            }
            if (idx >= GlobalVar.out_model.length) {
                const tmp = GlobalVar.out_model
                tmp.push({
                    name: GlobalVar.list_model[idx].name,
                    source: url
                })
                GlobalVar.out_model = tmp
            } else {
                GlobalVar.out_model[idx] = {
                    name: GlobalVar.list_model[idx].name,
                    source: url
                }
                if (GlobalVar.img_out_mul !== undefined)
                    GlobalVar.img_out_mul.set(idx, GlobalVar.out_model[idx])
            }
            if (idx === 0)
                GlobalVar.img_out_sig_url = GlobalVar.out_model[0].source
            GlobalVar.cvt_btn_progrs = 1.0 * (idx + 1) / GlobalVar.list_model.length
            if (GlobalVar.cvt_btn_progrs === 1.0) showSuccess(qsTr("Conversion Complete!"))
        }
    }
    Connections {
        target: VideoProvider
        function onVideoCvtSignal(i, len) {
            if (len === 0) {
                GlobalVar.apply_params_progrs = 1.0
                showError(qsTr("Conversion Failed/Stopped!"))
            } else {
                GlobalVar.apply_params_progrs = i * 1.0 / len
                if (i === len) {
                    showSuccess(qsTr("Conversion Complete!"))
                }
            }
        }
    }
    // sys tray icon
    SystemTrayIcon {
        id: system_tray
        visible: true
        icon.source: "qrc:/main/res/image/favicon.ico"
        tooltip: AppInfo.name
        menu: Menu {
            MenuItem {
                text: "退出"
                onTriggered: {
                    FluRouter.exit()
                }
            }
        }
        onActivated:
                (reason) => {
            if (reason === SystemTrayIcon.Trigger) {
                window.show()
                window.raise()
                window.requestActivate()
            }
        }
    }

    Timer {
        id: timer_window_hide_delay
        interval: 150
        onTriggered: {
            window.hide()
        }
    }

    FluContentDialog {
        id: dialog_close
        title: qsTr("Quit")
        message: qsTr("Are you sure you want to exit the program?")
        negativeText: qsTr("Minimize")
        buttonFlags: FluContentDialogType.NegativeButton | FluContentDialogType.NeutralButton | FluContentDialogType.PositiveButton
        onNegativeClicked: {
            // system_tray.showMessage(qsTr("Friendly Reminder"), qsTr("FluentUI is hidden from the tray, click on the tray to activate the window again"));
            // timer_window_hide_delay.restart()
        }
        positiveText: qsTr("Quit")
        neutralText: qsTr("Cancel")
        onPositiveClicked: {
            if (GlobalVar.cvt_btn_progrs !== 1.0)
                Crypto.stopCrypto()
            if (GlobalVar.save_all_btn_progrs !== 1.0)
                Crypto.stopSave()
            if (GlobalVar.apply_params_progrs !== 1.0)
                VideoProvider.force_stop_cvt()
            if (GlobalVar.video_url !== "")
                GlobalVar.video_url = ""
            FluRouter.exit(0)
        }
    }

    Flipable {
        id: flipable
        anchors.fill: parent
        property bool flipped: false
        property real flipAngle: 0
        transform: Rotation {
            id: rotation
            origin.x: flipable.width / 2
            origin.y: flipable.height / 2
            axis {
                x: 0; y: 1; z: 0
            }
            angle: flipable.flipAngle

        }
        states: State {
            PropertyChanges {
                target: flipable; flipAngle: 180
            }
            when: flipable.flipped
        }
        transitions: Transition {
            NumberAnimation {
                target: flipable; property: "flipAngle";
                duration: 1000; easing.type: Easing.OutCubic
            }
        }
        back: Item {
            anchors.fill: flipable
            visible: flipable.flipAngle !== 0
            Row {
                id: layout_back_buttons
                z: 8
                anchors {
                    top: parent.top
                    left: parent.left
                    topMargin: FluTools.isMacos() ? 20 : 5
                    leftMargin: 5
                }
            }
        }
        front: Item {
            id: page_front
            visible: flipable.flipAngle !== 180
            anchors.fill: flipable
            FluNavigationView {
                buttonBack.visible: false
                imageLogo.Layout.leftMargin: 10
                id: nav_view
                width: parent.width
                height: parent.height
                z: 999
                //Stack模式，每次切换都会将页面压入栈中，随着栈的页面增多，消耗的内存也越多，内存消耗多就会卡顿，这时候就需要按返回将页面pop掉，释放内存。该模式可以配合FluPage中的launchMode属性，设置页面的启动模式
                //                pageMode: FluNavigationViewType.Stack
                //NoStack模式，每次切换都会销毁之前的页面然后创建一个新的页面，只需消耗少量内存
                pageMode: FluNavigationViewType.NoStack
                items: ItemsOriginal
                footerItems: ItemsFooter
                topPadding: {
                    if (window.useSystemAppBar) {
                        return 0
                    }
                    return FluTools.isMacos() ? 20 : 0
                }
                displayMode: GlobalModel.displayMode
                logo: "qrc:/main/res/image/favicon.ico"
                title: AppInfo.name
                onLogoClicked: {
                }
                // autoSuggestBox:FluAutoSuggestBox{
                //     iconSource: FluentIcons.Search
                //     items: ItemsOriginal.getSearchData()
                //     placeholderText: qsTr("Search")
                //     onItemClicked:
                //         (data)=>{
                //             ItemsOriginal.startPageByItem(data)
                //         }
                // }
                Component.onCompleted: {
                    ItemsOriginal.navigationView = nav_view
                    ItemsFooter.navigationView = nav_view
                    window.setHitTestVisible(nav_view.buttonMenu)
                    window.setHitTestVisible(nav_view.buttonBack)
                    window.setHitTestVisible(nav_view.imageLogo)
                    setCurrentIndex(0)
                }
            }
        }
    }

    Component {
        id: com_reveal
        CircularReveal {
            id: reveal
            target: window.containerItem()
            anchors.fill: parent
            darkToLight: FluTheme.dark
            onAnimationFinished: {
                //动画结束后释放资源
                loader_reveal.sourceComponent = undefined
            }
            onImageChanged: {
                changeDark()
            }
        }
    }

    FluLoader {
        id: loader_reveal
        anchors.fill: parent
    }

    function distance(x1, y1, x2, y2) {
        return Math.sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2))
    }

    function handleDarkChanged(button) {
        if (FluTools.isMacos() || !FluTheme.animationEnabled) {
            changeDark()
        } else {
            loader_reveal.sourceComponent = com_reveal
            var target = window.containerItem()
            var pos = button.mapToItem(target, 0, 0)
            var mouseX = pos.x + button.width / 2
            var mouseY = pos.y + button.height / 2
            var radius = Math.max(distance(mouseX, mouseY, 0, 0), distance(mouseX, mouseY, target.width, 0), distance(mouseX, mouseY, 0, target.height), distance(mouseX, mouseY, target.width, target.height))
            var reveal = loader_reveal.item
            reveal.start(reveal.width * Screen.devicePixelRatio, reveal.height * Screen.devicePixelRatio, Qt.point(mouseX, mouseY), radius)
        }
    }

    function changeDark() {
        if (FluTheme.dark) {
            FluTheme.darkMode = FluThemeType.Light
        } else {
            FluTheme.darkMode = FluThemeType.Dark
        }
    }

    FpsItem {
        id: fps_item
    }

    Connections {
        target: GlobalModel

        function onShowFPSChanged() {
            SettingsHelper.saveShowFPS(GlobalModel.showFPS)
        }
    }

    FluText {
        visible: GlobalModel.showFPS
        text: "fps %1".arg(fps_item.fps)
        opacity: 0.3
        anchors {
            bottom: parent.bottom
            right: parent.right
            bottomMargin: 5
            rightMargin: 5
        }
    }
}
