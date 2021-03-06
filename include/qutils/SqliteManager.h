#pragma once
// Qt
#include <QSqlDatabase>
#include <QSqlError>
#include <QVariantList>
// std
#include <ostream>

namespace zmc
{

class SqliteManager
{
public:
    enum class ColumnTypes : unsigned int {
        TEXT,
        PK_INTEGER,
        PK_AUTOINCREMENT,
        INTEGER,
        REAL,
        BLOB,
        NULL_TYPE,
        NONE
    };

    struct SelectOrder {
        enum class OrderType {
            ASC,
            DESC
        };

        SelectOrder() = default;

        SelectOrder(OrderType _order, QString columnName)
            : order(_order)
            , fieldName(columnName)
        {
        }

        OrderType order;
        QString fieldName;
    };

    struct ColumnDefinition {
        bool isNull = false;
        ColumnTypes type = ColumnTypes::TEXT;
        QString name;

        ColumnDefinition() = default;
        ColumnDefinition(bool _null, ColumnTypes _type, const QString _name)
            : isNull(_null)
            , type(_type)
            , name(_name)
        {}

        QString getNullText() const
        {
            return isNull ? "" : "NOT NULL";
        }
    };

    struct Index {
        Index() = default;
        Index(int c, int r)
            : column(c)
            , row(r)
        {}

        int column = 0, row = 0;
    };

    struct SqliteError {
        SqliteError() = default;

        QString query;
        QSqlError error;

        friend std::ostream &operator<<(std::ostream &os, const SqliteError &err)
        {
            os << "Sqlite Error: " << err.error.text().toStdString() << "\nQuery: " << err.query.toStdString() << std::endl;
            return os;
        }
    };

    using Constraint = std::tuple<QString/*columnName*/, QString/*value*/, QString/*AND|OR*/>;

public:
    SqliteManager();

    /**
     * @brief Creates a sqlite3 instance and returns it. If there's an error, you can get the error with getLastError().
     * If another database with the same databasePath has been opened before, returns that database connection to avoid multiple connections to the same
     * database.
     * @param databasePath
     * @param createIfFileAbsent
     * @return QSqlDatabase
     */
    QSqlDatabase openDatabase(const QString &databasePath);

    /**
     * @brief Closes the given database
     * @param database
     * @return void
     */
    void closeDatabase(QSqlDatabase &database);

    /**
     * @brief createTable
     * **Example Usage:**
     * @code
     *    SqliteManager man;
     *    QList<SqliteManager::ColumnDefinition> columns {
     *        SqliteManager::ColumnDefinition(false, SqliteManager::ColumnTypes::INTEGER, "first"),
     *        SqliteManager::ColumnDefinition(false, SqliteManager::ColumnTypes::INTEGER, "second")
     *    };
     *
     *    QSqlDatabase db = man.openDatabase("C:/Users/Furkanzmc/Desktop/test.sqlite");
     *    man.createTable(db, columns, "my_table");
     * @endcode
     * @param database
     * @param columns
     * @return bool
     */
    bool createTable(QSqlDatabase &database, const QList<ColumnDefinition> &columns, const QString &tableName);

    /**
     * @brief Returns true If the table with the given name exists, returns false otherwise.
     * @param database
     * @param tableName
     * @return bool
     */
    bool isTableExist(QSqlDatabase &database, const QString &tableName);

    /**
     * @brief Deletes the given table from the database
     * @param database
     * @param tableName
     * @return bool
     */
    bool dropTable(QSqlDatabase &database, const QString &tableName);

    /**
     * @brief Constructs a string for the WHERE queries.
     * @param values - It's a tuple where item:
     *                 #1 is column name
     *                 #2 is value
     *                 #3 is constraint (OR or AND)
     * The constraint is added to the end of the query. So for odd number of items, the query will not end with the contraint.
     * **Example Usage:**
     * @code
     *    SqliteManager man;
     *    QList<std::tuple<QString, QString, QString>> values {
     *        std::make_tuple("first", "1", "AND"),
     *        std::make_tuple("second", "1", "OR")
     *    };
     *
     *    std::cerr << man.constructWhereQuery(values) << std::endl;
     *    // Output is: WHERE first=1 AND second=1
     * @endcode
     * @return QString Returns the query WITHOUT the WHERE clause
     */
    QString constructWhereQuery(const QList<Constraint> &values);

