#include "secure_text_widget.h"
#include "utils/widget_helpers.h"

#include <QApplication>
#include <qtimer.h>
#include <sodium.h>

#include <QPainter>
#include <QStyleOptionFrame>
#include <QToolButton>
#include <QActionEvent>
#include <QEvent>

namespace {
const uint8_t* getStaticDotsData(size_t charCount, size_t& outByteLen) {
    static const size_t MAX_DOTS = 1024;
    static const std::vector<uint8_t> dotsBuffer = []() {
        std::vector<uint8_t> buffer;
        buffer.reserve(MAX_DOTS * 3);
        for (size_t i = 0; i < MAX_DOTS; ++i) {
            buffer.push_back(0xE2);
            buffer.push_back(0x80);
            buffer.push_back(0xA2);
        }
        return buffer;
    }();

    size_t count = std::min<size_t>(charCount, MAX_DOTS);
    outByteLen = count * 3;
    return dotsBuffer.data();
}
}

SecureTextWidget::SecureTextWidget(QWidget *parent) : QFrame(parent) {

    m_renderer.loadFont(m_fontPath);
    m_renderer.setFontSize(m_fontSize);

    qApp->installEventFilter(this);
}

SecureTextWidget::~SecureTextWidget() {
    if (qApp) {
        qApp->removeEventFilter(this);
    }
}


// for tomorrow me: rembeber of this bug, I have to filter main app events, because QSS blocks
// it when QSS is present in widget object
bool SecureTextWidget::eventFilter(QObject *watched, QEvent *event) {
    if (watched == qApp) {
        if (event->type() == QEvent::ApplicationPaletteChange ||
            event->type() == QEvent::ThemeChange)
        {
            m_needsRender = true;
            update();
        }
    }
    return QFrame::eventFilter(watched, event);
}

void SecureTextWidget::changeEvent(QEvent *event) {
    QFrame::changeEvent(event);

    if (event->type() == QEvent::FontChange) {
        m_renderer.setFontSize(m_fontSize);
    }

    if (event->type() == QEvent::PaletteChange ||
        event->type() == QEvent::ApplicationPaletteChange ||
        event->type() == QEvent::StyleChange)
    {
        m_needsRender = true;
        updateGeometry();
        update();
    }
}

void SecureTextWidget::clear() {
    if (m_textBuffer.data())
        sodium_memzero(m_textBuffer.data(), m_textBuffer.capacity());

    m_textLen = 0;
    m_cursorCharIdx = 0;
    m_selectionStartCharIdx = 0;
    m_needsRender = true;
    updateGeometry();
    update();
}

void SecureTextWidget::initStyleOptionForText(QStyleOptionFrame *opt) const {
    opt->initFrom(this);

    if (frameShape() != QFrame::NoFrame) {
        opt->lineWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, opt, this);
        opt->state |= QStyle::State_Sunken;
    } else {
        opt->lineWidth = 0;
        opt->features |= QStyleOptionFrame::Flat;
    }
}

SecureBuffer SecureTextWidget::getSecureText() const {
    if (m_textLen == 0)
        return SecureBuffer();

    SecureBuffer copy(m_textLen);
    std::memcpy(copy.data(), m_textBuffer.data(), m_textLen);
    return copy;
}

void SecureTextWidget::setPlaceholderText(const QString& placeholder) {
    if (m_placeholderText != placeholder) { m_placeholderText = placeholder; update(); }
}

void SecureTextWidget::setAlignment(Qt::Alignment align) {
    m_alignment = align; m_needsRender = true; update();
}

void SecureTextWidget::setObfuscated(bool obfuscate) {
    m_obfuscated = obfuscate; m_needsRender = true; updateGeometry(); update();
}

QRect SecureTextWidget::textRect() const {
    QStyleOptionFrame opt;
    initStyleOptionForText(&opt);

    QRect r;
    if (frameShape() != QFrame::NoFrame) {
        r = style()->subElementRect(QStyle::SE_LineEditContents, &opt, this);
    } else {
        r = rect();
    }

    r = r.marginsRemoved(contentsMargins());
    r = r.marginsRemoved(m_textMargins);
    r = r.marginsRemoved(m_buttonMargins);

    return r;
}

size_t SecureTextWidget::totalChars() const {
    size_t count = 0;
    const uint8_t* ptr = m_textBuffer.data();
    const uint8_t* end = ptr + m_textLen;
    while(ptr < end) {
        nextUtf8Codepoint(ptr, end);
        count++;
    }
    return count;
}

size_t SecureTextWidget::charIndexToByteOffset(int targetIdx) const {
    int idx = 0;
    const uint8_t* ptr = m_textBuffer.data();
    const uint8_t* end = ptr + m_textLen;
    while (ptr < end && idx < targetIdx) { nextUtf8Codepoint(ptr, end); idx++; }
    return ptr - m_textBuffer.data();
}

