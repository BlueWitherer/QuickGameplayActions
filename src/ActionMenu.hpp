#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

template <>
struct matjson::Serialize<CCPoint> final {
    static geode::Result<CCPoint> fromJson(matjson::Value const& value);
    static matjson::Value toJson(CCPoint const& value);
};

class ActionMenu final : public CCLayer {
private:
    class Impl;
    std::unique_ptr<Impl> m_impl;

    using Callback = Function<void(CCMenuItem*)>;

    struct ActionItem final {
        bool enabled;
        const char* sprite;
        const char* id;
        Callback callback;
        float scale = 1.f;
    };

    void onScaleEnd();

protected:
    ActionMenu();
    ~ActionMenu();

    void setOpacity(GLubyte opacity);

    void setScale(float scale) override;
    void setVisible(bool visible) override;

    bool init(PlayLayer* pl);

public:
    static ActionMenu* create(PlayLayer* pl);

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;
    void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
    void ccTouchEnded(CCTouch* touch, CCEvent* event) override;
};