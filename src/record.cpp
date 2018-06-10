#include <QVariant>

#include "record.h"

Record::Record(const QString &pk, QObject *parent)
    : QObject(parent)
    , m_primaryKey(pk)
{

}

Record::Record(const Record &other)
{
    m_data = other.m_data;
}

Record &Record::operator =(const Record &other)
{
    m_data = other.m_data;
    return *this;
}

bool Record::operator ==(const Record &other)
{
    return m_data[m_primaryKey] == other.m_data[m_primaryKey];
}

Record::~Record()
{
    m_data.clear();
}

void Record::setData(const QString &field, const QVariant &value)
{
    if(!field.isEmpty())
        m_data.insert(field, value);
}

QVariant Record::data(const QString &field) const
{
    if(field.isEmpty())
        return QVariant();

    return m_data.value(field, QVariant());
}

int Record::id() const
{
    QVariant val = m_data.value(m_primaryKey, QVariant(0));
    return val.toInt();
}

int Record::count() const
{
    return m_data.size();
}

bool Record::isValid() const
{
    return !m_data.isEmpty();
}

bool Record::isEmpty() const
{
    return m_data.isEmpty();
}

bool Record::contains(const QString &fieldName) const
{
    return m_data.keys().contains(fieldName);
}

QList<QString> Record::fieldNames() const
{
    return m_data.keys();
}

void Record::clear()
{
    m_data.clear();
}
