pragma Singleton

import QtQuick 2.15
import FluentUI 1.0

FluObject{

    property var navigationView

    id:footer_items

    FluPaneItemSeparator{}

    FluPaneItem{
        title:qsTr("Settings")
        icon:FluentIcons.Settings
        url:"qrc:/main/qml/page/Settings.qml"
        onTap:{
            navigationView.push(url)
        }
    }

    FluPaneItem{
        title:qsTr("About")
        icon:FluentIcons.Contact
        url:"qrc:/main/qml/page/About.qml"
        onTap:{
            navigationView.push(url)
        }
    }

}
