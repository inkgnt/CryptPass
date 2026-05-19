#include "secure_label.h"
#include <cstring>

SecureLabel::SecureLabel(QWidget *parent) : SecureTextWidget(parent) {
    setFrameStyle(QFrame::NoFrame);

    setForegroundRole(QPalette::WindowText);

    setFocusPolicy(Qt::NoFocus);
    setObfuscated(false);
}

void SecureLabel::setSecureText(const uint8_t* utf8_data, size_t size) {
    clear();

    if (utf8_data != nullptr && size > 0) {
        m_textBuffer.resize(size);
        std::memcpy(m_textBuffer.data(), utf8_data, size);
        m_textLen = size;
    }

    m_needsRender = true;
    updateGeometry();
    update();
}