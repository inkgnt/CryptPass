#include "secure_line_edit.h"
#include "utils/widget_helpers.h"
#include <QStyleOptionFrame>
#include <sodium.h>

SecureLineEdit::SecureLineEdit(QWidget *parent) : SecureTextWidget(parent) {
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setAttribute(Qt::WA_TranslucentBackground);
    setForegroundRole(QPalette::Text);
    setFocusPolicy(Qt::StrongFocus);

    setCursor(Qt::IBeamCursor);

    m_textBuffer.reserve(64);
}

QSize SecureLineEdit::minimumSizeHint() const {
    QSize baseS = SecureTextWidget::minimumSizeHint();

    float textWidth = 0.0f;
    if (m_textBuffer.size() > 0) {
        std::size_t len;
        const std::uint8_t* data = getRenderData(len);
        textWidth = m_renderer.calculateTextWidth(data, len);
    } else {
        textWidth = fontMetrics().horizontalAdvance(placeholderText());
    }

    int decorationsWidth = baseS.width() - static_cast<int>(textWidth);

    return QSize(decorationsWidth + 10, baseS.height());
}

QSize SecureLineEdit::sizeHint() const {
    QSize minS = minimumSizeHint();

    int typicalWidth = fontMetrics().horizontalAdvance('x') * 15;

    int finalWidth = (minS.width() - 10) + typicalWidth;

    return QSize(finalWidth, minS.height());
}

void SecureLineEdit::ensureCursorVisible() {
    std::size_t renderLen = 0;
    const std::uint8_t* renderData = getRenderData(renderLen);
    if (renderLen == 0) {
        m_scrollOffset = 0.0f;
        return;
    }

    float cursorLogicalX = m_renderer.charIndexToOffset(m_cursorCharIdx, renderData, renderLen);
    float textWidth = m_renderer.calculateTextWidth(renderData, renderLen);
    QRect cRect = textRect();

    const float cursorMargin = 4.0f;

    if (cursorLogicalX - m_scrollOffset < cursorMargin) {
        m_scrollOffset = std::max(0.0f, cursorLogicalX - cursorMargin);
    }
    else if (cursorLogicalX - m_scrollOffset > cRect.width() - cursorMargin) {
        m_scrollOffset = cursorLogicalX - cRect.width() + cursorMargin;
    }

    if (textWidth <= cRect.width()) {
        m_scrollOffset = 0.0f;
    } else if (m_scrollOffset > textWidth - cRect.width() + cursorMargin) {
        m_scrollOffset = textWidth - cRect.width() + cursorMargin;
    }

    m_needsRender = true;
}

void SecureLineEdit::resizeEvent(QResizeEvent *event) {
    SecureTextWidget::resizeEvent(event);

    ensureCursorVisible();
    update();
}

bool SecureLineEdit::deleteSelectedText() {
    if (m_selectionStartCharIdx == m_cursorCharIdx) return false;

    int minIdx = std::min(m_selectionStartCharIdx, m_cursorCharIdx);
    int maxIdx = std::max(m_selectionStartCharIdx, m_cursorCharIdx);

    std::size_t offsetStart = charIndexToByteOffset(minIdx);
    std::size_t offsetEnd = charIndexToByteOffset(maxIdx);
    std::size_t bytesToDelete = offsetEnd - offsetStart;

    if (m_textBuffer.size() > offsetEnd) {
        std::memmove(m_textBuffer.data() + offsetStart, m_textBuffer.data() + offsetEnd, m_textBuffer.size() - offsetEnd);
    }

    m_textBuffer.resize(m_textBuffer.size() - bytesToDelete);

    /*
    if (m_textBuffer.data())
        sodium_memzero(m_textBuffer.data() + m_textBuffer.size() - bytesToDelete, bytesToDelete);

    m_textLen -= bytesToDelete;
*/
    m_cursorCharIdx = minIdx;
    m_selectionStartCharIdx = minIdx;

    emit textChanged(getSecureText());
    return true;
}