const uint8_t* SecureTextWidget::getRenderData(size_t& outLen) const {
    if (!m_obfuscated) {
        outLen = m_textLen;
        return m_textBuffer.data();
    } else {
        return getStaticDotsData(totalChars(), outLen);
    }
}

QSize SecureTextWidget::sizeHint() const { return minimumSizeHint(); }

QSize SecureTextWidget::minimumSizeHint() const {
    if (!m_renderer.isLoaded())
        return {0, 0};

    float textWidth = 0.0f;
    if (m_textLen > 0) {
        size_t renderLen = 0;
        const uint8_t* renderData = getRenderData(renderLen);
        textWidth = m_renderer.calculateTextWidth(renderData, renderLen);
    } else if (!m_placeholderText.isEmpty()) {
        QFont f = font();
        f.setPixelSize(static_cast<int>(m_fontSize));
        QFontMetrics fm(f);
        textWidth = fm.horizontalAdvance(m_placeholderText);
    }

    auto metrics = m_renderer.getMetrics();
    int textHeight = std::round(metrics.textHeight);

    int maxButtonHeight = 0;
    for (auto *btn : std::as_const(m_leadingButtons)) maxButtonHeight = std::max(maxButtonHeight, btn->sizeHint().height());
    for (auto *btn : std::as_const(m_trailingButtons)) maxButtonHeight = std::max(maxButtonHeight, btn->sizeHint().height());

    int contentHeight = std::max(textHeight, maxButtonHeight);

    int totalWidth = static_cast<int>(textWidth)
                     + m_textMargins.left() + m_textMargins.right()
                     + m_buttonMargins.left() + m_buttonMargins.right();

    int totalHeight = contentHeight
                      + m_textMargins.top() + m_textMargins.bottom();

    QStyleOptionFrame opt;
    initStyleOptionForText(&opt);
    opt.rect = QRect(0, 0, 1000, 1000);

    if (frameShape() != QFrame::NoFrame) {
        QRect cRect = style()->subElementRect(QStyle::SE_LineEditContents, &opt, this);
        return {
            totalWidth + (1000 - cRect.width()) + 2,
            totalHeight + (1000 - cRect.height())
        };
    } else {
        return {
            totalWidth + contentsMargins().left() + contentsMargins().right() + 2,
            totalHeight + contentsMargins().top() + contentsMargins().bottom()
        };
    }
}

void SecureTextWidget::paintEvent(QPaintEvent *event) {
    QStyleOptionFrame opt;
    initStyleOptionForText(&opt);
    QPainter painter(this);

    if (frameShape() != QFrame::NoFrame) {
        style()->drawPrimitive(QStyle::PE_PanelLineEdit, &opt, &painter, this);
    } else {
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    }

    QRect cRect = textRect();
    if (cRect.isEmpty())
        return;

    auto metrics = m_renderer.getMetrics();

    if (m_textLen == 0 && !m_placeholderText.isEmpty()) {
        painter.setPen(palette().color(QPalette::PlaceholderText));
        QFont f = font();
        f.setPixelSize(static_cast<int>(m_fontSize));
        painter.setFont(f);
        painter.drawText(cRect, m_alignment, m_placeholderText);
    }

    size_t renderLen = 0;
    const uint8_t* renderData = getRenderData(renderLen);

    int selMin = std::min(m_selectionStartCharIdx, m_cursorCharIdx);
    int selMax = std::max(m_selectionStartCharIdx, m_cursorCharIdx);


    if (m_needsRender) {
        this->style()->unpolish(this);
        this->style()->polish(this);

        qDebug() << "current Role:" << foregroundRole() << "color:" << palette().color(foregroundRole()).name() << "palette Color:" << palette().color(foregroundRole());

        m_renderer.renderText(
            renderData, renderLen, cRect, devicePixelRatioF(), m_alignment,
            m_scrollOffset,
            selMin, selMax, palette().color(foregroundRole()), palette().color(QPalette::HighlightedText),
            m_textStartX, m_textStartY
            );
        m_needsRender = false;
    }

    float globalStartX = cRect.left() + m_textStartX;
    float globalStartY = cRect.top() + m_textStartY;

    if (renderLen == 0) {
        if (m_alignment & Qt::AlignVCenter) globalStartY = cRect.top() + (cRect.height() - metrics.textHeight) / 2.0f;
        else if (m_alignment & Qt::AlignBottom) globalStartY = cRect.bottom() - metrics.textHeight;
        else globalStartY = cRect.top();

        if (m_alignment & Qt::AlignHCenter) globalStartX = cRect.left() + cRect.width() / 2.0f;
        else if (m_alignment & Qt::AlignRight) globalStartX = cRect.right();
        else globalStartX = cRect.left();
    }

    painter.setClipRect(cRect);

    if (selMin != selMax && renderLen > 0) {
        float x1 = m_renderer.charIndexToOffset(selMin, renderData, renderLen);
        float x2 = m_renderer.charIndexToOffset(selMax, renderData, renderLen);

        QRectF highlightRect(globalStartX + x1, globalStartY, x2 - x1, metrics.textHeight);
        painter.fillRect(highlightRect, palette().color(QPalette::Highlight));
    }

    if (!m_renderer.pixelBuffer().empty() && m_renderer.imageWidth() > 0 && renderLen > 0) {
        QImage image(m_renderer.pixelBuffer().data(), m_renderer.imageWidth(), m_renderer.imageHeight(),
                     m_renderer.imageWidth() * 4, QImage::Format_ARGB32_Premultiplied);
        image.setDevicePixelRatio(devicePixelRatioF());
        painter.drawImage(cRect.topLeft(), image);
    }

    if (hasFocus() && m_cursorVisible) {
        painter.setPen(palette().color(foregroundRole()));

        float cursorOffset = 0.0f;
        if (renderLen > 0) {
            cursorOffset = m_renderer.charIndexToOffset(m_cursorCharIdx, renderData, renderLen);
        }

        int cx = std::round(globalStartX + cursorOffset);
        int cy = std::round(globalStartY);
        int cHeight = std::round(metrics.textHeight);

        painter.drawLine(cx, cy, cx, cy + cHeight);
    }
}

