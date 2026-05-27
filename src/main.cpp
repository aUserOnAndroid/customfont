#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

// --- Custom GUI Sınıfı ---
class MyGuiPanel : public FLAlertLayer {
public:
    static MyGuiPanel* create() {
        auto ret = new MyGuiPanel();
        if (ret && ret->init(200, 150, "GJ_square01.png", "Baslik")) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool setup() override {
        auto winSize = CCDirector::get()->getWinSize();
        
        // Panel içeriği buraya gelir
        auto label = CCLabelBMFont::create("Arayüz Açıldı!", "bigFont.fnt");
        label->setScale(0.6f);
        label->setPosition(m_mainLayer->getContentSize() / 2);
        m_mainLayer->addChild(label);

        return true;
    }
};

// --- MenuLayer Hook ---
class $modify(MyMenuLayer, MenuLayer) {
    // Panel referansını tutmak için bir değişken (zayıf referans)
    Ref<MyGuiPanel> m_myPanel = nullptr;

    bool init() {
        if (!MenuLayer::init()) return false;

        auto menu = this->getChildByID("bottom-menu");

        // Özel butonumuz
        auto myButton = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png"),
            this,
            menu_selector(MyMenuLayer::onMyButton)
        );

        myButton->setID("my-gui-button"_spr);
        menu->addChild(myButton);
        menu->updateLayout();

        return true;
    }

    void onMyButton(CCObject*) {
        // Eğer panel zaten varsa ve ekrandaysa kapat
        if (m_myPanel) {
            m_myPanel->onClose(nullptr);
            m_myPanel = nullptr;
            return;
        }

        // Panel yoksa oluştur ve göster
        m_myPanel = MyGuiPanel::create();
        m_myPanel->show();
    }
};
