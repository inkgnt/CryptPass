#include "databasemanager.h"

#include <QSqlQuery>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent) {}

DatabaseManager::~DatabaseManager() {}

bool DatabaseManager::openDatabase(const QString &dbPath)
{
    closeDatabase();

    m_database = QSqlDatabase::addDatabase("QSQLITE", "passDB");
    m_database.setDatabaseName(dbPath);

    if (!m_database.open())
        return false;

    return true;
}

void DatabaseManager::closeDatabase()
{
    if (m_database.isOpen())
        m_database.close();

    QString cname = m_database.connectionName();
    m_database = QSqlDatabase();

    if (QSqlDatabase::contains(cname))
        QSqlDatabase::removeDatabase(cname);
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_database);
    QString createTableQuery = "CREATE TABLE IF NOT EXISTS passwords ("
                               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                               "url TEXT NOT NULL, "
                               "login BLOB NOT NULL, "
                               "password BLOB NOT NULL"
                               ")";

    if (!query.exec(createTableQuery))
    {
        return false;
    }
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


    while (query.next())
    {
        PasswordRecord rec;
        rec.id = query.value(0).toInt();
        rec.url = query.value(1).toString();
        rec.login = query.value(2).toByteArray();
        rec.password = query.value(3).toByteArray();
        records.append(rec);
    }
    return records;
}
