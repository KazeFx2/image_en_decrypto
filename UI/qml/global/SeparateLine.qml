import QtQuick
import QtQuick.Layouts
import FluentUI

Rectangle {

    property int padding: 5

    width: parent.width
    height: childrenRect.height

    color: FluColors.Transparent

    Column {

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
