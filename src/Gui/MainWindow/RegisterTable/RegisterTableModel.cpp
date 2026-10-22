/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#include "RegisterTableModel.h"
#include "Core/RegisterStore.h"
#include "Theme/ThemeManager.h"

#include <QMessageBox>
#include <QRegularExpression>
#include <QTimer>
#include <cfloat>
#include <cstdint>

namespace
{
// 浮点显示/录入的有效数字位数:贴近各自类型真实精度,受列宽约束做的折中
constexpr int kFloatSigDigits = 7;    // float 约 7 位有效数字
constexpr int kDoubleSigDigits = 15;  // double 约 15-16 位,取 15

// 闪烁淡出参数
constexpr int kFlashDurationMs = 400;  // 淡出总时长
constexpr int kFlashPeakAlpha  = 140;  // 峰值不透明度(0~255)
constexpr int kFlashTickMs     = 30;   // 重绘步进

// 浮点统一格式化:'g' 按有效数字。录入规范化与显示共用,保证录完刷新不变样
QString formatReal(double value, int sigDigits)
{
    return QString::number(value, 'g', sigDigits);
}

// 类型特征:把每种数据类型的属性集中到一处(单一真相源)。
// format/parse/validate 按 family 分派 + 读本表参数,联合体槽位再按 registersPerValue 推导。
struct TypeTrait
{
    int registersPerValue;                       // 每值占几个 Int16(1/1/2/2/4)
    enum Family { Char, Integer, Real } family;   // 三大族,决定 format/parse/validate 走向
    long long intMin, intMax;                     // Integer 族取值范围
    int hexDigits;                                // Integer 族十六进制位宽(4/8)
    double realMin, realMax;                      // Real 族取值范围
    int sigDigits;                                // Real 族有效数字位数
};

const TypeTrait& traitOf(RegisterDataType type)
{
    static const TypeTrait kChar8 { 1, TypeTrait::Char,    0, 0, 0, 0.0, 0.0, 0 };
    static const TypeTrait kInt16 { 1, TypeTrait::Integer, INT16_MIN, INT16_MAX, 4, 0.0, 0.0, 0 };
    static const TypeTrait kInt32 { 2, TypeTrait::Integer, INT32_MIN, INT32_MAX, 8, 0.0, 0.0, 0 };
    static const TypeTrait kFloat { 2, TypeTrait::Real,    0, 0, 0, -FLT_MAX, FLT_MAX, kFloatSigDigits };
    static const TypeTrait kDouble{ 4, TypeTrait::Real,    0, 0, 0, -DBL_MAX, DBL_MAX, kDoubleSigDigits };
    switch (type)
    {
    case RegisterDataType::eDataTypeInt16:  return kInt16;
    case RegisterDataType::eDataTypeInt32:  return kInt32;
    case RegisterDataType::eDataTypeFloat:  return kFloat;
    case RegisterDataType::eDataTypeDouble: return kDouble;
    default:                                return kChar8;  // Char8
    }
}

// 校验结果(纯数据):ok=能否写入;normalized=规范化后的文本;title/msg 非空则需提示。
// 注:title 非空但 ok=true 表示"截断后接受"——既写入也提示。
struct ValidationResult
{
    bool ok;
    QString normalized;
    QString title;
    QString msg;
};

// 以下校验为纯函数:只判定/规范化,不弹框、不改单元格。提示与写入由 setData 决策。
ValidationResult validateChar(const QString& text)
{
    if (text.isEmpty())  // 空串:拒写保留原值(与 integer/real 一致;空格是合法字符故不 trim)
        return { false, QString(), RegisterTableModel::tr("输入非法"),
                 RegisterTableModel::tr("输入为空,已保留原值!") };
    if (text.length() > 2)
        return { true, text.left(2), RegisterTableModel::tr("输入截断"),
                 RegisterTableModel::tr("输入值 %1 长度超过2,只保留前2位!").arg(text) };
    return { true, text, QString(), QString() };
}

ValidationResult validateInteger(const QString& text, const TypeTrait& t, int intStat)
{
    const QString s = text.trimmed();
    if (intStat == 1)  // 十六进制
    {
        QRegularExpression hexRe("^[0-9A-Fa-f]+$");  // 至少一位:空串落入非法→拒写保留原值(与 dec/real 一致)
        if (!hexRe.match(s).hasMatch())
            return { false, QString(), RegisterTableModel::tr("输入非法"),
                     RegisterTableModel::tr("输入值 %1 非十六进制数").arg(s) };
        if (s.length() > t.hexDigits)  // 超位宽:截断后接受
            return { true, s.left(t.hexDigits).toUpper(), RegisterTableModel::tr("输入截断"),
                     RegisterTableModel::tr("输入值 %1 超过范围,将截断输入数据!").arg(s) };
        return { true, s.toUpper().rightJustified(t.hexDigits, '0'), QString(), QString() };
    }

    // 十进制(放宽:接受前导零/正号)
    QRegularExpression decRe("^[+-]?\\d+$");
    if (!decRe.match(s).hasMatch())
        return { false, QString(), RegisterTableModel::tr("输入非法"),
                 RegisterTableModel::tr("输入值 %1 非整型数").arg(s) };
    bool ok = false;
    long long v = s.toLongLong(&ok);  // 超 long long 也会 !ok → 判超范围
    if (!ok || v < t.intMin || v > t.intMax)
        return { false, QString(), RegisterTableModel::tr("输入超范围"),
                 RegisterTableModel::tr("输入值 %1 超过范围(%2 ~ %3)").arg(s).arg(t.intMin).arg(t.intMax) };
    return { true, QString::number(v), QString(), QString() };  // 规范化(去前导零/正号)
}

ValidationResult validateReal(const QString& text, const TypeTrait& t)
{
    const QString s = text.trimmed();
    // 放宽:接受前导零/正号/.5/5.;不含科学计数法
    QRegularExpression re("^[+-]?(\\d+\\.?\\d*|\\.\\d+)$");
    if (!re.match(s).hasMatch())
        return { false, QString(), RegisterTableModel::tr("输入非法"),
                 RegisterTableModel::tr("输入值 %1 非浮点数").arg(s) };
    double v = s.toDouble();
    if (v < t.realMin || v > t.realMax)
        return { false, QString(), RegisterTableModel::tr("输入超范围"),
                 RegisterTableModel::tr("输入值 %1 超过浮点数范围").arg(s) };
    return { true, formatReal(v, t.sigDigits), QString(), QString() };
}

ValidationResult validateInput(const QString& text, RegisterDataType type, int intStat)
{
    const TypeTrait& t = traitOf(type);
    switch (t.family)
    {
    case TypeTrait::Char:    return validateChar(text);
    case TypeTrait::Integer: return validateInteger(text, t, intStat);
    case TypeTrait::Real:    return validateReal(text, t);
    }
    return { false, QString(), QString(), QString() };
}
}  // namespace

