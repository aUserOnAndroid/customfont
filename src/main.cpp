#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <algorithm>
#include <cstdio>

using namespace geode::prelude;
using namespace cocos2d;
using namespace cocos2d::extension;

namespace {
    bool g_ignoreDamage = false;
    bool g_practiceMode = false;

    void showToast(char const* text) {
        if (auto notification = Notification::create(text)) {
            notification->show();
        }
    }

    bool nodeContainsWorldPoint(CCNode* node, CCPoint worldPoint) {
        if (!node || !node->isVisible()) {
            return false;
        }

        auto parent = node->getParent();
        auto localPoint = parent ? parent->convertToNodeSpace(worldPoint) : worldPoint;
        return node->boundingBox().containsPoint(localPoint);
    }

    CCLabelBMFont* makeLabel(char const* text, char const* font, float scale, CCPoint position, CCNode* parent, int zOrder = 1) {
        auto label = CCLabelBMFont::create(text, font);
        label->setScale(scale);
        label->setPosition(position);
        parent->addChild(label, zOrder);
        return label;
    }
}

class ModernMenu : public CCLayer {
protected:
    CCNode* m_floatButton = nullptr;
    CCLayerColor* m_panel = nullptr;
    CCScale9Sprite* m_background = nullptr;
    CCLabelBMFont* m_statusLabel = nullptr;
    CCLabelBMFont* m_damageLabel = nullptr;
    CCLabelBMFont* m_practiceLabel = nullptr;
    bool m_open = false;
    bool m_dragging = false;
    bool m_movedTouch = false;
    CCPoint m_dragOffset = CCPointZero;
    CCPoint m_touchStart = CCPointZero;

public:
    static ModernMenu* create() {
        auto ret = new ModernMenu();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    bool init() override {
        if (!CCLayer::init()) {
            return false;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        this->setTouchEnabled(true);
        this->setKeypadEnabled(true);
        this->setID("emir-hub-layer"_spr);

        m_floatButton = ButtonSprite::create("EH", 44, true, "goldFont.fnt", "GJ_button_04.png", 28.f, 0.65f);
        m_floatButton->setPosition({ 62.f, winSize.height * 0.58f });
        m_floatButton->setID("open-emir-hub"_spr);
        this->addChild(m_floatButton, 20);

        this->createPanel(winSize);
        this->refreshStateLabels();
        this->schedule(schedule_selector(ModernMenu::tickStatus), 0.25f);
        return true;
    }

    void registerWithTouchDispatcher() override {
        CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -1, true);
    }

    bool ccTouchBegan(CCTouch* touch, CCEvent*) override {
        auto worldPoint = touch->getLocation();
        m_touchStart = worldPoint;
        m_movedTouch = false;

        if (nodeContainsWorldPoint(m_floatButton, worldPoint)) {
            m_dragging = true;
            m_dragOffset = m_floatButton->getPosition() - this->convertToNodeSpace(worldPoint);
            return true;
        }

        if (m_open && nodeContainsWorldPoint(m_panel, worldPoint)) {
            return true;
        }

        return false;
    }

    void ccTouchMoved(CCTouch* touch, CCEvent*) override {
        if (!m_dragging) {
            return;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto worldPoint = touch->getLocation();
        if (worldPoint.getDistance(m_touchStart) > 6.f) {
            m_movedTouch = true;
        }

        auto nextPosition = this->convertToNodeSpace(worldPoint) + m_dragOffset;
        nextPosition.x = std::clamp(nextPosition.x, 28.f, winSize.width - 28.f);
        nextPosition.y = std::clamp(nextPosition.y, 28.f, winSize.height - 28.f);
        m_floatButton->setPosition(nextPosition);
    }

    void ccTouchEnded(CCTouch*, CCEvent*) override {
        if (m_dragging && !m_movedTouch) {
            this->togglePanel();
        }
        m_dragging = false;
        m_movedTouch = false;
    }

    void ccTouchCancelled(CCTouch*, CCEvent*) override {
        m_dragging = false;
        m_movedTouch = false;
    }

    void keyBackClicked() override {
        if (m_open) {
            this->closePanel();
            return;
        }
        CCLayer::keyBackClicked();
    }

private:
    void createPanel(CCSize winSize) {
        m_panel = CCLayerColor::create({ 0, 0, 0, 0 }, 265.f, 185.f);
        m_panel->setPosition({ winSize.width / 2.f - 132.5f, winSize.height / 2.f - 92.5f });
        m_panel->setScale(0.86f);
        m_panel->setOpacity(0);
        m_panel->setVisible(false);
        m_panel->setID("emir-hub-panel"_spr);
        this->addChild(m_panel, 15);

        m_background = CCScale9Sprite::create("GJ_square01.png");
        m_background->setContentSize({ 265.f, 185.f });
        m_background->setPosition({ 132.5f, 92.5f });
        m_background->setColor({ 24, 29, 47 });
        m_background->setOpacity(238);
        m_panel->addChild(m_background);

        auto glow = CCLayerColor::create({ 57, 172, 255, 44 }, 255.f, 28.f);
        glow->setPosition({ 5.f, 151.f });
        m_panel->addChild(glow, 1);

        makeLabel("Emir Hub", "goldFont.fnt", 0.72f, { 132.5f, 164.f }, m_panel, 2);
        makeLabel("Geode play menu", "bigFont.fnt", 0.32f, { 132.5f, 142.f }, m_panel, 2)->setColor({ 165, 210, 255 });

        m_damageLabel = this->addActionButton("No Death", { 74.f, 110.f }, menu_selector(ModernMenu::onToggleDamage), "damage-toggle"_spr);
        m_practiceLabel = this->addActionButton("Practice", { 191.f, 110.f }, menu_selector(ModernMenu::onTogglePractice), "practice-toggle"_spr);
        this->addActionButton("Progress", { 74.f, 73.f }, menu_selector(ModernMenu::onProgressbar), "progress-toggle"_spr);
        this->addActionButton("Info", { 191.f, 73.f }, menu_selector(ModernMenu::onInfoLabel), "info-toggle"_spr);
        this->addActionButton("Restart", { 74.f, 36.f }, menu_selector(ModernMenu::onRestart), "restart-level"_spr);
        this->addActionButton("About", { 191.f, 36.f }, menu_selector(ModernMenu::onAbout), "about-menu"_spr);

        m_statusLabel = makeLabel("Ready", "chatFont.fnt", 0.55f, { 132.5f, 12.f }, m_panel, 2);
        m_statusLabel->setColor({ 170, 240, 170 });
    }

    CCLabelBMFont* addActionButton(char const* text, CCPoint position, SEL_MenuHandler callback, char const* nodeID) {
        auto menu = CCMenu::create();
        menu->setPosition(CCPointZero);
        m_panel->addChild(menu, 3);

        auto sprite = ButtonSprite::create(text, 96, true, "bigFont.fnt", "GJ_button_01.png", 25.f, 0.45f);
        auto button = CCMenuItemSpriteExtra::create(sprite, this, callback);
        button->setPosition(position);
        button->setID(nodeID);
        menu->addChild(button);

        auto label = CCLabelBMFont::create(text, "bigFont.fnt");
        label->setScale(0.32f);
        label->setPosition(position + CCPoint { 0.f, -20.f });
        label->setOpacity(215);
        m_panel->addChild(label, 2);
        return label;
    }

    void togglePanel() {
        if (m_open) {
            this->closePanel();
        } else {
            this->openPanel();
        }
    }

    void openPanel() {
        m_open = true;
        m_panel->stopAllActions();
        m_panel->setVisible(true);
        m_panel->runAction(CCEaseBackOut::create(CCScaleTo::create(0.18f, 1.f)));
        m_panel->runAction(CCFadeTo::create(0.12f, 255));
        this->refreshStateLabels();
    }

    void closePanel() {
        m_open = false;
        m_panel->stopAllActions();
        m_panel->runAction(CCSequence::create(
            CCSpawn::create(CCScaleTo::create(0.12f, 0.86f), CCFadeTo::create(0.12f, 0), nullptr),
            CCHide::create(),
            nullptr
        ));
    }

    void refreshStateLabels() {
        if (m_damageLabel) {
            m_damageLabel->setString(g_ignoreDamage ? "No Death: ON" : "No Death: OFF");
            m_damageLabel->setColor(g_ignoreDamage ? ccColor3B { 100, 255, 125 } : ccColor3B { 255, 135, 135 });
        }
        if (m_practiceLabel) {
            m_practiceLabel->setString(g_practiceMode ? "Practice: ON" : "Practice: OFF");
            m_practiceLabel->setColor(g_practiceMode ? ccColor3B { 100, 255, 125 } : ccColor3B { 255, 220, 120 });
        }
    }

    void tickStatus(float) {
        auto playLayer = PlayLayer::get();
        if (!m_statusLabel || !playLayer) {
            return;
        }

        auto percent = playLayer->getCurrentPercent();
        char buffer[96];
        std::snprintf(
            buffer,
            sizeof(buffer),
            "%.2f%%  |  %s%s",
            percent,
            g_ignoreDamage ? "NoDeath " : "",
            g_practiceMode ? "Practice" : "Normal"
        );
        m_statusLabel->setString(buffer);
    }

    void applyGameplayOptions() {
        if (auto playLayer = PlayLayer::get()) {
            playLayer->toggleIgnoreDamage(g_ignoreDamage);
            if (g_practiceMode) {
                playLayer->togglePracticeMode(true);
            }
        }
    }

    void onToggleDamage(CCObject*) {
        g_ignoreDamage = !g_ignoreDamage;
        this->applyGameplayOptions();
        this->refreshStateLabels();
        showToast(g_ignoreDamage ? "No Death enabled" : "No Death disabled");
    }

    void onTogglePractice(CCObject*) {
        g_practiceMode = !g_practiceMode;
        if (auto playLayer = PlayLayer::get()) {
            playLayer->togglePracticeMode(g_practiceMode);
        }
        this->refreshStateLabels();
        showToast(g_practiceMode ? "Practice Mode enabled" : "Practice Mode disabled");
    }

    void onProgressbar(CCObject*) {
        if (auto playLayer = PlayLayer::get()) {
            playLayer->toggleProgressbar();
            showToast("Progress bar toggled");
        }
    }

    void onInfoLabel(CCObject*) {
        if (auto playLayer = PlayLayer::get()) {
            playLayer->toggleInfoLabel();
            showToast("Info label toggled");
        }
    }

    void onRestart(CCObject*) {
        if (auto playLayer = PlayLayer::get()) {
            playLayer->resetLevelFromStart();
            this->applyGameplayOptions();
            showToast("Level restarted");
        }
    }

    void onAbout(CCObject*) {
        FLAlertLayer::create(
            "Emir Hub",
            "<cg>Emir Hub</c> is a compact in-level Geode menu.\n\n"
            "Drag the <cy>EH</c> bubble anywhere, tap it to open, and use safe PlayLayer actions from the SDK reference.",
            "OK"
        )->show();
    }
};

class $modify(EmirHubPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        if (auto menu = ModernMenu::create()) {
            this->addChild(menu, 9999);
        }

        this->toggleIgnoreDamage(g_ignoreDamage);
        if (g_practiceMode) {
            this->togglePracticeMode(true);
        }
        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        this->toggleIgnoreDamage(g_ignoreDamage);
        if (g_practiceMode) {
            this->togglePracticeMode(true);
        }
    }
};
