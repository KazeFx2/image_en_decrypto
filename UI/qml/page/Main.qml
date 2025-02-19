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
            height: parent.height - title.height - bottom_control.height - pad.height
            // paddings: 10
            x: 10

            MyFluPivot {
                id: main_menu

                width: parent.width
                height: parent.height
                currentIndex: 0

                FluPivotItem {
                    title: qsTr("Image")

                    contentItem: FluLoader {
                        id: sys_st

                        width: parent.width
                        height: parent.height
                        source: "qrc:/main/qml/page/crypto/CryptoImage.qml"
                        onVisibleChanged: {
                            if (visible) {
                                source = "qrc:/main/qml/page/crypto/CryptoImage.qml";
                                bottom_control.refreshbuttons();
                            } else {
                                source = "";
                                bottom_control.disablebuttons();
                            }
                        }
                        onLoaded: {
                            if (visible) {
                                // item.refreshbuttons.connect(bottom_control.refreshbuttons);
                                global_ctx.refreshsub.connect(item.refresh);
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
                            else
                                source = "";
                        }
                    }

                }

            }

        }

        Row {
            id: bottom_control

            function refreshbuttons() {
                // prev.disabled = CppBankAlgorithm.isBegin();
                // next.disabled = CppBankAlgorithm.isEnd();
            }

            function disablebuttons() {
                prev.disabled = true;
                next.disabled = true;
            }

            width: parent.width
            padding: 10

            FluFilledButton {
                id: prev

                text: qsTr("PrevStatus")
                // disabled: main_menu.currentIndex !== 0 || CppBankAlgorithm.isBegin()
                onClicked: {
                    // var ret = CppBankAlgorithm.prevStatus();
                    // if (ret) {
                    //     global_ctx.refreshsub();
                    //     bottom_control.refreshbuttons();
                    // }
                }
            }

            FluRectangle {
                height: prev.height
                width: (parent.width - prev.width - next.width - reset.width - parent.padding * 2) / 2
                color: FluColors.Transparent
            }

            FluFilledButton {
                id: reset

                text: qsTr("Reset")
                onClicked: {
                    // CppBankAlgorithm.reset();
                    // if (main_menu.currentIndex === 0)
                    //     global_ctx.refreshsub();

                    // main_menu.currentIndex = 0;
                }
            }

            FluRectangle {
                height: prev.height
                width: (parent.width - prev.width - next.width - reset.width - parent.padding * 2) / 2
                color: FluColors.Transparent
            }

            FluFilledButton {
                id: next

                text: qsTr("NextStatus")
                // disabled: main_menu.currentIndex !== 0 || CppBankAlgorithm.isEnd()
                onClicked: {
                    // var ret = CppBankAlgorithm.nextStatus();
                    // if (ret) {
                    //     global_ctx.refreshsub();
                    //     bottom_control.refreshbuttons();
                    // }
                }
            }

        }

    }

}
