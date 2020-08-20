#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QList>
#include <QSettings>
#include <sliceprocessor.h>
#include <QStringListModel>
#include <QTimer>

class QGraphicsPixmapItem;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);
    void dropEvent(QDropEvent* event);


    void addFiles(QStringList pathList);

private slots:

    void on_load_project_btn_clicked();

    void on_add_files_btn_clicked();

    void on_save_project_btn_clicked();

    void on_add_folder_btn_clicked();

    void on_run_button_clicked();

    void processor_progress(int i);

    void processor_finished();

    void processor_error(const QString& error);

    void set_ui_values(bool for_preview);

    void something_changed();

    void update_preview();
    void preview_ready();

private:
    Ui::MainWindow *ui;
    QStringList images;
    QSettings settings;
    SliceProcessor processor;
    QStringListModel* model;
    QTimer updateTimer;

    QGraphicsPixmapItem* preview_item;


};

#endif // MAINWINDOW_H
