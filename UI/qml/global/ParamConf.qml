import QtQuick
import FluentUI
import QtQuick.Dialogs
import QtCore
import "."
import "qrc:/main/src/js/tools.js" as Tools

Item {
    width: parent.width
    height: childrenRect.height

    function paramKey() {
        return KeyKeeper.getKey(
                        init_cond_1.current, ctrl_cond_1.current,
                        init_cond_2.current, ctrl_cond_2.current,
                        confuse_seed.current, threads.current,
                        byte_reserve.current, pre_iteration.current,
                        confusion_iteration.current, diffusion_confusion_iteration.current
                    )
    }

    function loadKey(key_id) {
        let ret = KeyKeeper.loadKey(key_id)
        init_cond_1.current = ret.initCond1
        ctrl_cond_1.current = ret.ctrlCond1
        init_cond_2.current = ret.initCond2
        ctrl_cond_2.current = ret.ctrlCond2
        confuse_seed.current = ret.confSeed
        threads.current = ret.threads
        byte_reserve.current = ret.byteReserve
        pre_iteration.current = ret.preIter
        confusion_iteration.current = ret.confIter
        diffusion_confusion_iteration.current = ret.diffConfIter
    }

    FileDialog {
        id: confSavePickDialog
        title: qsTr("File Select")
        acceptLabel: qsTr("Save")
        rejectLabel: qsTr("Cancel")
        fileMode: FileDialog.SaveFile
        currentFolder: StandardPaths.standardLocations(StandardPaths.DocumentsLocation)[0]
        defaultSuffix: "ikey"
        nameFilters: [qsTr("Image Crypro Key Files (*.ikey)")]
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
            KeyKeeper.saveParam(Tools.auto_suffix(String(selectedFile), "ikey"), paramKey())
            showSuccess(qsTr("Key Saved Successfully"))
        }
        onRejected: {
        }
    }

    FileDialog {
        id: confLoadPickDialog
        title: qsTr("File Select")
        acceptLabel: qsTr("Open")
        rejectLabel: qsTr("Cancel")
        fileMode: FileDialog.OpenFile
        currentFolder: StandardPaths.standardLocations(StandardPaths.DocumentsLocation)[0]
        defaultSuffix: "ikey"
        nameFilters: [qsTr("Image Crypro Key Files (*.ikey)")]
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
            let ret = KeyKeeper.loadParam(selectedFile)
            init_cond_1.current = ret.initCond1
            ctrl_cond_1.current = ret.ctrlCond1
            init_cond_2.current = ret.initCond2
            ctrl_cond_2.current = ret.ctrlCond2
            confuse_seed.current = ret.confSeed
            threads.current = ret.threads
            byte_reserve.current = ret.byteReserve
            pre_iteration.current = ret.preIter
            confusion_iteration.current = ret.confIter
            diffusion_confusion_iteration.current = ret.diffConfIter
            showSuccess(qsTr("Key Loaded Successfully"))
        }
        onRejected: {
        }
    }

    Column {
        Rectangle {
            width: parent.parent.width
            height: childrenRect.height
            color: FluColors.Transparent

            // col 1
            Rectangle {
                id: col_container_1
                anchors {
                    top: parent.top
                    left: parent.left
                }
                width: parent.width / 4
                height: childrenRect.height
                color: FluColors.Transparent
                Column {
                    Rectangle {
                        width: parent.parent.width
                        height: childrenRect.height
                        color: FluColors.Transparent
                        FluText {
                            id: init_cond_1_txt
                            anchors.right: parent.right
                            padding: 10
                            text: qsTr("Initial Condition 1")
                        }
                    }
                    Rectangle {
                        width: parent.parent.width
                        height: childrenRect.height
                        color: FluColors.Transparent
                        FluText {
                            id: init_cond_2_txt
                            anchors.right: parent.right
                            padding: 10
                            text: qsTr("Initial Condition 2")
                        }
                    }
                    Rectangle {
                        width: parent.parent.width
                        height: childrenRect.height
                        color: FluColors.Transparent
                        FluText {
                            id: confuse_seed_txt
                            anchors.right: parent.right
                            padding: 10
                            text: qsTr("Confusion Seed")
                        }
                    }
                    Rectangle {
                        width: parent.parent.width
                        height: childrenRect.height
                        color: FluColors.Transparent
                        FluText {
                            id: byte_reserve_txt
                            anchors.right: parent.right
                            padding: 10
                            text: qsTr("Byte Reserved")
                        }
                    }
                    Rectangle {
                        width: parent.parent.width
                        height: childrenRect.height
                        color: FluColors.Transparent
                        FluText {
                            id: confusion_iteration_txt
                            anchors.right: parent.right
                            padding: 10
                            text: qsTr("Confusion Iteration")
                        }
                    }
                }
            }

            // col 2
            Rectangle {
                id: col_container_2
                anchors {
                    top: parent.top
                    left: col_container_1.right
                }
                width: parent.width / 4
                height: childrenRect.height
                color: FluColors.Transparent
                Column {
                    Rectangle {
                        width: parent.parent.width
                        height: init_cond_1_txt.height
                        color: FluColors.Transparent
                        FloatSelector {
                            id: init_cond_1
                            current: 0.1
                            anchors.centerIn: parent
                            width: parent.width
                            chkValue: function(value) {
                                if (value * 2 === ctrl_cond_1.current || value === ctrl_cond_1.current) {
                                    return qsTr("value of initial condition can not be a half of control condition")
                                } else return ""
                            }
                        }
                    }
                    Rectangle {
                        width: parent.parent.width
                        height: init_cond_2_txt.height
                        color: FluColors.Transparent
                        FloatSelector {
                            id: init_cond_2
                            current: 0.1
                            anchors.centerIn: parent
                            width: parent.width
                            chkValue: function(value) {
                                if (value * 2 === ctrl_cond_2.current || value === ctrl_cond_2.current) {
                                    return qsTr("value of initial condition can not be a half of control condition")
                                } else return ""
                            }
                        }
                    }
                    Rectangle {
                        width: parent.parent.width
                        height: confuse_seed_txt.height
                        color: FluColors.Transparent
                        IntSelector {
                            id: confuse_seed
                            min: 0
                            max: 0xffff
                            steps: 0xffff
                            anchors.centerIn: parent
                            width: parent.width
                            chkValue: function(value) {return ""}
                        }
                    }
                    Rectangle {
                        width: parent.parent.width
                        height: byte_reserve_txt.height
                        color: FluColors.Transparent
                        IntSelector {
                            id: byte_reserve
                            min: 1
                            max: 8
                            steps: max - 1
                            anchors.centerIn: parent
                            width: parent.width
                            chkValue: function(value) {return ""}
                        }
                    }
                    Rectangle {
                        width: parent.parent.width
                        height: confusion_iteration_txt.height
                        color: FluColors.Transparent
                        IntSelector {
                            id: confusion_iteration
                            min: 1
                            max: 10
                            steps: max - 1
                            anchors.centerIn: parent
                            width: parent.width
                            chkValue: function(value) {return ""}
                        }
                    }
                }
            }

            // col 3
            Rectangle {
                id: col_container_3
                anchors {
                    top: parent.top
                    left: col_container_2.right
                }
                width: parent.width / 4
                height: childrenRect.height
                color: FluColors.Transparent
                Column {
                    Rectangle {
                        width: parent.parent.width
                        height: childrenRect.height
                        color: FluColors.Transparent
                        FluText {
                            id: ctrl_cond_1_txt
                            anchors.right: parent.right
                            padding: 10
                            text: qsTr("Control Condition 1")
                        }
                    }
                    Rectangle {
                        width: parent.parent.width
                        height: childrenRect.height
                        color: FluColors.Transparent
                        FluText {
                            id: ctrl_cond_2_txt
                            anchors.right: parent.right
                            padding: 10
                            text: qsTr("Control Condition 2")
                        }
                    }
                    Rectangle {
                        width: parent.parent.width
                        height: childrenRect.height
                        color: FluColors.Transparent
                        FluText {
                            id: threads_txt
                            anchors.right: parent.right
                            padding: 10
                            text: qsTr("Thread Count")
                        }
                    }
                    Rectangle {
                        width: parent.parent.width
                        height: childrenRect.height
                        color: FluColors.Transparent
                        FluText {
                            id: pre_iteration_txt
                            anchors.right: parent.right
                            padding: 10
                            text: qsTr("Pre Iteration")
                        }
                    }
                    Rectangle {
                        width: parent.parent.width
                        height: childrenRect.height
                        color: FluColors.Transparent
                        FluText {
                            id: diffusion_confusion_iteration_txt
                            anchors.right: parent.right
                            padding: 10
                            text: qsTr("Diff/Confusion Iteration")
                        }
                    }
                }
            }

            // col 4
            Rectangle {
                id: col_container_4
                anchors {
                    top: parent.top
                    left: col_container_3.right
                    right: parent.right
                }
                height: childrenRect.height
                color: FluColors.Transparent
                Column {
                    Rectangle {
                        width: parent.parent.width
                        height: ctrl_cond_1_txt.height
                        color: FluColors.Transparent
                        FloatSelector {
                            id: ctrl_cond_1
                            current: 0.15
                            max: 0.5
                            anchors.centerIn: parent
                            width: parent.width
                            chkValue: function(value) {
                                if (value === init_cond_1.current * 2 || value === init_cond_1.current) {
                                    return qsTr("value of control condition can not be 2 times of initial condition")
                                } else return ""
                            }
                        }
                    }
                    Rectangle {
                        width: parent.parent.width
                        height: ctrl_cond_2_txt.height
                        color: FluColors.Transparent
                        FloatSelector {
                            id: ctrl_cond_2
                            current: 0.15
                            max: 0.5
                            anchors.centerIn: parent
                            width: parent.width
                            chkValue: function(value) {
                                if (value === init_cond_2.current * 2 || value === init_cond_2.current) {
                                    return qsTr("value of control condition can not be 2 times of initial condition")
                                } else return ""
                            }
                        }
                    }
                    Rectangle {
                        width: parent.parent.width
                        height: threads_txt.height
                        color: FluColors.Transparent
                        IntSelector {
                            id: threads
                            min: 1
                            max: 128
                            steps: max - 1
                            anchors.centerIn: parent
                            width: parent.width
                            chkValue: function(value) {
                                return ""
                            }
                        }
                    }
                    Rectangle {
                        width: parent.parent.width
                        height: pre_iteration_txt.height
                        color: FluColors.Transparent
                        IntSelector {
                            id: pre_iteration
                            min: 1
                            max: 512
                            steps: max - 1
                            anchors.centerIn: parent
                            width: parent.width
                            chkValue: function(value) {
                                return ""
                            }
                        }
                    }
                    Rectangle {
                        width: parent.parent.width
                        height: diffusion_confusion_iteration_txt.height
                        color: FluColors.Transparent
                        IntSelector {
                            id: diffusion_confusion_iteration
                            min: 1
                            max: 10
                            steps: max - 1
                            anchors.centerIn: parent
                            width: parent.width
                            chkValue: function(value) {
                                return ""
                            }
                        }
                    }
                }
            }
        }

        FluRectangle {width: parent.parent.width; height: 10; color: FluColors.Transparent}

        Rectangle {
            width: parent.parent.width
            height: gen_random_btn.height
            color: FluColors.Transparent

            FluFilledButton {
                id: gen_random_btn
                anchors.centerIn: parent
                text: qsTr("Generate Randomly")
                onClicked: {
                    const items = [init_cond_1, ctrl_cond_1, init_cond_2, ctrl_cond_2, confuse_seed, threads,
                                byte_reserve, pre_iteration, confusion_iteration, diffusion_confusion_iteration]
                    for (let i = 0; i < items.length; i++) {
                        items[i].random()
                        while(items[i].chkValue(items[i].d_value) !== '')
                            items[i].random()
                        items[i].current = items[i].d_value
                    }
                }
            }
        }

        FluRectangle {width: parent.parent.width; height: 10; color: FluColors.Transparent}

        Rectangle {
            width: parent.parent.width
            height: Math.max(load_key_btn.height, save_key_btn.height)
            color: FluColors.Transparent
            Row {
                Rectangle {
                    width: parent.parent.width / 2
                    height: load_key_btn.height
                    color: FluColors.Transparent
                    Rectangle {
                        width: parent.width - 10
                        height: load_key_btn.height
                        color: FluColors.Transparent
                        anchors.centerIn: parent
                        FluFilledButton {
                            id: load_key_btn
                            anchors.right: parent.right
                            text: qsTr("Load Key")
                            onClicked: {
                                confLoadPickDialog.open()
                            }
                        }
                    }
                }
                Rectangle {
                    width: parent.parent.width / 2
                    height: save_key_btn.height
                    color: FluColors.Transparent
                    Rectangle {
                        width: parent.width - 10
                        height: save_key_btn.height
                        color: FluColors.Transparent
                        anchors.centerIn: parent
                        FluFilledButton {
                            id: save_key_btn
                            anchors.left: parent.left
                            text: qsTr("Save Key")
                            onClicked: {
                                confSavePickDialog.open()
                            }
                        }
                    }
                }
            }
        }
    }
}
