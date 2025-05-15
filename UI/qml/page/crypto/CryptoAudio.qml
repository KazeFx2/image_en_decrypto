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
    id: crypto_audio_page

    function pause_audio() {
        if (play_img.audioId !== -1)
            AudioProvider.pause(play_img.audioId)
    }

    property string open_file: GlobalVar.au_open_file
    property string open_name: GlobalVar.au_open_name
    property string out_file: GlobalVar.au_out_file
    property string out_name: GlobalVar.au_out_name

    Component.onCompleted: {
        if (GlobalVar.audio_cvt_param_key_id !== "")
            paramConf.loadKey(GlobalVar.audio_cvt_param_key_id)
    }

    Component.onDestruction: {
        GlobalVar.audio_cvt_param_key_id = paramConf.paramKey()
    }

    width: parent.width
    height: parent.height

    FileDialog {
        id: audioPickDialog
        title: qsTr("File Select")
        acceptLabel: qsTr("Open")
        rejectLabel: qsTr("Cancel")
        fileMode: FileDialog.OpenFile
        currentFolder: StandardPaths.standardLocations(StandardPaths.MusicLocation)[0]
        defaultSuffix: "mp3"
        nameFilters: [qsTr("Audio Files (*.mp3 *.aac *.m4a *.flac *.wav .*ogg *.opus *.alac *.wma *.ac3 *.eac3 *.dts *.amr)")]
        flags: FileDialog.ReadOnly
        onAccepted: {
            let id = AudioProvider.loadAudio(selectedFile, paramConf.paramKey(), DecodeType.Raw, GlobalVar.au_cvt_width_rel, GlobalVar.au_cvt_height_rel, true)
            GlobalVar.audio_id = id
        }
        onRejected: {
        }
    }

    FileDialog {
        id: audioCvtPickDialog
        title: qsTr("File Select")
        acceptLabel: qsTr("Open")
        rejectLabel: qsTr("Cancel")
        fileMode: FileDialog.OpenFile
        currentFolder: StandardPaths.standardLocations(StandardPaths.MusicLocation)[0]
        defaultSuffix: "mp3"
        nameFilters: audioPickDialog.nameFilters
        flags: FileDialog.ReadOnly
        onAccepted: {
            GlobalVar.au_open_file = String(selectedFile)
            GlobalVar.au_open_name = GlobalVar.au_open_file.split("/").pop()
            GlobalVar.au_out_file = ""
            GlobalVar.au_out_name = ""
        }
        onRejected: {
        }
    }

    FileDialog {
        id: audioSaveDialog
        title: qsTr("Save File Select")
        acceptLabel: qsTr("Save")
        rejectLabel: qsTr("Cancel")
        fileMode: FileDialog.SaveFile
        currentFolder: StandardPaths.standardLocations(StandardPaths.MusicLocation)[0]
        defaultSuffix: "wav"
        flags: FileDialog.DontConfirmOverwrite
        nameFilters: [qsTr("Audio Files (*.wav)")]
        onAccepted: {
            GlobalVar.au_out_file = Tools.auto_suffix(String(selectedFile), "wav")
            GlobalVar.au_out_name = GlobalVar.au_out_file.split("/").pop()
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
                currentIndex: GlobalVar.audio_sel
                anchors.horizontalCenter: parent.horizontalCenter
                FluRadioButton {
                    text: qsTr("Audio Convert")
                    onClicked: {
                        GlobalVar.audio_sel = 0
                    }
                }
                FluRadioButton {
                    text: qsTr("Realtime Play")
                    onClicked: {
                        GlobalVar.audio_sel = 1
                    }
                }
            }

            FluRectangle {
                width: parent.width; height: 10; color: FluColors.Transparent
            }

            Rectangle {
                id: audio_cvt
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
                        visible: GlobalVar.au_open_file !== ""
                        width: parent.width / 2
                        height: in_icon.height + input_file_name.height + in_del.height + 10
                        color: FluColors.Transparent
                        anchors.centerIn: parent

                        FluIcon {
                            id: in_icon
                            iconSource: FluentIcons.Audio
                            iconSize: parent.parent.height * 0.4
                            anchors {
                                horizontalCenter: parent.horizontalCenter
                                top: parent.top
                            }
                        }

                        FluText {
                            id: input_file_name
                            text: GlobalVar.au_open_name
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
                                GlobalVar.au_open_file = ""
                                GlobalVar.au_open_name = ""
                                GlobalVar.au_out_file = ""
                                GlobalVar.au_out_name = ""
                            }
                        }
                    }

                    FluIconButton {
                        visible: GlobalVar.au_open_file === ""
                        iconSource: FluentIcons.OpenFile
                        iconSize: parent.height / 2
                        anchors.centerIn: parent

                        FluTooltip {
                            visible: parent.hovered
                            delay: 1000
                            text: qsTr("Select Input Audio")
                        }

                        onClicked: {
                            audioCvtPickDialog.open()
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
                        visible: GlobalVar.au_out_file !== ""
                        width: parent.width / 2
                        height: out_icon.height + output_file_name.height + out_del.height + 5
                        color: FluColors.Transparent
                        anchors.centerIn: parent

                        FluIcon {
                            id: out_icon
                            iconSource: FluentIcons.Audio
                            iconSize: parent.parent.height * 0.4
                            anchors {
                                horizontalCenter: parent.horizontalCenter
                                top: parent.top
                            }
                        }

                        FluText {
                            id: output_file_name
                            text: GlobalVar.au_out_name
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
                                GlobalVar.au_out_file = ""
                                GlobalVar.au_out_name = ""
                            }
                        }
                    }

                    FluIconButton {
                        visible: GlobalVar.au_out_file === ""
                        iconSource: FluentIcons.SaveLocal
                        iconSize: parent.height / 2
                        anchors.centerIn: parent

                        FluTooltip {
                            visible: parent.hovered
                            delay: 1000
                            text: qsTr("Select Output File")
                        }

                        onClicked: {
                            audioSaveDialog.open()
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
                        pause_audio()
                    }
                }

                FluText {
                    visible: play_img.audioId === -1
                    anchors.centerIn: parent
                    text: qsTr("No Audio File Selected.")
                    font.pixelSize: 24
                }

                AudioPlayer {
                    id: play_img
                    audioId: GlobalVar.audio_id
                    visible: play_img.audioId !== -1
                    width: parent.width
                    height: parent.height

                    onWidthChanged: {
                        if (width < 0) {
                            pause_audio()
                        }
                    }

                    Component.onDestruction: {
                        pause_audio()
                    }
                }

            }

            FluRectangle {
                width: parent.width; height: 10; color: FluColors.Transparent
            }

            Rectangle {
                visible: func_select.currentIndex === 0
                width: upper_box.width
                height: upper_box.height + audio_size_box.height + 10
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
                        checked: GlobalVar.audio_cvt_cuda
                        text: qsTr("Cuda")
                        anchors {
                            left: parent.left
                            verticalCenter: parent.verticalCenter
                        }
                        onCheckedChanged: {
                            GlobalVar.audio_cvt_cuda = checked
                        }
                    }

                    FluDropDownButton {
                        id: cvt_type_sel
                        text: option_en.text
                        property int currentIndex: GlobalVar.audio_cvt_encrypt ? 0 : 1
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
                                GlobalVar.audio_cvt_encrypt = true
                            }
                        }
                        FluMenuItem {
                            id: option_de
                            text: qsTr("Decrypt")
                            onClicked: {
                                cvt_type_sel.text = text
                                cvt_type_sel.currentIndex = 1
                                GlobalVar.audio_cvt_encrypt = false
                            }
                        }
                    }

                    FluProgressButton {
                        id: _apply_params
                        progress: GlobalVar.au_apply_params_progrs
                        enabled: GlobalVar.au_open_file !== "" && GlobalVar.au_out_file !== "" && progress === 1.0 && GlobalVar.au_cvt_width !== 0 && GlobalVar.au_cvt_height !== 0
                        text: qsTr("Convert!")
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.verticalCenter: parent.verticalCenter
                        onClicked: {
                            GlobalVar.au_apply_params_progrs = 0.0
                            AudioProvider.cvtAudio(GlobalVar.au_open_file, GlobalVar.au_out_file, paramConf.paramKey(), GlobalVar.au_cvt_width, GlobalVar.au_cvt_height, cvt_type_sel.currentIndex === 0, cuda.checked)
                        }
                        Component.onDestruction: {
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
                                AudioProvider.force_stop_cvt()
                            }
                        }
                    }
                }

                Rectangle {
                    id: audio_size_box
                    // property bool flag: false
                    width: input_w.width + 10 + h_txt.width + 10 + input_h.width
                    height: Math.max(input_w.height, h_txt.height, input_h.height)
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: FluColors.Transparent

                    FluText {
                        text: qsTr("Width")
                        anchors.right: input_w.left
                        anchors.rightMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    FluTextBox {
                        id: input_w
                        cleanEnabled: false
                        text: String(GlobalVar.au_cvt_width)
                        enabled: GlobalVar.au_open_file !== "" && _apply_params.progress === 1.0
                        width: 100
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        validator: RegularExpressionValidator {
                            regularExpression: /[0-9]*/
                        }
                        onTextChanged: {
                            if (text === "") return
                            if (String(GlobalVar.au_cvt_width) !== text)
                                GlobalVar.au_cvt_width = Number(text)
                        }
                    }
                    FluText {
                        id: h_txt
                        anchors.left: input_w.right
                        anchors.leftMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("Height")
                    }
                    FluTextBox {
                        id: input_h
                        cleanEnabled: false
                        text: String(GlobalVar.au_cvt_height)
                        enabled: GlobalVar.au_open_file !== "" && _apply_params.progress === 1.0
                        width: 100
                        anchors.left: h_txt.right
                        anchors.leftMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        validator: RegularExpressionValidator {
                            regularExpression: /[0-9]*/
                        }
                        onTextChanged: {
                            if (text === "") return
                            if (String(GlobalVar.au_cvt_height) !== text)
                                GlobalVar.au_cvt_height = Number(text)
                        }
                    }
                    FluIconButton {
                        enabled: GlobalVar.au_open_file !== "" && _apply_params.progress === 1.0 && GlobalVar.au_cvt_witdh !== GlobalVar.au_def_width && GlobalVar.au_cvt_height !== GlobalVar.au_def_height
                        anchors.left: input_h.right
                        anchors.leftMargin: 10
                        anchors.top: audio_size_box.top
                        iconSource: FluentIcons.Refresh

                        FluTooltip {
                            visible: parent.hovered
                            delay: 1000
                            text: qsTr("Reset Matrix Shape")
                        }

                        onClicked: {
                            GlobalVar.au_cvt_width = GlobalVar.au_def_width
                            GlobalVar.au_cvt_height = GlobalVar.au_def_height
                        }
                    }
                }
            }

            Rectangle {
                visible: func_select.currentIndex === 1
                width: select_audio.width + apply_params.width + 10 + del_audio_box.width + (del_audio_box.width !== 0 ? 10 : 0)
                height: Math.max(select_audio.height, apply_params.height) + audio_size_box_rel.height + 10
                anchors.horizontalCenter: parent.horizontalCenter
                color: FluColors.Transparent

                FluFilledButton {
                    id: select_audio
                    text: qsTr("Select Audio")
                    anchors.top: parent.top
                    anchors.left: parent.left
                    onClicked: {
                        audioPickDialog.open()
                    }
                }

                Rectangle {
                    id: del_audio_box
                    width: play_img.audioId !== -1 ? del_audio_btn.width : 0
                    height: del_audio_btn.height
                    anchors.left: select_audio.right
                    anchors.top: parent.top
                    anchors.leftMargin: 10
                    color: FluColors.Transparent
                    clip: true
                    Behavior on width {
                        PropertyAnimation {
                            duration: 100
                        }
                    }
                    FluIconButton {
                        id: del_audio_btn
                        iconSource: FluentIcons.Delete
                        iconColor: FluColors.Red.normal
                        anchors.centerIn: parent
                        enabled: play_img.audioId !== -1
                        onClicked: {
                            GlobalVar.audio_id = -1
                        }
                    }
                }

                FluFilledButton {
                    id: apply_params
                    text: qsTr("Update Params")
                    enabled: GlobalVar.audio_id !== -1 && GlobalVar.au_cvt_width_rel !== 0 && GlobalVar.au_cvt_height_rel !== 0
                    anchors.top: parent.top
                    anchors.right: parent.right
                    onClicked: {
                        AudioProvider.set_param(play_img.audioId, paramConf.paramKey(), GlobalVar.au_cvt_width_rel, GlobalVar.au_cvt_height_rel)
                    }
                }

                Rectangle {
                    id: audio_size_box_rel
                    width: input_w_rel.width + 10 + h_txt_rel.width + 10 + input_h_rel.width
                    height: Math.max(input_w_rel.height, h_txt_rel.height, input_h_rel.height)
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: FluColors.Transparent

                    FluText {
                        text: qsTr("Width")
                        anchors.right: input_w_rel.left
                        anchors.rightMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    FluTextBox {
                        id: input_w_rel
                        cleanEnabled: false
                        text: String(GlobalVar.au_cvt_width_rel)
                        enabled: GlobalVar.audio_id !== -1
                        width: 100
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        validator: RegularExpressionValidator {
                            regularExpression: /[0-9]*/
                        }
                        onTextChanged: {
                            if (text === "") return
                            if (String(GlobalVar.au_cvt_width_rel) !== text)
                                GlobalVar.au_cvt_width_rel = Number(text)
                        }
                    }
                    FluText {
                        id: h_txt_rel
                        anchors.left: input_w_rel.right
                        anchors.leftMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("Height")
                    }
                    FluTextBox {
                        id: input_h_rel
                        cleanEnabled: false
                        text: String(GlobalVar.au_cvt_height_rel)
                        enabled: GlobalVar.audio_id !== -1
                        width: 100
                        anchors.left: h_txt_rel.right
                        anchors.leftMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        validator: RegularExpressionValidator {
                            regularExpression: /[0-9]*/
                        }
                        onTextChanged: {
                            if (text === "") return
                            if (String(GlobalVar.au_cvt_height_rel) !== text)
                                GlobalVar.au_cvt_height_rel = Number(text)
                        }
                    }
                    FluIconButton {
                        enabled: GlobalVar.audio_id !== -1 && GlobalVar.au_cvt_witdh_rel !== GlobalVar.au_def_width && GlobalVar.au_cvt_height_rel !== GlobalVar.au_def_height
                        anchors.left: input_h_rel.right
                        anchors.leftMargin: 10
                        anchors.top: audio_size_box_rel.top
                        iconSource: FluentIcons.Refresh

                        FluTooltip {
                            visible: parent.hovered
                            delay: 1000
                            text: qsTr("Reset Matrix Shape")
                        }

                        onClicked: {
                            GlobalVar.au_cvt_width_rel = GlobalVar.au_def_width
                            GlobalVar.au_cvt_height_rel = GlobalVar.au_def_height
                        }
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
