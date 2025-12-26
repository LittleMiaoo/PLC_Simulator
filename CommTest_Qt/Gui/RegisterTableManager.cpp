#include "RegisterTableManager.h"

RegisterTableManager::RegisterTableManager(
    QTableWidget* tableWidget,
    QComboBox* dataTypeCombo,
    QLineEdit* addrEdit,
    MainWorkFlow* workFlow,
    QWidget* parent)
    : QObject(parent)
    , m_tableWidget(tableWidget)
    , m_dataTypeCombo(dataTypeCombo)
    , m_addrEdit(addrEdit)
    , m_workFlow(workFlow)
    , m_parentWidget(parent)
    , m_nIntStat(0)
    , m_bShouldFlash(true)
{
    // 初始化寄存器数据缓存
    int dataCellCount = REGISTER_TABLE_ROW_COUNT * (REGISTER_TABLE_COLUMN_COUNT / 2);
    int convertCount = (dataCellCount + 3) / 4;  // 向上取整到4的倍数
    m_vecRegisterVal.resize(convertCount);
}

void RegisterTableManager::initTable()
{
    if (!m_tableWidget) return;

    m_tableWidget->setColumnCount(REGISTER_TABLE_COLUMN_COUNT);
    m_tableWidget->setRowCount(REGISTER_TABLE_ROW_COUNT);

    QTableWidgetItem* Item;
    QString strItemInfo;
    for (int i = 0; i < REGISTER_TABLE_COLUMN_COUNT; i++)
    {
        strItemInfo = i % 2 ? "值" : "地址";
        Item = new QTableWidgetItem(strItemInfo);

        m_tableWidget->setColumnWidth(i, 80);
        m_tableWidget->setHorizontalHeaderItem(i, Item);
    }

    for (int i = 0; i < REGISTER_TABLE_ROW_COUNT; i++)
    {
        strItemInfo = " ";
        Item = new QTableWidgetItem(strItemInfo);
        m_tableWidget->setVerticalHeaderItem(i, Item);
    }

    for (int row = 0; row < REGISTER_TABLE_ROW_COUNT; ++row)
    {
        for (int col = 0; col < REGISTER_TABLE_COLUMN_COUNT; ++col)
        {
            strItemInfo = "";
            Item = new QTableWidgetItem(strItemInfo);
            Item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            m_tableWidget->setItem(row, col, Item);
        }
    }

    m_tableWidget->setAlternatingRowColors(true);

    m_tableWidget->setStyleSheet(
        "QTableWidget {"
        "   background-color: rgb(255, 255, 255);"
        "   alternate-background-color: rgb(240, 240, 240);"
        "   gridline-color: rgb(200, 200, 200);"
        "}"
    );
}

void RegisterTableManager::updateTableInfo(int nStart, bool bInitialize)
{
    if (!m_tableWidget || !m_workFlow) return;

    const int rowCount = m_tableWidget->rowCount();
    const int colCount = m_tableWidget->columnCount();
    const int step = rowCount;

    for (int row = 0; row < rowCount; ++row)
    {
        for (int col = 0; col < colCount; col += 2)
        {
            QTableWidgetItem* item = m_tableWidget->item(row, col);

            int group = col / 2;
            int addrNum = nStart + row + group * step;
            item->setText(QString("D%1").arg(addrNum, 5, 10, QChar('0')));
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);

            m_tableWidget->setItem(row, col, item);
        }
    }

    getRegisterVals(nStart);
    displayRegisterVals();
}

void RegisterTableManager::getRegisterVals(int nStart)
{
    if (!m_workFlow) return;

    for (size_t i = 0; i < m_vecRegisterVal.size(); i++)
    {
        for (int j = 0; j < 4; j++)
        {
            m_vecRegisterVal[i].u_Int16[j] = m_workFlow->GetRegisterVal(nStart + i * 4 + j);
        }
    }
}

void RegisterTableManager::setRegisterVals(int nStart)
{
    if (!m_workFlow) return;

    for (size_t i = 0; i < m_vecRegisterVal.size(); i++)
    {
        for (int j = 0; j < 4; j++)
        {
            m_workFlow->SetRegisterVal(nStart + i * 4 + j, m_vecRegisterVal[i].u_Int16[j]);
        }
    }
}

