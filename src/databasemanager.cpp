#include "databasemanager.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <QDebug>

static DatabaseManager *g_instance = DatabaseManager::instance();

class DatabaseManager::DatabaseManagerPrivate
{
public:
    ~DatabaseManagerPrivate()
    {
        qDeleteAll(objects);
        objects.clear();
    }

    bool addObject(QObject *obj)
    {
        QWriteLocker lock(&this->lock);
        if(obj == nullptr)
        {
            qWarning() << "DatabaseManager::addObject(): trying to add null object";
            return false;
        }

        if (objects.contains(obj))
        {
            qWarning() << "DatabaseManager::addObject(): trying to add duplicate object";
            return false;
        }

        this->objects.append(obj);

        return true;
    }

    bool removeObject(QObject *obj)
    {
        if (obj == 0)
        {
            qWarning() << "DatabaseManager::removeObject(): trying to remove null object";
            return false;
        }

        if (!objects.contains(obj))
        {
            qWarning() << "DatabaseManager::removeObject(): object not in list:"
                       << obj << obj->objectName();
            return false;
        }

        QWriteLocker lock(&this->lock);
        this->objects.removeAll(obj);

        return true;
    }

public:
    mutable QReadWriteLock lock;
    QList<QObject *> objects;
};

void DatabaseManager::debugQuery(const QSqlQuery &query)
{
    if (query.lastError().type() == QSqlError::ErrorType::NoError)
    {
        qDebug() << "Query OK:"  << query.lastQuery();
    }
    else
    {
        qWarning() << "Query KO:" << query.lastError().text();
        qWarning() << "Query text:" << query.lastQuery();
    }
}

DatabaseManager::DatabaseManager(const QString &path)
{
    g_instance = this;

    m_database.reset(new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE")));
    d.reset(new DatabaseManagerPrivate);

    m_database->setDatabaseName(path);
    bool status = m_database->open();
    qDebug() << "Database connection: " << (status ? "OK" : "Error");
}

DatabaseManager::~DatabaseManager()
{
    m_database->close();
}

DatabaseManager *DatabaseManager::instance()
{
    if(!g_instance)
    {
        g_instance = new DatabaseManager;
    }
    return g_instance;
}

QSqlDatabase &DatabaseManager::database()
{
    return *g_instance->m_database.data();
}

QReadWriteLock *DatabaseManager::lock()
{
    return &g_instance->d->lock;
}

bool DatabaseManager::addObject(QObject *obj)
{
    return g_instance->d->addObject(obj);
}

bool DatabaseManager::removeObject(QObject *obj)
{
    return g_instance->d->removeObject(obj);
}

QList<QObject *> DatabaseManager::allObjects()
{
    return g_instance->d->objects;
}
