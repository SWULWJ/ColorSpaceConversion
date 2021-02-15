#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <vector>
#include <opencv.hpp>

//定义hash值数据类型
typedef std::uint64_t hash_t;
//定义hash值产生基底
constexpr hash_t prime = 0x100000001B3ull;
constexpr hash_t basis = 0xCBF29CE484222325ull;
//constexpr函数自定义string转hash
//通过递归生成hash值
constexpr hash_t hash_char(char const *str, hash_t last_value = basis) {
    return *str ? hash_char(str + 1, (*str ^ last_value) * prime) : last_value;
}
//重载""_hash生成hash值
constexpr size_t operator "" _hash(char const *p, size_t) {
    return hash_char(p);
}

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void deal_readPath();
    void deal_changeTo();
    void connectGammaSlid();
    void connectBinarySlid();
    void colorChange();

private:
    //应用ui文件
    Ui::MainWindow *ui;

    //应用变量
    std::string m_inPath;//读入路径

    cv::Mat srcimg;//源图像
    cv::Mat proimg;//处理后图像
    cv::Mat singleimg[3];//分离多通道
    cv::Mat Rsingleimg[3];//R通道处理
    cv::Mat Gsingleimg[3];//G通道处理
    cv::Mat Bsingleimg[3];//B通道处理
    cv::Mat Rimg;//R通道
    cv::Mat Gimg;//B通道
    cv::Mat Bimg;//G通道

    //窗体设计变量
    std::vector<QMenu *> vecMenu;//菜单容器，在容器中添加菜单即可自动实现菜单的加入
    std::vector<QAction *> vecAction_0;//Action容器_0，在容器中添加动作即可自动实现Action的加入
    std::vector<QAction *> vecAction_1;//Action容器_1，在容器中添加动作即可自动实现Action的加入
    QStatusBar *pstausBar;//窗体状态栏指针
    QLabel *label_statusbar;//窗体状态显示

    //窗体方法
    void putInMenu();//添入菜单
    void putInAction(QMenu *, std::vector<QAction *> &, bool flag = false);//添入Action
    void getconnect(std::vector<QAction *> &, void(MainWindow:: *)());//批量连接信号与槽

    //应用方法
    void changeColorSpace(int);//转换颜色空间
    void showRGB(cv::Mat);//显示RGB分量
    void toBinary();//转成二值图
    void gammaCorrection();//伽马校正

    //底层依赖方法
    int selectFromMode(QString);//确定颜色空间转换方法
    void showImg(QString, QString, QString);//图像分量显示
    void mergeImg(QString, QString, QString);//图像分量显示（彩色）
    std::string qstr2str_forZN_CH(const QString);//转换编码以适配中文
    QString str2qstr(const std::string);//string转QString
    hash_t hash_string(std::string);//string转hash
};
#endif // MAINWINDOW_H
