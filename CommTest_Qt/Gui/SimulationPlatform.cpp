#include "SimulationPlatform.h"


// Canvas widget for drawing
class CanvasWidget : public QWidget
{
public:
    CanvasWidget(SimulationPlatform* parent) : QWidget(parent), m_parent(parent)
    {
        setStyleSheet("background-color: white;");
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // 设置背景为白色
        painter.fillRect(rect(), Qt::white);
        
        // 更新原点和缩放比例
        m_parent->updateOriginAndScale();
        
        // 绘制坐标系
        m_parent->drawCoordinateSystem(painter);
        
        // 绘制基准平台
        if (m_parent->showBasePlatformCheckBox->isChecked())
        {
            m_parent->drawPlatform(painter, m_parent->basePlatform, Qt::blue);
        }

        // 绘制实时平台
        if (m_parent->showRealTimePlatformCheckBox->isChecked())
        {
            m_parent->drawPlatform(painter, m_parent->realTimePlatform, Qt::green);
        }

        // 绘制Mark1
        if (m_parent->ShowMark1CheckBox->isChecked())
        {
            m_parent->drawMark1(painter);
        }

         // 绘制Mark2
        if(m_parent->ShowMark2CheckBox->isChecked())
        {
            m_parent->drawMark2(painter);
        }
        
       
       
        
        QWidget::paintEvent(event);
    }

private:
    SimulationPlatform* m_parent;
};

SimulationPlatform::SimulationPlatform(QWidget *parent)
    : QWidget(parent)
{
    // 初始化默认值
    basePlatform = {0, 0, 0};
    realTimePlatform = {0, 0, 0};
    mark1 = {0, 0, 0, true};
    mark2 = {0, 0, 0, true};

    m_Ratio = 200.0;
    m_markSpacing = 20.0;
    
    setupUI();
    setupValidators();
    setupConnections();
    
    setWindowTitle("Simulation Platform");
    resize(800, 600);

    m_ScreenWidth = QGuiApplication::primaryScreen()->availableGeometry().width();;
    
    m_scale = m_ScreenWidth / m_Ratio; // 默认缩放比例 2像素/mm
}

// 平台控制公共接口实现
void SimulationPlatform::SetRealTimePlatformAbs(double x, double y, double angle)
{
    // 设置绝对位置
    realTimePlatformXEdit->setText(QString::number(x, 'f', 2));
    realTimePlatformYEdit->setText(QString::number(y, 'f', 2));
    realTimePlatformAngleEdit->setText(QString::number(angle, 'f', 2));
    
    // 手动触发更新
    updateRealTimePlatform();
}

void SimulationPlatform::SetSimulationPlatformParams(double markCenterDistance, double screenRatio)
{
    m_markSpacing = markCenterDistance;
    markCenterDistanceEdit->setText(QString::number(markCenterDistance, 'f', 2));
    m_Ratio = screenRatio;
    ScreenRatio->setText(QString::number(screenRatio, 'f', 2));
    m_scale = m_ScreenWidth / m_Ratio;
    updateOriginAndScale();
    
    updateMark1();
    updateMark2();
}
void SimulationPlatform::SetRealTimePlatformRelative(double x, double y, double angle)
{
    // 在当前位置基础上增加偏移
    double newX = realTimePlatform.x + x;
    double newY = realTimePlatform.y + y;
    double newAngle = realTimePlatform.angle + angle;
    
    realTimePlatformXEdit->setText(QString::number(newX, 'f', 2));
    realTimePlatformYEdit->setText(QString::number(newY, 'f', 2));
    realTimePlatformAngleEdit->setText(QString::number(newAngle, 'f', 2));
    
    // 手动触发更新
    updateRealTimePlatform();

}

void SimulationPlatform::GetRealTimePlatformData(double& x, double& y, double& angle) const
{
    x = realTimePlatform.x;
    y = realTimePlatform.y;
    angle = realTimePlatform.angle;
}

