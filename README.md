# qt-querybuilder
基于Qt5的数据库查询构造器的封装，避免写繁琐的slq语句

### DatabaseManager
* 单例类，用于连接本地SQLITE数据库，扩展多数据库连接

### QueryBuilder
* 数据库查询构造器，把sql语句封装成接口方式

### Record
* 数据表记录集类，向数据库插入数据行

## 使用方法
* 表查询
```
QueryBuilder builder;
QList<Record> records;
records = builder.select("*")->from("topics")->get();
```

* 条件查询
```
QueryBuilder builder;
Record record;
record = builder.select("title, content")->from("topics")->where("id", "=", "5")->get();
```


------------
qq:12319597
