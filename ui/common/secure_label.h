#pragma once
#include "secure_text_widget.h"

class SecureLabel : public SecureTextWidget {
    Q_OBJECT
public:
    explicit SecureLabel(QWidget *parent = nullptr);
    ~SecureLabel() override = default;

    void setSecureText(const std::uint8_t* utf8_data, std::size_t size);
    const std::uint8_t* getRenderData(std::size_t& outLen) const override;
};