void RegisterTableManager::updateRegisterVals(QTableWidgetItem* pItem)
{
    if (!pItem || !m_tableWidget || !m_workFlow || !m_addrEdit) return;

    checkInput(pItem);

    int nRow = pItem->row();
    int nCol = pItem->column();

    if (nCol % 2 == 0) return;  // 偶数列为地址列,不处理

    int rowCount = m_tableWidget->rowCount();
    int nStart = m_addrEdit->text().toUInt();

    int group = (nCol - 1) / 2;
    int addr = nStart + nRow + group * rowCount;
    int ndataIndex = nRow + group * rowCount;
    int nRegisterValIndex = ndataIndex / 4;

    if (!m_dataTypeCombo) return;
    int nCurIndex = m_dataTypeCombo->currentIndex();
    if (nCurIndex < 0) return;

    QVariant data = m_dataTypeCombo->itemData(nCurIndex);
    RegisterDataType type = data.value<RegisterDataType>();

    switch (type)
    {
    case RegisterDataType::eDataTypeChar8:
    {
        int nCurChar = 0;
        while (nCurChar < 2 && pItem->text().length() > nCurChar)
        {
            int ncharIndex = ndataIndex % 8 + nCurChar;
            char nchar = pItem->text().at(nCurChar).toLatin1();
            m_vecRegisterVal[nRegisterValIndex].u_chars[ncharIndex] = nchar;
            nCurChar++;
        }
        m_workFlow->SetRegisterVal(addr, m_vecRegisterVal[nRegisterValIndex].u_Int16[0]);
    }
    break;
    case RegisterDataType::eDataTypeInt16:
    {
        int16_t nVal;
        if (m_nIntStat == 0)
        {
            nVal = pItem->text().toInt() & 0xFFFF;
        }
        else
        {
            bool bOk = false;
            nVal = (pItem->text().toInt(&bOk, 16) & 0xFFFF);
        }
        m_vecRegisterVal[nRegisterValIndex].u_Int16[ndataIndex % 4] = nVal;
        m_workFlow->SetRegisterVal(addr, nVal);
    }
    break;
    case RegisterDataType::eDataTypeInt32:
    {
        int32_t nVal;
        if (m_nIntStat == 0)
        {
            nVal = pItem->text().toInt();
        }
        else
        {
            bool bOk = false;
            nVal = pItem->text().toInt(&bOk, 16);
        }
        m_vecRegisterVal[nRegisterValIndex].u_Int32[ndataIndex % 2] = nVal;
        m_workFlow->SetRegisterVal(addr, m_vecRegisterVal[nRegisterValIndex].u_Int16[0]);
        m_workFlow->SetRegisterVal(addr + 1, m_vecRegisterVal[nRegisterValIndex].u_Int16[1]);
    }
    break;
    case RegisterDataType::eDataTypeFloat:
    {
        float nVal = pItem->text().toFloat();
        m_vecRegisterVal[nRegisterValIndex].u_float[ndataIndex % 2] = nVal;
        m_workFlow->SetRegisterVal(addr, m_vecRegisterVal[nRegisterValIndex].u_Int16[0]);
        m_workFlow->SetRegisterVal(addr + 1, m_vecRegisterVal[nRegisterValIndex].u_Int16[1]);
    }
    break;
    case RegisterDataType::eDataTypeDouble:
    {
        double nVal = pItem->text().toDouble();
        m_vecRegisterVal[nRegisterValIndex].u_double = nVal;
        m_workFlow->SetRegisterVal(addr, m_vecRegisterVal[nRegisterValIndex].u_Int16[0]);
        m_workFlow->SetRegisterVal(addr + 1, m_vecRegisterVal[nRegisterValIndex].u_Int16[1]);
        m_workFlow->SetRegisterVal(addr + 2, m_vecRegisterVal[nRegisterValIndex].u_Int16[2]);
        m_workFlow->SetRegisterVal(addr + 3, m_vecRegisterVal[nRegisterValIndex].u_Int16[3]);
    }
    break;
    default:
        break;
    }
}

