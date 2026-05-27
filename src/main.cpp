#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/ui/GeodeUI.hpp> // Popup için KESİNLİKLE gerekli

using namespace geode::prelude;

// GUI Paneli Sınıfı
class EmirMenu : public geode::Popup<> { // Sadece geode::Popup<> kullan
protected:
    // setup fonksiyonunu doğru şekilde override ediyoruz
    bool setup() override {
        this->setTitle("Emir Mod - All In One");
        
        auto menu = CCMenu::create();
        this->m_mainLayer->addChild(menu);

        // Toggler'ları oluştururken target olarak 'this' kullanabiliyoruz
        auto noclip = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(EmirMenu::onToggle), 0.8f);
        noclip->setPosition({-50, 20});
        menu->addChild(noclip);

        auto esp = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(EmirMenu::onToggle), 0.8f);
        esp->setPosition({-50, -20});
        menu->addChild(esp);

        return true;
    }

    void onToggle(CCObject* sender) {
        Notification::create("Ayar Değiştirildi!", NotificationIcon::Success)->show();
    }

public:
    static EmirMenu* create() {
        auto ret = new EmirMenu();
        // Popup'ı init ederken geode::Popup'ın istediği parametreler:
        if (ret && ret->init(300, 200)) { 
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// Ana Menü Modifikasyonu
class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto menu = static_cast<CCMenu*>(this->getChildByID("bottom-menu"));
        
        auto btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Emir Mod"), 
            this, 
            menu_selector(MyMenuLayer::onOpenEmir)
        );
        
        btn->setID("emir-btn"_spr);
        menu->addChild(btn);
        menu->updateLayout();

        return true;
    }

    void onOpenEmir(CCObject* sender) {
        EmirMenu::create()->show();
    }
};
