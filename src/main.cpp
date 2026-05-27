#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class EmirMenu : public FLAlertLayer { // geode::Popup yerine doğrudan FLAlertLayer miras al
public:
    static EmirMenu* create() {
        auto ret = new EmirMenu();
        if (ret && ret->init(nullptr, "Emir Mod", "Tamam", nullptr, 300)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init(const char* title, const char* content, const char* btn1, const char* btn2, float width) {
        if (!FLAlertLayer::init(nullptr, title, content, btn1, btn2, width)) return false;
        
        // Buraya Toggle butonlarını ekleyebilirsin
        return true;
    }
};

class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        // Butonun buraya gelecek...
        return true;
    }
};
