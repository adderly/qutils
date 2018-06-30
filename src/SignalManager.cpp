#include "qutils/SignalManager.h"

namespace zmc
{

QMap<int, SignalManager *> SignalManager::m_Instances = QMap<int, SignalManager *>();

SignalManager::SignalManager(QObject *parent)
    : QObject(parent)
    , m_InstanceIndex(m_Instances.size())
{
    m_Instances.insert(m_InstanceIndex, this);
}

SignalManager::~SignalManager()
{
    m_Instances.remove(m_InstanceIndex);
}

void SignalManager::emitSignal(const QString &signalName, const QString &targetObjectName, const QVariantMap data)
{
    auto begin = m_Instances.begin();
    auto end = m_Instances.end();
    for (auto it = begin; it != end; it++) {
        SignalManager *instance = it.value();
        if (instance) {
            if (targetObjectName.length() > 0) {
                if (targetObjectName == instance->objectName()) {
                    QMetaObject::invokeMethod(instance, std::bind(&SignalManager::signalReceived, instance, signalName, data), Qt::QueuedConnection);
                }
            }
            else {
                QMetaObject::invokeMethod(instance, std::bind(&SignalManager::signalReceived, instance, signalName, data), Qt::QueuedConnection);
            }
        }
    }
}

}
