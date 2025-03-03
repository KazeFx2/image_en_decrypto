import QtQuick
import QtQuick.Layouts
import FluentUI

Rectangle {

    property int padding: 5

    width: horizontal ? parent.width : childrenRect.width
    height: horizontal ? childrenRect.height : parent.height

    color: FluColors.Transparent

    property bool horizontal: true

    Row {
        visible: !horizontal

        width: childrenRect.width
        height: parent.height

        Rectangle {
            width: padding
            height: parent.height
            color: FluColors.Transparent
        }

        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            width: 1
            height: parent.height
            radius: 1
            color: FluColors.Grey110
        }

        Rectangle {
            width: padding
            height: parent.height
            color: FluColors.Transparent
        }

    }

    Column {

        visible: horizontal

        width: parent.width
        height: childrenRect.height

        Rectangle {
            width: parent.width
            height: padding
            color: FluColors.Transparent
        }

        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            width: parent.width
            height: 1
            radius: 1
            color: FluColors.Grey110
        }

        Rectangle {
            width: parent.width
            height: padding
            color: FluColors.Transparent
        }

    }

}