void SecureLineEdit::insertText(const std::uint8_t* ptr, std::size_t len) {
    if (!ptr) return;

    deleteSelectedText();

    std::size_t bytesToInsert = len;
    const std::size_t oldSize = m_textBuffer.size();
    const std::size_t requiredByteSize = oldSize + bytesToInsert;
    const std::size_t currentByteOffset = charIndexToByteOffset(m_cursorCharIdx);

    if (requiredByteSize > m_textBuffer.capacity()) {
        std::size_t newCapacity = std::max(requiredByteSize, m_textBuffer.capacity() * 2);
        m_textBuffer.reserve(newCapacity);
    }

    m_textBuffer.resize(requiredByteSize);

    if (currentByteOffset < m_textBuffer.size()) {
        std::memmove(
            m_textBuffer.data() + currentByteOffset + bytesToInsert,
            m_textBuffer.data() + currentByteOffset,
            oldSize - currentByteOffset
            );
    }

    std::memcpy(m_textBuffer.data() + currentByteOffset, ptr, bytesToInsert);

    int insertedChars = 0;
    const std::uint8_t* p = ptr;
    const std::uint8_t* e = p + len;
    while (p < e) {
        nextUtf8Codepoint(p, e);
        insertedChars++;
    }

    m_cursorCharIdx += insertedChars;
    m_selectionStartCharIdx = m_cursorCharIdx;


    m_needsRender = true;
    ensureCursorVisible();
    updateGeometry();
    update();

    emit textChanged(getSecureText());
}

void SecureLineEdit::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {

        std::size_t renderLen = 0;
        const std::uint8_t* renderData = getRenderData(renderLen);

        float localX = event->pos().x() - (textRect().left() + m_textStartX);
        m_cursorCharIdx = m_renderer.offsetToCharIndex(localX, renderData, renderLen);

        if (!(event->modifiers() & Qt::ShiftModifier)) {
            m_selectionStartCharIdx = m_cursorCharIdx;
        }

        if (m_cursorTimerId != 0) killTimer(m_cursorTimerId);
        m_cursorTimerId = startTimer(500);
        m_cursorVisible = true;
        m_needsRender = true;
        ensureCursorVisible();
        update();
    }
}

void SecureLineEdit::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        QRect cRect = textRect();
        int x = event->pos().x();

        if (x < cRect.left())
        {
            m_autoScrollDirection = -1;

            int distance = cRect.left() - x;

            m_autoScrollStep = std::min(1 + (distance * distance) / 200, 60 ) ;
            if (m_autoScrollTimerId == 0)
                m_autoScrollTimerId = startTimer(m_autoScrollTimerValue);
        }
        else if (x > cRect.right())
        {
            m_autoScrollDirection = 1;

            int distance = x - cRect.right();

            m_autoScrollStep = std::min(1 + (distance * distance) / 200, 60);
            if (m_autoScrollTimerId == 0)
                m_autoScrollTimerId = startTimer(m_autoScrollTimerValue);
        }
        else
        {
            if (m_autoScrollTimerId != 0) {
                killTimer(m_autoScrollTimerId);
                m_autoScrollTimerId = 0;
            }

            std::size_t renderLen = 0;
            const std::uint8_t* renderData = getRenderData(renderLen);
            float localX = x - (cRect.left() + m_textStartX);
            m_cursorCharIdx = m_renderer.offsetToCharIndex(localX, renderData, renderLen);

            m_needsRender = true;
            ensureCursorVisible();
            update();
        }
    }
}

void SecureLineEdit::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_autoScrollTimerId != 0) {
        killTimer(m_autoScrollTimerId);
        m_autoScrollTimerId = 0;
        m_autoScrollStep = 1;
        m_autoScrollDirection = 0;
    }
    SecureTextWidget::mouseReleaseEvent(event);
}