void SecureTextWidget::resizeEvent(QResizeEvent *event) {
    QFrame::resizeEvent(event);

    updateButtonPositions();

    m_needsRender = true;
    update();
}

QAction* SecureTextWidget::addAction(const QIcon &icon, QLineEdit::ActionPosition position) {
    QAction *action = new QAction(icon, "", this);
    QFrame::addAction(action);

    QToolButton *button = new QToolButton(this);
    button->setDefaultAction(action);
    button->setCursor(Qt::ArrowCursor);
    button->setFocusPolicy(Qt::NoFocus);
    button->setStyleSheet("QToolButton { border: none; background: transparent; }");

    if (position == QLineEdit::LeadingPosition) {
        m_leadingButtons.append(button);
    } else {
        m_trailingButtons.append(button);
    }

    button->show();
    updateButtonPositions();
    return action;
}

void SecureTextWidget::updateButtonPositions() {
    QStyleOptionFrame opt;
    initStyleOptionForText(&opt);
    QRect innerRect = style()->subElementRect(QStyle::SE_LineEditContents, &opt, this);

    int left = 0;
    for (auto *btn : std::as_const(m_leadingButtons)) {
        QSize sz = btn->sizeHint();
        btn->resize(sz);
        int y = innerRect.top() + qRound((innerRect.height() - sz.height()) / 2.0);
        btn->move(innerRect.left() + left, y);
        left += sz.width() + m_actionSpacing;
    }

    int right = 0;
    for (auto *btn : std::as_const(m_trailingButtons)) {
        QSize sz = btn->sizeHint();
        btn->resize(sz);
        int y = innerRect.top() + qRound((innerRect.height() - sz.height()) / 2.0);
        btn->move(innerRect.right() - right - sz.width() + 1, y);
        right += sz.width() + m_actionSpacing;
    }

    m_buttonMargins = QMargins(left, 0, right, 0);

    m_needsRender = true;
    update();
}

void SecureTextWidget::actionEvent(QActionEvent *event) {
    QFrame::actionEvent(event);

    if (event->type() == QEvent::ActionRemoved) {
        QAction *action = event->action();

        auto removeBtn = [&](QList<QToolButton*>& list) {
            for (int i = 0; i < list.size(); ++i) {
                if (list[i]->defaultAction() == action) {
                    list[i]->deleteLater();
                    list.removeAt(i);
                    return true;
                }
            }
            return false;
        };

        if (removeBtn(m_leadingButtons) || removeBtn(m_trailingButtons)) {
            updateButtonPositions();
        }
    }
}

void SecureTextWidget::setActionSpacing(int spacing) {
    if (m_actionSpacing != spacing) {
        m_actionSpacing = spacing;
        updateButtonPositions();
    }
}

void SecureTextWidget::setTextMargins(int left, int top, int right, int bottom) {
    setTextMargins(QMargins(left, top, right, bottom));
}

void SecureTextWidget::setTextMargins(const QMargins &margins) {
    if (m_textMargins != margins) {
        m_textMargins = margins;
        m_needsRender = true;
        updateGeometry();
        update();
    }
}