void SimulationPlatform::setupUI()
{
    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 创建画布区域
    canvas = new CanvasWidget(this);
    mainLayout->addWidget(canvas, 1); // 画布占主要空间
    
   

    // 创建控制区域
    controlGroup = new QGroupBox("Controls");
    QHBoxLayout* controlLayout = new QHBoxLayout(controlGroup);
    
    // 创建控件
    basePlatformXEdit = new QLineEdit(this);
    basePlatformYEdit = new QLineEdit(this);
    basePlatformAngleEdit = new QLineEdit(this);
    showBasePlatformCheckBox = new QCheckBox("显示", this);
    showBasePlatformCheckBox->setChecked(true);
    
    realTimePlatformXEdit = new QLineEdit(this);
    realTimePlatformYEdit = new QLineEdit(this);
    realTimePlatformAngleEdit = new QLineEdit();
    showRealTimePlatformCheckBox = new QCheckBox("显示", this);
    showRealTimePlatformCheckBox->setChecked(true);
    
    mark1XEdit = new QLineEdit(this);
    mark1YEdit = new QLineEdit(this);
    mark1AngleEdit = new QLineEdit(this);
    mark1FollowBaseCheckBox = new QCheckBox("跟随目标平台", this);
    mark1FollowBaseCheckBox->setChecked(true);
    ShowMark1CheckBox = new QCheckBox("显示", this);
    ShowMark1CheckBox->setChecked(true);

    mark2XEdit = new QLineEdit(this);
    mark2YEdit = new QLineEdit(this);
    mark2AngleEdit = new QLineEdit(this);
    mark2FollowRealTimeCheckBox = new QCheckBox("跟随对象平台", this);
    mark2FollowRealTimeCheckBox->setChecked(true);
    ShowMark2CheckBox = new QCheckBox("显示");
    ShowMark2CheckBox->setChecked(true);
    
    // 基准平台控制组
    QGroupBox *baseGroup = new QGroupBox("目标平台", this);
    QGridLayout *baseLayout = new QGridLayout(baseGroup);
    baseLayout->addWidget(new QLabel("X (mm):", this), 0, 0);
    baseLayout->addWidget(basePlatformXEdit, 0, 1);
    baseLayout->addWidget(new QLabel("Y (mm):", this), 1, 0);
    baseLayout->addWidget(basePlatformYEdit, 1, 1);
    baseLayout->addWidget(new QLabel("Angle (deg):", this), 2, 0);
    baseLayout->addWidget(basePlatformAngleEdit, 2, 1);
    baseLayout->addWidget(showBasePlatformCheckBox, 3, 0, 1, 2);
    
    // 实时平台控制组
    QGroupBox *realTimeGroup = new QGroupBox("对象平台", this);
    QGridLayout *realTimeLayout = new QGridLayout(realTimeGroup);
    realTimeLayout->addWidget(new QLabel("X (mm):", this), 0, 0);
    realTimeLayout->addWidget(realTimePlatformXEdit, 0, 1);
    realTimeLayout->addWidget(new QLabel("Y (mm):", this), 1, 0);
    realTimeLayout->addWidget(realTimePlatformYEdit, 1, 1);
    realTimeLayout->addWidget(new QLabel("Angle (deg):", this), 2, 0);
    realTimeLayout->addWidget(realTimePlatformAngleEdit, 2, 1);
    realTimeLayout->addWidget(showRealTimePlatformCheckBox, 3, 0, 1, 2);
    
    // Mark1控制组
    QGroupBox *mark1Group = new QGroupBox("目标", this);
    QGridLayout *mark1Layout = new QGridLayout(mark1Group);
    mark1Layout->addWidget(new QLabel("X (mm):", this), 0, 0);
    mark1Layout->addWidget(mark1XEdit, 0, 1);
    mark1Layout->addWidget(new QLabel("Y (mm):", this), 1, 0);
    mark1Layout->addWidget(mark1YEdit, 1, 1);
    mark1Layout->addWidget(new QLabel("Angle (deg):", this), 2, 0);
    mark1Layout->addWidget(mark1AngleEdit, 2, 1);
    mark1Layout->addWidget(mark1FollowBaseCheckBox, 3, 0);
    mark1Layout->addWidget(ShowMark1CheckBox, 3, 1);
    
    // Mark2控制组
    QGroupBox *mark2Group = new QGroupBox("对象", this);
    QGridLayout *mark2Layout = new QGridLayout(mark2Group);
    mark2Layout->addWidget(new QLabel("X (mm):", this), 0, 0);
    mark2Layout->addWidget(mark2XEdit, 0, 1);
    mark2Layout->addWidget(new QLabel("Y (mm):", this), 1, 0);
    mark2Layout->addWidget(mark2YEdit, 1, 1);
    mark2Layout->addWidget(new QLabel("Angle (deg):", this), 2, 0);
    mark2Layout->addWidget(mark2AngleEdit, 2, 1);
    mark2Layout->addWidget(mark2FollowRealTimeCheckBox, 3, 0);
    mark2Layout->addWidget(ShowMark2CheckBox, 3, 1);
    
    //Mark中心间距、屏幕缩放系数
    markCenterDistanceEdit = new QLineEdit(this);
    ScreenRatio = new QLineEdit(this);
    ShowPlatformUL = new QRadioButton("左上显示", this);
    ShowPlatformUR = new QRadioButton("右上显示", this);
    ShowPlatformDL = new QRadioButton("左下显示", this);
    ShowPlatformDR = new QRadioButton("右下显示", this);
    QButtonGroup* group1 = new QButtonGroup(this);
    group1->addButton(ShowPlatformUL);
    group1->addButton(ShowPlatformUR);
    group1->addButton(ShowPlatformDL);
    group1->addButton(ShowPlatformDR);
    ShowPlatformUL->setChecked(true);

    QGroupBox *otherGroup = new QGroupBox("Setting", this);
    QGridLayout *otherLayout = new QGridLayout(otherGroup);
    otherLayout->addWidget(new QLabel("产品尺寸:", this), 0, 0);
    otherLayout->addWidget(markCenterDistanceEdit, 0, 1);
    otherLayout->addWidget(new QLabel("缩放比(px/mm):", this), 1, 0);
    otherLayout->addWidget(ScreenRatio, 1, 1);
    otherLayout->addWidget(ShowPlatformUL, 2, 0);
    otherLayout->addWidget(ShowPlatformUR, 2, 1);
    otherLayout->addWidget(ShowPlatformDL, 3, 0);
    otherLayout->addWidget(ShowPlatformDR, 3, 1); 

    // 将四个控制组添加到控制布局中
    controlLayout->addWidget(baseGroup);
    controlLayout->addWidget(realTimeGroup);
    controlLayout->addWidget(mark1Group);
    controlLayout->addWidget(mark2Group);
    controlLayout->addWidget(otherGroup);
    
    // 添加控制组到主布局
    mainLayout->addWidget(controlGroup);
    
    // 设置初始值
    basePlatformXEdit->setText(QString::number(basePlatform.x));
    basePlatformYEdit->setText(QString::number(basePlatform.y));
    basePlatformAngleEdit->setText(QString::number(basePlatform.angle));
    
    realTimePlatformXEdit->setText(QString::number(realTimePlatform.x));
    realTimePlatformYEdit->setText(QString::number(realTimePlatform.y));
    realTimePlatformAngleEdit->setText(QString::number(realTimePlatform.angle));
    
    mark1XEdit->setText(QString::number(mark1.x));
    mark1YEdit->setText(QString::number(mark1.y));
    mark1AngleEdit->setText(QString::number(mark1.angle));
    
    mark2XEdit->setText(QString::number(mark2.x));
    mark2YEdit->setText(QString::number(mark2.y));
    mark2AngleEdit->setText(QString::number(mark2.angle));

    markCenterDistanceEdit->setText(QString::number(m_markSpacing));
    ScreenRatio->setText(QString::number(m_Ratio));
}

