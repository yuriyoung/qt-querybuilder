#include <QSqlQuery>
#include <QMap>
#include <QVariant>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>

#include <QDebug>

#include "querybuider.h"
#include "private/querybuilder_p.h"
#include "record.h"

QueryBuider::QueryBuider(const QString &table)
{
    d.reset(new QueryBuiderPrivate(this));
    d->table = table;
    d->froms.append(table);
}

QueryBuider::~QueryBuider()
{
    d.reset();
}

int QueryBuider::insert(const Record &record)
{
    QStringList fields = d->fieldNames();
    QStringList prepares;
    for(int i = 0; i < fields.count(); ++i)
    {
        QString field = fields.at(i);
        prepares.append(":" + field);
    }

    QString sql = "INSERT INTO " + d->table
            + QString(" (%0)" ).arg(fields.join(","))
            + QString(" VALUES (%0)").arg(prepares.join(","));
    d->query.prepare(sql);

    for(int i = 0; i < fields.count(); ++i)
    {
        QString field = fields.at(i);
        if(record.contains(field))
        {
            d->query.bindValue(":" + field, record.data(field));
        }
    }

    if(d->query.exec())
    {
        return d->query.lastInsertId().toInt();
    }

    qDebug() <<"error: " << d->query.lastError().databaseText();
    qDebug() <<"sql: " << sql;

    return 0;
}

QueryBuider *QueryBuider::select(const QString &select)
{
    QStringList selects = select.split(",", QString::SkipEmptyParts);
    foreach (QString val, selects)
    {
        val = val.trimmed();
        if(!val.isEmpty())
        {
            d->selects.append(val);
        }
    }

    return this;
}

QueryBuider *QueryBuider::from(const QString &table)
{
    QStringList froms = table.split(",", QString::SkipEmptyParts);
    foreach (QString val, froms)
    {
        val = val.trimmed();
        if(!val.isEmpty())
        {
            // TODO: trackAliases
            // ...

            d->froms.append(val);
        }
    }

    return this;
}

QueryBuider *QueryBuider::join(const QString &table, const QString &condition, const QString &type)
{
    QString upperType = type;
    if(type != "")
    {
        upperType = type.trimmed().toUpper();
        if( d->joinTypes.contains(upperType) )
        {
            upperType += " ";
        }
        else
        {
            upperType = "";
        }
    }

    //TODO: trackAliases
    // ...

    QString cond = condition;
    if( !d->hasOperator(condition) )
    {
        cond = QString(" USING(%0)").arg(condition);
    }
    else
    {
        cond = QString(" ON ").arg(condition);
    }

    QString join = upperType + "JOIN " + table + cond;
    d->joins.append(join);

    return this;
}

QueryBuider *QueryBuider::where(const QString &field, const QString &cond, const QString &value)
{
    QString where;
    if(value.isEmpty())
    {
        where = field + " " + cond;
    }
    else
    {
        where = field + cond + "'" + value + "'";
    }
    d->whereConditions.append(where);
    return this;
}

QueryBuider *QueryBuider::like(const QString &field, const QString &match, const QString &side)
{
    QString _side =  side.toLower();
    QString likeStatment;
    if (_side == "contains")
    {
        likeStatment = field + " LIKE '%" + match + "%'";
    }
    else if (_side == "begin")
    {
        likeStatment = field + " LIKE '" + match + "%'";
    }
    else if (_side == "end")
    {
        likeStatment = field + " LIKE '%" + match + "'";
    }
    else
    {
        likeStatment = field + "='" + match + "'";
    }

    d->whereConditions.append(likeStatment);

    return this;
}

QueryBuider *QueryBuider::groupBy(const QString &byField)
{
    QStringList fields = byField.split(",", QString::SkipEmptyParts);
    foreach (QString f, fields)
    {
        f = f.trimmed();
        d->groupbyFields.append(f);
    }

    return this;
}

QueryBuider *QueryBuider::having(const QString &field, const QString &value)
{
    QString have;
    if(!value.isEmpty())
    {
        have = field + "='" + value + "'";
    }
    d->havingConditions.append(have);
    return this;
}

QueryBuider *QueryBuider::limit(int size, int offset)
{
    d->limit = size;
    d->offset = offset;

    return this;
}

QList<Record> QueryBuider::get(int limit, int offset)
{
    if(limit > 0)
    {
        this->limit(limit, offset);
    }
    QList<Record> records;
    QString sql = d->compileSelect();
    if( d->query.exec(sql) )
    {
        while (d->query.next())
        {
            Record r;
            foreach (QString field, d->fieldNames())
            {
                r.setData(field, d->query.record().value(field));
            }
            records.append(r);
        }
        qDebug() << "Number of rows: " << records.count() ;
        qDebug() << "sql: " << sql;
    }
    else
    {
        QString error = d->query.lastError().databaseText();
        qDebug() << "exec query failue: " << error;
        qDebug() << "sql: " << sql;
    }

    return records;
}

void QueryBuider::trackAliases(const QString &table)
{

}

QStringList QueryBuiderPrivate::fieldNames() const
{
    QStringList fieldNames;
    QSqlRecord record = DatabaseManager::database().record(table);
    int fieldCount = record.count();
    for(int i = 0; i < fieldCount; ++i)
    {
        fieldNames.append(record.fieldName(i));
    }

    return fieldNames;
}

