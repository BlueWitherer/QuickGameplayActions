#include "ActionMenu.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

// it's modding time >:3
static auto qga = Mod::get();

class ActionMenu::Impl final {
public:
    Ref<PlayLayer> m_playLayer = nullptr;

    Ref<CircleButtonSprite> m_sprite = nullptr;

    CCMenu* m_menu = nullptr;
    CCScale9Sprite* m_menuBg = nullptr;

    bool m_useRestart = qga->getSettingValue<bool>("restart");
    bool m_usePractice = qga->getSettingValue<bool>("practice");
    bool m_useExit = qga->getSettingValue<bool>("exit");

    bool m_show = qga->getSavedValue<bool>("visible", true);
    bool m_toggleOnPress = qga->getSettingValue<bool>("toggle-press");

    float m_scale = static_cast<float>(qga->getSettingValue<double>("scale"));
    int64_t m_opacity = qga->getSettingValue<int64_t>("opacity");

    CCSize const m_screenSize = CCDirector::sharedDirector()->getWinSize();
    CCPoint m_dragStartPos = { 0, 0 };

    bool m_isAnimating = false;
    bool m_isDragging = false;
    bool m_isMoving = false;
};

ActionMenu::ActionMenu() {
    m_impl = std::make_unique<Impl>();
};

ActionMenu::~ActionMenu() {};

bool ActionMenu::init(PlayLayer* pl) {
    m_impl->m_playLayer = pl;

    if (!CCLayer::init()) return false;

    // get the saved position
    auto x = qga->getSavedValue<float>("menu-x", 75.f);
    auto y = qga->getSavedValue<float>("menu-y", m_impl->m_screenSize.height - 75.f);

    setID("menu"_spr);
    setPosition({ x, y });
    setAnchorPoint({ 0.5, 0.5 });
    setTouchMode(kCCTouchesOneByOne);
    setTouchPriority(-512);
    setTouchEnabled(true);
    setZOrder(9999);

    m_impl->m_sprite = CircleButtonSprite::createWithSpriteFrameName("edit_areaModeBtn04_001.png");
    m_impl->m_sprite->setScale(m_impl->m_scale * 0.875f);
    m_impl->m_sprite->setAnchorPoint({ 0.5, 0.5 });

    setContentSize(m_impl->m_sprite->getScaledContentSize());

    m_impl->m_sprite->setPosition({ getScaledContentWidth() / 2.f, getScaledContentHeight() / 2.f });

    setScale(m_impl->m_scale); // set initial scale
    setOpacity(m_impl->m_opacity); // set initial opacity

    addChild(m_impl->m_sprite, 9);

    auto layout = RowLayout::create()
        ->setGap(2.5f)
        ->setAutoGrowAxis(0.f);

    m_impl->m_menu = CCMenu::create();
    m_impl->m_menu->setID("actions-menu");
    m_impl->m_menu->setAnchorPoint({ 0, 1 });
    m_impl->m_menu->setContentSize({ 0.f, 25.f });
    m_impl->m_menu->setPosition({ (getScaledContentWidth() / 2.f) + 10.f, getScaledContentHeight() / 2.f });
    m_impl->m_menu->setVisible(m_impl->m_show);
    m_impl->m_menu->setLayout(layout);

    std::vector<ActionItem> btns = {
        { m_impl->m_useRestart, "GJ_replayBtn_001.png", "restart-btn", menu_selector(ActionMenu::onRestart) },
        { m_impl->m_usePractice, "GJ_practiceBtn_001.png", "toggle-practice-btn", menu_selector(ActionMenu::onPractice) },
        { m_impl->m_useExit, "GJ_menuBtn_001.png", "exit-btn", menu_selector(ActionMenu::onExit) },
    };

    if (pl->m_isPlatformer) btns.push_back({ m_impl->m_useRestart, "GJ_replayFullBtn_001.png", "full-restart-btn", menu_selector(ActionMenu::onFullRestart) });

    for (auto const& b : btns) {
        if (b.enabled) {
            auto btnSprite = CCSprite::createWithSpriteFrameName(b.sprite);
            btnSprite->setScale(m_impl->m_scale);
            btnSprite->setOpacity(m_impl->m_opacity);

            auto btn = CCMenuItemSpriteExtra::create(
                btnSprite,
                this,
                b.selector
            );
            btn->setID(b.id);

            m_impl->m_menu->addChild(btn);
        };
    };

    addChild(m_impl->m_menu, 1);
    m_impl->m_menu->updateLayout();

    m_impl->m_menuBg = CCScale9Sprite::create("square02_001.png");
    m_impl->m_menuBg->setScale(0.125f);
    m_impl->m_menuBg->setOpacity(m_impl->m_opacity / 2);
    m_impl->m_menuBg->setAnchorPoint(m_impl->m_menu->getAnchorPoint());
    m_impl->m_menuBg->setContentSize({ (m_impl->m_menu->getScaledContentWidth() + 20.f) * 8.f, m_impl->m_menu->getScaledContentHeight() * 8.f });
    m_impl->m_menuBg->setPosition(m_impl->m_sprite->getPosition());
    m_impl->m_menuBg->setVisible(m_impl->m_show);

    addChild(m_impl->m_menuBg, 0);

    return true;
};