void SimulationPlatform::setupValidators()
{
    QDoubleValidator *validator = new QDoubleValidator(this);
    validator->setDecimals(2);
    
    basePlatformXEdit->setValidator(validator);
    basePlatformYEdit->setValidator(validator);
    basePlatformAngleEdit->setValidator(validator);
    
    realTimePlatformXEdit->setValidator(validator);
    realTimePlatformYEdit->setValidator(validator);
    realTimePlatformAngleEdit->setValidator(validator);
    
    mark1XEdit->setValidator(validator);
    mark1YEdit->setValidator(validator);
    mark1AngleEdit->setValidator(validator);
    
    mark2XEdit->setValidator(validator);
    mark2YEdit->setValidator(validator);
    mark2AngleEdit->setValidator(validator);
}

void SimulationPlatform::setupConnections()
{
    connect(basePlatformXEdit, &QLineEdit::editingFinished, this, &SimulationPlatform::updateBasePlatform);
    connect(basePlatformYEdit, &QLineEdit::editingFinished, this, &SimulationPlatform::updateBasePlatform);
    connect(basePlatformAngleEdit, &QLineEdit::editingFinished, this, &SimulationPlatform::updateBasePlatform);
    connect(showBasePlatformCheckBox, &QRadioButton::clicked, this, [this]() {
        update();
    });
    
    connect(realTimePlatformXEdit, &QLineEdit::editingFinished, this, &SimulationPlatform::updateRealTimePlatform);
    connect(realTimePlatformYEdit, &QLineEdit::editingFinished, this, &SimulationPlatform::updateRealTimePlatform);
    connect(realTimePlatformAngleEdit, &QLineEdit::editingFinished, this, &SimulationPlatform::updateRealTimePlatform);
    connect(showRealTimePlatformCheckBox, &QRadioButton::clicked, this, [this]() {
        update();
    });

    connect(mark1XEdit, &QLineEdit::editingFinished, this, &SimulationPlatform::updateMark1);
    connect(mark1YEdit, &QLineEdit::editingFinished, this, &SimulationPlatform::updateMark1);
    connect(mark1AngleEdit, &QLineEdit::editingFinished, this, &SimulationPlatform::updateMark1);
    connect(mark1FollowBaseCheckBox, &QRadioButton::clicked, this, [this]() {
        update();
    });
    connect(ShowMark1CheckBox, &QRadioButton::clicked, this, [this]() {
        update();
    });
    
    connect(mark2XEdit, &QLineEdit::editingFinished, this, &SimulationPlatform::updateMark2);
    connect(mark2YEdit, &QLineEdit::editingFinished, this, &SimulationPlatform::updateMark2);
    connect(mark2AngleEdit, &QLineEdit::editingFinished, this, &SimulationPlatform::updateMark2);
    connect(mark2FollowRealTimeCheckBox, &QRadioButton::toggled, this, [this]() {
        update();
    });
    connect(ShowMark2CheckBox, &QRadioButton::clicked, this, [this]() {
        update();
    });

    connect(markCenterDistanceEdit, &QLineEdit::editingFinished, this, [this]() {
        m_markSpacing = markCenterDistanceEdit->text().toDouble();
        canvas->update();
        emit parametersChanged(m_markSpacing, m_Ratio);
    });

    connect(ScreenRatio, &QLineEdit::editingFinished, this, [this]() {
        m_Ratio = ScreenRatio->text().toDouble();
        m_scale = m_ScreenWidth / m_Ratio;
        canvas->update();
        emit parametersChanged(m_markSpacing, m_Ratio);
    });

   connect(ShowPlatformUL->group(), &QButtonGroup::buttonToggled, this, [=](QAbstractButton* button, bool checked) {
    if (checked) {

        QRect frameGeometry = this->frameGeometry();
        int totalHeight = frameGeometry.height();  // 包含标题栏的高度
        int totalWidth = frameGeometry.width();    // 包含边框的宽度

        int screenWidth = QGuiApplication::primaryScreen()->availableGeometry().width();
        int screenHeight = QGuiApplication::primaryScreen()->availableGeometry().height();
        if(button == ShowPlatformUL)
        {
            //将当前窗口移动到屏幕左上角显示
            this->move(0, 0);
        }
        else if(button == ShowPlatformUR)
        {
            //将当前窗口移动到屏幕右上角显示
            this->move(screenWidth - totalWidth, 0);
        }
        else if(button == ShowPlatformDL)
        {
            //将当前窗口移动到屏幕左下角显示
            this->move(0, screenHeight - totalHeight);
        }
        else if(button == ShowPlatformDR)
        {
            //将当前窗口移动到屏幕右下角显示
            this->move(screenWidth - totalWidth, screenHeight - totalHeight);
        }

        //canvas->update();
    }
   });
}

