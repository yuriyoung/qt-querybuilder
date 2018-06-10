#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QReadWriteLock>


class QSqlQuery;
class QSqlDatabase;

const QString DATABASE_FILENAME = "project.db";

class DatabaseManager
{
public:
    ~DatabaseManager();

    static void debugQuery(const QSqlQuery& query);
    static DatabaseManager *instance();
    static QSqlDatabase &database();

    static QReadWriteLock *lock();
    static bool addObject(QObject *obj);
    static bool removeObject(QObject *obj);
    static QList<QObject *> allObjects();

    template <typename T>
    static T *getObjects()
    {
        QReadLocker lock(this->lock());
        QList<T *> results;
        QList<QObject *> all = allObjects();
        foreach (QObject *obj, all)
        {
            T *result = qobject_cast<T *>(obj);
            if (result)
                results += result;
        }
        return results;
    }

    template <typename T>
    static T *getObject()
    {
        QReadLocker lock(this->lock());
        QList<QObject *> all = allObjects();
        foreach (QObject *obj, all)
        {
            if (T *result = qobject_cast<T *>(obj))
                return result;
        }
        return 0;
    }

protected:
    /*! Q_DISABLE_COPY(DatabaseManager) */
    DatabaseManager(const QString& path = DATABASE_FILENAME);
    DatabaseManager& operator=(const DatabaseManager& rhs);

private:
    QScopedPointer<QSqlDatabase> m_database;

    class DatabaseManagerPrivate;
    QScopedPointer<DatabaseManagerPrivate> d;
};

#endif // DATABASEMANAGER_H
