#ifndef LUAHIGHLIGHTER_H
#define LUAHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>
#include <QStringList>

class LuaHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    LuaHighlighter(QTextDocument *parent = nullptr);

    void setCustomFunctions(const QStringList &functions);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
};

#endif // LUAHIGHLIGHTER_H
