#pragma once

#include "utils/secure_buffer.h"
#include "secure_font_renderer.h"

#include<QLineEdit>
#include <QFrame>

class SecureTextWidget : public QFrame {
    Q_OBJECT

    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(bool obfuscated READ isObfuscated WRITE setObfuscated)

public:
    explicit SecureTextWidget(QWidget *parent = nullptr);
    virtual ~SecureTextWidget() = default;

    void clear();
    SecureBuffer getSecureText() const;

    int actionSpacing() const { return m_actionSpacing; }
    void setActionSpacing(int spacing);

    QMargins textMargins() const { return m_textMargins; }
    void setTextMargins(int left, int top, int right, int bottom);
    void setTextMargins(const QMargins &margins);

    QAction* addAction(const QIcon &icon, QLineEdit::ActionPosition position);
    QString placeholderText() const { return m_placeholderText; }
    void setPlaceholderText(const QString& placeholder);

    QString fontPath() const { return m_fontPath; }
    void setFontPath(const QString& m_fontPath);

    Qt::Alignment alignment() const { return m_alignment; }
    void setAlignment(Qt::Alignment align);

    bool isObfuscated() const { return m_obfuscated; }
    void setObfuscated(bool obfuscate);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void initStyleOptionForText(QStyleOptionFrame *opt) const;

    void actionEvent(QActionEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;

    QRect textRect() const;
    int totalChars() const;
    size_t charIndexToByteOffset(int targetIdx) const;


    const uint8_t* getRenderData(size_t& outLen) const;
    void updateObfuscationBuffer();

    SecureBuffer m_textBuffer;
    size_t m_textLen = 0;
    size_t m_textCapacity = 0;

    QString m_placeholderText;

    float m_fontSize = 12.0f;
    QString m_fontPath = ":/fonts/RobotoMono-Regular";

    float m_scrollOffset = 0.0f;

    Qt::Alignment m_alignment = Qt::AlignLeft | Qt::AlignVCenter;

    bool m_obfuscated = true;
    SecureBuffer m_obfuscationBuffer;
    size_t m_obfuscationCapacity = 0;

    float m_textStartX = 0;
    float m_textStartY = 0;

    int m_cursorCharIdx = 0;
    int m_selectionStartCharIdx = 0;
    bool m_needsRender = true;
    bool m_cursorVisible = false;

    SecureFontRenderer m_renderer;


    int m_actionSpacing = 0;
    QMargins m_textMargins;
    QMargins m_buttonMargins;

private:

    void updateButtonPositions();
    QList<QToolButton*> m_leadingButtons;
    QList<QToolButton*> m_trailingButtons;
};