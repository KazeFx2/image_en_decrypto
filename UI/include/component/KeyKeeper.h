//
// Created by kazefx on 25-2-22.
//

#ifndef KEYKEEPER_H
#define KEYKEEPER_H

#include <ImageCrypto.h>
#include <QtQml/qqml.h>
#include "stdafx.h"
#include "singleton.h"
#include "private/includes.h"

extern ThreadPool threadPool;

void initRandom();

typedef struct
{
    ParamControl control;
    Keys keys;
} FullKey;

std::string calcSha256(FullKey key);

class C_FullKey
{
public:
    FullKey key;
    ImageCrypto imageCrypto;

    C_FullKey(): imageCrypto(threadPool)
    {
        key.control = DEFAULT_CONFIG;
        key.keys = RANDOM_KEYS;
    }

    C_FullKey(const FullKey& _key): key(_key), imageCrypto(threadPool, key.control, key.keys)
    {
    }

    C_FullKey(const C_FullKey& other): key(other.key), imageCrypto(other.imageCrypto)
    {
    }
};

extern std::map<QString, C_FullKey> keyMap;

class KeyKeeper : public QObject
{
    Q_OBJECT
    Q_PROPERTY_AUTO(QString, path)
    QML_NAMED_ELEMENT(KeyKeeper)
    QML_SINGLETON
    explicit KeyKeeper(QObject* parent = nullptr);

public:
    SINGLETON(KeyKeeper)

    Q_SIGNAL void fileChanged();

    Q_INVOKABLE QString getKey(
        double initCond1, double ctrlCond1,
        double initCond2, double ctrlCond2,
        u16 confuseSeed, u8 nThread,
        u8 byteReserve, u16 preIteration,
        u8 confuseIter, u8 diffConfIter
    );


    Q_INVOKABLE void saveParam(const QUrl& path, const QString& key_id);

    Q_INVOKABLE QVariantMap loadParam(const QUrl& path);
private:
    // void clean();

private:
};

#endif //KEYKEEPER_H