void SimulationPlatform::updateBasePlatform()
{
    basePlatform.x = basePlatformXEdit->text().toDouble();
    basePlatform.y = basePlatformYEdit->text().toDouble();
    basePlatform.angle = basePlatformAngleEdit->text().toDouble();
    canvas->update();
}

void SimulationPlatform::updateRealTimePlatform()
{
    realTimePlatform.x = realTimePlatformXEdit->text().toDouble();
    realTimePlatform.y = realTimePlatformYEdit->text().toDouble();
    realTimePlatform.angle = realTimePlatformAngleEdit->text().toDouble();
    canvas->update();
}

void SimulationPlatform::updateMark1()
{
    mark1.x = mark1XEdit->text().toDouble();
    mark1.y = mark1YEdit->text().toDouble();
    mark1.angle = mark1AngleEdit->text().toDouble();
    mark1.followPlatform = mark1FollowBaseCheckBox->isChecked();
    canvas->update();
}

void SimulationPlatform::updateMark2()
{
    mark2.x = mark2XEdit->text().toDouble();
    mark2.y = mark2YEdit->text().toDouble();
    mark2.angle = mark2AngleEdit->text().toDouble();
    mark2.followPlatform = mark2FollowRealTimeCheckBox->isChecked();
    canvas->update();
}