void SecureLineEdit::keyPressEvent(QKeyEvent *event) {
    if (event->matches(QKeySequence::SelectAll)) {
        m_selectionStartCharIdx = 0;
        m_cursorCharIdx = totalChars();
        m_needsRender = true; ensureCursorVisible(); update();
        return;
    }

    if (event->key() == Qt::Key_Backspace) {
        if (!deleteSelectedText() && m_cursorCharIdx > 0) {
            m_selectionStartCharIdx = (event->modifiers() & Qt::ControlModifier) ? 0 : m_cursorCharIdx - 1;
            deleteSelectedText();
        }
        m_needsRender = true; ensureCursorVisible(); updateGeometry(); update();
    }
    else if (event->key() == Qt::Key_Delete) {
        if (!deleteSelectedText() && m_cursorCharIdx < totalChars()) {
            m_selectionStartCharIdx = (event->modifiers() & Qt::ControlModifier) ? totalChars() : m_cursorCharIdx + 1;
            deleteSelectedText();
        }
        m_needsRender = true; ensureCursorVisible(); updateGeometry(); update();
    }
    else if (event->key() == Qt::Key_Left) {
        int target = std::max<int>(0, m_cursorCharIdx - 1);
        if (event->modifiers() & Qt::ShiftModifier) {
            m_cursorCharIdx = target;
        } else {
            if (m_selectionStartCharIdx != m_cursorCharIdx) m_cursorCharIdx = std::min(m_selectionStartCharIdx, m_cursorCharIdx);
            else m_cursorCharIdx = target;
            m_selectionStartCharIdx = m_cursorCharIdx;
        }
        m_needsRender = true; ensureCursorVisible(); update();
    }
    else if (event->key() == Qt::Key_Right) {
        int target = std::min<int>(static_cast<int>(totalChars()), m_cursorCharIdx + 1);
        if (event->modifiers() & Qt::ShiftModifier) {
            m_cursorCharIdx = target;
        } else {
            if (m_selectionStartCharIdx != m_cursorCharIdx) m_cursorCharIdx = std::max(m_selectionStartCharIdx, m_cursorCharIdx);
            else m_cursorCharIdx = target;
            m_selectionStartCharIdx = m_cursorCharIdx;
        }
        m_needsRender = true; ensureCursorVisible(); update();
    }
    else if (!event->text().isEmpty()) {
        const QString& t = event->text();
        if (t.length() > 0 && t[0].isPrint()) {
            QByteArray tempUtf8 = t.toUtf8();
            insertText(reinterpret_cast<const std::uint8_t*>(tempUtf8.data()), tempUtf8.size());

            sodium_memzero(tempUtf8.data(), tempUtf8.size());
        }
    }

    if (m_cursorTimerId != 0) {
        killTimer(m_cursorTimerId);
        m_cursorTimerId = startTimer(500);
        m_cursorVisible = true;
    }
}

void SecureLineEdit::focusInEvent(QFocusEvent *event) {
    SecureTextWidget::focusInEvent(event);
    m_cursorTimerId = startTimer(500);
    m_cursorVisible = true;
    ensureCursorVisible();
    update();
}

void SecureLineEdit::focusOutEvent(QFocusEvent *event) {
    SecureTextWidget::focusOutEvent(event);
    if (m_cursorTimerId != 0) {
        killTimer(m_cursorTimerId);
        m_cursorTimerId = 0;
    }
    m_cursorVisible = false;

    if (m_selectionStartCharIdx != m_cursorCharIdx) {
        m_selectionStartCharIdx = m_cursorCharIdx;
        m_needsRender = true;
    }
    ensureCursorVisible();
    update();
}

void SecureLineEdit::timerEvent(QTimerEvent *event) {
    if (event->timerId() == m_cursorTimerId) {
        m_cursorVisible = !m_cursorVisible;
        update();
    }

    else if (event->timerId() == m_autoScrollTimerId) {
        int oldIdx = m_cursorCharIdx;

        if (m_autoScrollDirection < 0) {
            m_cursorCharIdx = std::max<int>(0, m_cursorCharIdx - m_autoScrollStep);
        } else {
            m_cursorCharIdx = std::min<int>(static_cast<int>(totalChars()), m_cursorCharIdx + m_autoScrollStep);
        }

        if (oldIdx != m_cursorCharIdx) {
            ensureCursorVisible();
            m_needsRender = true;
            update();
        }
    }
    else {
        SecureTextWidget::timerEvent(event);
    }
}
