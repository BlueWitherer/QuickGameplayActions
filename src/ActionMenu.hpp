#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ActionMenu : public CCLayer {
private:
    class Impl;
    std::unique_ptr<Impl> m_impl;

    struct ActionItem {
        bool enabled;
        std::string sprite;
        std::string id;
        SEL_MenuHandler selector;

        ActionItem(
            bool e,
            std::string spr,
            std::string i,
            SEL_MenuHandler sel
        ) : enabled(e),
            sprite(std::move(spr)),
            id(std::move(i)),
            selector(sel) {};
    };

protected:
    ActionMenu();
    virtual ~ActionMenu();

    void onScaleEnd();

    void onRestart(CCObject*);
    void onFullRestart(CCObject*);
    void onPractice(CCObject*);
    void onExit(CCObject*);

    void setOpacity(GLubyte opacity);
    void setScale(float scale);

    bool init(PlayLayer* pl);

public:
    static ActionMenu* create(PlayLayer* pl);

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;
    void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
    void ccTouchEnded(CCTouch* touch, CCEvent* event) override;
};