bool RegisterTableManager::checkInput(QTableWidgetItem* pItem)
{
    if (!pItem || !m_dataTypeCombo) return false;

    int nCurIndex = m_dataTypeCombo->currentIndex();
    if (nCurIndex < 0) return false;

    QVariant data = m_dataTypeCombo->itemData(nCurIndex);
    QString text = pItem->text().trimmed();

    if (pItem->column() % 2 == 0) return true;

    RegisterDataType type = data.value<RegisterDataType>();

    if (m_nIntStat == 1 && (type == RegisterDataType::eDataTypeInt16 ||
        type == RegisterDataType::eDataTypeInt32))
    {
        return checkInput_int_Hex(pItem, type);
    }

    switch (type)
    {
    case RegisterDataType::eDataTypeChar8:
        return checkInput_str(pItem, text);
    case RegisterDataType::eDataTypeInt16:
        return checkInput_int(pItem, text, INT16_MIN, INT16_MAX);
    case RegisterDataType::eDataTypeInt32:
        return checkInput_int(pItem, text, INT32_MIN, INT32_MAX);
    case RegisterDataType::eDataTypeFloat:
        return checkInput_float(pItem, text, -FLT_MAX, FLT_MAX);
    case RegisterDataType::eDataTypeDouble:
        return checkInput_float(pItem, text, -DBL_MAX, DBL_MAX);
    default:
        return false;
    }
}

bool RegisterTableManager::checkInput_str(QTableWidgetItem* pItem, const QString& text)
{
    if (pItem->text().length() > 2)
    {
        QMessageBox::warning(
            m_parentWidget,
            "输入截断",
            QString("输入值 %1 长度超过2,只保留前2位!").arg(text)
        );
        pItem->setText(pItem->text().left(2));
    }
    return true;
}

bool RegisterTableManager::checkInput_int(QTableWidgetItem* pItem, const QString& text, int32_t minVal, int32_t maxVal)
{
    QRegularExpression regExp("^(0|-?[1-9]\\d*)$");
    QRegularExpressionMatch match = regExp.match(text);

    if (!match.hasMatch())
    {
        QMessageBox::warning(
            m_parentWidget,
            "输入非法",
            QString("输入值 %1 非整型数").arg(text)
        );
        pItem->setText("0");
    }
    else
    {
        int tmp = text.toInt();
        if (tmp > maxVal || tmp < minVal)
        {
            QMessageBox::warning(
                m_parentWidget,
                "输入重置",
                QString("输入值 %1 超过整数范围(%2-%3),重置为0")
                    .arg(text)
                    .arg(minVal)
                    .arg(maxVal)
            );
            pItem->setText("0");
            return true;
        }
        pItem->setText(QString("%1").arg(tmp));
    }
    return true;
}

bool RegisterTableManager::checkInput_float(QTableWidgetItem* pItem, const QString& text, double minVal, double maxVal)
{
    QRegularExpression regExp("^(?!-0(\\.0*)?$)-?\\d+(\\.\\d+)?$");
    QRegularExpressionMatch match = regExp.match(text);

    if (!match.hasMatch())
    {
        QMessageBox::warning(
            m_parentWidget,
            "输入非法",
            QString("输入值 %1 非浮点数").arg(text)
        );
        pItem->setText("0.0");
    }
    else
    {
        double tmp = text.toDouble();
        if (tmp > maxVal || tmp < minVal)
        {
            QMessageBox::warning(
                m_parentWidget,
                "输入重置",
                QString("输入值 %1 超过浮点数范围,重置为0").arg(text)
            );
            pItem->setText("0.0");
        }
        pItem->setText(QString("%1").arg(tmp, 0, 'f', 6));
    }
    return true;
}

