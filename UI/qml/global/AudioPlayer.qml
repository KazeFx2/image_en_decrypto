import QtQuick 2.15
import QtQuick.Templates 2.15 as T
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs
import QtCore
import FluentUI 1.0
import "qrc:/main/src/js/tools.js" as Tools
import main 1.0

FluRectangle {
    id: play_img
    width: parent.width
    height: 300
    color: FluColors.Transparent
    property int audioId: -1
    property int idPrev: -1
    property bool paused: true
    property bool hovered: false

    Component.onDestruction: {
    }

    onAudioIdChanged: {
        AudioProvider.delAudio(idPrev)
        idPrev = audioId
        procedure.max = AudioProvider.total_msec(audioId)
        procedure.value = AudioProvider.get_msec(audioId) * procedure.to / procedure.max
        update_decode_type()
        update_cuda()
        paused = AudioProvider.get_pause(audioId)
    }

    function update_decode_type() {
        switch(AudioProvider.get_decode_type(audioId)){
        case DecodeType.Raw:
            decode_type.text = option_raw.text
            break
        case DecodeType.Encrypt:
            decode_type.text = option_encrypt.text
            break
        case DecodeType.Decrypt:
            decode_type.text = option_decrypt.text
            break
        }
    }

    function update_cuda() {
        cuda.checked = AudioProvider.get_cuda(audioId)
    }

    FluStatusLayout {
        id: loading
        visible: shouldLoad && show_load.checked
        anchors.fill: parent
        property bool shouldLoad: false
    }

    Connections {
        target: AudioProvider
        function onAudioUpdated(id, msec) {
            if (id === play_img.audioId) {
                if (!procedure.pressed)
                    procedure.set_msec(msec)
                procedure.current_msec = msec
            }
        }
        function onAudioPaused(id) {
            if (id === play_img.audioId) {
                play_img.paused = true
            }
        }
        function onAudioResumed(id) {
            if (id === play_img.audioId) {
                play_img.paused = false
            }
        }
        function onAudioDecodeTypeChanged(id) {
            if (id === play_img.audioId) {
                play_img.update_decode_type()
            }
        }
        function onAudioCudaChanged(id) {
            if (id === play_img.audioId) {
                play_img.update_cuda()
            }
        }
    }

    Connections {
        target: VideoProvider
        function onVideoLoading(id) {
            if (id === play_img.mid) {
                loading.shouldLoad = true
            }
        }
        function onVideoLoaded(id) {
            if (id === play_img.mid) {
                loading.shouldLoad = false
            }
        }
    }

    FluIcon {
        iconSource: FluentIcons.Audio
        anchors.centerIn: parent
        iconSize: 150
    }

    MouseArea {
        anchors.fill: control_panel
        hoverEnabled: true

        onEntered: {
            play_img.hovered = true
        }
        onExited: {
            play_img.hovered = false
        }
    }

    Rectangle {
        id: control_panel
        visible: play_img.hovered || procedure.hovered || prev.hovered || play.hovered || next.hovered || decode_type.hovered || decode_type.backShadow.hovered || cuda.hovered || show_load.hovered
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: 85
        color: FluColors.Transparent
        Rectangle {
            width: parent.width - 20
            height: parent.height - 10
            anchors.centerIn: parent
            radius: 5
            color: FluColors.Grey120.alpha(0.8)
            clip: true

            FluText {
                id: pos
                anchors.left: parent.left
                anchors.leftMargin: 5
                y: procedure.height - 6
                font.pixelSize: 12
                text: procedure.text
                onTextChanged: {
                    remain.timeLeft = procedure.max / 1000 - procedure.i_value
                }
            }

            FluText {
                id: remain
                property int timeLeft: 0
                property int min: 0
                property int sec: 0
                anchors.right: parent.right
                anchors.rightMargin: 5
                y: procedure.height - 6
                font.pixelSize: 12
                onTimeLeftChanged: {
                    min = timeLeft / 60
                    sec = timeLeft % 60
                }
                text: "-" + String(min).padStart(2, '0') + ":" + String(sec).padStart(2, '0')
            }

            Column {
                Rectangle {
                    width: parent.parent.width
                    height: procedure.height
                    color: FluColors.Transparent

                    TimeSlider {
                        id: procedure
                        width: parent.width
                        to: 1000
                        property double current_msec: 0.0

                        function set_msec(msec) {
                            procedure.value = procedure.to <= 1 ? 0 : msec * (procedure.to - 1) / procedure.max
                        }

                        onPressedChanged: {
                            if (!pressed)
                                AudioProvider.goto_msec(play_img.audioId, to <= 1 ? 0 : max * value / (to - 1))
                        }

                        onMaxChanged: {
                            remain.timeLeft = procedure.max / 1000 - procedure.i_value
                        }
                    }
                }

                // FluRectangle {width: parent.parent.width; height: 5; color: FluColors.Transparent}

                Rectangle {
                    width: parent.parent.width
                    height: Math.max(prev.height, play.height, next.height, decode_type.height, cuda.height)
                    color: FluColors.Transparent

                    // color: FluColors.Transparent

                    Rectangle {
                        color: FluColors.Transparent
                        width: prev.width + play.width + next.width + 20
                        height: Math.max(prev.height, play.height, next.height)

                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter

                        Row {
                            FluIconButton {
                                id: prev
                                iconSource: FluentIcons.Rewind
                                onClicked: {
                                    const delta = procedure.max / 100 < 15000 ? procedure.max / 100 : 15000
                                    const new_val = procedure.current_msec - delta < 0 ? 0 : procedure.current_msec - delta
                                    AudioProvider.goto_msec(play_img.audioId, new_val)
                                    procedure.set_msec(new_val)
                                }
                            }
                            FluRectangle {width: 10; height: 10; color: FluColors.Transparent}
                            FluIconButton {
                                id: play
                                iconSource: play_img.paused ? FluentIcons.Play : FluentIcons.Pause
                                onClicked: {
                                    if (play_img.paused) {
                                        AudioProvider.resume(play_img.audioId)
                                    } else {
                                        AudioProvider.pause(play_img.audioId)
                                    }
                                }
                            }
                            FluRectangle {width: 10; height: 10; color: FluColors.Transparent}
                            FluIconButton {
                                id: next
                                iconSource: FluentIcons.FastForward
                                onClicked: {
                                    const delta = procedure.max / 100 < 15000 ? procedure.max / 100 : 15000
                                    const new_val = procedure.current_msec + delta > procedure.max ? procedure.max : procedure.current_msec + delta
                                    AudioProvider.goto_msec(play_img.audioId, new_val)
                                    procedure.set_msec(new_val)
                                }
                            }
                        }
                    }

                    Rectangle {id: __right_pad; width: 10; height: 10; color: FluColors.Transparent; anchors.right: parent.right}

                    MyUpMenu{
                        id: decode_type
                        text: option_raw.text

                        anchors {
                            right: __right_pad.left
                            verticalCenter: parent.verticalCenter
                        }

                        FluMenuItem{
                            id: option_raw
                            text: qsTr("Raw")
                            onClicked: {
                                AudioProvider.set_type(play_img.audioId, DecodeType.Raw)
                            }
                        }
                        FluMenuItem{
                            id: option_encrypt
                            text: qsTr("Encrypt")
                            onClicked: {
                                AudioProvider.set_type(play_img.audioId, DecodeType.Encrypt)
                            }
                        }
                        FluMenuItem{
                            id: option_decrypt
                            text: qsTr("Decrypt")
                            onClicked: {
                                AudioProvider.set_type(play_img.audioId, DecodeType.Decrypt)
                            }
                        }
                    }

                    Rectangle {id: __mid_pad; width: 5; height: 5; color: FluColors.Transparent; anchors.right: decode_type.left}

                    FluCheckBox {
                        id: cuda
                        enabled: Crypto.cudaAvailable()
                        text: qsTr("Cuda")
                        textRight: false
                        anchors.right: __mid_pad.left
                        anchors.verticalCenter: parent.verticalCenter

                        onCheckedChanged: {
                            AudioProvider.set_cuda(play_img.audioId, checked)
                        }
                    }

                    FluCheckBox {
                        id: show_load
                        checked: true
                        text: qsTr("Show Loading")
                        textRight: false
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: 10
                    }

                }
            }
        }
    }

}
