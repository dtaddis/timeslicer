#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QGraphicsPixmapItem>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settings("TimeSlicer", "petrinm")
{
    ui->setupUi(this);
    ui->statusBar->showMessage("TimeSlice 0.1");
    setAcceptDrops(true);

    model = new QStringListModel(this);
    ui->file_list->setModel(model);
    ui->file_list->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Setup update timer
    updateTimer.setSingleShot(true);
    updateTimer.setInterval(0.1);
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(update_preview()));

    // Connect to the slice processor
    connect(&processor, SIGNAL(info(QString)), ui->processor_info, SLOT(setText(QString)));
    connect(&processor, SIGNAL(error(QString)), this, SLOT(processor_error(QString)));
    connect(&processor, SIGNAL(progress(int)), this, SLOT(processor_progress(int)));
    connect(&processor, SIGNAL(finished()), this, SLOT(processor_finished()));
    connect(&processor, SIGNAL(preview_ready()), this, SLOT(preview_ready()));


    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(something_changed()));

    //preview_item->setPixmap(QPixmap("1_time.jpg"));

    ui->preview->setScene(new QGraphicsScene());
    preview_item = new QGraphicsPixmapItem();
    ui->preview->scene()->addItem(preview_item);
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_load_project_btn_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("Images (*.slc)");

    if (dialog.exec()) {


    }
}

void MainWindow::on_save_project_btn_clicked()
{

    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("Images (*.slc)");

    if (dialog.exec()) {


    }

    QSettings project(QString("configs/config.ini"), QSettings::IniFormat);

}


void MainWindow::on_add_files_btn_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("Images (*.jpg)");

    if (dialog.exec()) {

        QStringList pathList;
        QList<QUrl> urlList = dialog.selectedUrls();

        // extract the local paths of the files
        for (int i = 0; i < urlList.size(); ++i) {
            pathList.append(urlList.at(i).toLocalFile());
        }

        addFiles(pathList);
    }
}


void MainWindow::on_add_folder_btn_clicked()
{
    //
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);

    if (dialog.exec()) {

        // Find all JPEGS
        //QDir dir(dialog);
        // if (!dir.exists())
        //    qWarning("Cannot find the example directory");

        //addFiles(dialog.selectedUrls());
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    event->acceptProposedAction();
}

void MainWindow::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent* event)
{
    event->accept();
}

void MainWindow::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();

    // check for our needed mime type, here a file or a list of files
    if (mimeData->hasUrls())
    {
        QStringList pathList;
        QList<QUrl> urlList = mimeData->urls();

        // extract the local paths of the files
        for (int i = 0; i < urlList.size(); ++i) {
            pathList.append(urlList.at(i).toLocalFile());
        }

        // call a function to open the files
        addFiles(pathList);
        event->acceptProposedAction();
    }
}


void MainWindow::addFiles(QStringList pathList) {

    for (int i = 0; i < pathList.size(); ++i) {
        qDebug() << pathList[i];
        images.append(pathList[i]);
        model->setStringList(images);
    }
    images.sort();

    ui->statusBar->showMessage(QString("%1 new file(s) were added").arg(pathList.size()));

}


void MainWindow::on_run_button_clicked()
{
    ui->run_button->setEnabled(false);
    ui->progressBar->setMaximum(images.size());

    set_ui_values(false);
    processor.start();
}

void MainWindow::processor_progress(int i) {

     ui->progressBar->setValue(i);
}

void MainWindow::processor_finished() {
    //QMessageBox::information(this, "TimeSlicer", "Processing done!");
    ui->run_button->setEnabled(true);
}

void MainWindow::processor_error(const QString& error) {
    ui->processor_info->setText(error);
    ui->statusBar->showMessage(error);
    QMessageBox::critical(this, "TimeSlicer", error);
}


void MainWindow::set_ui_values(bool for_preview) {
  processor.preview = for_preview;
  processor.images = images;
  processor.angle = ui->linear_angle->value();
  processor.reverse = ui->reverse_order->isChecked();
  processor.scale_x = ui->scale_x->value();
  processor.scale_y = ui->scale_y->value();
  processor.blending = ui->blending->value();
  processor.slice_type = ui->radial_button->isChecked() ? SliceProcessor::SliceType::Radial : SliceProcessor::SliceType::Linear;
  processor.radial_coverage = ui->radial_coverage->value();
  processor.radial_start = ui->radial_start->value();
  processor.origin_x = ui->origin_x->value();
  processor.origin_y = ui->origin_y->value();
}


void MainWindow::something_changed() {
    updateTimer.start();
}


void MainWindow::update_preview() {


    // If running
    if (!processor.isFinished()) {

        if (processor.preview) {
            // Stop previous so we can start working new one
            processor.terminate();
            processor.wait();
        }
        else {
            return; // Cannot do anything!
        }
    }

    set_ui_values(true);
    processor.start();
}

void MainWindow::preview_ready() {

    QPixmap p;
    p.convertFromImage(processor.preview_image);
    preview_item->setPixmap(p);

}

