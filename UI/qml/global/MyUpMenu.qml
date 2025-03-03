import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic
import QtQuick.Window
import FluentUI

FluTextButton {
    id: control
    default property alias contentData: menu.contentData
    rightPadding:35
    verticalPadding: 0
    horizontalPadding:12

    property alias backShadow: shadow

    textColor: {
        if(FluTheme.dark){
            if(!enabled){
                return Qt.rgba(131/255,131/255,131/255,1)
            }
            if(pressed){
                return Qt.rgba(162/255,162/255,162/255,1)
            }
            return Qt.rgba(1,1,1,1)
        }else{
            if(!enabled){
                return Qt.rgba(160/255,160/255,160/255,1)
            }
            if(pressed){
                return Qt.rgba(96/255,96/255,96/255,1)
            }
            return Qt.rgba(0,0,0,1)
        }
    }

    FluIcon{
        iconSource:FluentIcons.ChevronUp
        iconSize: 15
        anchors{
            right: parent.right
            rightMargin: 10
            verticalCenter: parent.verticalCenter
        }
        iconColor:control.textColor
    }
    Item{
        id: d
        property var window: Window.window
    }
    onClicked: {
        if(menu.count !==0){
            var pos = control.mapToItem(null, 0, 0)
            var containerHeight = menu.count*36
            if(d.window.height>pos.y+control.height+containerHeight){
                menu.y = -containerHeight // menu.y = control.height
            }else if(pos.y>containerHeight){
                menu.y = -containerHeight
            }else{
                // menu.y = d.window.height-(pos.y+containerHeight)
                menu.y = -pos.y
            }
            menu.open()
        }
    }
    FluMenu{
        id:menu
        modal:true
        width: control.width

        onVisibleChanged: {
            shadow.hovered = visible
        }

        background: Rectangle {
            implicitWidth: 150
            implicitHeight: 36
            color:FluTheme.dark ? Qt.rgba(45/255,45/255,45/255,1) : Qt.rgba(252/255,252/255,252/255,1)
            border.color: FluTheme.dark ? Qt.rgba(26/255,26/255,26/255,1) : Qt.rgba(191/255,191/255,191/255,1)
            border.width: 1
            radius: 5

            FluShadow{
                id: shadow
                property bool hovered: false
            }
        }
    }
}
