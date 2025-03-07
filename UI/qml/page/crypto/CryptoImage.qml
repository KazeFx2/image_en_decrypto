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

    property var list_model: GlobalVar.list_model
    property var out_model: GlobalVar.out_model
    property bool onePic: GlobalVar.onePic
    property bool addition: GlobalVar.addition

    Connections {
        target: img_in_mul
        function onRemove(source, idx) {
            GlobalVar.list_model = GlobalVar.list_model.filter(item => String(item.source) !== String(source))
            if (source !== undefined)
                GlobalVar.out_model = GlobalVar.out_model.filter(item => String(item.source) !== String(GlobalVar.out_model[idx].source))
        }
        function onAdd(){
            GlobalVar.addition = true
            imagePickDialog.open()
        }
    }

    Component.onCompleted: {
        GlobalVar.img_out_mul = img_out_mul
        if (GlobalVar.image_param_key_id !== "")
            paramConf.loadKey(GlobalVar.image_param_key_id)
    }
    Component.onDestruction: {
        GlobalVar.img_out_mul = undefined
        GlobalVar.image_param_key_id = paramConf.paramKey()
        // for (var i = 0; i < GlobalVar.out_model.length; i++)
        //     Crypto.removeImage(GlobalVar.out_model[i].source)
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
            if (!GlobalVar.addition) {
                for (var i = 0; i < GlobalVar.out_model.length; i++)
                    Crypto.removeImage(GlobalVar.out_model[i].source)
                GlobalVar.out_model = []
            }
            const tmp = GlobalVar.addition ? GlobalVar.list_model : []
            const prev_len = GlobalVar.list_model.length
            for (let i = 0; i < selectedFiles.length; i++) {
                if (GlobalVar.addition) {
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
            GlobalVar.list_model = tmp
            if (GlobalVar.list_model.length > 1) {
                GlobalVar.onePic = false
            } else {
                GlobalVar.onePic = true
                img_in_sig.source = GlobalVar.list_model[0].source
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
            Crypto.doSave(GlobalVar.out_model, selectedFolder)
        }
        onRejected: {
            GlobalVar.save_all_btn_progrs = 1.0
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
                height: GlobalVar.onePic && GlobalVar.list_model.length !== 0 ? 300 : 0
                source: ""
            }

            GroupImageViewEX {
                id: img_in_mul
                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.width
                height: GlobalVar.onePic || GlobalVar.list_model.length === 0 ? 0 : 300
                edit: cvt_btn.enabled && save_all_btn.progress === 1.0
                sourceList: GlobalVar.list_model
            }

            Rectangle {
                id: img_in_box
                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.width
                height: GlobalVar.list_model.length === 0 ? 300 : 0
                color: FluColors.Grey120.alpha(0.3)
                radius: 5

                clip: true

                FluText {
                    anchors.centerIn: parent
                    text: qsTr("No Images Selected.")
                    font.pixelSize: 24
                }
            }

            FluRectangle {width: parent.width; height: 10; color: FluColors.Transparent}

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                color: FluColors.Transparent
                width: sel_img_btn.width + sel_img_box.width
                height: Math.max(sel_img_btn.height, sel_img_box.height)
                FluFilledButton {
                    id: sel_img_btn
                    anchors.left: parent.left
                    text: qsTr("Select Images")
                    enabled: cvt_btn.progress === 1.0 && save_all_btn.progress === 1.0
                    onClicked: {
                        GlobalVar.addition = false
                        imagePickDialog.open()
                    }
                }
                Rectangle {
                    id: sel_img_box
                    anchors.right: parent.right
                    width: GlobalVar.list_model.length > 0 ? del_img_btn.width + 10 : 0
                    height: del_img_btn.height
                    color: FluColors.Transparent
                    clip: true
                    Behavior on width {
                        PropertyAnimation {duration: 100}
                    }
                    FluFilledButton {
                        id: del_img_btn
                        anchors.right: parent.right
                        text: qsTr("Delete All Images")
                        enabled: cvt_btn.progress === 1.0 && save_all_btn.progress === 1.0
                        onClicked: {
                            for (var i = 0; i < GlobalVar.out_model.length; i++)
                                Crypto.removeImage(GlobalVar.out_model[i].source)
                            GlobalVar.out_model = []
                            GlobalVar.list_model = []
                        }
                    }
                }
            }

            FluText {text: qsTr("Output"); padding: 10}

            ImageViewEX {
                id: img_out_sig
                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.width
                height: GlobalVar.onePic && GlobalVar.out_model.length !== 0 ? 300 : 0
                property string url: GlobalVar.img_out_sig_url
                property int ct: 0
                property string mid: url.split("/").pop()
                source: url === "" ? "" : url + "/" + String(ct)

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
                height: GlobalVar.onePic || GlobalVar.out_model.length === 0 ? 0 : 300
                sourceList: GlobalVar.out_model
            }

            Rectangle {
                id: img_out_box
                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.width
                height: GlobalVar.out_model.length === 0 ? 300 : 0
                color: FluColors.Grey120.alpha(0.3)
                radius: 5

                clip: true

                FluText {
                    anchors.centerIn: parent
                    text: qsTr("No Outputs, Please Click Convert First.")
                    font.pixelSize: 24
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
                    progress: GlobalVar.cvt_btn_progrs
                    anchors.left: parent.left
                    text: qsTr("Convert!")
                    enabled: progress === 1.0 && save_all_btn.progress === 1.0 && GlobalVar.list_model.length > 0
                    onClicked: {
                        if (GlobalVar.list_model.length === 0) return
                        GlobalVar.cvt_btn_progrs = 0.0
                        for (var i = 0; i < GlobalVar.out_model.length; i++)
                            Crypto.removeImage(GlobalVar.out_model[i].source)
                        Crypto.doCrypto(GlobalVar.list_model, paramConf.paramKey(), cvt_type.currentIndex === 0, cuda.checked)
                        // GlobalVar.out_model = []
                        // const tmp = []
                        // console.log(paramConf.paramKey())
                        // for (i = 0; i < GlobalVar.list_model.length; i++){
                        //     tmp.push({
                        //         name: GlobalVar.list_model[i].name,
                        //         source: cvt_type.currentIndex === 0 ? Crypto.encrypt(GlobalVar.list_model[i].source, paramConf.paramKey(), cuda.checked): Crypto.decrypt(GlobalVar.list_model[i].source, paramConf.paramKey(), cuda.checked)
                        //     })
                        // }
                        // GlobalVar.out_model = tmp
                        // GlobalVar.img_out_sig_url = GlobalVar.out_model[0].source
                    }
                }

                Rectangle {
                    id: save_all_box
                    width: (cvt_btn.progress === 1.0 && GlobalVar.out_model.length !== 0) ? save_all_btn.width : 0
                    height: save_all_btn.height
                    anchors.right: parent.right
                    color: FluColors.Transparent
                    clip: true
                    Behavior on width {
                        PropertyAnimation {duration: 100}
                    }
                    FluProgressButton {
                        id: save_all_btn
                        progress: GlobalVar.save_all_btn_progrs
                        anchors.centerIn: parent
                        text: qsTr("Save Images")
                        enabled: cvt_btn.enabled && GlobalVar.out_model.length !== 0 && progress === 1.0
                        onClicked: {
                            GlobalVar.save_all_btn_progrs = 0.0
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
                            currentIndex: GlobalVar.image_encrypt ? 0 : 1
                            onCurrentIndexChanged: {
                                GlobalVar.image_encrypt = currentIndex === 0
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
                        checked: GlobalVar.image_cuda
                        anchors.centerIn: parent
                        padding: 10
                        text: qsTr("Use Cuda")
                        onCheckedChanged: {
                            GlobalVar.image_cuda = checked
                        }
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