void ActionMenu::onRestart(CCObject*) {
    if (m_impl->m_playLayer) m_impl->m_playLayer->resetLevel();
};

void ActionMenu::onFullRestart(CCObject*) {
    if (m_impl->m_playLayer) m_impl->m_playLayer->resetLevelFromStart();
};

void ActionMenu::onPractice(CCObject*) {
    if (m_impl->m_playLayer) m_impl->m_playLayer->togglePracticeMode(!m_impl->m_playLayer->m_isPracticeMode);
};

void ActionMenu::onExit(CCObject*) {
    if (m_impl->m_playLayer) m_impl->m_playLayer->onQuit();
};

void ActionMenu::setOpacity(GLubyte opacity) {
    m_impl->m_opacity = opacity;
    if (m_impl->m_sprite) m_impl->m_sprite->setOpacity(opacity);
};

void ActionMenu::setScale(float scale) {
    m_impl->m_scale = scale;

    if (!m_impl->m_isDragging && !m_impl->m_isAnimating) {
        if (m_impl->m_sprite) {
            m_impl->m_sprite->setScale(scale);
            setContentSize(m_impl->m_sprite->getScaledContentSize());
        };

        if (m_impl->m_menu) m_impl->m_menu->setScale(scale);
        if (m_impl->m_menuBg) m_impl->m_menuBg->setScale(scale);
    };
};

void ActionMenu::onScaleEnd() {
    m_impl->m_isAnimating = false;
};

bool ActionMenu::ccTouchBegan(CCTouch* touch, CCEvent* ev) {
    if (m_impl->m_sprite) {
        auto const touchLocation = convertToNodeSpace(touch->getLocation());
        auto const box = m_impl->m_sprite->boundingBox();

        if (box.containsPoint(touchLocation)) {
            m_impl->m_isDragging = true;

            m_impl->m_dragStartPos = ccpSub(getPosition(), touch->getLocation());

            m_impl->m_sprite->stopAllActions();
            m_impl->m_isAnimating = true;
            m_impl->m_sprite->runAction(CCSequence::createWithTwoActions(
                CCSpawn::createWithTwoActions(
                    CCEaseExponentialOut::create(CCScaleTo::create(0.25f, m_impl->m_scale * 0.875f)),
                    CCFadeTo::create(0.25f, 255)
                ),
                CCCallFunc::create(this, callfunc_selector(ActionMenu::onScaleEnd))
            ));

            return true; // swallow touch
        };
    };

    return false;
};

void ActionMenu::ccTouchMoved(CCTouch* touch, CCEvent* ev) {
    if (m_impl->m_isDragging) {
        auto const touchLocation = touch->getLocation();
        auto const newLocation = ccpAdd(touchLocation, m_impl->m_dragStartPos);

        auto clampX = std::max(0.f, std::min(newLocation.x, m_impl->m_screenSize.width - getScaledContentWidth()));
        auto clampY = std::max(0.f, std::min(newLocation.y, m_impl->m_screenSize.height - getScaledContentHeight()));

        setPosition(ccp(clampX, clampY));

        m_impl->m_isMoving = true;
    };
};

void ActionMenu::ccTouchEnded(CCTouch* touch, CCEvent* ev) {
    if (!m_impl->m_isMoving && m_impl->m_toggleOnPress) {
        if (m_impl->m_menu) {
            m_impl->m_menu->setVisible(!m_impl->m_show);
            if (m_impl->m_menuBg) m_impl->m_menuBg->setVisible(!m_impl->m_show);

            m_impl->m_show = !qga->setSavedValue("visible", !m_impl->m_show);
        };
    };

    // reset state
    m_impl->m_isDragging = false;
    m_impl->m_isMoving = false;

    // store position
    qga->setSavedValue<float>("menu-x", getPosition().x);
    qga->setSavedValue<float>("menu-y", getPosition().y);

    m_impl->m_isAnimating = true;

    if (m_impl->m_sprite) {
        // reset scale
        m_impl->m_sprite->stopAllActions();
        m_impl->m_sprite->runAction(CCSequence::create(
            CCSpawn::createWithTwoActions(
                CCFadeTo::create(0.125f, 255),
                CCEaseElasticOut::create(CCScaleTo::create(0.875f, m_impl->m_scale))
            ),
            CCCallFunc::create(this, callfunc_selector(ActionMenu::onScaleEnd)),
            CCDelayTime::create(1.f),
            CCFadeTo::create(0.5f, m_impl->m_opacity),
            nullptr
        ));
    };
};

ActionMenu* ActionMenu::create(PlayLayer* pl) {
    auto ret = new ActionMenu();
    if (ret->init(pl)) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};