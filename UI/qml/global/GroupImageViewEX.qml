import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import Qt5Compat.GraphicalEffects 1.0
import FluentUI 1.0
import "."

Rectangle {

    property var sourceList: []

    property string active_url: ""
    property string active_name: ""
    property bool detailed: false
    property bool edit: false

    signal remove(var source, var idx)
    signal add()

    clip: true

    onSourceListChanged: {
        staggered_view.clear()
        for (let i = 0; i < sourceList.length; i++)
            list_model.append(sourceList[i])
        if (edit) {
            list_model.append({name: qsTr("Add Image")})
        }
    }

    function set(idx, obj) {
        list_model.set(idx, obj)
    }

    width: 400
    height: 400
    color: FluColors.Grey120.alpha(0.3)
    radius: 5

    ListModel{
        id: list_model
    }

    Flickable{
        id: scroll
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        topMargin: 10
        leftMargin: 10
        rightMargin: 10
        bottomMargin: 10

        height: detailed ? 0 : parent.height
        boundsBehavior:Flickable.StopAtBounds
        contentHeight: staggered_view.implicitHeight
        clip: true
        ScrollBar.vertical: FluScrollBar {}
        MyStaggeredLayout{
            id:staggered_view
            width: parent.width
            itemWidth: 80
            model:list_model
            delegate: Rectangle{
                id: item_rect

                property int padding: 5
                property bool hovered: false
                height: 100
                width: parent.itemWidth + padding * 2
                color: FluColors.Transparent

                FluTooltip {
                        visible: item_rect.hovered
                        text: model.name
                        delay: 1000
                }

                Rectangle {
                    id: inner_box

                    width: parent.width - item_rect.padding * 2
                    height: parent.height - item_rect.padding * 2

                    color: FluColors.Transparent
                    anchors.centerIn: parent

                    Rectangle {
                        id: img_rect

                        anchors.top: parent.top
                        width: parent.width
                        height: width
                        color: FluColors.Transparent

                        layer.enabled: true
                        layer.effect: OpacityMask {
                            maskSource: Rectangle {
                                width: img_rect.width
                                height: img_rect.height
                                radius: 5
                            }
                        }

                        FluImage {
                            id: img
                            visible: model.source !== undefined
                            width: parent.width
                            height: parent.width
                            fillMode: Image.PreserveAspectCrop
                            property string url: model.source !== undefined ? model.source : ""
                            property int ct: 0
                            property string mid: url.split("/").pop()
                            source: model.source !== undefined ? ((url.length > 20 && url.slice(0, 20) === "image://MemoryImage/") ? url + "/" + String(ct) : url) : source

                            Connections {
                                target: MemImage
                                function onImageUpdated(id) {
                                    if (img.url.length > 20 && img.url.slice(0, 20) === "image://MemoryImage/" && id === img.mid) {
                                        img.ct++
                                    }
                                }
                            }
                        }

                        FluIcon {
                            width: parent.width
                            height: width
                            visible: model.source === undefined
                            anchors.centerIn: parent
                            iconSize: parent.width / 2
                            iconSource: edit ? FluentIcons.Add : FluentIcons.StatusCircleBlock
                        }
                    }

                    FluText {
                        width: parent.width
                        anchors.bottom: parent.bottom
                        text: model.name
                        elide: Text.ElideRight
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    color: FluColors.Grey110.alpha(item_rect.hovered ? 0.2 : 0.0)

                    MouseArea {
                        id: mouse_area
                        anchors.fill: parent

                        hoverEnabled: true
                        onEntered: {
                            item_rect.hovered = true
                        }
                        onExited: {
                            item_rect.hovered = false
                        }
                        onClicked: {
                            if (model.source === undefined) {
                                if (edit)
                                    add()
                            } else {
                                active_url = model.source
                                active_name = model.name
                                detailed = true
                            }
                        }
                    }

                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: Rectangle {
                            width: item_rect.width
                            height: item_rect.height
                            radius: 5
                        }
                    }
                }

                Rectangle {
                    id: remove_btn

                    property bool hovered: false
                    visible: edit && model.source !== undefined && (item_rect.hovered || remove_btn.hovered)
                    width: parent.width / 5
                    height: width
                    radius: width / 2
                    color: FluColors.Red.normal.alpha(remove_btn.hovered ? 0.7 : 0.5)

                    anchors {
                        horizontalCenter: parent.right
                        verticalCenter: parent.top
                    }

                    FluIcon {
                        anchors.centerIn: parent
                        iconSize: parent.width * 0.8
                        iconSource: FluentIcons.Cancel
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled:true

                        onEntered: {remove_btn.hovered = true}
                        onExited: {remove_btn.hovered = false}
                        onClicked: {
                            remove(String(model.source), index)
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: detailed ? nav_btn.height : 0
        clip: true
        color: FluColors.Transparent

        FluIconButton {
            id: nav_btn
            anchors {
                top: parent.top
                left: parent.left
            }
            onClicked: {
                detailed = false
            }
            iconSource: FluentIcons.Back
        }

        FluText {
            anchors {
                top: parent.top
                left: nav_btn.right
                right: parent.right
                bottom: nav_btn.bottom
            }
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: active_name
            elide: Text.ElideRight
        }
    }

    ImageViewEX {
        id: ext_img
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        height: detailed ? parent.height - nav_btn.height - 5 : 0

        property string url: active_url
        property int ct: 0
        property string mid: url.split("/").pop()
        source: url.length > 20 && url.slice(0, 20) === "image://MemoryImage/" ? url + "/" + String(ct) : url

        Connections {
            target: MemImage
            function onImageUpdated(id) {
                if (ext_img.url.length > 20 && ext_img.url.slice(0, 20) === "image://MemoryImage/" && id === ext_img.mid) {
                    ext_img.ct++
                }
            }
        }
    }
}
