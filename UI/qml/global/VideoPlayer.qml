import QtQuick 2.15
import QtQuick.Templates 2.15 as T
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs
import QtCore
import FluentUI 1.0
import "qrc:/main/src/js/tools.js" as Tools
import main 1.0

FluImage {
    id: play_img
    width: parent.width
    height: 300
    fillMode: Image.Pad
    property int ct: 0
    property string url: ""
    property string url_prev: ""
    property string mid: url.split("/").pop()
    property bool paused: true
    source: url === "" ? "" : url + "/" + String(ct)
    property bool hovered: false

    Component.onDestruction: {
        // VideoProvider.delVideo(url)
    }

    onUrlChanged: {
        VideoProvider.delVideo(url_prev)
        url_prev = url
        procedure.max = VideoProvider.total_msec(url)
        procedure.value = VideoProvider.get_msec(url) * procedure.to / procedure.max
        update_decode_type()
        update_cuda()
        paused = VideoProvider.get_pause(url)
        // console.log(procedure.max)
    }

    onWidthChanged: {
        VideoProvider.set_wh(url, width, height)
    }
    onHeightChanged: {
        VideoProvider.set_wh(url, width, height)
    }

    function update_decode_type() {
        switch(VideoProvider.get_decode_type(url)){
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
        cuda.checked = VideoProvider.get_cuda(url)
    }

    FluStatusLayout {
        id: loading
        visible: shouldLoad && show_load.checked
        anchors.fill: parent
        property bool shouldLoad: false
    }

    Connections {
        target: VideoProvider
        function onVideoUpdated(id, msec) {
            if (id === play_img.mid) {
                play_img.ct++
                if (!procedure.pressed)
                    procedure.set_msec(msec)
                procedure.current_msec = msec
            }
        }
        function onVideoPaused(id) {
            if (id === play_img.mid) {
                play_img.paused = true
            }
        }
        function onVideoResumed(id) {
            if (id === play_img.mid) {
                play_img.paused = false
            }
        }
        function onVideoDecodeTypeChanged(id) {
            if (id === play_img.mid) {
                play_img.update_decode_type()
            }
        }
        function onVideoCudaChanged(id) {
            if (id === play_img.mid) {
                play_img.update_cuda()
            }
        }
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
                                VideoProvider.goto_msec(play_img.url, to <= 1 ? 0 : max * value / (to - 1))
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
                                    const new_val = procedure.current_msec - 15000 < 0 ? 0 : procedure.current_msec - 15000
                                    VideoProvider.goto_msec(play_img.url, new_val)
                                    procedure.set_msec(new_val)
                                }
                            }
                            FluRectangle {width: 10; height: 10; color: FluColors.Transparent}
                            FluIconButton {
                                id: play
                                iconSource: play_img.paused ? FluentIcons.Play : FluentIcons.Pause
                                onClicked: {
                                    if (play_img.paused) {
                                        VideoProvider.resume(play_img.url)
                                    } else
                                        VideoProvider.pause(play_img.url)
                                }
                            }
                            FluRectangle {width: 10; height: 10; color: FluColors.Transparent}
                            FluIconButton {
                                id: next
                                iconSource: FluentIcons.FastForward
                                onClicked: {
                                    const new_val = procedure.current_msec + 15000 > procedure.max ? procedure.max : procedure.current_msec + 15000
                                    VideoProvider.goto_msec(play_img.url, new_val)
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
                                VideoProvider.set_type(play_img.url, DecodeType.Raw)
                            }
                        }
                        FluMenuItem{
                            id: option_encrypt
                            text: qsTr("Encrypt")
                            onClicked: {
                                VideoProvider.set_type(play_img.url, DecodeType.Encrypt)
                            }
                        }
                        FluMenuItem{
                            id: option_decrypt
                            text: qsTr("Decrypt")
                            onClicked: {
                                VideoProvider.set_type(play_img.url, DecodeType.Decrypt)
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
                            VideoProvider.set_cuda(play_img.url, checked)
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