void SimulationPlatform::paintEvent(QPaintEvent *event)
{
    // 主窗口不需要绘制，由CanvasWidget负责绘制
    QWidget::paintEvent(event);
}

void SimulationPlatform::resizeEvent(QResizeEvent *event)
{
    updateOriginAndScale();
    QWidget::resizeEvent(event);
    canvas->update();
    
}

void SimulationPlatform::updateOriginAndScale()
{
    // 设置坐标原点为中心点
    m_origin.setX(canvas->width() / 2);
    m_origin.setY(canvas->height() / 2);
    
    // 计算合适的缩放比例，确保至少能看到±100mm的范围
    // double scaleX = canvas->width() / 2.0 / 10.0;
    // double scaleY = canvas->height() / 2.0 / 10.0;
    // scale = qMin(scaleX, scaleY);
    // scale = qMax(scale, 1.0); // 至少1像素/mm

    //ScreenWidth = canvas->width();
    //scale = ScreenWidth / 200.0; // 默认缩放比例 2像
}

void SimulationPlatform::drawCoordinateSystem(QPainter &painter)
{
    painter.save();
    
    QPen pen(Qt::black, 1, Qt::SolidLine);
    painter.setPen(pen);
    
    // 绘制X轴和Y轴
    painter.drawLine(0, m_origin.y(), canvas->width(), m_origin.y()); // X轴
    painter.drawLine(m_origin.x(), 0, m_origin.x(), canvas->height()); // Y轴
    
    // 绘制网格线和刻度标签
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    
    // X轴正方向刻度
    for (int i = 0; i * m_scale < canvas->width() - m_origin.x(); i+=10) {
        int x = m_origin.x() + i * m_scale;
        painter.drawLine(x, m_origin.y() - 3, x, m_origin.y() + 3);
        //间隔一个循环显示标签
        if((i/10)%2 ==0 || i==0)
            painter.drawText(x - 10, m_origin.y() + 15, QString::number(i));
    }
    
    // X轴负方向刻度
    for (int i = 0; m_origin.x() - i * m_scale > 0; i+=10) {
        int x = m_origin.x() - i * m_scale;
        painter.drawLine(x, m_origin.y() - 3, x, m_origin.y() + 3);
       if((i/10)%2 ==0 && i!=0)
            painter.drawText(x - 10, m_origin.y() + 15, QString::number(-i));
    }
    
    // Y轴正方向刻度
    for (int i = 0; i * m_scale < m_origin.y(); i+=10) {
        int y = m_origin.y() - i * m_scale;
        painter.drawLine(m_origin.x() - 3, y, m_origin.x() + 3, y);
        if((i/10)%2 ==0 && i!=0)
            painter.drawText(m_origin.x() + 5, y + 5, QString::number(i));
    }
    
    // Y轴负方向刻度
    for (int i = 0; m_origin.y() + i * m_scale < canvas->height(); i+=10) {
        int y = m_origin.y() + i * m_scale;
        painter.drawLine(m_origin.x() - 3, y, m_origin.x() + 3, y);
        if((i/10)%2 ==0 && i!=0)
            painter.drawText(m_origin.x() + 5, y + 5, QString::number(-i));
    }
    
    painter.restore();
}

