#ifndef QUERYBUIDER_H
#define QUERYBUIDER_H

#include <QObject>

class QVariant;
class Record;
class QueryBuiderPrivate;

class QueryBuider : public QObject
{
    Q_OBJECT
public:
    explicit QueryBuider(const QString &table = "");
    ~QueryBuider();

    int insert(const Record &record);

    QueryBuider *select(const QString &select = "*");
    QueryBuider *from(const QString &table);
    QueryBuider *join(const QString &table, const QString &condition, const QString &type = "");
    QueryBuider *where(const QString &field, const QString &cond, const QString &value);
    QueryBuider *like(const QString &field, const QString &match = "", const QString &side = "");
    QueryBuider *groupBy(const QString &byField);
    QueryBuider *having(const QString &field, const QString &value);
    QueryBuider *limit(int size = 0, int offset = 0);
    QList<Record> get(int limit = 0, int offset = 0);


    /*! use  to track sql statments written with aliases tables */
    void trackAliases(const QString &table);

private:
    QScopedPointer<QueryBuiderPrivate> d;
    friend class PluginManagerPrivate;
};


#endif // QUERYBUIDER_H
