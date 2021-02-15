#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QImage>
#include <QStatusBar>
#include <QPixmap>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>
#include <opencv.hpp>

//采用宏定义ColorConventionCode避免编译器报warning，并减少链接
#define COLOR_BGR2GRAY (6)
#define COLOR_GRAY2BGR (8)
#define COLOR_YUV2BGR (84)
#define COLOR_BGR2YUV (82)
#define COLOR_YCrCb2BGR (38)
#define COLOR_BGR2YCrCb (36)
#define COLOR_HSV2BGR (54)
#define COLOR_BGR2HSV (40)
#define COLOR_XYZ2BGR (34)
#define COLOR_BGR2XYZ (32)
#define COLOR_Lab2BGR (56)
#define COLOR_BGR2Lab (44)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    pstausBar = statusBar();
    label_statusbar = new QLabel(this);
    label_statusbar->setText("Loading...");
    pstausBar->addWidget(label_statusbar);

    //push_back菜单
    vecMenu.push_back(new QMenu("File"));
    vecMenu.push_back(new QMenu("Mode"));

    //push_back Action
    vecAction_0.push_back(new QAction("path..."));

    //push_back Action
    vecAction_1.push_back(new QAction("show RGB"));
    vecAction_1.at(0)->setObjectName("RGB");
    vecAction_1.push_back(new QAction("Gamma correction"));
    vecAction_1.at(1)->setObjectName("Gamma");
    vecAction_1.push_back(new QAction("to Binary"));
    vecAction_1.at(2)->setObjectName("Binary");
    vecAction_1.push_back(new QAction("to Gray"));
    vecAction_1.at(3)->setObjectName("GRAY");
    vecAction_1.push_back(new QAction("to YUV"));
    vecAction_1.at(4)->setObjectName("YUV");
    vecAction_1.push_back(new QAction("to YCrCb"));
    vecAction_1.at(5)->setObjectName("YCrCb");
    vecAction_1.push_back(new QAction("to HSV"));
    vecAction_1.at(6)->setObjectName("HSV");
    vecAction_1.push_back(new QAction("to XYZ"));
    vecAction_1.at(7)->setObjectName("XYZ");
    vecAction_1.push_back(new QAction("to Lab"));
    vecAction_1.at(8)->setObjectName("Lab");


    //调用相应的putinXXX()函数
    putInMenu();
    putInAction(vecMenu.at(0), vecAction_0);
    putInAction(vecMenu.at(1), vecAction_1, true);


    //添加信号与槽
    getconnect(vecAction_1, &MainWindow::deal_changeTo);
    connect(vecAction_0.at(0), &QAction::triggered, this, &MainWindow::deal_readPath);
    connect(ui->runButton, &QPushButton::clicked, this, &MainWindow::colorChange);
    //链接滑块与label显示
    connect(ui->binarySlider, &QSlider::valueChanged, this, [=]() {
        ui->binaryLabel->setNum(ui->binarySlider->value()); });
    connect(ui->gammaSlider, &QSlider::valueChanged, this, [=]() {
        ui->gammaLabel->setNum(ui->gammaSlider->value() / 10.0); });
    ui->binarySlider->setMinimum(0);
    ui->binarySlider->setMaximum(255);
    ui->gammaSlider->setMinimum(0);
    ui->gammaSlider->setMaximum(100);
    ui->binarySlider->setValue(130);
    ui->gammaSlider->setValue(22);
    //链接滑块与图像显示
    connect(ui->binarySlider, &QSlider::valueChanged, this, &MainWindow::connectBinarySlid);
    connect(ui->gammaSlider, &QSlider::valueChanged, this, &MainWindow::connectGammaSlid);

    label_statusbar->setText("Done");
}

MainWindow::~MainWindow()
{
    delete ui;
}

