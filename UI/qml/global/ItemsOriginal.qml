pragma Singleton

import QtQuick 2.15
import FluentUI 1.0

FluObject{

    property var navigationView

    function rename(item, newName){
        if(newName && newName.trim().length>0){
            item.title = newName;
        }
    }

    FluPaneItem{
        id:item_home
        title: qsTr("Home")
        icon: FluentIcons.Home
        url: "qrc:/main/qml/page/Main.qml"
        onTap: {
            navigationView.push(url)
        }
    }

}
