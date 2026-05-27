#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "EmirGui.hpp"

using namespace geode::prelude;

class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        // Ekranda her zaman görünecek "Emir" butonu
        auto btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Emir"), 
            this, 
            menu_selector(MyMenuLayer::onOpenEmir)
        );
        
        auto menu = CCMenu::create();
        menu->setPosition({50, 50}); // Ekranın sol alt köşesi
        menu->addChild(btn);
        this->addChild(menu);

        return true;
    }

    void onOpenEmir(CCObject* sender) {
        EmirGui::EmirMenu::create()->show();
    }
};