void SimulationPlatform::drawPlatform(QPainter &painter, const Platform &platform, QColor color)
{
    painter.save();
    
    // 计算平台在屏幕上的位置
    QPointF center = transformPoint(QPointF(platform.x, platform.y));
    
    // 设置画笔和画刷
    QPen pen(color, 2);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    
    // 绘制圆形平台 (直径40mm)
    double radius = 15 * m_scale;
    if (&platform == &basePlatform)
    {
        radius = 20 * m_scale;
    }
    
    painter.drawEllipse(center, radius, radius);
    
    // 保存当前变换矩阵
    painter.save();
    
    // 移动到平台中心并旋转
    painter.translate(center.x(), center.y());
    painter.rotate(platform.angle);
    
    // 绘制表示方向的十字线 (长度30mm)
    double lineLength = 30 * m_scale;
    painter.drawLine(-lineLength, 0, lineLength, 0); // X轴方向线
    painter.drawLine(0, -lineLength, 0, lineLength); // Y轴方向线
    
    // 恢复变换矩阵
    painter.restore();
    
    // 绘制平台标签
    // QFont font = painter.font();
    // font.setBold(true);
    // painter.setFont(font);
    // if (&platform == &basePlatform) {
    //     painter.setPen(Qt::blue);
    //     painter.drawText(center.x() + radius + 5, center.y(), "Base");
    // } else {
    //     painter.setPen(Qt::green);
    //     painter.drawText(center.x() + radius + 5, center.y(), "Real-time");
    // }
    
    // painter.restore();
}