bool RegisterTableManager::checkInput_int_Hex(QTableWidgetItem* pItem, const RegisterDataType& type)
{
    if (!pItem) return false;

    int nMaxDigits = 0;
    if (type == RegisterDataType::eDataTypeInt16)
    {
        nMaxDigits = 4;
    }
    else if (type == RegisterDataType::eDataTypeInt32)
    {
        nMaxDigits = 8;
    }

    QString text = pItem->text().trimmed();

    QRegularExpression hexRegExp("^[0-9A-Fa-f]*$");
    if (!hexRegExp.match(text).hasMatch())
    {
        QMessageBox::warning(
            m_parentWidget,
            "输入非法",
            QString("输入值 %1 非十六进制数").arg(text)
        );
        QString tmp = "0";
        pItem->setText(tmp.rightJustified(nMaxDigits, '0'));
        return true;
    }

    if (text.length() > nMaxDigits)
    {
        QMessageBox::warning(
            m_parentWidget,
            "输入截断",
            QString("输入值 %1 超过范围,将截断输入数据!").arg(text)
        );
        pItem->setText(text.left(nMaxDigits).toUpper());
        return true;
    }

    pItem->setText(text.toUpper().rightJustified(nMaxDigits, '0'));
    return true;
}

void RegisterTableManager::displayRegisterVals()
{
    if (!m_tableWidget || !m_dataTypeCombo) return;

    int nCurIndex = m_dataTypeCombo->currentIndex();
    if (nCurIndex < 0) return;

    m_bShouldFlash = false;

    QVariant data = m_dataTypeCombo->itemData(nCurIndex);

    const int rowCount = m_tableWidget->rowCount();
    const int colCount = m_tableWidget->columnCount();

    // 先将数据列清空
    for (int col = 1; col < colCount; col += 2)
    {
        for (int row = 0; row < rowCount; row++)
        {
            QTableWidgetItem* item = m_tableWidget->item(row, col);
            if (item)
            {
                item->setText("");
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            }
        }
    }

    m_bShouldFlash = true;

    RegisterDataType type = data.value<RegisterDataType>();
    switch (type)
    {
    case RegisterDataType::eDataTypeChar8:
        displayRegisterVals_Char8();
        break;
    case RegisterDataType::eDataTypeInt16:
        displayRegisterVals_Int16();
        break;
    case RegisterDataType::eDataTypeInt32:
        displayRegisterVals_Int32();
        break;
    case RegisterDataType::eDataTypeFloat:
        displayRegisterVals_Float();
        break;
    case RegisterDataType::eDataTypeDouble:
        displayRegisterVals_Double();
        break;
    }
}

void RegisterTableManager::displayRegisterVals_Char8()
{
    const int rowCount = m_tableWidget->rowCount();
    const int colCount = m_tableWidget->columnCount();

    int nRegisterCount = 0;
    int nRealDataCount = 0;
    DataTypeConvert* currentData = &m_vecRegisterVal[nRegisterCount];

    for (int col = 1; col < colCount; col += 2)
    {
        for (int row = 0; row < rowCount; row++)
        {
            QString strInfo(QString("%1%2")
                .arg(QChar(currentData->u_chars[nRealDataCount++]))
                .arg(QChar(currentData->u_chars[nRealDataCount++])));

            m_tableWidget->item(row, col)->setText(strInfo);
            m_tableWidget->item(row, col)->setFlags(
                m_tableWidget->item(row, col)->flags() | Qt::ItemIsEditable);

            if (nRealDataCount == 8)
            {
                nRealDataCount = 0;
                nRegisterCount++;
                if (nRegisterCount >= static_cast<int>(m_vecRegisterVal.size()))
                {
                    break;
                }
                currentData = &m_vecRegisterVal[nRegisterCount];
            }
        }
    }
}

void RegisterTableManager::displayRegisterVals_Int16()
{
    const int rowCount = m_tableWidget->rowCount();
    const int colCount = m_tableWidget->columnCount();

    int nRegisterCount = 0;
    int nRealDataCount = 0;
    DataTypeConvert* currentData = &m_vecRegisterVal[nRegisterCount];

    for (int col = 1; col < colCount; col += 2)
    {
        for (int row = 0; row < rowCount; row++)
        {
            QString strInfo;
            if (m_nIntStat == 1)
            {
                strInfo = QString("%1").arg(
                    QString::number(currentData->u_Int16[nRealDataCount++], 16), 4, '0').toUpper();
            }
            else
            {
                strInfo = QString("%1").arg(currentData->u_Int16[nRealDataCount++]);
            }

            m_tableWidget->item(row, col)->setText(strInfo);
            m_tableWidget->item(row, col)->setFlags(
                m_tableWidget->item(row, col)->flags() | Qt::ItemIsEditable);

            if (nRealDataCount == 4)
            {
                nRealDataCount = 0;
                nRegisterCount++;
                if (nRegisterCount >= static_cast<int>(m_vecRegisterVal.size()))
                {
                    break;
                }
                currentData = &m_vecRegisterVal[nRegisterCount];
            }
        }
    }
}