RegisterTableModel::RegisterTableModel(RegisterStore* store, QWidget* dialogParent, QObject* parent)
    : QAbstractTableModel(parent)
    , m_store(store)
    , m_dialogParent(dialogParent)
    , m_layout{0, 0}
    , m_currentType(RegisterDataType::eDataTypeInt16)
    , m_startAddr(0)
    , m_intStat(0)
    , m_flashTimer(nullptr)
{
    m_clock.start();

    m_flashTimer = new QTimer(this);
    m_flashTimer->setInterval(kFlashTickMs);
    connect(m_flashTimer, &QTimer::timeout, this, &RegisterTableModel::onFlashTick);

    m_flashColor = ThemeManager::instance().color("@flashBg");
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this]
    {
        m_flashColor = ThemeManager::instance().color("@flashBg");
        emit flashTick();
    });

    if (m_store)
        connect(m_store, &RegisterStore::dataChanged, this, &RegisterTableModel::onStoreChanged,
                Qt::UniqueConnection);
}

void RegisterTableModel::setDimensions(int rowCount, int colCount)
{
    beginResetModel();
    m_layout = { rowCount, colCount };
    m_registerVals.assign(m_layout.convertCount(), DataTypeConvert());
    m_flashStartMs.assign(rowCount * colCount, 0);
    if (m_flashTimer->isActive()) m_flashTimer->stop();
    m_editIndex = QPersistentModelIndex();
    refreshSnapshot();
    endResetModel();
}

int RegisterTableModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_layout.rowCount;
}

int RegisterTableModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_layout.colCount;
}

QVariant RegisterTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant();

    if (role == Qt::TextAlignmentRole)
        return int(Qt::AlignCenter);
    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    const int k = m_layout.linearIndex(index.row(), index.column());
    if (m_layout.isValueColumn(index.column()))
    {
        if (!isAnchorValueCell(index.row(), index.column()))  // 越界/非锚点格留空
            return QString();
        return formatCell(m_currentType, k);
    }

    // 地址列:地址越过 store 容量则留空(窗口可大于寄存器空间)
    const int addr = m_layout.registerAddr(k, m_startAddr);
    if (m_store && addr >= m_store->size())
        return QString();
    return QString("D%1").arg(addr, 5, 10, QChar('0'));
}

QVariant RegisterTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return QVariant();
    if (orientation == Qt::Horizontal)
        return m_layout.isValueColumn(section) ? tr("值") : tr("地址");
    return QStringLiteral(" ");  // 垂直表头留空(对齐原行为)
}

Qt::ItemFlags RegisterTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;

    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (isAnchorValueCell(index.row(), index.column()))
        f |= Qt::ItemIsEditable;
    return f;
}

bool RegisterTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole || !index.isValid() || !m_store) return false;
    if (!m_layout.isValueColumn(index.column())) return false;

    const ValidationResult r = validateInput(value.toString(), m_currentType, m_intStat);

    // 有提示则延迟弹框:推到事件循环下一拍(编辑器已关闭)再弹,避免夺焦重入
    if (!r.title.isEmpty())
    {
        const QString title = r.title;
        const QString msg = r.msg;
        QTimer::singleShot(0, this, [this, title, msg]
        {
            QMessageBox::warning(m_dialogParent, title, msg);
        });
    }

    if (!r.ok) return false;  // 非法:不写(单元格保留原值)

    const int k = m_layout.linearIndex(index.row(), index.column());
    writeCell(m_currentType, k, r.normalized);

    // 统一回写:每值占 rpv 个 Int16,从 subIndex 起连续 rpv 个推入 store
    const int addr = m_layout.registerAddr(k, m_startAddr);
    const int rpv = registersPerValue(m_currentType);
    const DataTypeConvert& cell = m_registerVals[m_layout.cacheIndex(k)];
    const int s = m_layout.subIndex(k);
    for (int j = 0; j < rpv; ++j)
        m_store->setCell(addr + j, cell.u_Int16[s + j]);

    stampFlash(index.row(), index.column());
    emit dataChanged(index, index);
    return true;
}

void RegisterTableModel::setDataType(RegisterDataType type)
{
    m_currentType = type;
    refreshAll();
}

void RegisterTableModel::setStartAddr(int startAddr)
{
    m_startAddr = startAddr;
    refreshAll();
}

void RegisterTableModel::setNumberBase(bool hex)
{
    m_intStat = hex ? 1 : 0;
    refreshAll();
}

QColor RegisterTableModel::flashOverlay(int row, int col) const
{
    const int idx = flashIndex(row, col);
    if (idx < 0) return QColor();
    const qint64 start = m_flashStartMs[idx];
    if (start == 0) return QColor();
    const qint64 elapsed = m_clock.elapsed() - start;
    if (elapsed < 0 || elapsed >= kFlashDurationMs) return QColor();

    QColor c = m_flashColor;
    c.setAlpha(static_cast<int>(kFlashPeakAlpha * (kFlashDurationMs - elapsed) / kFlashDurationMs));
    return c;
}

