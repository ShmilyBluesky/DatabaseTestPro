#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QDir>
#include <QSqlDatabase>
#include <QElapsedTimer>
#include <QDateTime>
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>

// 数据库相关路径宏定义
#define DATABASE_DIR                        "/database"
#define DATABASE_FILENAME4SQLITE            "/sqlitedb.db"
#define DATABASE_FILENAME4MYSQL             "/mysqldb.db"

#define TimeFormat "yyyy-MM-dd hh:mm:ss.zzz"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    // 创建文件夹
    QDir dir(QString(".") + DATABASE_DIR);
    if (!dir.exists())
        QDir().mkpath(QString(".") + DATABASE_DIR);
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::on_btn_createTable_clicked()
{
    handler(CreateTable);
}

void MainWidget::handler(MainWidget::HandType type)
{
    if (!isMysql())
        return;

    // 耗时计时
    QElapsedTimer eTimer;
    eTimer.start();

    // 数据库类型
    QString strConnectName("mysqlconnect");

    if (!QSqlDatabase::contains(strConnectName))
    {
        // 数据库初始化
        QSqlDatabase dbObj = QSqlDatabase::addDatabase("QMYSQL", strConnectName);
        dbObj.setHostName("localhost");
        dbObj.setPort(3309);
        dbObj.setUserName("root");
        dbObj.setPassword("newpwd");
        dbObj.setDatabaseName("testmysql");

        // 打开数据库
        if (dbObj.isOpen() || (!dbObj.isOpen() && dbObj.open()))
        {
            // 新建查询工具
            QSqlQuery *pQuery = new QSqlQuery(dbObj);

            // 处理
            process(&dbObj, pQuery, type);

            // 关闭数据库
            dbObj.close();

            // 删除查询工具
            delete pQuery;
        }
        else
        {
            qDebug() << tr("Open1 db fail:%1").arg(dbObj.lastError().text());
        }

        dbObj = QSqlDatabase();
        QSqlDatabase::removeDatabase(strConnectName);
    }
    else
    {
        // 数据库初始化
        QSqlDatabase dbObj = QSqlDatabase::database(strConnectName);

        // 打开数据库
        if (dbObj.isOpen() || (!dbObj.isOpen() && dbObj.open()))
        {
            // 新建查询工具
            QSqlQuery *pQuery = new QSqlQuery(dbObj);

            // 处理
            process(&dbObj, pQuery, type);

            // 关闭数据库
            dbObj.close();

            // 删除查询工具
            delete pQuery;
        }
        else
        {
            qDebug() << tr("Open1 db fail:%1").arg(dbObj.lastError().text());
        }

        dbObj = QSqlDatabase();
        QSqlDatabase::removeDatabase(strConnectName);
    }

    // 打印耗时日志
    QString strCost("");
    strCost += getCurrentTime();
    strCost += QString(" : ") + QString("all cost: ") + QString::number(eTimer.elapsed());
    addResultLog(strCost);
}

void MainWidget::process(QSqlDatabase *db, QSqlQuery *pQuery, MainWidget::HandType type)
{
    // 创建数据库表
    if (CreateTable == type)
    {
        // 如果表不存在，则自动创建表
        if (!db->tables().contains(ui->edit_tableName->text(), Qt::CaseInsensitive))
        {
            if (!pQuery->exec(ui->edit_createTable->text()))
                qDebug() << "create error: " << ui->edit_createTable->text() << " :::: " << pQuery->lastError().text();
        }
    }

    // 新增数据
    if (Insert == type)
    {
        QStringList list = ui->edit_insertData->text().split(";");
        for (int i=0; i<list.size(); i++)
        {
            QString strData = list.at(i);

            QElapsedTimer eTimer;
            eTimer.start();

            QString strQuery = QString(ui->edit_insertState->text()).arg(strData);
            if (!pQuery->exec(strQuery))
                qDebug() << "insert error: " << strQuery << " :::: " << pQuery->lastError().text();

            // 打印耗时日志
            QString strCost("");
            strCost += getCurrentTime();
            strCost += QString(" : ") + QString("insert cost: ") + QString::number(i) + " " + QString::number(eTimer.elapsed());
            addResultLog(strCost);
        }
    }

    // 删除数据
    if (Delete == type)
    {
        QElapsedTimer eTimer;
        eTimer.start();

        QString strQuery = QString(ui->edit_deleteState->text()).arg(ui->edit_deleteData->text());
        if (!pQuery->exec(strQuery))
            qDebug() << "delete error: " << strQuery << " :::: " << pQuery->lastError().text();

        // 打印耗时日志
        QString strCost("");
        strCost += getCurrentTime();
        strCost += QString(" : ") + QString("delete cost: ") + " " + QString::number(eTimer.elapsed());
        addResultLog(strCost);
    }

    // 改数据
    if (Mod == type)
    {
        QElapsedTimer eTimer;
        eTimer.start();

        QString strQuery = QString(ui->edit_modState->text()).arg(ui->edit_modData->text()).arg(ui->edit_modKey->text());
        if (!pQuery->exec(strQuery))
            qDebug() << "create error: " << strQuery << " :::: " << pQuery->lastError().text();

        // 打印耗时日志
        QString strCost("");
        strCost += getCurrentTime();
        strCost += QString(" : ") + QString("mod cost: ") + " " + QString::number(eTimer.elapsed());
        addResultLog(strCost);
    }

    // 查询数据
    if (Query == type)
    {
        QElapsedTimer eTimer;
        eTimer.start();

        QString strQuery = QString(ui->edit_queryState->text()).arg(ui->edit_queryData->text());
        if (!pQuery->exec(strQuery))
            qDebug() << "query error: " << strQuery << " :::: " << pQuery->lastError().text();

        pQuery->next();
        QString strKeys = ui->edit_queryResultKey->text();
        QStringList listKeys = strKeys.split(",");
        QString strResult("");
        foreach (const QString& value, listKeys)
        {
            QString tmpResult = pQuery->value(pQuery->record().indexOf(value)).toString();
            if (strResult.length() > 0)
                strResult += ", ";
            strResult += tmpResult;
        }

        // 打印耗时日志
        QString strCost("");
        strCost += getCurrentTime();
        strCost += QString(" : ") + QString("query cost: ") + " " + QString::number(eTimer.elapsed());
        addResultLog(strCost);

        ui->edit_queryResult->setText(strResult);
    }
}

QString MainWidget::getCurrentTime() const
{
    return QDateTime::currentDateTime().toString(TimeFormat);
}

void MainWidget::addResultLog(const QString &log)
{
    QString str = ui->edit_result->toPlainText();
    if (str.length() > 0)
        str += "\n";
    str += log;
    ui->edit_result->setText(str);
}

bool MainWidget::isSqlite()
{
    return ui->radio_sqlite->isChecked();
}

bool MainWidget::isMysql()
{
    return ui->radio_mysql->isChecked();
}

void MainWidget::on_btn_insert_clicked()
{
    handler(Insert);
}

void MainWidget::on_btn_delete_clicked()
{
    handler(Delete);
}

void MainWidget::on_btn_mod_clicked()
{
    handler(Mod);
}

void MainWidget::on_btn_query_clicked()
{
    handler(Query);
}
