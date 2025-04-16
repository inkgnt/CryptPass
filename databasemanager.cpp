#include "databasemanager.h"

#include <QSqlQuery>

#include "keymanager.h"

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{

}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

bool DatabaseManager::openDatabase(const QString &dbPath)
{
    if (QSqlDatabase::contains("passDB"))
        QSqlDatabase::removeDatabase("passDB");

    m_database = QSqlDatabase::addDatabase("QSQLITE", "passDB");
    m_database.setDatabaseName(dbPath);

    if (!m_database.open())
        return false;


    if (!setEncryptionKey())
        return false;

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
    if (!setEncryptionKey()) {
        return false;
    }

    QSqlQuery query(m_database);
    QString createTableQuery = "CREATE TABLE IF NOT EXISTS passwords ("
                               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                               "url TEXT NOT NULL, "
                               "login BLOB NOT NULL, "
                               "password BLOB NOT NULL"
                               ")";
    if (!query.exec(createTableQuery)) {
        return false;
    }
    return true;
}

bool DatabaseManager::setEncryptionKey()
{
    std::vector<unsigned char> key = KeyManager::instance().getKey();

    if (KeyManager::instance().isSessionValid()) {
        QSqlQuery pragmaQuery(m_database);

        QByteArray keyBytes(reinterpret_cast<const char*>(key.data()), key.size());

        QString pragmaStmt = QString("PRAGMA key = \"x'%1'\"").arg(QString(keyBytes.toHex()));
        if (!pragmaQuery.exec(pragmaStmt))
            return false;
    } else
        return false;


    return true;
}


bool DatabaseManager::addRecord(const QString &url, const QByteArray &login, const QByteArray &encryptedPassword)
{
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO passwords (url, login, password) VALUES (:url, :login, :password)");
    query.bindValue(":url", url);
    query.bindValue(":login", login);
    query.bindValue(":password", encryptedPassword);

    if (!query.exec())
        return false;

    return true;
}

bool DatabaseManager::deleteRecord(int id)
{
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM passwords WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec())
        return false;

    return true;
}

QList<PasswordRecord> DatabaseManager::getAllRecords() const
{
    QList<PasswordRecord> records;
    QSqlQuery query(m_database);
    if (!query.exec("SELECT id, url, login, password FROM passwords"))
        return {};


    while (query.next()) {
        PasswordRecord rec;
        rec.id = query.value(0).toInt();
        rec.url = query.value(1).toString();
        rec.login = query.value(2).toByteArray();
        rec.password = query.value(3).toByteArray();
        records.append(rec);
    }
    return records;
}
