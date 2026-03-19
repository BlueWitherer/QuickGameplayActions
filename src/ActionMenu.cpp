#include "ActionMenu.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

// it's modding time >:3
static auto qga = Mod::get();

Result<CCPoint> matjson::Serialize<CCPoint>::fromJson(matjson::Value const& value) {
    GEODE_UNWRAP_INTO(float x, value["x"].asDouble());
    GEODE_UNWRAP_INTO(float y, value["y"].asDouble());

    return Ok(CCPoint{x, y});
};

matjson::Value matjson::Serialize<CCPoint>::toJson(CCPoint const& value) {
    auto obj = matjson::Value();
    obj["x"] = value.x;
    obj["y"] = value.y;

    return obj;
};

class ActionMenu::Impl final {
public:
    Ref<CircleButtonSprite> sprite = nullptr;

    CCMenu* menu = nullptr;
    NineSlice* menuBg = nullptr;

    bool useRestart = qga->getSettingValue<bool>("restart");
    bool usePractice = qga->getSettingValue<bool>("practice");
    bool usePause = qga->getSettingValue<bool>("pause");
    bool useExit = qga->getSettingValue<bool>("exit");

    bool show = qga->getSavedValue<bool>("visible", true);
    bool toggleOnPress = qga->getSettingValue<bool>("toggle-press");

    float scale = static_cast<float>(qga->getSettingValue<double>("scale"));
    int64_t opacity = qga->getSettingValue<int64_t>("opacity");

    CCSize const screenSize = CCDirector::sharedDirector()->getWinSize();
    CCPoint dragStartPos = {0, 0};
    CCPoint comparePos = {0, 0};

    bool isAnimating = false;
    bool isDragging = false;

    bool isDistant(CCPoint const& ccp1, CCPoint const& ccp2, float max) const {
        return ccpDistance(ccp1, ccp2) <= max;
    };
};

ActionMenu::ActionMenu() : m_impl(std::make_unique<Impl>()) {};
ActionMenu::~ActionMenu() {};

bool ActionMenu::init(PlayLayer* pl) {
    if (!CCLayer::init()) return false;

    // get the saved position
    auto pos = qga->getSavedValue<CCPoint>("menu-pos", {75.f, m_impl->screenSize.height - 75.f});

    setID("menu"_spr);
    setPosition({pos.x, pos.y});
    setTouchMode(kCCTouchesOneByOne);
    setAnchorPoint({0.5, 0.5});
    setTouchEnabled(true);

    m_impl->sprite = CircleButtonSprite::createWithSpriteFrameName("edit_areaModeBtn04_001.png");
    m_impl->sprite->setScale(m_impl->scale * 0.875f);
    m_impl->sprite->setAnchorPoint({0.5, 0.5});

    setContentSize(m_impl->sprite->getScaledContentSize());

    m_impl->sprite->setPosition(getScaledContentSize() / 2.f);

    setScale(m_impl->scale);      // set initial scale
    setOpacity(m_impl->opacity);  // set initial opacity

    addChild(m_impl->sprite, 9);

    auto showMenu = m_impl->toggleOnPress ? m_impl->show : true;

    auto layout = RowLayout::create()
                      ->setGap(2.5f)
                      ->setAutoGrowAxis(0.f);

    m_impl->menu = CCMenu::create();
    m_impl->menu->setID("actions-menu");
    m_impl->menu->setAnchorPoint({0, 1});
    m_impl->menu->setContentSize({0.f, 25.f});
    m_impl->menu->setPosition({(getScaledContentWidth() / 2.f) + 7.5f, (getScaledContentHeight() / 2.f) - 5.f});
    m_impl->menu->setVisible(showMenu);
    m_impl->menu->setLayout(layout);

    auto btns = std::to_array<ActionItem>({
        {
            m_impl->useRestart,
            "GJ_replayBtn_001.png",
            "restart-btn",
            [playLayer = WeakRef(pl)](auto) {
                if (auto pl = playLayer.lock()) pl->resetLevel();
            },
        },
        {
            m_impl->usePractice,
            "GJ_practiceBtn_001.png",
            "toggle-practice-btn",
            [self = WeakRef(this), playLayer = WeakRef(pl)](CCMenuItem* sender) {
                if (auto pl = playLayer.lock()) {
                    pl->togglePracticeMode(!pl->m_isPracticeMode);

                    if (auto s = self.lock()) {
                        if (auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender)) {
                            auto btnSprite = CCSprite::createWithSpriteFrameName(pl->m_isPracticeMode ? "GJ_normalBtn_001.png" : "GJ_practiceBtn_001.png");
                            btnSprite->setOpacity(s->m_impl->opacity);
                            btnSprite->setScale(s->m_impl->scale);

                            btn->setNormalImage(btnSprite);
                            btn->updateSprite();
                        } else {
                            log::error("Couldn't get toggle practice button");
                        };
                    };
                };
            },
        },
        {
            m_impl->usePause,
            "GJ_pauseBtn_001.png",
            "pause-btn",
            [playLayer = WeakRef(pl)](auto) {
                if (auto pl = playLayer.lock()) pl->pauseGame(false);
            },
            1.62f,
        },
        {
            m_impl->useExit,
            "GJ_menuBtn_001.png",
            "exit-btn",
            [playLayer = WeakRef(pl)](auto) {
                if (auto pl = playLayer.lock()) {
                    pl->onQuit();

                    // @geode-ignore(unknown-resource)
                    if (auto fmod = FMODAudioEngine::sharedEngine()) fmod->playEffectAsync("quitSound_01.ogg");
                };
            },
        },
        {
            m_impl->useRestart && pl->m_isPlatformer,
            "GJ_replayFullBtn_001.png",
            "full-restart-btn",
            [playLayer = WeakRef(pl)](auto) {
                if (auto pl = playLayer.lock()) pl->resetLevelFromStart();
            },
        },
    });

    for (auto& b : btns) {
        if (b.enabled) {
            auto btnSprite = CCSprite::createWithSpriteFrameName(b.sprite);
            btnSprite->setScale(m_impl->scale * b.scale);
            btnSprite->setOpacity(m_impl->opacity);

            auto btn = CCMenuItemExt::createSpriteExtra(
                btnSprite,
                std::move(b.callback));
            btn->setID(b.id);

            m_impl->menu->addChild(btn);
        };
    };

    addChild(m_impl->menu, 1);
    m_impl->menu->updateLayout();

    m_impl->menuBg = NineSlice::create("square02_001.png");
    m_impl->menuBg->setOpacity(m_impl->opacity / 2);
    m_impl->menuBg->setAnchorPoint(m_impl->menu->getAnchorPoint());
    m_impl->menuBg->setContentSize({m_impl->menu->getScaledContentWidth() + 15.f, m_impl->menu->getScaledContentHeight() + 5.f});
    m_impl->menuBg->setPosition(m_impl->sprite->getPosition());
    m_impl->menuBg->setScaleMultiplier(0.5f);
    m_impl->menuBg->setVisible(showMenu);

    addChild(m_impl->menuBg, 0);

    return true;
};

