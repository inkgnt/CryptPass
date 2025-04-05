#pragma once

#include <mutex>
#include <QDateTime>
#include <QObject>

class KeyManager : public QObject
{
    Q_OBJECT
public:
    static constexpr int KEY_SIZE = 32;
    static constexpr int SESSION_TIMEOUT_MS = 300000; // 5 min


    KeyManager(const KeyManager&) = delete;
    KeyManager& operator=(const KeyManager&) = delete;

    static KeyManager& instance();

    void setKey(const std::vector<unsigned char>& newKey);
    std::vector<unsigned char> getKey();

    void clearKey();

    bool isSessionValid() const;
    void updateLastActivity();

signals:
    void keyUpdated();
    void keyCleared();

private:
    KeyManager() = default;
    ~KeyManager();

    mutable std::mutex mtx;
    std::vector<unsigned char> key{};
    QDateTime lastActivity;
    bool initialized = false;
};
