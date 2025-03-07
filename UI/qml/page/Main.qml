import QtQuick 2.15
import QtQuick.Layouts 1.15
import FluentUI 1.0
import "../global"

Rectangle {
    id: global_ctx

    property var src_names: []

    signal refreshsub()

    width: parent.width
    height: parent.height
    color: FluColors.Transparent

    Column {
        id: col

        height: parent.height
        width: parent.width

        FluText {
            id: title

            leftPadding: 10
            text: qsTr("ImageCrypto")
            font.pixelSize: 28
            font.bold: true
        }

        Rectangle {
            id: pad

            width: parent.width
            height: 10
            color: FluColors.Transparent
        }

        FluArea {
            id: area

            width: parent.width - 20
            height: parent.height - title.height - pad.height - 10
            // paddings: 10
            x: 10

            MyFluPivot {
                id: main_menu

                width: parent.width
                height: parent.height
                currentIndex: GlobalVar.pivot_idx

                onCurrentIndexChanged: {
                    GlobalVar.pivot_idx = currentIndex
                }

                FluPivotItem {
                    title: qsTr("Image")

                    contentItem: FluLoader {
                        id: sys_st
                        anchors.fill: parent
                        source: "qrc:/main/qml/page/crypto/CryptoImage.qml"
                        onVisibleChanged: {
                            if (visible) {
                                // source = "qrc:/main/qml/page/crypto/CryptoImage.qml";
                                // bottom_control.refreshbuttons();
                            } else {
                                // source = "";
                                // bottom_control.disablebuttons();
                            }
                        }
                        onLoaded: {
                            if (visible) {
                                // item.refreshbuttons.connect(bottom_control.refreshbuttons);
                                // global_ctx.refreshsub.connect(item.refresh);
                            }
                        }
                    }

                }

                FluPivotItem {
                    title: qsTr("Video")

                    contentItem: FluLoader {
                        width: parent.width
                        height: parent.height
                        source: "qrc:/main/qml/page/crypto/CryptoVideo.qml"
                        onVisibleChanged: {
                            if (visible)
                                source = "qrc:/main/qml/page/crypto/CryptoVideo.qml";
                            else {
                                // source = "";
                            }
                        }
                    }

                }

            }

        }

    }

}