void RegisterTableModel::stampFlash(int row, int col)
{
    const int idx = flashIndex(row, col);
    if (idx < 0) return;
    m_flashStartMs[idx] = m_clock.elapsed();
    if (!m_flashTimer->isActive()) m_flashTimer->start();
}

int RegisterTableModel::flashIndex(int row, int col) const
{
    const int cols = m_layout.colCount;
    if (cols <= 0) return -1;
    const int idx = row * cols + col;
    if (idx < 0 || idx >= static_cast<int>(m_flashStartMs.size())) return -1;
    return idx;
}

void RegisterTableModel::setEditingIndex(const QModelIndex& index)
{
    m_editIndex = index;
}

void RegisterTableModel::clearEditingIndex()
{
    const QModelIndex idx = m_editIndex;
    m_editIndex = QPersistentModelIndex();
    if (idx.isValid())
        emit dataChanged(idx, idx);  // 编辑结束:把该格刷到当前快照值
    emit editingFinished();          // 解除编辑保护后补齐编辑期被跳过的维度自适应
}

void RegisterTableModel::onStoreChanged()
{
    if (m_layout.colCount <= 0) return;

    m_prevVals = m_registerVals;  // 差异基线:复用成员缓冲,容量足够时不再每拍堆分配
    refreshSnapshot();

    const int rows = m_layout.rowCount;
    const int cols = m_layout.colCount;
    const int rpv = registersPerValue(m_currentType);

    for (int col = 1; col < cols; col += 2)  // 值列(奇)
    {
        for (int row = 0; row < rows; ++row)
        {
            if (!isAnchorValueCell(row, col)) continue;  // 仅范围内锚点格

            const int k = m_layout.linearIndex(row, col);
            const int ci = m_layout.cacheIndex(k);
            const int s = m_layout.subIndex(k);
            bool changed = false;
            for (int j = 0; j < rpv; ++j)
                if (m_prevVals[ci].u_Int16[s + j] != m_registerVals[ci].u_Int16[s + j]) { changed = true; break; }
            if (!changed) continue;

            const QModelIndex idx = index(row, col);
            if (m_editIndex == idx) continue;  // 编辑保护:不刷正在编辑格(否则复原用户输入)
            stampFlash(row, col);
            emit dataChanged(idx, idx);
        }
    }
}

void RegisterTableModel::onFlashTick()
{
    emit flashTick();

    const qint64 now = m_clock.elapsed();
    bool anyActive = false;
    for (qint64& start : m_flashStartMs)
    {
        if (start == 0) continue;
        if (now - start >= kFlashDurationMs) start = 0;  // 过期清零
        else anyActive = true;
    }
    if (!anyActive) m_flashTimer->stop();
}

void RegisterTableModel::refreshSnapshot()
{
    if (!m_store) return;
    constexpr int kPerUnion = RegisterCellLayout::kInt16PerUnion;
    for (size_t i = 0; i < m_registerVals.size(); ++i)
        for (int j = 0; j < kPerUnion; ++j)
            m_registerVals[i].u_Int16[j] = m_store->cell(m_startAddr + static_cast<int>(i) * kPerUnion + j);
}

void RegisterTableModel::refreshAll()
{
    refreshSnapshot();
    if (m_layout.rowCount > 0 && m_layout.colCount > 0)
        emit dataChanged(index(0, 0), index(m_layout.rowCount - 1, m_layout.colCount - 1));
}

int RegisterTableModel::registersPerValue(RegisterDataType type) const
{
    return traitOf(type).registersPerValue;
}