////////////////////////public function////////////////////////
///////////////////////////槽函数///////////////////////////
//设置源图像读取
void MainWindow::deal_readPath() {
    label_statusbar->setText("read path...");
    QString path = QFileDialog::getOpenFileName(this, "open", "../",
                                                "All(*.*);;"
                                                "JPEG files(*.jpg *.jpeg *.jpe);;"
                                                "Portable Network Graphics(*.png)");
    this->m_inPath = qstr2str_forZN_CH(path);
    srcimg = cv::imread(m_inPath);
    ui->channleLabel->setNum(srcimg.type());
    ui->dimsLabel->setNum(srcimg.dims);
    ui->sizeLabel->setNum(srcimg.cols);
    ui->sizeLabel_2->setNum(srcimg.rows);
    label_statusbar->setText("Done");
}
//控制Mode选单
void MainWindow::deal_changeTo() {
    label_statusbar->setText("Select");
    QObject *p = QObject::sender();
    QString objName = p->objectName();
    for (auto each : vecAction_1) {
        if (!each->objectName().compare(objName))
            (*each).setChecked(true);
        else
            (*each).setChecked(false);
    }
    label_statusbar->setText("Done");
}
//伽马校正
void MainWindow::connectGammaSlid() {
    using namespace cv;
    if (srcimg.empty() || !vecAction_1.at(1)->isChecked())
        return;

    unsigned char lut[256];
    //创建查找表，提高伽马校正效率
    for (int i = 0; i < 256; i++) {
        lut[i] = saturate_cast<uchar>(pow((float)(i / 255.0), ui->gammaSlider->value() / 10.0) * 255.0f);
    }
    proimg = srcimg.clone();
    const int channels = proimg.channels();
    switch (channels)
    {
    case 1:
        for (MatIterator_<uchar> it = proimg.begin<uchar>(), end = proimg.end<uchar>(); it != end; it++)
            *it = lut[(*it)];
        break;

    case 3:
        for (MatIterator_<Vec3b> it = proimg.begin<Vec3b>(), end = proimg.end<Vec3b>(); it != end; it++) {
            (*it)[0] = lut[((*it)[0])];
            (*it)[1] = lut[((*it)[1])];
            (*it)[2] = lut[((*it)[2])];
        }
        break;
    }

    ui->center_img->setPixmap(QPixmap::fromImage(QImage((const unsigned char *)(proimg.data), proimg.cols, proimg.rows, proimg.step, QImage::Format_BGR888)));
    ui->center_img->show();
}
//二值图
void MainWindow::connectBinarySlid() {
    if (srcimg.empty() || !vecAction_1.at(2)->isChecked())
        return;

    cv::cvtColor(srcimg, proimg, COLOR_BGR2GRAY);
    cv::threshold(proimg, proimg, ui->binarySlider->value(), 255, cv::THRESH_BINARY);

    ui->center_img->setPixmap(QPixmap::fromImage(QImage((const unsigned char *)(proimg.data), proimg.cols, proimg.rows, proimg.step, QImage::Format_Grayscale8)));
    ui->center_img->show();
}
//验证颜色空间转换前置条件
void MainWindow::colorChange() {
    label_statusbar->setText("Checking...");
    //验证当前状态
    //path
    if (m_inPath.empty()) {
        label_statusbar->setText("Error");
        QMessageBox::warning(this, "Warning", "source_Path is empty !");
        label_statusbar->setText("Done");
        return;
    }
    //读取图像
    label_statusbar->setText("Loading...");
    srcimg = cv::imread(m_inPath);
    label_statusbar->setText("Checking...");
    if (srcimg.empty()) {
        label_statusbar->setText("Error");
        QMessageBox::warning(this, "Error", "Image loading failed ! source_Path may be wrong");
        label_statusbar->setText("Done");
        return;
    }
    //mode_From
    label_statusbar->setText("Checking...");
    QString toName;
    int count = 1;
    //mode_TO
    for (auto each : vecAction_1) {
        if (each->isChecked()) {
            toName = each->objectName();
            break;
        }
        if (count == 9) {
            label_statusbar->setText("Warning");
            QMessageBox::warning(this, "Warning", "You have to chooes a To mode");
            label_statusbar->setText("Done");
            return;
        }
        ++count;
    }
    //调用selectFromMode
    //调用changeColorSpace
    label_statusbar->setText("Select done, running...");
    changeColorSpace(selectFromMode(toName));
}
////////////////////////privat function////////////////////////
///////////////////////////窗体方法///////////////////////////
//添入菜单
void MainWindow::putInMenu() {
    for (auto each : vecMenu) {
        ui->menubar->addMenu(each);
    }
}
//添入Action
void MainWindow::putInAction(QMenu *menu, std::vector<QAction *> &vecaction, bool flag) {
    for (auto each : vecaction) {
        (*menu).addAction(each);
        if (flag)
            each->setCheckable(true);
    }
}
//批量连接信号与槽
void MainWindow::getconnect(std::vector<QAction *> &action, void(MainWindow:: *pslot)()) {
    for (auto each : action) {
        connect(each, &QAction::triggered, this, pslot);
    }
}