    /**
     * @brief Executes the given query and if it is successful iterates through the results and calls the `callback` in each step.
     * You can then use the `sqlite3_stmt` to get the value of the column. You can either provide the `callback` here, or you can use
     * `onRowProcessed` callback. The iteration will continue as long as the `callback` returns `true`, if it returns `false` the operation
     * will be terminated.
     * @param database
     * @param query
     * @param callback
     *
     * **Example Usage:**
     * @code
     *    SqliteManager man;
     *    sqlite3 *db = man.openDatabase("C:/Users/Furkanzmc/Desktop/test.sqlite");
     *    const QString query = "SELECT * from new_table_2";
     *    const QVariantList data = man.executeSelectQuery(db, query);
     *    qDebug() << data;
     * @endcode
     * @return QList<QMap<QString, QVariant>>
     */
    QList<QMap<QString, QVariant>> executeSelectQuery(QSqlDatabase &database, const QString &sqlQueryStr);

    /**
     * @brief Executes a select query with the given constraints on the given table. If it succeeds,
     * the table data is returned as a QList<QMap<QString, QVariant>>.
     * If there's an error, the map will be empty.
     * @param tableName
     * @param constraints
     * @param limit
     * @param selectOrder If it is empty, it is ignored.
     * @return QList<QMap<QString, QVariant>>
     */
    QList<QMap<QString, QVariant>> getFromTable(QSqlDatabase &database, const QString &tableName, const unsigned int &limit = -1,
                                const QList<Constraint> *constraints = nullptr,
                                const SelectOrder *selectOrder = nullptr);

    /**
     * @brief Insert row(s) into the given table.
     * **Example Usage:**
     * @code
     *    qutils::SqliteManager man;
     *    QList<qutils::SqliteManager::ColumnDefinition> columns {
     *        qutils::SqliteManager::ColumnDefinition(false, qutils::SqliteManager::ColumnTypes::INTEGER, "first"),
     *        qutils::SqliteManager::ColumnDefinition(false, qutils::SqliteManager::ColumnTypes::INTEGER, "second"),
     *        qutils::SqliteManager::ColumnDefinition(false, qutils::SqliteManager::ColumnTypes::BLOB, "image")
     *    };
     *
     *    QFile file("E:/Users/Furkanzmc/Pictures/Wallpapers/batman_trilogy_2-wallpaper-1440x1080.jpg");
     *    file.open(QIODevice::ReadOnly);
     *    const QByteArray inByteArray = file.readAll();
     *
     *    QSqlDatabase db = man.openDatabase("E:/Users/Furkanzmc/Desktop/test.sqlite");
     *    man.createTable(db, columns, "my_table");
     *    QMap<QString, QVariant> map;
     *    map["first"] = 122;
     *    map["second"] = 122;
     *    map["image"] = inByteArray;
     *    man.insertIntoTable(db, "my_table", map);
     * @endcode
     * @param database
     * @param tableName
     * @param rows Rows must be a list of QVarianMap
     * @return bool
     */
    bool insertIntoTable(QSqlDatabase &database, const QString &tableName, const QMap<QString, QVariant> &row);

    /**
     * @brief Update the data in table with the new data.
     * @param database
     * @param tableName
     * @param row
     * @paragraph constraints This is required.
     * @return
     */
    bool updateInTable(QSqlDatabase &database, const QString &tableName, const QMap<QString, QVariant> &row, const QList<Constraint> &constraints);

    /**
     * @brief Deletes the records in the table acording to the given constraints. If constraints have a size of 0, then everythin is deleted.
     * @param database
     * @param tableName
     * @param constraints
     * @return
     */
    bool deleteInTable(QSqlDatabase &database, const QString &tableName, const QList<Constraint> &constraints);

    /**
     * @brief Returns true If a row with the given constraints exists.
     * @param database
     * @param constraints
     * @return
     */
    bool exists(QSqlDatabase &database, const QString &tableName, const QList<Constraint> &constraints);

    const SqliteError &getLastError() const;

    QString getColumnTypeName(const ColumnTypes &type) const;
    ColumnTypes getColumnType(const QString &typeName) const;

private:
    SqliteError m_LastError;

private:
    void updateError(QSqlDatabase &db, const QString &query = "");

};

}