QString QueryBuiderPrivate::compileSelect()
{
    // select
    QString sql = isDistinct ? "SELECT DISTINCT ": "SELECT ";
    if(selects.isEmpty())
    {
        sql.append("*");
    }
    else
    {
        foreach (const QString select, selects)
        {
            // TODO: checking field exists
            // TODO: checking condition and aliase
            // ...
        }
        sql += selects.join(",");
    }

    // from
    if(!froms.isEmpty())
    {
        sql += " FROM " + this->froms.join(",");
    }

    // joins
    if(!this->joins.isEmpty())
    {
        sql += "\n" + this->joins.join("\n");
    }

    // TODO: conditions
    sql += compileWhere();
    sql += compileGroupBy();
    sql += compileHaving();

    // limit and offset
    sql += compileLimit();

    return sql;
}

QString QueryBuiderPrivate::compileWhere()
{
    QString condition;
    if(!whereConditions.isEmpty())
    {
        bool start = true;
        foreach (QString cond, whereConditions)
        {
            condition.append(start ? " WHERE " : " AND WHERE ");
            condition.append(cond);
            start = false;
        }
    }

    return condition;
}

QString QueryBuiderPrivate::compileGroupBy()
{
    QString condition;
    if(!groupbyFields.isEmpty())
    {
        QStringList list = fieldNames();
        foreach (const QString &field, groupbyFields)
        {
           // checing field exists
            if( !list.contains(field) )
            {
                groupbyFields.removeAll(field);
            }
        }

        if(groupbyFields.isEmpty())
        {
            return condition;
        }
        else if(groupbyFields.size() == 1)
        {
            condition = " GROUP BY " + groupbyFields.at(0);
        }
        else
        {
            condition = " GROUP BY " + groupbyFields.join(",");
        }
    }

    return condition;
}

QString QueryBuiderPrivate::compileHaving()
{
    QString condition;
    if(!havingConditions.isEmpty())
    {
        bool start = true;
        foreach (QString cond, havingConditions)
        {
            condition.append(start ? " HAVING " : " AND HAVING ");
            condition.append(cond);
            start = false;
        }
    }

    return condition;
}

bool QueryBuiderPrivate::hasOperator(const QString &str)
{
    QRegExp rx("(<|>|!|=|\\sIS NULL|\\sIS NOT NULL|\\sEXISTS|\\sBETWEEN|\\sLIKE|\\sIN\\s*\\(|\\s)");
    return rx.exactMatch(str.trimmed());
}

QString QueryBuiderPrivate::getOperator(const QString &str)
{
    static QStringList operators;
    if (operators.isEmpty())
    {
        operators << "(\\s*(?:<|>|!)?=\\s*)"    // =, <=, >=, !=
                  << "(\\s*<>?\\s*)"            // <, <>
                  << "(\\s*>\\s*)"              // >
                  << "(\\s+IS NULL)"            // IS NULL
                  << "(\\s+IS NOT NULL)"        // IS NOT NULL
                  << "(\\s+EXISTS\\s*\\(.*\\))"         // EXISTS(sql)
                  << "(\\s+NOT EXISTS\\s*\\(.*\\))"     // NOT EXISTS(sql)
                  << "(\\s+BETWEEN\\s+)"                // BETWEEN value AND value
                  << "(\\s+IN\\s*\\(.*\\))"             // IN(list)
                  << "(\\s+NOT IN\\s*\\(.*\\))"         // NOT IN (list)
                  << "(\\s+LIKE\\s+\\S.*(%0)?)"         // LIKE 'expr'[ ESCAPE '%s']
                  << "(\\s+NOT LIKE\\s+\\S.*(%0)?";     // NOT LIKE 'expr'[ ESCAPE '%s']
    }

    QString ops = operators.join("|");
    QRegExp rx(ops);
    if( rx.indexIn(str) != -1)
    {
        return rx.cap(1);
    }

    return QString();
}

bool QueryBuiderPrivate::whereHaving(const QString &key, const QString &value,
                                     ConditionOp op)
{
    if(key.isEmpty())
        return false;

    QString prefix = op == NONE ? "" : "AND";
    QString keyWithOp = key;

    if(!value.isNull())
    {
        // =
        QString op = this->getOperator(key);
        QString keyWithoutOp = keyWithOp.replace(op, "").trimmed();

    }
    else if(!this->hasOperator(keyWithOp))
    {
        keyWithOp += " IS NULL";
    }
    else if(this->regMatch("\\s*(!?=|<>|IS(?:\\s+NOT)?)\\s*$", keyWithOp))
    {

    }

    QString cond;
    conditionMapper[op] = cond.append(prefix).append(key).append(value);
}

QString QueryBuiderPrivate::compileLimit()
{
    if(limit <= 0)
        return "";

    QString _offset = offset <= 0 ? "" : QString("%0, ").arg(offset);
    return " LIMIT " + _offset + QString("%0").arg(limit);
}


bool QueryBuiderPrivate::regMatch(const QString &pattern, const QString &text, QString &capture)
{
    QRegExp rx(pattern, Qt::CaseInsensitive);
    bool ok = rx.exactMatch(text.trimmed());
    if(ok)
    {
        capture = rx.cap(1);
    }

    return ok;
}