///////////////////////////应用方法///////////////////////////
//转换颜色空间
void MainWindow::changeColorSpace(int colorCode) {

    ////虚创建pixmap，设置图像在label中显示的大小
    ////!!!考虑优化，采用函数解决图像显示大小!!!
    ////!!!图像过大时，ui界面卡顿现象严重，考虑使用多线程!!!
    ////!!!空间复杂度过大，考虑优化数据结构!!!
    //使用Release编译后，代码得到优化，未出现卡顿现象
    QPixmap *ppixMap = new QPixmap(str2qstr(m_inPath));
    ppixMap->scaled(ui->left_img->size(), Qt::KeepAspectRatio);
    ppixMap->scaled(ui->center_img->size(), Qt::KeepAspectRatio);
    ppixMap->scaled(ui->right_img->size(), Qt::KeepAspectRatio);
    //使得图像充满label
    ui->left_img->setScaledContents(true);
    ui->center_img->setScaledContents(true);
    ui->right_img->setScaledContents(true);

    if (vecAction_1.at(0)->isChecked()) {
        if (ui->reverseRadio->isChecked()) {
            label_statusbar->setText("Error");
            QMessageBox::warning(this, "Error", "This Mode is used only to display RGB components"
                                                " and cannot be reversed");
            label_statusbar->setText("Done");
            ui->reverseRadio->setChecked(false);
            return;
        }
        showRGB(srcimg);
        label_statusbar->setText("Done");
        return;

    }
    if (vecAction_1.at(1)->isChecked()) {
        if (ui->reverseRadio->isChecked()) {
            label_statusbar->setText("Error");
            QMessageBox::warning(this, "Error", "Just gamma correction, no reverse conversion");
            label_statusbar->setText("Done");
            ui->reverseRadio->setChecked(false);
            return;
        }
        gammaCorrection();
        label_statusbar->setText("Done");
        return;
    }
    if (vecAction_1.at(2)->isChecked()) {
        if (ui->reverseRadio->isChecked()) {
            label_statusbar->setText("Error");
            QMessageBox::warning(this, "Error", "Binary graphs cannot be converted to RGB");
            label_statusbar->setText("Done");
            ui->reverseRadio->setChecked(false);
            return;
        }
        toBinary();
        label_statusbar->setText("Done");
        return;
    }
    //转换颜色空间
    cv::cvtColor(srcimg, proimg, colorCode);

    switch (colorCode)
    {
    case COLOR_BGR2GRAY:
        ui->left_channle->setText("");
        ui->center_channle->setText("Gray");
        ui->right_channle->setText("");
        ui->center_img->setPixmap(QPixmap::fromImage(QImage((const unsigned char *)(proimg.data), proimg.cols, proimg.rows, proimg.step, QImage::Format_Grayscale8)));
        ui->left_channle->clear();
        ui->right_img->clear();
        ui->center_img->show();
        break;
    case COLOR_GRAY2BGR:
        showRGB(proimg);
        break;

    case COLOR_BGR2YUV:
        if(ui->colorfulRadio->isChecked())
            mergeImg("Y", "U", "V");
        else
            showImg("Y", "U", "V");
        break;
    case COLOR_YUV2BGR:
        showRGB(proimg);
        break;

    case COLOR_BGR2YCrCb:
        if(ui->colorfulRadio->isChecked())
            mergeImg("Y", "Cr", "Cb");
        else
            showImg("Y", "Cr", "Cb");
        break;
    case COLOR_YCrCb2BGR:
        showRGB(proimg);
        break;

    case COLOR_BGR2HSV:
        if(ui->colorfulRadio->isChecked())
            mergeImg("H", "S", "V");
        else
            showImg("H", "S", "V");
        break;
    case COLOR_HSV2BGR:
        showRGB(proimg);
        break;

    case COLOR_BGR2XYZ:
        if(ui->colorfulRadio->isChecked())
            mergeImg("X", "Y", "Z");
        else
            showImg("X", "Y", "Z");
        break;
    case COLOR_XYZ2BGR:
        showRGB(proimg);
        break;

    case COLOR_BGR2Lab:
        if(ui->colorfulRadio->isChecked())
            mergeImg("X", "Y", "Z");
        else
            showImg("L", "a", "b");
        break;
    case COLOR_Lab2BGR:
        showRGB(proimg);
        break;
    default:
        label_statusbar->setText("Error");
        QMessageBox::warning(this, "Error", "Unexpected error!");
        label_statusbar->setText("Done");
        break;
    }
    label_statusbar->setText("Done");
}
//显示RGB
void MainWindow::showRGB(cv::Mat img) {
    ui->left_channle->setText("R");
    ui->center_channle->setText("G");
    ui->right_channle->setText("B");
    cv::split(img, Rsingleimg);
    cv::split(img, Gsingleimg);
    cv::split(img, Bsingleimg);
    Rsingleimg[0] = 0;
    Rsingleimg[1] = 0;
    Gsingleimg[0] = 0;
    Gsingleimg[2] = 0;
    Bsingleimg[1] = 0;
    Bsingleimg[2] = 0;
    cv::merge(Rsingleimg, 3, Rimg);
    cv::merge(Gsingleimg, 3, Gimg);
    cv::merge(Bsingleimg, 3, Bimg);
    ui->left_img->setPixmap(QPixmap::fromImage(QImage((const unsigned char *)(Rimg.data), Rimg.cols, Rimg.rows, Rimg.step, QImage::Format_BGR888)));
    ui->left_img->show();
    ui->center_img->setPixmap(QPixmap::fromImage(QImage((const unsigned char *)(Gimg.data), Gimg.cols, Gimg.rows, Gimg.step, QImage::Format_BGR888)));
    ui->center_img->show();
    ui->right_img->setPixmap(QPixmap::fromImage(QImage((const unsigned char *)(Bimg.data), Bimg.cols, Bimg.rows, Bimg.step, QImage::Format_BGR888)));
    ui->right_img->show();
}
//转成二值图
void MainWindow::toBinary() {
    ui->left_channle->setText("");
    ui->center_channle->setText("Binary");
    ui->right_channle->setText("");
    ui->left_img->clear();
    ui->right_img->clear();

    connectBinarySlid();
}
//伽马校正
void MainWindow::gammaCorrection() {
    ui->left_channle->setText("");
    ui->center_channle->setText("Gamma Correction");
    ui->right_channle->setText("");
    ui->left_img->clear();
    ui->right_img->clear();

    connectGammaSlid();
}

