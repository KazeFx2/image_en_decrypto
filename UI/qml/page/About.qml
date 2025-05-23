import QtQuick 2.15
import QtQuick.Layouts
import FluentUI 1.0
import "../global"

FluScrollablePage {
    title: qsTr("About")

    FluText {
        // topPadding: 10
        text: qsTr("Author")
        font.pixelSize: 20
        font.bold: true
    }

    FluClip {
        width: 75
        height: width
        Layout.topMargin: 10
        Layout.alignment: Qt.AlignHCenter
        radius: [width / 2, width / 2, width / 2, width / 2]

        Image {
            anchors.fill: parent
            source: "qrc:/main/res/png/Kaze.png"
            sourceSize: Qt.size(80, 80)
        }

        MouseArea {
            width: parent.width
            height: parent.height
            onClicked: (Mouse) => {
                Qt.openUrlExternally("https://github.com/KazeFx2");
            }
        }

    }

    FluText {
        topPadding: 10
        text: "KazeFx"
        font.pixelSize: 20
        Layout.alignment: Qt.AlignHCenter
    }

    FluText {
        padding: 10
        Layout.alignment: Qt.AlignHCenter
        text: qsTr("Version") + " " + "1.1"
    }

    FluArea {
        Layout.topMargin: 10
        width: parent.width
        height: col_lay.height

        Column {
            id: col_lay

            width: parent.width

            FluText {
                padding: 10
                font.bold: true
                font.pixelSize: 15
                text: qsTr("What's this?")
            }

            FluText {
                width: parent.width
                wrapMode: Text.WordWrap
                padding: 10
                text: qsTr("    An image crypto system based on chaotic mapping, confusion and diffusion.(Also support video and audio)")
            }

            FluText {
                width: parent.width
                wrapMode: Text.WordWrap
                padding: 10
                text: qsTr("    Have fun! :)")
            }

            Row {

                FluText {
                    padding: 10
                    font.bold: true
                    font.pixelSize: 15
                    text: "GitHub: "
                }

                FluText {
                    padding: 10
                    text: "https://github.com/KazeFx2/image_en_decrypto"
                    color: FluColors.Blue.normal

                    MouseArea {
                        width: parent.width
                        height: parent.height
                        onClicked: {
                            Qt.openUrlExternally("https://github.com/KazeFx2/image_en_decrypto");
                        }
                    }
                }
            }
        }
    }

    FluText {
        topPadding: 10
        text: qsTr("Open Source Licenses")
        font.pixelSize: 20
        font.bold: true
    }

    ScrollableItems {
        height: col.height

        Column {
            id: col

            width: parent.width
            Component.onCompleted: {
                col.children[children.length - 1].is_last = true;
            }

            NamedUrlItem {
                ico: "qrc:/main/res/png/OpenCV.png"
                name: "OpenCV"
                url: "https://github.com/opencv/opencv"
                license: "Apache License 2.0"
            }

            NamedUrlItem {
                ico: "qrc:/main/res/png/FFmpeg.png"
                name: "FFmpeg"
                url: "https://ffmpeg.org"
                license: "GNU LGPL v2.1+"
            }

            NamedUrlItem {
                ico: "qrc:/main/res/ico/Qt.ico"
                name: "Qt Project"
                url: "https://github.com/qtproject"
                license: "GNU LGPL"
            }

            NamedUrlItem {
                ico: "qrc:/main/res/svg/FluentUI.svg"
                name: "FluenUI"
                url: "https://github.com/zhuzichu520/FluentUI"
                license: "MIT License"
            }

            NamedUrlItem {
                ico: "qrc:/main/res/ico/GitHub.ico"
                name: "FramelessHelper 2.x"
                url: "https://github.com/wangwenx190/framelesshelper"
                license: "MIT License"
            }

            NamedUrlItem {
                ico: "qrc:/main/res/ico/GitHub.ico"
                name: "ZXing-C++"
                url: "https://github.com/zhuzichu520/zxing-cpps"
                license: "Apache License 2.0"
            }

        }

    }

}
