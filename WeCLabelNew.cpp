#include "WeCLabel.h"
#include "2d/CCSprite.h"
#include "2d/CCSpriteFrameCache.h"
#include "ui/UILayoutComponent.h"
#include "base/ccUTF8.h"
#include "base/CCDirector.h"
#include "base/ccUtils.h"
#include "base/HierarchicalProfiler.h"
#include "renderer/CCGLProgram.h"
#include "renderer/CCRenderer.h"
#include "renderer/CCQuadCommand.h"
#include "renderer/CCGLProgramCache.h"
#include "2d/CCCamera.h"
#include "WeCChartFontManager.h"
#include "i18n/I18nMgr.h"
#include <string>
#include <array>
#include <set>

#include "external/HarfBuzz/hbshaper.h"

#define DEFAULT_IMG_BORDER  (4)

// #define WECLABEL_LOG_ENABLE 1

namespace cocos2d
{
namespace ui
{
    IMPLEMENT_CLASS_GUI_INFO(WeCLabel)

    // [b]
    static const std::u16string BOLD_TAG_BEGIN = { '[', 'b', ']' };
    // [/b]
    static const std::u16string BOLD_TAG_END = { '[', '/', 'b', ']' };
    // [i]
    static const std::u16string ITALIC_TAG_BEGIN = { '[', 'i', ']' };
    // [/i]
    static const std::u16string ITALIC_TAG_END = { '[', '/', 'i', ']' };
    // [u]
    static const std::u16string UNDERLINE_TAG_BEGIN = { '[', 'u', ']' };
    // [/u]
    static const std::u16string UNDERLINE_TAG_END = { '[', '/', 'u', ']' };
    // [s]
    static const std::u16string STRIKE_TAG_BEGIN = { '[', 's', ']' };
    // [/s]
    static const std::u16string STRIKE_TAG_END = { '[', '/', 's', ']' };
    // [c]
    static const std::u16string COLOR_TAG_BEGIN = { '[', 'c', ']' };
    // [/c]
    static const std::u16string COLOR_TAG_END = { '[', '/', 'c', ']' };
    // [sub]
    static const std::u16string SUB_TAG_BEGIN = { '[', 's', 'u', 'b', ']' };
    // [/sub]
    static const std::u16string SUB_TAG_END = { '[', '/', 's', 'u', 'b', ']' };
    // [sup]
    static const std::u16string SUP_TAG_BEGIN = { '[', 's', 'u', 'p', ']' };
    // [/sup]
    static const std::u16string SUP_TAG_END = { '[', '/', 's', 'u', 'p', ']' };
    //[-]
    static const std::u16string STRIP_TAG_END = { '[', '-', ']' };
    //[@]
    static const std::u16string RAW_TAG_BEGIN = { '[', '@', ']' };
    //[/@]
    static const std::u16string RAW_TAB_END = { '[', '/', '@', ']' };

    static const std::u16string STR_BREAKE_POINT = { '\t', '`' };//单词分割字符，换行时优先考虑从分隔符处断开，中日韩字符无效

    WeCLabel::WeCLabel(void)
        :mFontName("Font_i18n/NotoSansCJKsc-Regular.otf")
    {
       
    }

    WeCLabel* WeCLabel::create(void)
    {
        WeCLabel *obj = new WeCLabel();

        if (obj && obj->init())
        {
            obj->autorelease();
            return obj;
        }
        else
        {
            delete obj;
            obj = nullptr;
            return nullptr;
        }
    }

    bool WeCLabel::init()
    {
        if (!Widget::init())
            return false;

#ifdef CC_STUDIO_ENABLED_VIEW
        this->setGLProgramState(GLProgramState::getOrCreateWithGLProgramName("ShaderWeCLabel"));
#else
        this->setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_WEC_LABEL));
#endif

        mBlend.src = GL_SRC_ALPHA;
        mBlend.dst = GL_ONE_MINUS_SRC_ALPHA;

#if WECLABEL_LOG_ENABLE
        CCLOG("WeCLabel: %d inited.", mWeCLabelId);
#endif

        return true;
    }

    void WeCLabel::forceUpdateQuads()
    {
        _clearOnlyQuads();
        _mergeQuads();
    }

    Size WeCLabel::getLabelSize()
    {
        _process();

        // return getContentSize();
        return Size(mCurMaxWidth / WeCCharFontManager::getInstance()->sFontScale, getContentSize().height);
    }

    void  WeCLabel::forceProcess()
    {
        _process();
    }

    void WeCLabel::setContentSize(const Size& contentSize)
    {
        Size previousSize = ProtectedNode::getContentSize();
        if (previousSize.equals(contentSize))
        {
            return;
        }

        Widget::setContentSize(contentSize);

        mFullContentSize = contentSize;
        mFullContentSize.width *= WeCCharFontManager::getInstance()->sFontScale;
        mFullContentSize.height *= WeCCharFontManager::getInstance()->sFontScale;

        mDirty = true;
    }

    void WeCLabel::setOpacity(GLubyte opacity)
    {
        Widget::setOpacity(opacity);

        // Update the quads and images.
        mDirty = true;
    }

    void WeCLabel::updateDisplayedOpacity(GLubyte parentOpacity)
    {
        GLubyte oldDisplayColor = _displayedOpacity;
        Widget::updateDisplayedOpacity(parentOpacity);

        if (oldDisplayColor != _displayedOpacity)
            mDirty = true;
    }

    void WeCLabel::visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags)
    {
        
    }

    void WeCLabel::draw(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags)
    {
       
    }

   
}  // namespace ui
}  // namespace cocos2d
