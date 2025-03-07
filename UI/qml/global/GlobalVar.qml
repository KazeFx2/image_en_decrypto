pragma Singleton

import QtQuick 2.15
import FluentUI 1.0

QtObject{

    property int pivot_idx: 0

    property var list_model: []
    property var out_model: []
    property bool onePic: true
    property bool addition: false
    property bool image_encrypt: true
    property bool image_cuda: Crypto.cudaAvailable()
    property double cvt_btn_progrs: 1.0
    property double save_all_btn_progrs: 1.0
    property string image_param_key_id: ""
    property string img_out_sig_url: ""
    property var img_out_mul

    property string open_file: ""
    property string open_name: ""
    property string out_file: ""
    property string out_name: ""
    property double apply_params_progrs: 1.0
    property bool video_cvt_encrypt: true
    property bool video_cvt_cuda: Crypto.cudaAvailable()
    property string video_cvt_param_key_id: ""
    property bool bind_wh: true
    property int real_width: 0
    property int real_height: 0
    property int cvt_width: 0
    property int cvt_height: 0

    property int video_sel: 0

    property string video_url: ""
}