void SimulationPlatform::drawMark1(QPainter &painter)
{
    painter.save();
    
    // 计算最终位置和角度
    double finalX, finalY, finalAngle;
    if (mark1.followPlatform) {
        // 跟随基准平台
        finalX = basePlatform.x + mark1.x;
        finalY = basePlatform.y + mark1.y;
        finalAngle = basePlatform.angle + mark1.angle;
    } else {
        finalX = mark1.x;
        finalY = mark1.y;
        finalAngle = mark1.angle;
    }
    
    QPointF pos = transformPoint(QPointF(finalX, finalY));
    
    // 设置画笔
    QPen pen(Qt::red, 1);
    painter.setPen(pen);
    painter.setBrush(Qt::red);
    
    // 保存当前变换
    painter.save();
    
    // 移动到Mark位置并旋转
    painter.translate(pos.x(), pos.y());
    painter.rotate(finalAngle);
    
    // 绘制L型
    double spacing = m_markSpacing/2 * m_scale; // 中心间距
    double rectWidth = 2 * m_scale; // 矩形宽度
    double rectHeight = 6 * m_scale; // 矩形高度
    
    // 绘制左边的L型mark
    // 水平部分（横杠）- 左边
    painter.drawRect(-spacing - rectHeight, -rectWidth, rectHeight, rectWidth);
    // 垂直部分（竖杠）- 左边  
    painter.drawRect(-spacing - rectWidth  , -rectHeight, rectWidth, rectHeight);

    // 绘制右边的L型mark
    // 水平部分（横杠）- 右边
    painter.drawRect(spacing, -rectWidth, rectHeight, rectWidth);
    // 垂直部分（竖杠）- 右边
   painter.drawRect(spacing, -rectHeight , rectWidth, rectHeight);
    
    // 恢复变换
    painter.restore();
    
    // // 绘制标签
    // painter.setPen(Qt::red);
    // QFont font = painter.font();
    // font.setBold(true);
    // painter.setFont(font);
    // painter.drawText(pos.x() + 10, pos.y() - 10, "Mark1");
    
    // painter.restore();
}

void SimulationPlatform::drawMark2(QPainter &painter)
{
    painter.save();
    
    // 计算最终位置和角度
    double finalX, finalY, finalAngle;
    if (mark2.followPlatform) {
        // 跟随实时平台
        finalX = realTimePlatform.x + mark2.x;
        finalY = realTimePlatform.y + mark2.y;
        finalAngle = realTimePlatform.angle + mark2.angle;
    } else {
        finalX = mark2.x;
        finalY = mark2.y;
        finalAngle = mark2.angle;
    }
    
    QPointF pos = transformPoint(QPointF(finalX, finalY));
    
    // 设置画笔
    QPen pen(Qt::magenta, 1);
    painter.setPen(pen);
    painter.setBrush(Qt::magenta);
    
    // 保存当前变换
    painter.save();
    
    // 移动到Mark位置并旋转
    painter.translate(pos.x(), pos.y());
    painter.rotate(finalAngle);
    
   // 绘制L型
    double spacing = m_markSpacing/2 * m_scale; // 中心间距
    double rectWidth = 2 * m_scale; // 矩形宽度
    double rectHeight = 6 * m_scale; // 矩形高度
    
    // 绘制水平部分
    painter.drawRect(-spacing, 0, rectHeight, rectWidth);
    // 绘制垂直部分
    painter.drawRect(-spacing, 0, rectWidth, rectHeight);
    
    // 绘制水平部分
    painter.drawRect(spacing - rectHeight, 0, rectHeight, rectWidth);
    // 绘制垂直部分
    painter.drawRect(spacing - rectWidth, 0, rectWidth, rectHeight);
    
    // 恢复变换
    painter.restore();
    
    // // 绘制标签
    // painter.setPen(Qt::magenta);
    // QFont font = painter.font();
    // font.setBold(true);
    // painter.setFont(font);
    // painter.drawText(pos.x() + 10, pos.y() + 20, "Mark2");
    
    // painter.restore();
}

QPointF SimulationPlatform::rotatePoint(const QPointF &point, double angle)
{
    double rad = angle * M_PI / 180.0;
    double cosA = cos(rad);
    double sinA = sin(rad);
    
    return QPointF(point.x() * cosA - point.y() * sinA,
                   point.x() * sinA + point.y() * cosA);
}

QPointF SimulationPlatform::transformPoint(const QPointF &point)
{
    // 将世界坐标(mm)转换为屏幕坐标(pixel)
    return QPointF(m_origin.x() + point.x() * m_scale,
                   m_origin.y() - point.y() * m_scale); // 注意Y轴翻转
}

QPointF SimulationPlatform::inverseTransformPoint(const QPointF &point)
{
    // 将屏幕坐标(pixel)转换为世界坐标(mm)
    return QPointF((point.x() - m_origin.x()) / m_scale,
                   (m_origin.y() - point.y()) / m_scale); // 注意Y轴翻转
}