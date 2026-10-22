/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#include "ImagePage.h"
#include "ImageViewer.h"
#include <QVBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QCoreApplication>
#include <QMessageBox>
#include <QImage>
#include <QFileInfo>
#include <QFile>

ImagePage::ImagePage(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_viewer = new ImageViewer(this);
    layout->addWidget(m_viewer, 1);

    // 将 ImageViewer 的缩放信号透传给外部
    connect(m_viewer, &ImageViewer::scaleChanged, this, &ImagePage::scaleChanged);

    loadDefaultImage();
}

double ImagePage::scale() const
{
    return m_viewer->scale();
}

void ImagePage::applyImage(const QImage& image, const QString& path)
{
    m_viewer->setImage(image);
    m_imagePath = path;
    emit imagePathChanged(m_imagePath);   // 路径变必发信号:加载图片的唯一出口
}

void ImagePage::loadDefaultImage()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString configDir = appDir + "/Config";

    QDir dir(configDir);
    if (!dir.exists())
    {
        dir.mkpath(".");
        return;
    }

    // 查找包含 'SimulationImage' 的图像文件
    QStringList filters;
    filters << "SimulationImage*.bmp" << "SimulationImage*.png"
            << "SimulationImage*.jpg" << "SimulationImage*.jpeg"
            << "SimulationImage*.tiff" << "SimulationImage*.tif";

    QStringList files = dir.entryList(filters, QDir::Files, QDir::Name);

    if (!files.isEmpty())
    {
        QString imagePath = configDir + "/" + files.first();
        QImage image(imagePath);
        if (!image.isNull())
            applyImage(image, imagePath);
    }
}

void ImagePage::loadImage()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString configDir = appDir + "/Config";

    // 确保目录存在
    QDir dir(configDir);
    if (!dir.exists())
    {
        dir.mkpath(".");
    }

    // 打开文件对话框
    QString filter = tr("图像文件 (*.bmp *.png *.jpg *.jpeg *.tiff *.tif)");
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择图像"), configDir, filter);

    if (filePath.isEmpty())
    {
        return;
    }

    // 加载图像
    QImage image(filePath);
    if (image.isNull())
    {
        QMessageBox::warning(this, tr("错误"), tr("无法加载所选图像文件。"));
        return;
    }

    // 显示图像
    applyImage(image, filePath);

    // 获取原文件的扩展名
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();

    // 保存到 Config 目录，命名为 SimulationImage
    QString destPath = configDir + "/SimulationImage." + suffix;

    // 如果已存在同名文件（包括不同扩展名），先删除旧的
    QStringList oldFiles = dir.entryList(QStringList() << "SimulationImage.*", QDir::Files);
    for (const QString &oldFile : oldFiles)
    {
        QFile::remove(configDir + "/" + oldFile);
    }

    // 复制文件到目标位置
    if (!QFile::copy(filePath, destPath))
    {
        // 如果复制失败，尝试直接保存
        if (!image.save(destPath))
        {
            QMessageBox::warning(this, tr("警告"), tr("图像加载成功，但无法保存到配置目录。"));
        }
    }
}
