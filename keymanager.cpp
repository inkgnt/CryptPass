#include "keymanager.h"

#include <QtDebug>

KeyManager& KeyManager::instance()
{
    static KeyManager instance;
    return instance;
}

KeyManager::KeyManager(QObject* parent)
    : QObject(parent)
{
    sessionCheckTimer = new QTimer(this);
    connect(sessionCheckTimer, &QTimer::timeout, this, &KeyManager::checkSessionValidity);
    sessionCheckTimer->start(SESSION_TIMEOUT_MS);
}

KeyManager::~KeyManager()
{
    clearKey();
}

void KeyManager::setKey(const std::vector<unsigned char>& newKey)
{
    std::lock_guard<std::mutex> lock(mtx);

    std::fill(key.begin(), key.end(), 0);

    std::copy(newKey.begin(), newKey.end(), key.begin());

    initialized = true;
    updateLastActivity();
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

    clearKeyUnsafe();
}

void KeyManager::clearKeyUnsafe()
{
    if (initialized) {
        std::fill(key.begin(), key.end(), 0);
        initialized = false;

        emit keyCleared();
    }
}

bool KeyManager::isSessionValid() const
{
    if (initialized)
        return lastActivity.msecsTo(QDateTime::currentDateTime()) < SESSION_TIMEOUT_MS;

    return false;
}

void KeyManager::updateLastActivity()
{
    lastActivity = QDateTime::currentDateTime();
}

void KeyManager::checkSessionValidity()
{
    qWarning() << "check session .. ";
    std::lock_guard<std::mutex> lock(mtx);

    if (initialized && lastActivity.msecsTo(QDateTime::currentDateTime()) >= SESSION_TIMEOUT_MS) {
        clearKeyUnsafe();
        qWarning() << "key cleared";
    }
}
