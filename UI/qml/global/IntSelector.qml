import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FluentUI
import "."
import "qrc:/main/src/js/tools.js" as Tool

FluButton {
    property int min: 0
    property int max: 100
    property int current: min
    property int steps: 1000

    property string cancelText: qsTr("Cancel")
    property string okText: qsTr("OK")
    property var chkValue: function(value) {return ""}

    id:control
    implicitHeight: 30
    implicitWidth: 300

    function random() {
        d.value =  Math.random() * (control.max - control.min) + control.min
    }

    property alias d_value: d.value

    Item {
        id: d
        visible: false
        property bool change
        property int value

        onValueChanged: {
            sld.value = (value - control.min) * control.steps / (control.max - control.min)
            txt.text = String(value)
        }
    }

    onClicked: {
        d.change = false
        d.value = current
        sld.value = (d.value - control.min) * control.steps / (control.max - control.min)
        txt.text = String(d.value)
        popup.showPopup()
    }
    FluText{
        id: float_value
        anchors{
            left: parent.left
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        text: String(current)
        color: control.textColor
    }
    Menu{
        id:popup
        modal: true
        Overlay.modal: Item {}
        enter: Transition {
            reversible: true
            NumberAnimation {
                property: "opacity"
                from:0
                to:1
                duration: FluTheme.animationEnabled ? 83 : 0
            }
        }
        exit:Transition {
            NumberAnimation {
                property: "opacity"
                from:1
                to:0
                duration: FluTheme.animationEnabled ? 83 : 0
            }
        }
        background:Rectangle{
            radius: 5
            color: FluTheme.dark ? Qt.rgba(43/255,43/255,43/255,1) : Qt.rgba(1,1,1,1)
            border.color: FluTheme.dark ? Qt.rgba(26/255,26/255,26/255,1) : Qt.rgba(191/255,191/255,191/255,1)
            FluShadow{
                radius: 5
            }
        }
        contentItem: Item{
            id:container
            implicitHeight: 205
            implicitWidth: flickable.width
            MouseArea{
                anchors.fill: parent
            }
            Flickable{
                id: flickable
                clip: true
                width: 300
                height: parent.height
                ScrollBar.vertical: FluScrollBar {}
                boundsBehavior: Flickable.StopAtBounds
                contentHeight: flick_container.height
                ColumnLayout{
                    id: flick_container
                    width: parent.width
                    height: childrenRect.height

                    FluRectangle {
                        height: 5
                        width: height
                        color: FluColors.Transparent
                    }

                    FluRectangle {
                        width: parent.width
                        height: childrenRect.height
                        color: FluColors.Transparent

                        ModSlider {
                            id: sld
                            useInt: true
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: parent.width - 20
                            min: control.min
                            max: control.max
                            to: control.steps

                            onValueChanged: {
                                d.value = value * (control.max - control.min) / control.steps + control.min
                            }
                        }
                    }

                    FluRectangle {
                        height: 5
                        width: height
                        color: FluColors.Transparent
                    }

                    FluRectangle {
                        width: parent.width
                        height: childrenRect.height
                        color: FluColors.Transparent

                        FluTextBox {
                            id: txt
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: parent.width - 20
                            validator: RegularExpressionValidator {
                                regularExpression: /-?[0-9]*/
                            }

                            onTextChanged: {
                                let tmp = Number(text)
                                if (isNaN(tmp)) return
                                if (tmp < control.min) {
                                    d.value = control.min
                                    text = String(d.value)
                                } else if (tmp > control.max) {
                                    d.value = control.max
                                    text = String(d.value)
                                } else
                                    d.value = tmp
                            }
                        }
                    }

                    FluRectangle {
                        height: 5
                        width: height
                        color: FluColors.Transparent
                    }

                    FluRectangle {
                        width: parent.width
                        height: childrenRect.height
                        color: FluColors.Transparent

                        FluFilledButton {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: qsTr("Generate Randomly")
                            width: parent.width - 20

                            onClicked: {
                                random()
                            }
                        }
                    }
                }
            }

            Rectangle{
                id:layout_actions
                height: 60
                color: FluTheme.dark ? Qt.rgba(32/255,32/255,32/255,1) : Qt.rgba(243/255,243/255,243/255,1)
                border.color: FluTheme.dark ? Qt.rgba(26/255,26/255,26/255,1) : Qt.rgba(191/255,191/255,191/255,1)
                radius: 5
                anchors{
                    bottom:parent.bottom
                    left: parent.left
                    right: parent.right
                }
                Item {
                    id:divider
                    width: 1
                    height: parent.height
                    anchors.centerIn: parent
                }
                FluButton{
                    anchors{
                        left: parent.left
                        leftMargin: 20
                        rightMargin: 10
                        right: divider.left
                        verticalCenter: parent.verticalCenter
                    }
                    text: control.cancelText
                    onClicked: {
                        d.change = false
                        popup.close()
                    }
                }
                FluFilledButton{
                    anchors{
                        right: parent.right
                        left: divider.right
                        rightMargin: 20
                        leftMargin: 10
                        verticalCenter: parent.verticalCenter
                    }
                    text: control.okText
                    onClicked: {
                        let i = chkValue(d.value)
                        if (typeof(i) === 'string' && i !== '')
                            showError(Tool.format(qsTr("Illegal value: {0}, {1}"), d.value, i))
                        else {
                            d.change = true
                            popup.close()
                        }
                    }
                }
            }
        }
        y:35
        function showPopup() {
            popup.open()
        }
        onClosed: {
            if (d.change)
                current = d.value
        }
    }
}
