import QtQuick 2.15
import QtQuick.Templates 2.15 as T
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs
import QtCore
import FluentUI 1.0
import "qrc:/main/src/js/tools.js" as Tools
import "../../global"
import main 1.0

FluScrollablePage {
    id: crypto_video_page

    function pause_video() {
        if (play_img.url !== "")
            VideoProvider.pause(play_img.url)
    }

    property string open_file: GlobalVar.open_file
    property string open_name: GlobalVar.open_name
    property string out_file: GlobalVar.out_file
    property string out_name: GlobalVar.out_name

    Connections {
    }

    Component.onCompleted: {
        if (GlobalVar.video_cvt_param_key_id !== "")
            paramConf.loadKey(GlobalVar.video_cvt_param_key_id)
    }
    Component.onDestruction: {
        GlobalVar.video_cvt_param_key_id = paramConf.paramKey()
    }

    onWidthChanged: {
    }

    width: parent.width
    height: parent.height

    FileDialog {
        id: videoPickDialog
        title: qsTr("File Select")
        acceptLabel: qsTr("Open")
        rejectLabel: qsTr("Cancel")
        fileMode: FileDialog.OpenFile
        currentFolder: StandardPaths.standardLocations(StandardPaths.MoviesLocation)[0]
        defaultSuffix: "mp4"
        nameFilters: [qsTr("Video Files (*.mp4 *.avi *.mov *.webm *.mkv)")]
        flags: FileDialog.ReadOnly
        /*
            FileDialog.DontResolveSymlinks
            FileDialog.DontConfirmOverwrite
            FileDialog.ReadOnly
            FileDialog.HideNameFilterDetails
            FileDialog.DontUseNativeDialog
        */
        /*
            selectedFile: url
            selectedFiles: url
        */
        onAccepted: {
            let source = VideoProvider.loadVideo(selectedFile, paramConf.paramKey(), DecodeType.Raw, true, play_img.width, play_img.height)
            // console.log(source)
            GlobalVar.video_url = source
        }
        onRejected: {
        }
    }

    FileDialog {
        id: videoCvtPickDialog
        title: qsTr("File Select")
        acceptLabel: qsTr("Open")
        rejectLabel: qsTr("Cancel")
        fileMode: FileDialog.OpenFile
        currentFolder: StandardPaths.standardLocations(StandardPaths.MoviesLocation)[0]
        defaultSuffix: "mp4"
        nameFilters: videoPickDialog.nameFilters
        flags: FileDialog.ReadOnly
        onAccepted: {
            GlobalVar.open_file = String(selectedFile)
            GlobalVar.open_name = GlobalVar.open_file.split("/").pop()
            GlobalVar.out_file = ""
            GlobalVar.out_name = ""
            let ret = VideoProvider.getVideoWH(selectedFile)
            GlobalVar.real_width = ret.width
            GlobalVar.real_height = ret.height
            input_w.update_me = false
            input_h.update_me = false
            GlobalVar.cvt_width = GlobalVar.real_width
            GlobalVar.cvt_height = GlobalVar.real_height
        }
        onRejected: {
        }
    }

    FileDialog {
        id: videoSaveDialog
        title: qsTr("Save File Select")
        acceptLabel: qsTr("Save")
        rejectLabel: qsTr("Cancel")
        fileMode: FileDialog.SaveFile
        currentFolder: StandardPaths.standardLocations(StandardPaths.MoviesLocation)[0]
        defaultSuffix: "avi"
        flags: FileDialog.DontConfirmOverwrite
        nameFilters: [qsTr("Video Files (*.avi)")]
        onAccepted: {
            GlobalVar.out_file = Tools.auto_suffix(String(selectedFile), "avi")
            GlobalVar.out_name = GlobalVar.out_file.split("/").pop()
        }
        onRejected: {
        }
        Connections {
        }
    }

    FluRectangle {
        Layout.alignment: Qt.AlignLeft

        width: parent.width - right_pad.width
        height: childrenRect.height

        color: FluColors.Transparent

        Column {

            width: parent.width
            height: childrenRect.height

            SubTitle {
                text: qsTr("Workspace")
            }

            SeparateLine {
            }

            FluRectangle {
                width: parent.width; height: 10; color: FluColors.Transparent
            }

            FluRadioButtons {
                id: func_select
                enabled: _apply_params.progress === 1.0
                spacing: 8
                orientation: Qt.Horizontal
                currentIndex: GlobalVar.video_sel
                anchors.horizontalCenter: parent.horizontalCenter
                FluRadioButton {
                    text: qsTr("Video Convert")
                    onClicked: {
                        GlobalVar.video_sel = 0
                    }
                }
                FluRadioButton {
                    text: qsTr("Realtime Play")
                    onClicked: {
                        GlobalVar.video_sel = 1
                    }
                }
            }

            FluRectangle {
                width: parent.width; height: 10; color: FluColors.Transparent
            }

            Rectangle {
                id: video_cvt
                visible: func_select.currentIndex === 0
                width: parent.width
                height: 300

                color: FluColors.Grey120.alpha(0.3)
                radius: 5

                Rectangle {
                    width: parent.width / 2
                    height: parent.height
                    anchors.left: parent.left
                    color: FluColors.Transparent

                    Rectangle {
                        visible: GlobalVar.open_file !== ""
                        width: parent.width / 2
                        height: in_icon.height + input_file_name.height + in_del.height + 10
                        color: FluColors.Transparent
                        anchors.centerIn: parent

                        FluIcon {
                            id: in_icon
                            iconSource: FluentIcons.Video
                            iconSize: parent.parent.height * 0.4
                            anchors {
                                horizontalCenter: parent.horizontalCenter
                                top: parent.top
                            }
                        }

                        FluText {
                            id: input_file_name
                            text: GlobalVar.open_name
                            anchors {
                                horizontalCenter: parent.horizontalCenter
                                top: in_icon.bottom
                            }
                        }

                        FluIconButton {
                            id: in_del
                            enabled: _apply_params.progress === 1.0
                            iconSource: FluentIcons.Delete
                            iconColor: FluColors.Red.normal
                            anchors {
                                horizontalCenter: parent.horizontalCenter
                                bottom: parent.bottom
                            }
                            onClicked: {
                                GlobalVar.open_file = ""
                                GlobalVar.open_name = ""
                                GlobalVar.out_file = ""
                                GlobalVar.out_name = ""
                            }
                        }
                    }

                    FluIconButton {
                        visible: GlobalVar.open_file === ""
                        iconSource: FluentIcons.OpenFile
                        iconSize: parent.height / 2
                        anchors.centerIn: parent

                        FluTooltip {
                            visible: parent.hovered
                            delay: 1000
                            text: qsTr("Select Input Video")
                        }

                        onClicked: {
                            videoCvtPickDialog.open()
                        }
                    }
                }

                FluText {
                    text: qsTr("Input")
                    anchors {
                        top: parent.top
                        left: parent.left
                        topMargin: 10
                        leftMargin: 10
                    }
                }

                SeparateLine {
                    id: center_sep; horizontal: false; anchors.centerIn: parent
                }

                Rectangle {
                    width: parent.width / 2
                    height: parent.height
                    anchors.right: parent.right
                    color: FluColors.Transparent

                    Rectangle {
                        visible: GlobalVar.out_file !== ""
                        width: parent.width / 2
                        height: out_icon.height + output_file_name.height + out_del.height + 5
                        color: FluColors.Transparent
                        anchors.centerIn: parent

                        FluIcon {
                            id: out_icon
                            iconSource: FluentIcons.Video
                            iconSize: parent.parent.height * 0.4
                            anchors {
                                horizontalCenter: parent.horizontalCenter
                                top: parent.top
                            }
                        }

                        FluText {
                            id: output_file_name
                            text: GlobalVar.out_name
                            anchors {
                                horizontalCenter: parent.horizontalCenter
                                top: out_icon.bottom
                            }
                        }

                        FluIconButton {
                            id: out_del
                            enabled: _apply_params.progress === 1.0
                            iconSource: FluentIcons.Delete
                            iconColor: FluColors.Red.normal
                            anchors {
                                horizontalCenter: parent.horizontalCenter
                                bottom: parent.bottom
                            }
                            onClicked: {
                                GlobalVar.out_file = ""
                                GlobalVar.out_name = ""
                            }
                        }
                    }

                    FluIconButton {
                        visible: GlobalVar.out_file === ""
                        iconSource: FluentIcons.SaveLocal
                        iconSize: parent.height / 2
                        anchors.centerIn: parent

                        FluTooltip {
                            visible: parent.hovered
                            delay: 1000
                            text: qsTr("Select Output File")
                        }

                        onClicked: {
                            videoSaveDialog.open()
                        }
                    }
                }

                FluText {
                    text: qsTr("Output")
                    anchors {
                        top: parent.top
                        left: center_sep.right
                        topMargin: 10
                        leftMargin: 10
                    }
                }
            }

            Rectangle {
                id: realtime_play
                visible: func_select.currentIndex === 1
                width: parent.width
                height: 300

                color: FluColors.Grey120.alpha(0.3)
                radius: 5

                onVisibleChanged: {
                    if (!visible) {
                        pause_video()
                    }
                }

                FluText {
                    visible: play_img.url === ""
                    anchors.centerIn: parent
                    text: qsTr("No Video File Selected.")
                    font.pixelSize: 24
                }

                VideoPlayer {
                    id: play_img
                    url: GlobalVar.video_url
                    visible: play_img.url !== ""
                    width: parent.width
                    height: parent.height

                    onWidthChanged: {
                        if (width < 0)
                            pause_video()
                    }

                    Component.onDestruction: {
                        pause_video()
                    }
                }

            }

            FluRectangle {
                width: parent.width; height: 10; color: FluColors.Transparent
            }

            Rectangle {
                visible: func_select.currentIndex === 0
                width: upper_box.width
                height: upper_box.height + video_size_box.height + 10
                anchors.horizontalCenter: parent.horizontalCenter
                color: FluColors.Transparent

                Rectangle {
                    id: upper_box
                    width: cuda.width + 10 + cvt_type_sel.width + _apply_params.width + 10
                    height: Math.max(cuda.height, cvt_type_sel.height, _apply_params.height, cancel_btn.height)
                    color: FluColors.Transparent
                    FluCheckBox {
                        id: cuda
                        textRight: false
                        enabled: Crypto.cudaAvailable() && _apply_params.enabled
                        checked: GlobalVar.video_cvt_cuda
                        text: qsTr("Cuda")
                        anchors {
                            left: parent.left
                            verticalCenter: parent.verticalCenter
                        }
                        onCheckedChanged: {
                            GlobalVar.video_cvt_cuda = checked
                        }
                    }

                    FluDropDownButton {
                        id: cvt_type_sel
                        text: option_en.text
                        property int currentIndex: GlobalVar.video_cvt_encrypt ? 0 : 1
                        enabled: _apply_params.enabled
                        anchors {
                            left: cuda.right
                            leftMargin: 10
                            verticalCenter: parent.verticalCenter
                        }
                        FluMenuItem {
                            id: option_en
                            text: qsTr("Encrypt")
                            onClicked: {
                                cvt_type_sel.text = text
                                cvt_type_sel.currentIndex = 0
                                GlobalVar.video_cvt_encrypt = true
                            }
                        }
                        FluMenuItem {
                            id: option_de
                            text: qsTr("Decrypt")
                            onClicked: {
                                cvt_type_sel.text = text
                                cvt_type_sel.currentIndex = 1
                                GlobalVar.video_cvt_encrypt = false
                            }
                        }
                    }

                    FluProgressButton {
                        id: _apply_params
                        progress: GlobalVar.apply_params_progrs
                        enabled: GlobalVar.open_file !== "" && GlobalVar.out_file !== "" && progress === 1.0 && GlobalVar.cvt_width !== 0 && GlobalVar.cvt_height !== 0
                        text: qsTr("Convert!")
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.verticalCenter: parent.verticalCenter
                        onClicked: {
                            GlobalVar.apply_params_progrs = 0.0
                            VideoProvider.cvtVideoWH(GlobalVar.open_file, GlobalVar.out_file, paramConf.paramKey(), cvt_type_sel.currentIndex === 0, cuda.checked, GlobalVar.cvt_width, GlobalVar.cvt_height)
                        }
                        Component.onDestruction: {
                            // if (progress !== 1.0) {
                            //     VideoProvider.force_stop_cvt()
                            // }
                        }
                    }
                    Rectangle {
                        width: _apply_params.progress !== 1.0 ? cancel_btn.width : 0
                        height: cancel_btn.height
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: _apply_params.right
                            leftMargin: 10
                        }
                        color: FluColors.Transparent
                        clip: true
                        Behavior on width {
                            PropertyAnimation {
                                duration: 100
                            }
                        }
                        FluFilledButton {
                            id: cancel_btn
                            enabled: _apply_params.progress !== 1.0
                            anchors {
                                centerIn: parent
                            }
                            text: qsTr("Cancel")
                            onClicked: {
                                VideoProvider.force_stop_cvt()
                            }
                        }
                    }
                }

                Rectangle {
                    id: video_size_box
                    // property bool flag: false
                    width: input_w.width + 10 + bind_wh.width + 10 + input_h.width
                    height: Math.max(input_w.height, bind_wh.height, input_h.height)
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: FluColors.Transparent

                    FluText {
                        text: qsTr("Size")
                        anchors.right: input_w.left
                        anchors.rightMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    FluTextBox {
                        id: input_w
                        property bool update_me: true
                        cleanEnabled: false
                        text: String(GlobalVar.cvt_width)
                        enabled: GlobalVar.open_file !== "" && _apply_params.progress === 1.0
                        width: 100
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        validator: RegularExpressionValidator {
                            regularExpression: /[0-9]*/
                        }
                        onTextChanged: {
                            if (text === "") return
                            if (String(GlobalVar.cvt_width) !== text)
                                GlobalVar.cvt_width = Number(text)
                            if (!update_me) {
                                update_me = true
                                return
                            }
                            if (GlobalVar.bind_wh) {
                                GlobalVar.cvt_height = GlobalVar.cvt_width * GlobalVar.real_height / GlobalVar.real_width
                                input_h.update_me = false
                            }
                        }
                    }
                    FluIconButton {
                        id: bind_wh
                        enabled: GlobalVar.open_file !== "" && _apply_params.progress === 1.0
                        anchors.left: input_w.right
                        anchors.leftMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        iconSource: GlobalVar.bind_wh ? FluentIcons.Link : FluentIcons.More
                        // FluentIcons.More
                        onClicked: {
                            GlobalVar.bind_wh = !GlobalVar.bind_wh
                            if (GlobalVar.bind_wh)
                                GlobalVar.cvt_height = GlobalVar.cvt_width * GlobalVar.real_height / GlobalVar.real_width
                        }
                    }
                    FluTextBox {
                        id: input_h
                        property bool update_me: true
                        cleanEnabled: false
                        text: String(GlobalVar.cvt_height)
                        enabled: GlobalVar.open_file !== "" && _apply_params.progress === 1.0
                        width: 100
                        anchors.left: bind_wh.right
                        anchors.leftMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        validator: RegularExpressionValidator {
                            regularExpression: /[0-9]*/
                        }
                        onTextChanged: {
                            if (text === "") return
                            if (String(GlobalVar.cvt_height) !== text)
                                GlobalVar.cvt_height = Number(text)
                            if (!update_me) {
                                update_me = true
                                return
                            }
                            if (GlobalVar.bind_wh) {
                                GlobalVar.cvt_width = GlobalVar.cvt_height * GlobalVar.real_width / GlobalVar.real_height
                                input_w.update_me = false
                            }
                        }
                    }
                    FluIconButton {
                        enabled: GlobalVar.open_file !== "" && _apply_params.progress === 1.0 && GlobalVar.cvt_witdh !== GlobalVar.real_width && GlobalVar.cvt_height !== GlobalVar.real_height
                        anchors.left: input_h.right
                        anchors.leftMargin: 10
                        anchors.top: bind_wh.top
                        iconSource: FluentIcons.Refresh
                        onClicked: {
                            input_w.update_me = false
                            input_h.update_me = false
                            GlobalVar.cvt_width = GlobalVar.real_width
                            GlobalVar.cvt_height = GlobalVar.real_height
                        }
                    }
                }
            }

            Rectangle {
                visible: func_select.currentIndex === 1
                width: select_video.width + apply_params.width + 10 + del_video_box.width + (del_video_box.width !== 0 ? 10 : 0)
                height: Math.max(select_video.height, apply_params.height)
                anchors.horizontalCenter: parent.horizontalCenter
                color: FluColors.Transparent

                FluFilledButton {
                    id: select_video
                    text: qsTr("Select Video")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    onClicked: {
                        videoPickDialog.open()
                    }
                }

                Rectangle {
                    id: del_video_box
                    width: play_img.url !== "" ? del_video_btn.width : 0
                    height: del_video_btn.height
                    anchors.left: select_video.right
                    anchors.leftMargin: 10
                    color: FluColors.Transparent
                    clip: true
                    Behavior on width {
                        PropertyAnimation {
                            duration: 100
                        }
                    }
                    FluIconButton {
                        id: del_video_btn
                        iconSource: FluentIcons.Delete
                        iconColor: FluColors.Red.normal
                        anchors.centerIn: parent
                        enabled: play_img.url !== ""
                        onClicked: {
                            // const tmp = play_img.url
                            GlobalVar.video_url = ""
                            // VideoProvider.delVideo(tmp)
                        }
                    }
                }

                FluFilledButton {
                    id: apply_params
                    text: qsTr("Update Params")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    onClicked: {
                        VideoProvider.set_param(play_img.url, paramConf.paramKey())
                    }
                }
            }

            FluRectangle {
                width: parent.width; height: 10; color: FluColors.Transparent
            }

            SubTitle {
                text: qsTr("Parameters")
            }

            SeparateLine {
            }

            FluRectangle {
                width: parent.width; height: 10; color: FluColors.Transparent
            }

            ParamConf {
                id: paramConf
            }
        }
    }

    FluRectangle {
        id: right_pad
        Layout.alignment: Qt.AlignRight
        width: 10
    }

}