void RegisterTableManager::displayRegisterVals_Int32()
{
    const int rowCount = m_tableWidget->rowCount();
    const int colCount = m_tableWidget->columnCount();

    int nRegisterCount = 0;
    int nRealDataCount = 0;
    int nCellCount = 0;
    DataTypeConvert* currentData = &m_vecRegisterVal[nRegisterCount];

    for (int col = 1; col < colCount; col += 2)
    {
        for (int row = 0; row < rowCount; row++)
        {
            nCellCount++;

            if (nCellCount % 2 == 1)
            {
                QString strInfo;
                if (m_nIntStat == 1)
                {
                    strInfo = QString("%1").arg(
                        QString::number(currentData->u_Int32[nRealDataCount++], 16), 8, '0').toUpper();
                }
                else
                {
                    strInfo = QString("%1").arg(currentData->u_Int32[nRealDataCount++]);
                }

                m_tableWidget->item(row, col)->setText(strInfo);
                m_tableWidget->item(row, col)->setFlags(
                    m_tableWidget->item(row, col)->flags() | Qt::ItemIsEditable);

                if (nRealDataCount == 2)
                {
                    nRealDataCount = 0;
                    nRegisterCount++;
                    if (nRegisterCount >= static_cast<int>(m_vecRegisterVal.size()))
                    {
                        break;
                    }
                    currentData = &m_vecRegisterVal[nRegisterCount];
                }
            }
        }
    }
}

void RegisterTableManager::displayRegisterVals_Float()
{
    const int rowCount = m_tableWidget->rowCount();
    const int colCount = m_tableWidget->columnCount();

    int nRegisterCount = 0;
    int nRealDataCount = 0;
    int nCellCount = 0;
    DataTypeConvert* currentData = &m_vecRegisterVal[nRegisterCount];

    for (int col = 1; col < colCount; col += 2)
    {
        for (int row = 0; row < rowCount; row++)
        {
            nCellCount++;

            if (nCellCount % 2 == 1)
            {
                QString strInfo = QString("%1").arg(currentData->u_float[nRealDataCount++]);

                m_tableWidget->item(row, col)->setText(strInfo);
                m_tableWidget->item(row, col)->setFlags(
                    m_tableWidget->item(row, col)->flags() | Qt::ItemIsEditable);

                if (nRealDataCount == 2)
                {
                    nRealDataCount = 0;
                    nRegisterCount++;
                    if (nRegisterCount >= static_cast<int>(m_vecRegisterVal.size()))
                    {
                        break;
                    }
                    currentData = &m_vecRegisterVal[nRegisterCount];
                }
            }
        }
    }
}

void RegisterTableManager::displayRegisterVals_Double()
{
    const int rowCount = m_tableWidget->rowCount();
    const int colCount = m_tableWidget->columnCount();

    int nRegisterCount = 0;
    int nCellCount = 0;
    DataTypeConvert* currentData = &m_vecRegisterVal[nRegisterCount];

    for (int col = 1; col < colCount; col += 2)
    {
        for (int row = 0; row < rowCount; row++)
        {
            nCellCount++;

            if (nCellCount % 4 == 1)
            {
                QString strInfo = QString("%1").arg(currentData->u_double);

                m_tableWidget->item(row, col)->setText(strInfo);
                m_tableWidget->item(row, col)->setFlags(
                    m_tableWidget->item(row, col)->flags() | Qt::ItemIsEditable);

                nRegisterCount++;
                if (nRegisterCount >= static_cast<int>(m_vecRegisterVal.size()))
                {
                    break;
                }
                currentData = &m_vecRegisterVal[nRegisterCount];
            }
        }
    }
}