void ActionMenu::setOpacity(GLubyte opacity) {
    m_impl->opacity = opacity;
    if (m_impl->sprite) m_impl->sprite->setOpacity(opacity / 1.25);
};

void ActionMenu::setScale(float scale) {
    m_impl->scale = scale;

    if (!m_impl->isDragging && !m_impl->isAnimating) {
        if (m_impl->sprite) {
            m_impl->sprite->setScale(scale);
            setContentSize(m_impl->sprite->getScaledContentSize());
        };

        if (m_impl->menu) m_impl->menu->setScale(scale);
        if (m_impl->menuBg) m_impl->menuBg->setScale(scale);
    };
};

void ActionMenu::setVisible(bool visible) {
    if (m_impl->menu) {
        m_impl->show = visible;

        m_impl->menu->setVisible(visible);
        if (m_impl->menuBg) m_impl->menuBg->setVisible(visible);

        (void)qga->setSavedValue("visible", visible);

        log::debug("Toggled action menu {}", m_impl->show ? "on" : "off");
    } else {
        log::error("Couldn't toggle action menu visibility");
    };
};

void ActionMenu::onScaleEnd() {
    m_impl->isAnimating = false;
};

bool ActionMenu::ccTouchBegan(CCTouch* touch, CCEvent* ev) {
    if (m_impl->sprite) {
        auto const box = m_impl->sprite->boundingBox();

        if (box.containsPoint(convertToNodeSpace(touch->getLocation()))) {
            m_impl->isDragging = true;

            m_impl->comparePos = getPosition();
            m_impl->dragStartPos = ccpSub(getPosition(), touch->getLocation());

            log::debug("Menu position starts at ({}, {})", m_impl->dragStartPos.x, m_impl->dragStartPos.y);

            m_impl->sprite->stopAllActions();
            m_impl->isAnimating = true;
            m_impl->sprite->runAction(CCSequence::createWithTwoActions(
                CCSpawn::createWithTwoActions(
                    CCEaseExponentialOut::create(CCScaleTo::create(0.25f, m_impl->scale * 0.875f)),
                    CCFadeTo::create(0.25f, 255)),
                CCCallFunc::create(this, callfunc_selector(ActionMenu::onScaleEnd))));

            return true;  // swallow touch
        };
    };

    return false;
};

void ActionMenu::ccTouchMoved(CCTouch* touch, CCEvent* ev) {
    if (m_impl->isDragging) {
        auto const touchLocation = touch->getLocation();
        auto const newLocation = ccpAdd(touchLocation, m_impl->dragStartPos);

        auto clampX = std::max(0.f, std::min(newLocation.x, m_impl->screenSize.width - getScaledContentWidth()));
        auto clampY = std::max(0.f, std::min(newLocation.y, m_impl->screenSize.height - getScaledContentHeight()));

        setPosition(ccp(clampX, clampY));
    };
};

void ActionMenu::ccTouchEnded(CCTouch* touch, CCEvent* ev) {
    if (m_impl->isDragging) {
        auto pos = ccpSub(getPosition(), touch->getLocation());
        if (m_impl->toggleOnPress && m_impl->isDistant(m_impl->comparePos, getPosition(), 5.f)) setVisible(!m_impl->show);

        m_impl->isDragging = false;

        qga->setSavedValue<CCPoint>("menu-pos", getPosition());

        if (m_impl->sprite) {
            m_impl->isAnimating = true;

            // reset scale
            m_impl->sprite->stopAllActions();
            m_impl->sprite->runAction(CCSequence::create(
                CCSpawn::createWithTwoActions(
                    CCFadeTo::create(0.125f, 255),
                    CCEaseElasticOut::create(CCScaleTo::create(0.875f, m_impl->scale))),
                CCCallFunc::create(this, callfunc_selector(ActionMenu::onScaleEnd)),
                CCDelayTime::create(1.f),
                CCFadeTo::create(0.5f, m_impl->opacity / 1.25),
                nullptr));
        };

        m_impl->dragStartPos = pos;

        log::debug("Menu position stopped and saved at ({}, {})", m_impl->dragStartPos.x, m_impl->dragStartPos.y);
    };
};

ActionMenu* ActionMenu::create(PlayLayer* pl) {
    auto ret = new ActionMenu();
    if (ret->init(pl)) {
        ret->autorelease();
        return ret;
    };

    delete ret;
    return nullptr;
};