/////////////////////////底层依赖方法/////////////////////////
//确定颜色空间转换方法
int MainWindow::selectFromMode(QString toMode) {
    label_statusbar->setText("Select checking...");
    hash_t Mode = hash_string(qstr2str_forZN_CH(toMode));
    switch (Mode) {
    case "GRAY"_hash: return COLOR_BGR2GRAY;
    case "YUV"_hash: return ui->reverseRadio->isChecked() ? COLOR_YUV2BGR : COLOR_BGR2YUV;
    case "YCrCb"_hash: return ui->reverseRadio->isChecked() ? COLOR_YCrCb2BGR : COLOR_BGR2YCrCb;
    case "HSV"_hash: return ui->reverseRadio->isChecked() ? COLOR_HSV2BGR : COLOR_BGR2HSV;
    case "XYZ"_hash: return ui->reverseRadio->isChecked() ? COLOR_XYZ2BGR : COLOR_BGR2XYZ;
    case "Lab"_hash: return ui->reverseRadio->isChecked() ? COLOR_Lab2BGR : COLOR_BGR2Lab;
    default: return -1;
    }
}
void MainWindow::showImg(QString str1, QString str2, QString str3) {
    ui->left_channle->setText(str1);
    ui->center_channle->setText(str2);
    ui->right_channle->setText(str3);
    cv::split(proimg, singleimg);
    ui->left_img->setPixmap(QPixmap::fromImage(QImage((const unsigned char *)(singleimg[0].data), singleimg[0].cols, singleimg[0].rows, singleimg[0].step, QImage::Format_Grayscale8)));
    ui->left_img->show();
    ui->center_img->setPixmap(QPixmap::fromImage(QImage((const unsigned char *)(singleimg[1].data), singleimg[1].cols, singleimg[1].rows, singleimg[1].step, QImage::Format_Grayscale8)));
    ui->center_img->show();
    ui->right_img->setPixmap(QPixmap::fromImage(QImage((const unsigned char *)(singleimg[2].data), singleimg[2].cols, singleimg[2].rows, singleimg[2].step, QImage::Format_Grayscale8)));
    ui->right_img->show();
}
//彩色显示分量
void MainWindow::mergeImg(QString str1, QString str2, QString str3){
    ui->left_channle->setText(str1);
    ui->center_channle->setText(str2);
    ui->right_channle->setText(str3);
    cv::split(proimg, Rsingleimg);
    cv::split(proimg, Gsingleimg);
    cv::split(proimg, Bsingleimg);
    Rsingleimg[0] = 0;
    Rsingleimg[1] = 0;
    Gsingleimg[0] = 0;
    Gsingleimg[2] = 0;
    Bsingleimg[1] = 0;
    Bsingleimg[2] = 0;
    cv::merge(Rsingleimg, 3, Rimg);
    cv::merge(Gsingleimg, 3, Gimg);
    cv::merge(Bsingleimg, 3, Bimg);
    ui->left_img->setPixmap(QPixmap::fromImage(QImage((const unsigned char *)(Rimg.data), Rimg.cols, Rimg.rows, Rimg.step, QImage::Format_BGR888)));
    ui->left_img->show();
    ui->center_img->setPixmap(QPixmap::fromImage(QImage((const unsigned char *)(Gimg.data), Gimg.cols, Gimg.rows, Gimg.step, QImage::Format_BGR888)));
    ui->center_img->show();
    ui->right_img->setPixmap(QPixmap::fromImage(QImage((const unsigned char *)(Bimg.data), Bimg.cols, Bimg.rows, Bimg.step, QImage::Format_BGR888)));
    ui->right_img->show();
}
//转换编码以适配中文
std::string MainWindow::qstr2str_forZN_CH(const QString qstr) {
    std::string tempQstr = qstr.toUtf8().data();
    QString temp = QString::fromUtf8(tempQstr.c_str());
    return temp.toLocal8Bit().data();
}
QString MainWindow::str2qstr(const std::string str) {
    return QString::fromLocal8Bit(str.data());
}
//string转hash
hash_t MainWindow::hash_string(std::string str) {
    hash_t ret{ basis };
    for (auto each : str) {
        ret ^= each;
        ret *= prime;
    }
    return ret;
}
