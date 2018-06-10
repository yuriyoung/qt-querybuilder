#ifndef QUERYBUILDER_P_H
#define QUERYBUILDER_P_H

#include <QObject>
#include <QSqlQuery>
#include <QMap>
#include <QVariant>
#include "databasemanager.h"

class QueryBuider;

class  QueryBuiderPrivate : public QObject
{
    Q_OBJECT
public:
    QueryBuiderPrivate(QueryBuider *builder)
        : builder(builder)
    {

    }

    enum ConditionOp
    {
        NONE,
        WHERE,
        HAVING
    };

    enum ConditionType
    {
        AND,
        OR
    };

    QStringList fieldNames() const;

    QString compileSelect();
    QString compileWhere();
    QString compileGroupBy();
    QString compileHaving();
    QString compileLimit();

    /*! Tests whether the string has an SQL operator*/
    bool hasOperator(const QString &str);
    QString getOperator(const QString &str);
    bool whereHaving(const QString &key, const QString &value = "",
                     ConditionOp op = NONE);
    bool regMatch(const QString &pattern, const QString &text,
                  QString &capture = QString());

public:
    QSqlQuery query{DatabaseManager::database()};
    QueryBuider *builder{nullptr};
    QString table;
    QStringList selects;
    QStringList froms;
    QStringList joins;
    QStringList joinTypes{{"LEFT", "RIGHT", "OUTER", "INNER", "LEFT OUTER", "RIGHT OUTER"}};

    bool isDistinct {false};
    int limit{0};
    int offset{0};

    QList<QString> whereConditions;
    QList<QString> groupbyFields;
    QList<QString> havingConditions;
    QMap<ConditionOp, QString> conditionMapper;
};


#endif // QUERYBUILDER_P_H

