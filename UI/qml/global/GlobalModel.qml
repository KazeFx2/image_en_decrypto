pragma Singleton

import QtQuick 2.15
import FluentUI 1.0

QtObject{

    property int displayMode: {
        switch(SettingsHelper.getDisplayMode()) {
            case 0:
                return GlobalModel.displayMode = FluNavigationViewType.Open
            case 1:
                return GlobalModel.displayMode = FluNavigationViewType.Compact
            case 2:
                return GlobalModel.displayMode = FluNavigationViewType.Minimal
            default:
            case 3:
                return GlobalModel.displayMode = FluNavigationViewType.Auto
        }
    }
    property bool showFPS: SettingsHelper.getShowFPS()
}
