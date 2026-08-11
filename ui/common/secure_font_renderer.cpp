#include "secure_font_renderer.h"
#include "utils/widget_helpers.h"
#include <QFile>
#include <cmath>
#include <sodium.h>

bool SecureFontRenderer::loadFont(const QString& fontPath) {
    QFile fontFile(fontPath);
    if (!fontFile.open(QIODevice::ReadOnly)) return false;

    m_fontData = fontFile.readAll();
    const unsigned char* fontPtr = reinterpret_cast<const unsigned char*>(m_fontData.constData());

    if (stbtt_InitFont(&m_fontInfo, fontPtr, stbtt_GetFontOffsetForIndex(fontPtr, 0))) {
        m_fontLoaded = true;
        return true;
    }

    return false;
}

void SecureFontRenderer::setFontSize(float fontSize) {
    m_fontSize = fontSize;
}

SecureFontRenderer::FontMetrics SecureFontRenderer::getMetrics() const {
    if (!m_fontLoaded) return {0, 0, 0};
    float scale = stbtt_ScaleForMappingEmToPixels(&m_fontInfo, m_fontSize);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&m_fontInfo, &ascent, &descent, &lineGap);
    return { ascent * scale, descent * scale, (ascent - descent) * scale };
}

float SecureFontRenderer::calculateTextWidth(const std::uint8_t* textData, std::size_t textLen) const {
    if (!m_fontLoaded || textLen == 0) return 0.0f;
    float scale = stbtt_ScaleForMappingEmToPixels(&m_fontInfo, m_fontSize);
    float width = 0.0f;

    const std::uint8_t* ptr = textData;
    const std::uint8_t* end = ptr + textLen;

    std::uint32_t prevCp = 0;

    while (ptr < end) {
        std::uint32_t cp = nextUtf8Codepoint(ptr, end);
        if (cp == 0) break;

        if (prevCp != 0) {
            width += stbtt_GetCodepointKernAdvance(&m_fontInfo, prevCp, cp) * scale;
        }

        int advanceWidth, lsb;
        stbtt_GetCodepointHMetrics(&m_fontInfo, cp, &advanceWidth, &lsb);
        width += advanceWidth * scale;

        prevCp = cp;
    }

    return width;
}

float SecureFontRenderer::charIndexToOffset(int targetIdx, const std::uint8_t* textData, std::size_t textLen) const {
    if (!m_fontLoaded || textLen == 0) return 0.0f;
    float scale = stbtt_ScaleForMappingEmToPixels(&m_fontInfo, m_fontSize);
    float offset = 0.0f;
    int idx = 0;
    std::uint32_t prevCp = 0;
    const std::uint8_t* ptr = textData;
    const std::uint8_t* end = ptr + textLen;

    while (ptr < end && idx < targetIdx) {
        std::uint32_t cp = nextUtf8Codepoint(ptr, end);

        if (prevCp != 0) {
            offset += stbtt_GetCodepointKernAdvance(&m_fontInfo, prevCp, cp) * scale;
        }

        int advanceWidth, lsb;
        stbtt_GetCodepointHMetrics(&m_fontInfo, cp, &advanceWidth, &lsb);
        offset += advanceWidth * scale;

        prevCp = cp;
        idx++;
    }

    return offset;
}

int SecureFontRenderer::offsetToCharIndex(float offsetX, const std::uint8_t* textData, std::size_t textLen) const {
    if (!m_fontLoaded || textLen == 0) return 0;
    float scale = stbtt_ScaleForMappingEmToPixels(&m_fontInfo, m_fontSize);
    float currentX = 0.0f;
    int charIdx = 0;
    std::uint32_t prevCp = 0;
    const std::uint8_t* ptr = textData;
    const std::uint8_t* end = ptr + textLen;

    while (ptr < end) {
        std::uint32_t cp = nextUtf8Codepoint(ptr, end);

        if (prevCp != 0) {
            currentX += stbtt_GetCodepointKernAdvance(&m_fontInfo, prevCp, cp) * scale;
        }

        int advanceWidth, lsb;
        stbtt_GetCodepointHMetrics(&m_fontInfo, cp, &advanceWidth, &lsb);
        float step = advanceWidth * scale;
        if (offsetX < currentX + step / 2.0f) return charIdx;
        currentX += step;
        charIdx++;

        prevCp = cp;
    }

    return charIdx;
}

