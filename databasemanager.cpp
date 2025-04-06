#include "databasemanager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "keymanager.h"

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
    // Можно заранее задать имя подключения, если нужно
}

DatabaseManager::~DatabaseManager()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

bool DatabaseManager::openDatabase(const QString &dbPath)
{
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(dbPath);  // Путь к базе данных

    if (!m_database.open()) {
        qWarning() << "Unable to open database:" << m_database.lastError().text();
        return false;
    }

    std::vector<unsigned char> key = KeyManager::instance().getKey();

    if (KeyManager::instance().isSessionValid()) {
        QSqlQuery pragmaQuery(m_database);

        QByteArray keyBytes(reinterpret_cast<const char*>(key.data()), key.size());

        QString pragmaStmt = QString("PRAGMA key = \"x'%1'\"").arg(QString(keyBytes.toHex()));
        if (!pragmaQuery.exec(pragmaStmt)) {
            qWarning() << "Unable to set database key:" << pragmaQuery.lastError().text();
            return false;
        }
    }

    return true;
}

void DatabaseManager::closeDatabase()
{
    if (m_database.isOpen())
        m_database.close();
}

QSqlDatabase DatabaseManager::database() const
{
    return m_database;
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_database);
    QString createTableQuery = "CREATE TABLE IF NOT EXISTS passwords ("
                               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                               "url TEXT NOT NULL, "
                               "password BLOB NOT NULL"
                               ")";
    if (!query.exec(createTableQuery)) {
        qWarning() << "Unable to create table:" << query.lastError().text();
        return false;
    }
    return true;
}
