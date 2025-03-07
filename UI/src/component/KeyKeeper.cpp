//
// Created by kazefx on 25-2-22.
//

#include "component/KeyKeeper.h"
#include <QRandomGenerator64>
#include <QCryptographicHash>

ThreadPool threadPool;

std::map<QString, C_FullKey> keyMap;

u8 __Rand8()
{
    return QRandomGenerator::global()->bounded(0xff);
}

u16 __Rand16()
{
    return QRandomGenerator::global()->bounded(0xffff);
}

u32 __Rand32()
{
    return QRandomGenerator::global()->generate();
}

u64 __Rand64()
{
    return QRandomGenerator64::global()->generate64();
}

void initRandom()
{
    Rand8 = __Rand8;
    Rand16 = __Rand16;
    Rand32 = __Rand32;
    Rand64 = __Rand64;
}

std::string calcSha256(FullKey key)
{
    key.control.cuda = false;
    return QCryptographicHash::hash(QByteArrayView(reinterpret_cast<const u8*>(&key), sizeof(FullKey)),
                                    QCryptographicHash::Sha256).toHex().toStdString();
}

KeyKeeper::KeyKeeper(QObject* parent) : QObject(parent)
{
}

QString KeyKeeper::getKey(
    double initCond1, double ctrlCond1,
    double initCond2, double ctrlCond2,
    u16 confuseSeed, u8 nThread,
    u8 byteReserve, u16 preIteration,
    u8 confuseIter, u8 diffConfIter
)
{
    const FullKey key{
        {
            byteReserve,
            preIteration,
            confuseIter,
            diffConfIter,
            nThread,
            3,
            false,
        },
        {
            confuseSeed,
            {
                initCond1,
                ctrlCond1,
            },
            {
                initCond2,
                ctrlCond2,
            }
        }
    };
    const auto k = QString::fromStdString(calcSha256(key));
    if (keyMap.find(k) != keyMap.end())
        return k;
    keyMap[k] = C_FullKey(key);
    return k;
}

void KeyKeeper::saveParam(const QUrl& path, const QString& key_id)
{
    const auto key = keyMap[key_id];
    SaveConfKey(key.key.control, key.key.keys, path.toLocalFile().toStdString().c_str());
}

QVariantMap KeyKeeper::loadParam(const QUrl& path)
{
    C_FullKey key;
    QVariantMap ret;
    LoadConfKey(key.key.control, key.key.keys, path.toLocalFile().toStdString().c_str());
    key.imageCrypto.setKeys(key.key.control, key.key.keys);
    auto k = QString::fromStdString(calcSha256(key.key));
    if (keyMap.find(k) == keyMap.end())
        keyMap[k] = C_FullKey(key);
    ret["id"] = k;
    ret["initCond1"] = key.key.keys.gParam1.initCondition;
    ret["ctrlCond1"] = key.key.keys.gParam1.ctrlCondition;
    ret["initCond2"] = key.key.keys.gParam2.initCondition;
    ret["ctrlCond2"] = key.key.keys.gParam2.ctrlCondition;
    ret["confSeed"] = key.key.keys.confusionSeed;
    ret["threads"] = key.key.control.nThread;
    ret["byteReserve"] = key.key.control.byteReserve;
    ret["preIter"] = key.key.control.preIterations;
    ret["confIter"] = key.key.control.confusionIterations;
    ret["diffConfIter"] = key.key.control.diffusionConfusionIterations;
    return ret;
}

QVariantMap KeyKeeper::loadKey(const QString& id)
{
    C_FullKey key = keyMap[id];
    QVariantMap ret;
    ret["id"] = id;
    ret["initCond1"] = key.key.keys.gParam1.initCondition;
    ret["ctrlCond1"] = key.key.keys.gParam1.ctrlCondition;
    ret["initCond2"] = key.key.keys.gParam2.initCondition;
    ret["ctrlCond2"] = key.key.keys.gParam2.ctrlCondition;
    ret["confSeed"] = key.key.keys.confusionSeed;
    ret["threads"] = key.key.control.nThread;
    ret["byteReserve"] = key.key.control.byteReserve;
    ret["preIter"] = key.key.control.preIterations;
    ret["confIter"] = key.key.control.confusionIterations;
    ret["diffConfIter"] = key.key.control.diffusionConfusionIterations;
    return ret;
}
