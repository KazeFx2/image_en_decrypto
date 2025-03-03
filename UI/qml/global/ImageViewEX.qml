import FluentUI
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Templates as T
import Qt5Compat.GraphicalEffects 1.0

FluRectangle {
    id: conf

    property bool hovered: false
    property double factor: 1.0
    property double factorStep: 0.1
    property double maxFa: 5.0
    property double ori: 1.0
    property double wid: 1.0
    property int i_width: 0
    property int i_height: 0
    property string source: ""


    Behavior on factor {
        PropertyAnimation {duration: FluTheme.animationEnabled ? 100 : 0}
    }

    width: 600
    height: 300

    onSourceChanged: {factor = 1.0}

    layer.enabled: true
    layer.effect: OpacityMask {
        maskSource: Rectangle {
            width: conf.width
            height: conf.height
            radius: 5
        }
    }

    color: FluColors.Grey120.alpha(0.5)

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onEntered: function() {
            conf.hovered = true
        }

        onExited: function() {
            conf.hovered = false
        }
    }

    Flickable {
        id: flick
        anchors.fill: parent

        ScrollBar.vertical: FluScrollBar {}
        ScrollBar.horizontal: FluScrollBar {}
        contentWidth: factor == 1.0 ? width : img.width
        contentHeight: factor == 1.0 ? height : img.height

        Behavior on contentWidth {
            PropertyAnimation {duration: FluTheme.animationEnabled ? 100 : 0}
        }
        Behavior on contentHeight {
            PropertyAnimation {duration: FluTheme.animationEnabled ? 100 : 0}
        }

        onWidthChanged: {
            img.reCalc()
        }

        onHeightChanged: {
            img.reCalc()
        }

        FluImage {
            id: img

            width: i_width * factor
            height: i_height * factor

            anchors.centerIn: parent

            function reCalc() {
                ori = sourceSize.width / sourceSize.height
                wid = flick.width / flick.height
                if (ori > wid) {
                    i_width = flick.width
                    i_height = i_width / ori
                } else {
                    i_width = flick.height * ori
                    i_height = flick.height
                }
            }

            onSourceSizeChanged: {
                reCalc()
            }
            source: conf.source

        }

    }

    FluRectangle {

        width: childrenRect.width
        height: childrenRect.height

        color: FluColors.Transparent

        anchors.right: parent.right
        anchors.bottom: bottom_pad.top

        opacity: conf.hovered || btn_out.hovered || btn_in.hovered ? 0.5 : 0

        Behavior on opacity {
            PropertyAnimation {duration: FluTheme.animationEnabled ? 100 : 0}
        }

        Row {

            FluIconButton{
                id: btn_out
                iconSource:FluentIcons.ZoomOut
                color: hovered && enabled ? FluColors.Grey120.alpha(0.8) : FluColors.Grey110
                enabled: factor != 1.0
                onClicked: {
                    if (factor > 1.0) {
                        let tmp = factor - factorStep * 5
                        factor = tmp < 1.0 ? 1.0 : tmp
                    }
                }
            }

            FluRectangle {
                width: 15
                height: 15
                color: FluColors.Transparent
            }

            FluIconButton{
                id: btn_in
                iconSource:FluentIcons.ZoomIn
                color: hovered && enabled ? FluColors.Grey120.alpha(0.8) : FluColors.Grey110
                enabled: factor != maxFa
                onClicked: {
                    if (factor < maxFa) {
                        let tmp = factor + factorStep * 5
                        factor = tmp > maxFa ? maxFa : tmp
                    }
                }
            }

            FluRectangle {
                width: 15
                height: 15
                color: FluColors.Transparent
            }
        }
    }

    FluRectangle {
        id: bottom_pad
        width: parent.width
        height: 15

        color: FluColors.Transparent

        anchors.bottom: parent.bottom
    }

}