void SecureFontRenderer::renderText(
    const std::uint8_t* textData, std::size_t textLen,
    const QRect& cRect, qreal dpr,
    Qt::Alignment alignment,
    float scrollOffset,
    int selMin, int selMax,
    const QColor& normalColor, const QColor& highlightColor,
    float& outTextStartX, float& outTextStartY)
{
    if (!m_fontLoaded || cRect.isEmpty() || textLen == 0) {

        if (m_pixelBuffer.data())
            sodium_memzero(m_pixelBuffer.data(), m_pixelBuffer.capacity());

        m_imageWidth = 0;
        m_imageHeight = 0;

        outTextStartX = 0;
        outTextStartY = 0;
        return;
    }

    m_imageWidth = qCeil(cRect.width() * dpr);
    m_imageHeight = qCeil(cRect.height() * dpr);

    float physScale = stbtt_ScaleForMappingEmToPixels(&m_fontInfo, m_fontSize * dpr);

    float physTextWidth = 0.0f;
    std::uint32_t prevCpWidth = 0;

    const std::uint8_t* wPtr = textData;
    const std::uint8_t* wEnd = wPtr + textLen;

    while (wPtr < wEnd) {
        std::uint32_t cp = nextUtf8Codepoint(wPtr, wEnd);

        if (prevCpWidth != 0)
            physTextWidth += stbtt_GetCodepointKernAdvance(&m_fontInfo, prevCpWidth, cp) * physScale;

        int aw, lsb;
        stbtt_GetCodepointHMetrics(&m_fontInfo, cp, &aw, &lsb);
        physTextWidth += aw * physScale;

        prevCpWidth = cp;
    }

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&m_fontInfo, &ascent, &descent, &lineGap);
    int physAscent = std::round(ascent * physScale);
    int physTextHeight = std::round((ascent - descent) * physScale);

    float cursorX = 0, cursorY = 0;

    if (alignment != (Qt::AlignLeft | Qt::AlignVCenter)) {
        if (alignment & Qt::AlignHCenter) cursorX = (m_imageWidth - physTextWidth) / 2.0f;
        else if (alignment & Qt::AlignRight) cursorX = m_imageWidth - physTextWidth;

        if (alignment & Qt::AlignVCenter) cursorY = (m_imageHeight - physTextHeight) / 2.0f + physAscent;
        else if (alignment & Qt::AlignBottom) cursorY = m_imageHeight - (physTextHeight - physAscent);
        else cursorY = physAscent;
    } else {
        cursorY = (m_imageHeight - physTextHeight) / 2.0f + physAscent;
    }

    cursorX -= scrollOffset * dpr;

    outTextStartX = cursorX / dpr;
    outTextStartY = (cursorY - physAscent) / dpr;

    std::size_t reqSize = static_cast<std::size_t>(m_imageWidth) * static_cast<std::size_t>(m_imageHeight) * 4;

    m_pixelBuffer.resize(reqSize);

    if (m_pixelBuffer.data())
        sodium_memzero(m_pixelBuffer.data(), m_pixelBuffer.capacity());

    std::uint32_t* destPixels = reinterpret_cast<std::uint32_t*>(m_pixelBuffer.data());
    const bool hasSelection = (selMin != selMax);

    const std::uint32_t nR = normalColor.red(), nG = normalColor.green(), nB = normalColor.blue();
    const std::uint32_t hR = highlightColor.red(), hG = highlightColor.green(), hB = highlightColor.blue();

    const std::uint8_t* ptr = textData;
    const std::uint8_t* end = ptr + textLen;

    int charIdx = 0;
    std::uint32_t prevCpRender = 0;

    while (ptr < end) {
        std::uint32_t cp = nextUtf8Codepoint(ptr, end);
        if (cp == 0) break;

        if (prevCpRender != 0) {
            cursorX += stbtt_GetCodepointKernAdvance(&m_fontInfo, prevCpRender, cp) * physScale;
        }

        bool isSelected = hasSelection && (charIdx >= selMin && charIdx < selMax);
        std::uint32_t rColor = isSelected ? hR : nR;
        std::uint32_t gColor = isSelected ? hG : nG;
        std::uint32_t bColor = isSelected ? hB : nB;

        float x_shift = cursorX - std::floor(cursorX);
        float y_shift = cursorY - std::floor(cursorY);

        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBoxSubpixel(&m_fontInfo, cp, physScale, physScale, x_shift, y_shift, &x0, &y0, &x1, &y1);

        int glyphWidth = x1 - x0;
        int glyphHeight = y1 - y0;

        if (glyphWidth > 0 && glyphHeight > 0) {
            m_glyphBuffer.resize(static_cast<std::size_t>(glyphWidth) * static_cast<std::size_t>(glyphHeight));
            stbtt_MakeCodepointBitmapSubpixel(&m_fontInfo, m_glyphBuffer.data(), glyphWidth, glyphHeight, glyphWidth, physScale, physScale, x_shift, y_shift, cp);

            int baseDrawX = static_cast<int>(std::floor(cursorX)) + x0;
            int baseDrawY = static_cast<int>(std::floor(cursorY)) + y0;

            int startY = std::max(0, -baseDrawY);
            int endY = std::min(glyphHeight, m_imageHeight - baseDrawY);
            int startX = std::max(0, -baseDrawX);
            int endX = std::min(glyphWidth, m_imageWidth - baseDrawX);

            for (int y = startY; y < endY; ++y) {
                const std::uint8_t* alphaRow = m_glyphBuffer.data() + (y * glyphWidth);
                std::uint32_t* destRow = destPixels + ((baseDrawY + y) * m_imageWidth);

                for (int x = startX; x < endX; ++x) {
                    std::uint8_t alpha = alphaRow[x];
                    if (alpha > 0) {

                        std::uint32_t bg = destRow[baseDrawX + x];
                        if (bg == 0) {
                            std::uint32_t r = (rColor * alpha + 127) / 255;
                            std::uint32_t g = (gColor * alpha + 127) / 255;
                            std::uint32_t b = (bColor * alpha + 127) / 255;
                            destRow[baseDrawX + x] = (alpha << 24) | (r << 16) | (g << 8) | b;
                        } else {
                            std::uint32_t bgA = (bg >> 24 ) & 0xFF;
                            std::uint32_t bgR = (bg >> 16) & 0xFF;
                            std::uint32_t bgG = (bg >> 8) & 0xFF;
                            std::uint32_t bgB = bg & 0xFF;

                            std::uint32_t invAlpha = 255 - alpha;
                            std::uint32_t r = ((rColor * alpha) + bgR * invAlpha + 127) / 255;
                            std::uint32_t g = ((gColor * alpha) + bgG * invAlpha + 127) / 255;
                            std::uint32_t b = ((bColor * alpha) + bgB * invAlpha + 127) / 255;
                            std::uint32_t a = alpha + (bgA * invAlpha + 127) / 255;

                            destRow[baseDrawX + x] = (a << 24) | (r << 16) | (g << 8) | b;
                        }
                    }
                }
            }
        }

        int advanceWidth, lsb;
        stbtt_GetCodepointHMetrics(&m_fontInfo, cp, &advanceWidth, &lsb);
        cursorX += advanceWidth * physScale;
        prevCpRender = cp;
        charIdx++;
    }
}