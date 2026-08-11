#pragma once

#include "utils/secure_buffer.h"
#include <QRect>
#include <QColor>
#include <QString>
#include <QByteArray>

#include "stb_truetype.h"

class SecureFontRenderer {
public:
    SecureFontRenderer() = default;
    ~SecureFontRenderer() = default;

    bool loadFont(const QString& fontPath);
    bool isLoaded() const { return m_fontLoaded; }

    void setFontSize(float fontSize);
    float fontSize() const { return m_fontSize; }

    struct FontMetrics {
        float ascent;
        float descent;
        float textHeight;
    };
    FontMetrics getMetrics() const;

    float calculateTextWidth(const std::uint8_t* textData, std::size_t textLen) const;

    float charIndexToOffset(int targetIdx, const std::uint8_t* textData, std::size_t textLen) const;

    int offsetToCharIndex(float offsetX, const std::uint8_t* textData, std::size_t textLen) const;

    void renderText(
        const std::uint8_t* textData, std::size_t textLen,
        const QRect& cRect, qreal dpr,
        Qt::Alignment alignment,
        float scrollOffset,
        int selMin, int selMax,
        const QColor& normalColor, const QColor& highlightColor,
        float& outTextStartX, float& outTextStartY
    );

    const SecureBuffer& pixelBuffer() const { return m_pixelBuffer; }
    int imageWidth() const { return m_imageWidth; }
    int imageHeight() const { return m_imageHeight; }
    stbtt_fontinfo m_fontInfo;
    QByteArray m_fontData;

private:
    bool m_fontLoaded = false;
    float m_fontSize = 16.0f;

    SecureBuffer m_pixelBuffer;

    int m_imageWidth = 0;
    int m_imageHeight = 0;
    SecureBuffer m_glyphBuffer;
};