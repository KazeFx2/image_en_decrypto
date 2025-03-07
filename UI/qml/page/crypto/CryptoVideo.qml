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
        nameFilters: [qsTr("Video Files (*.mp4 *.avi *.mov *.webm)")]
        flags: FileDialog.ReadOnly
        onAccepted: {
            GlobalVar.open_file = String(selectedFile)
            GlobalVar.open_name = GlobalVar.open_file.split("/").pop()
            GlobalVar.out_file = ""
            GlobalVar.out_name = ""
        }
        onRejected: {
        }
    }

    FileDialog {
        id: videoSaveDialog
        title: qsTr("Save File Select")
        acceptLabel: qsTr("Open")
        rejectLabel: qsTr("Cancel")
        fileMode: FileDialog.SaveFile
        currentFolder: StandardPaths.standardLocations(StandardPaths.MoviesLocation)[0]
        defaultSuffix: "avi"
        onAccepted: {
            GlobalVar.out_file = String(selectedFile)
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

            SubTitle {text: qsTr("Workspace")}

            SeparateLine {}

            FluRectangle {width: parent.width; height: 10; color: FluColors.Transparent}

            FluRadioButtons{
                id: func_select
                enabled: _apply_params.progress === 1.0
                spacing: 8
                orientation: Qt.Horizontal
                currentIndex: GlobalVar.video_sel
                anchors.horizontalCenter: parent.horizontalCenter
                FluRadioButton{
                    text:"Video Convert"
                    onClicked: {
                        GlobalVar.video_sel = 0
                    }
                }
                FluRadioButton{
                    text:"Realtime Play"
                    onClicked: {
                        GlobalVar.video_sel = 1
                    }
                }
            }

            FluRectangle {width: parent.width; height: 10; color: FluColors.Transparent}

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

                SeparateLine {id: center_sep; horizontal: false; anchors.centerIn: parent}

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

            FluRectangle {width: parent.width; height: 10; color: FluColors.Transparent}

            Rectangle {
                visible: func_select.currentIndex === 0
                width: cuda.width + 10 + cvt_type_sel.width + _apply_params.width + 10
                height: Math.max(cuda.height, cvt_type_sel.height, _apply_params.height)
                anchors.horizontalCenter: parent.horizontalCenter
                color: FluColors.Transparent

                FluCheckBox {
                    id: cuda
                    textRight: false
                    enabled: Crypto.cudaAvailable()
                    checked: GlobalVar.video_cvt_cuda
                    text: qsTr("Cuda")
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                    }
                    onCheckedChanged: {
                        GlobalVar.video_cvt_cuda = checked
                    }
                }

                FluDropDownButton{
                    id: cvt_type_sel
                    text: option_en.text
                    property int currentIndex: GlobalVar.video_cvt_encrypt ? 0 : 1
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: cuda.right
                        leftMargin: 10
                    }
                    FluMenuItem{
                        id: option_en
                        text: qsTr("Encrypt")
                        onClicked: {
                            cvt_type_sel.text = text
                            cvt_type_sel.currentIndex = 0
                            GlobalVar.video_cvt_encrypt = true
                        }
                    }
                    FluMenuItem{
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
                    enabled: GlobalVar.open_file !== "" && GlobalVar.out_file !== "" && progress === 1.0
                    text: qsTr("Convert!")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    onClicked: {
                        GlobalVar.apply_params_progrs = 0.0
                        VideoProvider.cvtVideo(GlobalVar.open_file, GlobalVar.out_file, paramConf.paramKey(), cvt_type_sel.currentIndex === 0, cuda.checked)
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
                        top: _apply_params.top
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
                        PropertyAnimation {duration: 100}
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

            FluRectangle {width: parent.width; height: 10; color: FluColors.Transparent}

            SubTitle {text: qsTr("Parameters")}

            SeparateLine {}

            FluRectangle {width: parent.width; height: 10; color: FluColors.Transparent}

            ParamConf { id: paramConf}
        }
    }

    FluRectangle {
        id: right_pad
        Layout.alignment: Qt.AlignRight
        width: 10
    }

}