bool RegisterTableModel::isAnchorValueCell(int row, int col) const
{
    if (!m_layout.isValueColumn(col)) return false;
    const int k = m_layout.linearIndex(row, col);
    if (m_layout.cacheIndex(k) >= static_cast<int>(m_registerVals.size())) return false;
    if (k % registersPerValue(m_currentType) != 0) return false;
    // 值占 rpv 个连续寄存器,整段须落在 store 容量内,否则该值格越界→留空/只读
    if (m_store)
    {
        const int rpv = registersPerValue(m_currentType);
        if (m_layout.registerAddr(k, m_startAddr) + rpv - 1 >= m_store->size())
            return false;
    }
    return true;
}

void RegisterTableModel::writeCell(RegisterDataType type, int k, const QString& text)
{
    const TypeTrait& t = traitOf(type);
    DataTypeConvert& cell = m_registerVals[m_layout.cacheIndex(k)];
    const int s = m_layout.subIndex(k);
    switch (t.family)
    {
    case TypeTrait::Char:
    {
        // 每个单元格写 2 个字符,字符下标 = Int16 子下标*2 + 字符序
        // 写前清两字节:单字符录入须清掉同格旧高字节(否则 'AB'→'C' 残留显示成 'CB')
        cell.u_chars[s * 2] = 0;
        cell.u_chars[s * 2 + 1] = 0;
        int curChar = 0;
        while (curChar < 2 && text.length() > curChar)
        {
            cell.u_chars[s * 2 + curChar] = text.at(curChar).toLatin1();
            curChar++;
        }
        break;
    }
    case TypeTrait::Integer:
    {
        int val;
        if (m_intStat == 0)
            val = text.toInt();
        else
            // 十六进制按无符号解析:FFFFFFFF/80000000 等高位值不会溢出归零(toInt 上限仅 INT_MAX)
            val = static_cast<int>(text.toUInt(nullptr, 16));
        if (t.registersPerValue == 1)
            cell.u_Int16[s] = val & 0xFFFF;      // Int16
        else
            cell.u_Int32[s / 2] = val;           // Int32(占 2 个 Int16)
        break;
    }
    case TypeTrait::Real:
        if (t.registersPerValue == 2)
            cell.u_float[s / 2] = text.toFloat();  // float(占 2 个 Int16)
        else
            cell.u_double = text.toDouble();       // double(占 4 个 Int16)
        break;
    }
}

QString RegisterTableModel::formatCell(RegisterDataType type, int k) const
{
    const TypeTrait& t = traitOf(type);
    const DataTypeConvert& cell = m_registerVals[m_layout.cacheIndex(k)];
    const int s = m_layout.subIndex(k);
    switch (t.family)
    {
    case TypeTrait::Char:
        return QString("%1%2")
            .arg(QChar(cell.u_chars[s * 2]))
            .arg(QChar(cell.u_chars[s * 2 + 1]));
    case TypeTrait::Integer:
    {
        if (m_intStat == 1)
        {
            // 十六进制按无符号位模式显示,保证 0xFFFF/0xFFFFFFFF 与录入往返(有符号会显示成 '-1'→'00-1')
            unsigned long long bits = (t.registersPerValue == 1)
                ? static_cast<unsigned long long>(static_cast<uint16_t>(cell.u_Int16[s]))
                : static_cast<unsigned long long>(static_cast<uint32_t>(cell.u_Int32[s / 2]));
            return QString("%1").arg(QString::number(bits, 16), t.hexDigits, QChar('0')).toUpper();
        }
        long long v = (t.registersPerValue == 1) ? cell.u_Int16[s] : cell.u_Int32[s / 2];
        return QString("%1").arg(v);
    }
    case TypeTrait::Real:
    {
        double v = (t.registersPerValue == 2) ? cell.u_float[s / 2] : cell.u_double;
        return formatReal(v, t.sigDigits);
    }
    }
    return QString();
}

void RegisterTableModel::refreshHeaders()
{
    if (columnCount() > 0)
        emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
}
