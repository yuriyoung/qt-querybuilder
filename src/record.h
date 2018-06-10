#ifndef RECORD_H
#define RECORD_H

#include <QObject>
#include <QMap>

class Record : public QObject
{
    Q_OBJECT
public:

    explicit Record(const QString &pk = "id", QObject *parent = nullptr);
    Record(const Record &other);
    Record &operator =(const Record &other);
    bool operator ==(const Record &other);

    virtual ~Record();

    virtual void setData(const QString &field, const QVariant &value);
    virtual QVariant data(const QString &field) const;
    int id() const;
    int count() const;
    bool isValid() const;
    bool isEmpty() const;
    bool contains(const QString &fieldName) const;
    QList<QString> fieldNames() const;
    void clear();

signals:

public slots:

private:
    QString m_primaryKey{"id"};
    QMap<QString, QVariant> m_data;
};

#endif // RECORD_H
