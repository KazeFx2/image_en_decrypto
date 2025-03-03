import QtQuick 2.15
import QtQuick.Templates 2.15 as T
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs
import QtCore
import FluentUI 1.0
import "qrc:/main/src/js/tools.js" as Tools
import "../../global"

FluScrollablePage {
    id: crypto_image_page

    property var list_model: []
    property var out_model: []
    property bool onePic: true
    property bool addition: false

    Connections {
        target: img_in_mul
        function onRemove(source, idx) {
            list_model = list_model.filter(item => String(item.source) !== String(source))
            out_model = out_model.filter(item => String(item.source) !== String(out_model[idx].source))
        }
        function onAdd(){
            addition = true
            imagePickDialog.open()
        }
    }

    Component.onCompleted: {
    }
    Component.onDestruction: {
        for (var i = 0; i < out_model.length; i++)
            Crypto.removeImage(out_model[i].source)
    }

    width: parent.width
    height: parent.height

    FileDialog {
        id: imagePickDialog
        title: qsTr("File Select")
        acceptLabel: qsTr("Open")
        rejectLabel: qsTr("Cancel")
        fileMode: FileDialog.OpenFiles
        currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
        defaultSuffix: "jpg"
        nameFilters: [qsTr("Image Files (*.jpg *.jpeg *.png)")]
        flags: FileDialog.ReadOnly
        /*
            FileDialog.DontResolveSymlinks
            FileDialog.DontConfirmOverwrite
            FileDialog.ReadOnly
            FileDialog.HideNameFilterDetails
            FileDialog.DontUseNativeDialog
        */
        /*
            selectedFile: url
            selectedFiles: url
        */
        onAccepted: {
            if (!addition) {
                for (var i = 0; i < out_model.length; i++)
                    Crypto.removeImage(out_model[i].source)
                out_model = []
            }
            const tmp = addition ? list_model : []
            const prev_len = list_model.length
            for (let i = 0; i < selectedFiles.length; i++) {
                if (addition) {
                    let find = false
                    for (let j = 0; j < prev_len; j++) {
                        if (tmp[j].source === selectedFiles[i]) {
                            find = true
                            break
                        }
                    }
                    if (find) continue
                }
                tmp.push({
                    source: selectedFiles[i],
                    name: String(selectedFiles[i]).split('/').pop()
                })
            }
            list_model = tmp
            if (list_model.length > 1) {
                onePic = false
            } else {
                onePic = true
                img_in_sig.source = list_model[0].source
            }
        }
        onRejected: {
        }
    }

    FolderDialog {
        id: imageSaveDialog
        acceptLabel: qsTr("Open")
        rejectLabel: qsTr("Cancel")
        currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
        onAccepted: {
            Crypto.doSave(out_model, selectedFolder)
        }
        onRejected: {
            save_all_btn.progress = 1.0
        }
        Connections {
            target: Crypto
            function onSaveImageFinished(idx) {
                if (idx === -1) {
                    save_all_btn.progress = 1.0
                    showError(qsTr("Save Cancelled"))
                    return
                }
                save_all_btn.progress = 1.0 * (idx + 1) / out_model.length
                if (save_all_btn.progress === 1.0) showSuccess(qsTr("Image Saving Complete!"))
            }
        }
    }

    FluRectangle {
        Layout.alignment: Qt.AlignLeft

        width: parent.width - right_pad.width
        height: childrenRect.height

        color: FluColors.Transparent

        Column {

            width: parent.width
            height: childrenRect.height

            SubTitle {text: qsTr("Workspace")}

            SeparateLine {}

            FluText {text: qsTr("Input"); padding: 10}

            ImageViewEX {
                id: img_in_sig
                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.width
                height: onePic && list_model.length !== 0 ? 300 : 0
                source: ""
            }

            GroupImageViewEX {
                id: img_in_mul
                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.width
                height: onePic || list_model.length === 0 ? 0 : 300
                edit: cvt_btn.enabled && save_all_btn.progress === 1.0
                sourceList: list_model
            }

            Rectangle {
                id: img_in_box
                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.width
                height: list_model.length === 0 ? 300 : 0
                color: FluColors.Grey120.alpha(0.3)
                radius: 5

                clip: true

                FluText {
                    anchors.centerIn: parent
                    text: qsTr("No Images Selected.")
                }
            }

            FluRectangle {width: parent.width; height: 10; color: FluColors.Transparent}

            FluFilledButton {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Select Images")
                enabled: cvt_btn.progress === 1.0 && save_all_btn.progress === 1.0
                onClicked: {
                    addition = false
                    imagePickDialog.open()
                }
            }

            FluText {text: qsTr("Output"); padding: 10}

            ImageViewEX {
                id: img_out_sig
                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.width
                height: onePic && out_model.length !== 0 ? 300 : 0
                property string url: ""
                property int ct: 0
                property string mid: url.split("/").pop()
                source: url + "/" + String(ct)

                Connections {
                    target: MemImage
                    function onImageUpdated(id) {
                        if (id === img_out_sig.mid) {
                            img_out_sig.ct++
                        }
                    }
                }
            }

            GroupImageViewEX {
                id: img_out_mul
                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.width
                height: onePic || out_model.length === 0 ? 0 : 300
                sourceList: out_model
            }

            Rectangle {
                id: img_out_box
                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.width
                height: out_model.length === 0 ? 300 : 0
                color: FluColors.Grey120.alpha(0.3)
                radius: 5

                clip: true

                FluText {
                    anchors.centerIn: parent
                    text: qsTr("No Outputs, Please Click Convert First.")
                }
            }

            FluRectangle {width: parent.width; height: 10; color: FluColors.Transparent}

            FluRectangle {
                width: cvt_btn.width + save_all_box.width + stop_cvt_box.width + 10
                height: childrenRect.height
                color: FluColors.Transparent
                anchors.horizontalCenter: parent.horizontalCenter

                FluProgressButton {
                    id: cvt_btn
                    progress: 1.0
                    anchors.left: parent.left
                    text: qsTr("Convert!")
                    enabled: progress === 1.0 && save_all_btn.progress === 1.0 && list_model.length !== 0
                    onClicked: {
                        if (list_model.length === 0) return
                        cvt_btn.progress = 0.0
                        for (var i = 0; i < out_model.length; i++)
                            Crypto.removeImage(out_model[i].source)
                        Crypto.doCrypto(list_model, paramConf.paramKey(), cvt_type.currentIndex === 0, cuda.checked)
                        // out_model = []
                        // const tmp = []
                        // console.log(paramConf.paramKey())
                        // for (i = 0; i < list_model.length; i++){
                        //     tmp.push({
                        //         name: list_model[i].name,
                        //         source: cvt_type.currentIndex === 0 ? Crypto.encrypt(list_model[i].source, paramConf.paramKey(), cuda.checked): Crypto.decrypt(list_model[i].source, paramConf.paramKey(), cuda.checked)
                        //     })
                        // }
                        // out_model = tmp
                        // img_out_sig.url = out_model[0].source
                    }
                    Connections {
                        target: Crypto
                        function onCryptoFinished(idx, url) {
                            if (idx === -1) {
                                cvt_btn.progress = 1.0
                                showError(qsTr("Conversion Cancelled"))
                                return
                            }
                            if (idx >= out_model.length) {
                                const tmp = out_model
                                tmp.push({
                                    name: list_model[idx].name,
                                    source: url
                                })
                                out_model = tmp
                            } else {
                                out_model[idx] = {
                                    name: list_model[idx].name,
                                    source: url
                                }
                                img_out_mul.set(idx, out_model[idx])
                            }
                            if (idx === 0)
                                img_out_sig.url = out_model[0].source
                            cvt_btn.progress = 1.0 * (idx + 1) / list_model.length
                            if (cvt_btn.progress === 1.0) showSuccess(qsTr("Conversion Complete!"))
                        }
                    }
                }

                Rectangle {
                    id: save_all_box
                    width: (cvt_btn.progress === 1.0 && out_model.length !== 0) ? save_all_btn.width : 0
                    height: save_all_btn.height
                    anchors.right: parent.right
                    color: FluColors.Transparent
                    clip: true
                    Behavior on width {
                        PropertyAnimation {duration: 100}
                    }
                    FluProgressButton {
                        id: save_all_btn
                        progress: 1.0
                        anchors.centerIn: parent
                        text: qsTr("Save Images")
                        enabled: cvt_btn.enabled && out_model.length !== 0 && progress === 1.0
                        onClicked: {
                            progress = 0.0
                            imageSaveDialog.open()
                        }
                    }
                }

                Rectangle {
                    id: stop_cvt_box
                    width: cvt_btn.progress !== 1.0 ? stop_cvt_btn.width : 0
                    height: stop_cvt_btn.height
                    anchors.right: stop_save_box.left
                    color: FluColors.Transparent
                    clip: true
                    Behavior on width {
                        PropertyAnimation {duration: 100}
                    }
                    FluFilledButton {
                        id: stop_cvt_btn
                        anchors.centerIn: parent
                        text: qsTr("Cancel")
                        enabled: cvt_btn.progress !== 1.0
                        onClicked: {
                            Crypto.stopCrypto()
                        }
                    }
                }

                Rectangle {
                    id: stop_save_box
                    width: save_all_btn.progress !== 1.0 ? stop_save_btn.width : 0
                    height: stop_save_btn.height
                    anchors.left: save_all_box.right
                    anchors.leftMargin: 10
                    color: FluColors.Transparent
                    clip: true
                    Behavior on width {
                        PropertyAnimation {duration: 100}
                    }
                    FluFilledButton {
                        id: stop_save_btn
                        anchors.centerIn: parent
                        text: qsTr("Cancel")
                        enabled: save_all_btn.progress !== 1.0
                        onClicked: {
                            Crypto.stopSave()
                        }
                    }
                }
            }

            FluRectangle {width: parent.width; height: 10; color: FluColors.Transparent}

            SubTitle {text: qsTr("Parameters")}

            SeparateLine {}

            FluRectangle {width: parent.width; height: 10; color: FluColors.Transparent}

            // 1
            Rectangle {
                width: parent.width
                height: Math.max(cvt_type_txt.height, cvt_type.height)
                color: FluColors.Transparent
                FluRectangle {
                    width: parent.width / 2
                    height: Math.max(cvt_type_txt.height, cvt_type.height)
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    color: FluColors.Transparent
                    FluRectangle {
                        width: childrenRect.width
                        height: Math.max(cvt_type_txt.height, cvt_type.height)
                        anchors.centerIn: parent
                        color: FluColors.Transparent

                        FluText {
                            id: cvt_type_txt
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            padding: 10
                            text: qsTr("Convert Type")
                        }

                        FluRadioButtons {
                            id: cvt_type
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: cvt_type_txt.right
                            spacing: 8
                            currentIndex: 0
                            onCurrentIndexChanged: {
                            }

                            FluRadioButton {
                                text: qsTr("Encrypt")
                            }

                            FluRadioButton {
                                text: qsTr("Decrypt")
                            }
                        }
                    }

                }

                FluRectangle {
                    width: parent.width / 2
                    height: cuda.height
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    color: FluColors.Transparent

                    FluCheckBox {
                        id: cuda
                        enabled: Crypto.cudaAvailable()
                        checked: enabled
                        anchors.centerIn: parent
                        padding: 10
                        text: qsTr("Use Cuda")
                    }
                }
            }

            FluRectangle {width: parent.width; height: 10; color: FluColors.Transparent}

            ParamConf { id: paramConf}
        }
    }

    FluRectangle {
        id: right_pad
        Layout.alignment: Qt.AlignRight
        width: 10
    }

}
