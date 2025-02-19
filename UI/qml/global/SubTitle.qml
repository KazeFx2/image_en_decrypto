import QtQuick
import FluentUI

Item {

    id: conf
    property string text: ""

    width: childrenRect.width
    height: childrenRect.height

    FluText {
        topPadding: 5
        leftPadding: 10
        bottomPadding: 5
        font.bold: true
        font.pixelSize: 16
        text: conf.text
    }

}
