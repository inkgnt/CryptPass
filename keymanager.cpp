#include "keymanager.h"
#include <algorithm>
#include <QtDebug>

KeyManager& KeyManager::instance()
{
    static KeyManager instance;
    return instance;
}

KeyManager::~KeyManager()
{
    clearKey();
}

void KeyManager::setKey(const std::vector<unsigned char>& newKey)
{
    std::lock_guard<std::mutex> lock(mtx);

    if(initialized)
    {
        std::fill(key.begin(), key.end(), 0);
    }

    std::copy(newKey.begin(), newKey.end(), key.begin());
    initialized = true;
    lastActivity = QDateTime::currentDateTime();

    qInfo() << "Key updated at" << lastActivity.toString(Qt::ISODate);
    emit keyUpdated();
}

std::vector<unsigned char> KeyManager::getKey()
{
    std::lock_guard<std::mutex> lock(mtx);

    if(!initialized)
    {
        throw std::runtime_error("Key not initialized");
    }

    if(!isSessionValid())
    {
        clearKey();
        throw std::runtime_error("Session expired");
    }

    updateLastActivity();
    return key;
}

void KeyManager::clearKey()
{
    std::lock_guard<std::mutex> lock(mtx);

    if (initialized)
    {
        std::fill(key.begin(), key.end(), 0);
        initialized = false;
        lastActivity = QDateTime();
        qInfo() << "Key cleared at" << QDateTime::currentDateTime().toString(Qt::ISODate);
        emit keyCleared();
    }
}


bool KeyManager::isSessionValid() const
{
    return lastActivity.msecsTo(QDateTime::currentDateTime()) < SESSION_TIMEOUT_MS;
}

void KeyManager::updateLastActivity()
{
    lastActivity = QDateTime::currentDateTime();
}
