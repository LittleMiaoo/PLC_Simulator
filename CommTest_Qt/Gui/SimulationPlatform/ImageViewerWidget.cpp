#include "ImageViewerWidget.h"
#include <QResizeEvent>

ImageViewerWidget::ImageViewerWidget(QWidget* parent)
    : QWidget(parent)
    , m_scale(1.0)
    , m_minScale(0.1)
    , m_maxScale(10.0)
    , m_offset(0, 0)
    , m_dragging(false)
{
    setMinimumSize(200, 200);
    setMouseTracking(true);
}

void ImageViewerWidget::setImage(const QImage& image)
{
    m_image = image;
    m_offset = QPointF(0, 0);
    // 自动计算缩放比例使图像完整显示
    fitToWindow();
}

void ImageViewerWidget::fitToWindow()
{
    if (m_image.isNull() || width() <= 0 || height() <= 0) {
        m_scale = 1.0;
        emit scaleChanged(m_scale);
        update();
        return;
    }

    // 计算使图像完整显示所需的缩放比例
    double scaleX = static_cast<double>(width()) / m_image.width();
    double scaleY = static_cast<double>(height()) / m_image.height();
    m_scale = qMin(scaleX, scaleY);

    // 确保缩放比例在有效范围内
    m_scale = qBound(m_minScale, m_scale, m_maxScale);

    m_offset = QPointF(0, 0);
    emit scaleChanged(m_scale);
    update();
}

bool ImageViewerWidget::hasImage() const
{
    return !m_image.isNull();
}

const QImage& ImageViewerWidget::image() const
{
    return m_image;
}

double ImageViewerWidget::scale() const
{
    return m_scale;
}

void ImageViewerWidget::setScale(double scale)
{
    double newScale = qBound(m_minScale, scale, m_maxScale);
    if (qFuzzyCompare(newScale, m_scale)) return;
    m_scale = newScale;
    update();
    emit scaleChanged(m_scale);
}

void ImageViewerWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 绘制灰白棋盘格背景
    drawCheckerboard(painter);

    if (m_image.isNull()) {
        painter.setPen(Qt::darkGray);
        painter.drawText(rect(), Qt::AlignCenter, "No Image Loaded");
        return;
    }

    // 计算缩放后的图像尺寸
    QSizeF scaledSize = m_image.size() * m_scale;

    // 计算绘制位置（居中 + 偏移）
    QPointF center(width() / 2.0, height() / 2.0);
    QPointF topLeft = center - QPointF(scaledSize.width() / 2.0, scaledSize.height() / 2.0) + m_offset;

    // 绘制图像
    painter.drawImage(QRectF(topLeft, scaledSize), m_image);
}

void ImageViewerWidget::drawCheckerboard(QPainter& painter)
{
    const int gridSize = 10;  // 棋盘格单元大小
    const QColor lightColor(255, 255, 255);  // 白色
    const QColor darkColor(204, 204, 204);   // 浅灰色

    for (int y = 0; y < height(); y += gridSize) {
        for (int x = 0; x < width(); x += gridSize) {
            bool isLight = ((x / gridSize) + (y / gridSize)) % 2 == 0;
            painter.fillRect(x, y, gridSize, gridSize, isLight ? lightColor : darkColor);
        }
    }
}

void ImageViewerWidget::wheelEvent(QWheelEvent* event)
{
    if (m_image.isNull()) return;

    // 获取鼠标位置（相对于控件）
    QPointF mousePos = event->position();

    // 计算鼠标位置相对于图像中心的偏移（缩放前）
    QPointF center(width() / 2.0, height() / 2.0);
    QPointF relativePos = mousePos - center - m_offset;

    // 计算新的缩放比例
    double oldScale = m_scale;
    double delta = event->angleDelta().y() / 120.0;
    double factor = 1.0 + delta * 0.1;
    m_scale *= factor;
    m_scale = qBound(m_minScale, m_scale, m_maxScale);

    // 调整偏移以保持鼠标位置不变
    double scaleRatio = m_scale / oldScale;
    m_offset = mousePos - center - relativePos * scaleRatio;

    update();
    emit scaleChanged(m_scale);
    event->accept();
}

void ImageViewerWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_lastMousePos = event->pos();
        m_dragging = true;
        setCursor(Qt::ClosedHandCursor);
    }
}

void ImageViewerWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging) {
        QPointF delta = event->pos() - m_lastMousePos;
        m_offset += delta;
        m_lastMousePos = event->pos();
        update();
    }
}

void ImageViewerWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
    }
}

void ImageViewerWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    // 双击恢复图像完整显示
    fitToWindow();
}

void ImageViewerWidget::resizeEvent(QResizeEvent* event)
{
    QSize oldSize = event->oldSize();
    QSize newSize = event->size();

    // 仅当有图像且旧尺寸有效时进行比例调整
    if (!m_image.isNull() && oldSize.isValid() && oldSize.width() > 0 && oldSize.height() > 0) {
        // 计算窗口尺寸变化比例
        double ratioX = static_cast<double>(newSize.width()) / oldSize.width();
        double ratioY = static_cast<double>(newSize.height()) / oldSize.height();
        // 取较小的比例以保持显示内容一致
        double ratio = qMin(ratioX, ratioY);

        // 按比例调整缩放值
        double newScale = m_scale * ratio;
        m_scale = qBound(m_minScale, newScale, m_maxScale);

        // 按比例调整偏移量
        m_offset *= ratio;

        emit scaleChanged(m_scale);
    }

    QWidget::resizeEvent(event);
    update();
}
