#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QSqlQuery>
#include <QSqlDatabase>

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    enum HandType
    {
        CreateTable,
        Insert,
        Delete,
        Mod,
        Query,
    };

    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

private slots:
    void on_btn_createTable_clicked();

    void on_btn_insert_clicked();

    void on_btn_delete_clicked();

    void on_btn_mod_clicked();

    void on_btn_query_clicked();

private:
    void handler(HandType type);
    void process(QSqlDatabase *db, QSqlQuery *pQuery, HandType type);

private:
    QString getCurrentTime() const;
    void addResultLog(const QString& log);
    bool isSqlite();
    bool isMysql();

private:
    Ui::MainWidget *ui;
};

#endif // MAINWIDGET_H
