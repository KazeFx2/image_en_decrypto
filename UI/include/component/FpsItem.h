#pragma once

#include <QQuickItem>
#include "stdafx.h"

class FpsItem : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY_AUTO(int, fps)
public:
    FpsItem();

private:
    int _frameCount = 0;
};
