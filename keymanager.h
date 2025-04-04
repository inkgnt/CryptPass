#pragma once

#include <array>
#include <mutex>
#include <QDateTime>
#include <QObject>

class KeyManager : public QObject
{
    Q_OBJECT
public:
    static constexpr int KEY_SIZE = 32;
    static constexpr int SESSION_TIMEOUT_MS = 300000; // 5 min

    //
    KeyManager(const KeyManager&) = delete;
    KeyManager& operator=(const KeyManager&) = delete;

    static KeyManager& instance();

    void setKey(const std::array<unsigned char, KEY_SIZE>& newKey);
    std::array<unsigned char, KEY_SIZE> getKey();
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
    std::array<unsigned char, KEY_SIZE> key{};
    QDateTime lastActivity;
    bool initialized = false;
};
