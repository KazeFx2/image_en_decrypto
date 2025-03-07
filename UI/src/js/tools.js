.pragma library

function format() {
    if (arguments.length === 0) {
        return "";
    }
    var s = arguments[0]
    for (var i = 1; i < arguments.length; i++) {
        s = s.replace(new RegExp("\\{" + (i - 1) + "\\}", "g"), arguments[i]);
    }
    return s;
}

function auto_suffix(file, suffix) {
    var array = file.split(".");
    if (array.length === 1) {
        return array[0] + "." + suffix;
    }
    var ret = ""
    for (var i = 0; i < array.length - 1; i++) {
        ret += (array[i] + ".");
    }
    return ret + suffix;
}
