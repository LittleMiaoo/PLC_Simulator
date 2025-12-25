#ifndef SIMULATIONPLATFORM_H
#define SIMULATIONPLATFORM_H

#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QLineEdit>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QButtonGroup>
#include <QGroupBox>
#include <QDoubleValidator>
#include <QResizeEvent>
#include <QPointF>
#include <QPoint>
#include <QMap>
#include <QTimer>
#include <QPalette>
#include <QScreen>
#include <QFont>
#include <QTabWidget>
#include <QPushButton>
#include <QImage>
#include <QWheelEvent>
#include <QMouseEvent>
#include <cmath>


// 前置声明
class CanvasWidget;
class ImageViewerWidget;

class SimulationPlatform : public QWidget
{
    Q_OBJECT
signals:
    void parametersChanged(double markCenterDistance, double screenRatio);

public:
    explicit SimulationPlatform(QWidget *parent = nullptr);

    // 平台控制公共接口
    void SetRealTimePlatformAbs(double x, double y, double angle);  // 绝对位置移动
    void SetRealTimePlatformRelative(double x, double y, double angle);  // 相对位置移动

    //void SetSimulationPlatformParams(double distance,double ratio);
    
    // 获取实时平台数据
    void GetRealTimePlatformData(double& x, double& y, double& angle) const;

    void SetSimulationPlatformParams(double distance,double ratio);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateBasePlatform();
    void updateRealTimePlatform();
    void updateMark1();
    void updateMark2();

private:
    // UI控件
    CanvasWidget* canvas;
    QGroupBox* controlGroup;
    
    QLineEdit *basePlatformXEdit;
    QLineEdit *basePlatformYEdit;
    QLineEdit *basePlatformAngleEdit;
    QCheckBox* showBasePlatformCheckBox;
    
    QLineEdit *realTimePlatformXEdit;
    QLineEdit *realTimePlatformYEdit;
    QLineEdit *realTimePlatformAngleEdit;
    QCheckBox* showRealTimePlatformCheckBox;
    
    QLineEdit *mark1XEdit;
    QLineEdit *mark1YEdit;
    QLineEdit *mark1AngleEdit;
    QCheckBox *mark1FollowBaseCheckBox;
    QCheckBox *ShowMark1CheckBox;
    
    QLineEdit *mark2XEdit;
    QLineEdit *mark2YEdit;
    QLineEdit *mark2AngleEdit;
    QCheckBox *mark2FollowRealTimeCheckBox;
    QCheckBox *ShowMark2CheckBox;
    
    QLineEdit* markCenterDistanceEdit;
    QLineEdit* ScreenRatio;
    QRadioButton* ShowPlatformUL;
    QRadioButton* ShowPlatformUR;
    QRadioButton* ShowPlatformDL;
    QRadioButton* ShowPlatformDR;

    // 页面切换相关
    QTabWidget* tabWidget;
    QWidget* simulationPage;      // 模拟平台页面
    QWidget* pictureShowPage;     // 图片显示页面 (PictureShow)

    // 图片显示页面控件
    ImageViewerWidget* imageViewer;
    QPushButton* setImageBtn;
    QLineEdit* zoomRatioEdit;  // 缩放倍率输入框
    QRadioButton* picShowPlatformUL;
    QRadioButton* picShowPlatformUR;
    QRadioButton* picShowPlatformDL;
    QRadioButton* picShowPlatformDR;

    void setupPictureShowPage();
    void loadDefaultImage();
    void onSetImageClicked();
    void onTabChanged(int index);

    // 数据
    struct Platform {
        double x;
        double y;
        double angle; // 角度（度）
    };
    
    struct Mark {
        double x;
        double y;
        double angle; // 角度（度）
        bool followPlatform;
    };
    
    Platform basePlatform;
    Platform realTimePlatform;
    Mark mark1;
    Mark mark2;
    
    // 绘图相关
    QPoint m_origin; // 坐标原点
    double m_Ratio;   // 屏幕分辨率比例
    double m_scale;  // 缩放比例 (像素/mm)
    double m_markSpacing; // Mark中心间距，单位mm
    double m_markHeight;
    double m_markWidth;
    double m_ScreenWidth;
    
    void setupUI();
    void setupValidators();
    void setupConnections();
    void applyStyle();
    void updateOriginAndScale();
    void drawCoordinateSystem(QPainter &painter);
    void drawPlatform(QPainter &painter, const Platform &platform, QColor color);
    void drawMark1(QPainter &painter);
    void drawMark2(QPainter &painter);
    QPointF rotatePoint(const QPointF &point, double angle);
    QPointF transformPoint(const QPointF &point);
    QPointF inverseTransformPoint(const QPointF &point);
    
    // 声明友元类，以便CanvasWidget可以访问SimulationPlatform的私有成员
    friend class CanvasWidget;
};

#endif // SIMULATIONPLATFORM_H