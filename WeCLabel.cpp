#include "WeCLabel.h"
#include "2d/CCSprite.h"
#include "2d/CCSpriteFrameCache.h"
#include "base/ccUTF8.h"
#include "base/CCDirector.h"
#include "renderer/CCGLProgram.h"
#include "renderer/CCRenderer.h"
#include "renderer/CCQuadCommand.h"
#include "renderer/CCGLProgramCache.h"
#include "2d/CCCamera.h"

#include "WeCChartFontManager.h"

//----------- DX ---------
#include "DXLabelParser.h"
#include "DXLabelEscapeParser.h"
#include "DXLabelBBCodeParser.h"
#include "DXShaper.h"

#include <string>
#include <array>

#define DEFAULT_IMG_BORDER  (4)

namespace cocos2d
{
    namespace ui
    {
		DXLabelParseOper* m_parser;


        IMPLEMENT_CLASS_GUI_INFO(WeCLabel)

            unsigned int WeCLabel::sWeCLabelCount = 0;

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

        WeCLabel::WeCLabel(void)
            :mFontName("Fonts/WeCFont.ttf"),
            mFontSize(20),
            mFontColor(255, 255, 255, 255),
            mEffectColor(255, 255, 255, 255),
            mHorAlign(HorAlign::Left),
            mVerAlign(VerAlign::Top),
            mOverflow(Overflow::ResizeFreely),
            mSpacingX(0.0f),
            mSpacingY(0.0f),
            mMaxLines(0),
            mLevel(0),
            mBbcode(false),
            mEffect(Effect::None),
            mBorderSize(0),
            mShadowOffsetX(0),
            mShadowOffsetY(0),
            mText(""),
            mDirty(true),
            mScale(1.0f),
            mCurMaxWidth(0.0f),
            mWeCLabelId(0),
            mLastSpriteScale(1.0f),
            mClampWithPoints(false),
            mWillDraw(false),
            mLineCount(0),
            mStrokerOutline(0),
            mGradientTopColor(cocos2d::Color4B::WHITE),
            mGradientBottomColor(cocos2d::Color4B::WHITE),
            mGradientEnabled(false)
        {
            setAnchorPoint(Vec2::ANCHOR_MIDDLE);

#if CC_LABEL_DEBUG_DRAW
            mDebugDrawNode = DrawNode::create();
            addChild(mDebugDrawNode);
#endif

            // Register to the WeCChartFontManager.
            mWeCLabelId = WeCCharFontManager::genWeCLabelId();
            WeCCharFontManager::emplaceWeCLabel(mWeCLabelId, this);
        }

        WeCLabel::~WeCLabel(void)
        {
            // Unregister to the WeCChartFontManager.
            WeCCharFontManager::eraseWeCLabel(mWeCLabelId);

            // _removeChars();
            _clearQuads();
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
                // if (!Node::init())
                return false;

#ifdef CC_STUDIO_ENABLED_VIEW
            this->setGLProgramState(GLProgramState::getOrCreateWithGLProgramName("ShaderWeCLabel"));
#else
            this->setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_WEC_LABEL));
#endif

            mBlend.src = GL_SRC_ALPHA;
            mBlend.dst = GL_ONE_MINUS_SRC_ALPHA;

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

        void WeCLabel::setContentSize(const Size& contentSize)
        {
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
            Widget::updateDisplayedOpacity(parentOpacity);

            mDirty = true;
        }

        void WeCLabel::visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags)
        {
            // quick return if not visible. children won't be drawn.
            if (!_visible)
            {
                return;
            }

            if (mDirty)
            {
                _process();
            }

#ifdef CC_STUDIO_ENABLED_VIEW
            WeCCharFontManager::getInstance()->_updateTextures();
#endif

            // Widget::visit(renderer, parentTransform, parentFlags);
            // Node::visit(renderer, parentTransform, parentFlags);

            uint32_t flags = processParentFlags(parentTransform, parentFlags);

            bool visibleByCamera = isVisitableByVisitingCamera();
            if (_children.empty() && !visibleByCamera)
            {
                return;
            }

            // IMPORTANT:
            // To ease the migration to v3.0, we still support the Mat4 stack,
            // but it is deprecated and your code should not rely on it
            _director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
            _director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, _modelViewTransform);

            int i = 0;

            if (!_children.empty())
            {
                sortAllChildren();
                // draw children zOrder < 0
                for (; i < _children.size(); i++)
                {
                    auto node = _children.at(i);

                    if (node && node->getLocalZOrder() < 0)
                        node->visit(renderer, _modelViewTransform, flags);
                    else
                        break;
                }
                // self draw
                if (visibleByCamera)
                    this->draw(renderer, _modelViewTransform, flags);

                for (auto it = _children.cbegin() + i; it != _children.cend(); ++it)
                    (*it)->visit(renderer, _modelViewTransform, flags);
            }
            else if (visibleByCamera)
            {
                this->draw(renderer, _modelViewTransform, flags);
            }

            _director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
        }

        void WeCLabel::draw(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags)
        {
            auto mvMat = parentTransform;
            float scale = WeCCharFontManager::getInstance()->sFontScale;

            auto mask = this->getCameraMask();
            if (mask & static_cast<unsigned short>(CameraFlag::DEFAULT))
            {
                // Ceil the transform matrix, to avoid sample problems.
                mvMat.m[12] = std::ceil(mvMat.m[12] * scale);
                mvMat.m[13] = std::ceil(mvMat.m[13] * scale);
                mvMat.m[14] = std::ceil(mvMat.m[14] * scale);
            }
            else
            {
                mvMat.m[0] = mvMat.m[0] / scale;
                mvMat.m[5] = mvMat.m[5] / scale;
                mvMat.m[10] = mvMat.m[10] / scale;
            }

            int i = 0;
            for (auto &iter : mQuads)
            {
                mQuadCmd[i].init(this->getGlobalZOrder(), iter.first,
                    this->getGLProgramState(), mBlend, iter.second.data(),
                    iter.second.size(), mvMat, parentFlags);

                mQuadCmd[i].setSkipBatching(false);

                renderer->addCommand(&mQuadCmd[i]);
                ++i;
            }

            i = 0;
            for (auto &iter : mImgQuads)
            {
                // Fix the bug of ETC.
                auto alphaFound = mQuadAlphas.find(iter.first);

                auto program = alphaFound == mQuadAlphas.end() ?
                    GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_WEC_LABEL_IMG) :
                    GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_WEC_LABEL_IMG_ETC);

                mImgQuadCmd[i].init(this->getGlobalZOrder(), iter.first, program, mBlend, iter.second.data(),
                    iter.second.size(), mvMat, parentFlags);

                mImgQuadCmd[i].setSkipBatching(false);
                // mImgQuadCmd[i].setTextureID2(alphaFound == mQuadAlphas.end() ? 0 : alphaFound->second);

                renderer->addCommand(&mImgQuadCmd[i]);
                ++i;
            }
        }

        cocos2d::Sprite * WeCLabel::getLetter(int index)
        {
            if (index < 0 || index >= mOrders.size())
                return nullptr;

            return mOrders.at(index);
        }

        cocos2d::Vec2 WeCLabel::getLetterPosition(int index)
        {
            if ((size_t)index >= mPositions.size())
                return Vec2::ZERO;

            return mPositions[index] / WeCCharFontManager::getInstance()->sFontScale;
        }

        float WeCLabel::getLineHeight()
        {
            auto inst = WeCCharFontManager::getInstance();
            std::string key = inst->generateKey(mFontName, mFontSize, mStrokerOutline);
            auto freeType = inst->getFontFreeType(key);
            if (nullptr != freeType)
            {
                return ((freeType->getPrintedLineHeight() + mSpacingY) * mScale) / WeCCharFontManager::getInstance()->sFontScale;
            }
            return 0.0f;
        }

        void WeCLabel::_process()
        {
            if (!mDirty)
                return;

            ///////////////////////////////////////////////////////////////////
            // Process text.

            _clearQuads();

            // Collect the fonts by the WeCFontManager.
            auto inst = cocos2d::WeCCharFontManager::getInstance();

            // Remove the symbols, than generate the characters.

            if (!StringUtils::UTF8ToUTF16(mText, mReuseU16Text))
                mReuseU16Text.clear(); // Convert error, clear the label.

            if (mReuseU16Text.empty())
            {
                mCurMaxWidth = 0;

                switch (mOverflow)
                {
                case cocos2d::ui::WeCLabel::Overflow::ResizeFreely:
                    setContentSize(Size::ZERO);
                    break;
                case cocos2d::ui::WeCLabel::Overflow::ResizeHeight:
                    setContentSize(Size(this->getContentSize().width, 0));
                    break;
                default:
                    break;
                }

                mDirty = false;
                return;
            }

			// �Ƚ��н���
            inst->collectCharacters(mReuseU16Text, mFontName, mFontSize, mStrokerOutline);

            if (mBbcode)
                mReuseU16Text = _stripSymbols(mReuseU16Text);

            ///////////////////////////////////////////////////////////////////
            // Update quads.

            // Process each character.
            _updateQuads();
            mDirty = false;
        }

        Sprite* WeCLabel::_createImageSprite(const std::string &imgName, const Vec2 &Pos, float scale, float customWidth)
        {
            if (imgName.empty())
                return nullptr;

            Sprite* sprite = Sprite::createWithSpriteFrameName(imgName);
            // float tmpScale = customWidth == -1 ? 1.0f : customWidth / sprite->getOriginalSize().width;
            float tmpScale = customWidth == -1 ? 1.0f : customWidth / sprite->getContentSize().width;
            tmpScale *= scale;
            sprite->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
            sprite->setPosition(Pos);
            sprite->setScaleX(tmpScale);
            sprite->setScaleY(tmpScale);
            sprite->retain();

            if (this->isRunning())
                sprite->onEnter();

            return sprite;
        }

        void WeCLabel::_updateQuadsInternal()
        {
            Size contentSize = mFullContentSize; // Use Full ContentSize to calculate the position.

            auto inst = WeCCharFontManager::getInstance();
            std::string key = inst->generateKey(mFontName, mFontSize, mStrokerOutline);
            for (auto &iter : mLetterInfos)
            {
                // If the imageName is not null, create the image and returned.
                if (!iter.imageName.empty())
                {
                    Sprite* imgSprite = _createImageSprite(iter.imageName, iter.pos, mLastSpriteScale, iter.customWidth);
                    mImages.pushBack(imgSprite);
                    mOrders.pushBack(imgSprite);
                    continue;
                }

                auto definition = inst->getCharFontDefinition(key, iter.code);

                if (nullptr == definition)
                    continue;

                // If the character is outsize the content, skip.
                if (iter.pos.x > contentSize.width || iter.pos.x + definition->printWidth < 0 ||
                    iter.pos.y - definition->printHeight > contentSize.height || iter.pos.y < 0)
                {
                    continue;
                }

                // If the character is overlap the content, clamp the character.
                // offsetxScale, widthScale, offsetyScale, heightScale.
                float tmpXOffset = 0;
                float tmpWidth = definition->printWidth;
                float tmpYOffset = 0;
                float tmpHeight = definition->printHeight;
                Vec2 tmpPos = iter.pos;

                if (mOverflow == Overflow::Clamp)
                {
                    // Process x offset.
                    if (iter.pos.x < 0)
                    {
                        tmpXOffset = -iter.pos.x;
                        tmpWidth = definition->printWidth + iter.pos.x;
                        tmpPos.x += -iter.pos.x;
                    }
                    else if (iter.pos.x + definition->printWidth > contentSize.width)
                    {
                        tmpWidth = (contentSize.width - iter.pos.x);
                    }

                    // Process y offset.
                    if (iter.pos.y > contentSize.height)
                    {
                        tmpHeight = definition->printHeight - iter.pos.y - contentSize.height;
                        tmpYOffset = (iter.pos.y - contentSize.height);
                        tmpPos.y -= (iter.pos.y - contentSize.height);
                    }
                    else if (iter.pos.y - definition->printHeight < 0)
                    {
                        tmpHeight = iter.pos.y;
                    }
                }

                // Generate the rectangle of the texture and vertex.
                mReusedTexRect.size.width = tmpWidth;
                mReusedTexRect.size.height = tmpHeight;
                mReusedTexRect.origin.x = definition->U + tmpXOffset;
                mReusedTexRect.origin.y = definition->V + tmpYOffset;

                mReusedRect.size.width = tmpWidth;
                mReusedRect.size.height = tmpHeight;
                mReusedRect.origin.x = definition->U + tmpXOffset;
                mReusedRect.origin.y = definition->V + tmpYOffset;

                // Generate sprites to show the characters.
                auto &textures = inst->getTextures();

                float scaleX = mLastSpriteScale;
                scaleX = iter.customWidth > 0 ? scaleX * (static_cast<float>(iter.customWidth) / tmpWidth) : scaleX;
                float scaleY = mLastSpriteScale;

                if (mEffect == Effect::Shadow)
                {
                    Color4B tmpClr(mEffectColor.r, mEffectColor.g, mEffectColor.b,
                        static_cast<GLubyte>((static_cast<float>(mEffectColor.a) / 255.0f) * this->getDisplayedOpacity()));

                    auto shadowsprite = _createFontSprite(textures[definition->texIndex],
                        Vec2(tmpPos.x + mShadowOffsetX, tmpPos.y + mShadowOffsetY), tmpClr, scaleX, scaleY, iter.italic);

                    mShadows.pushBack(shadowsprite);
                    //mOrders.pushBack(shadowsprite);
                }
                else if (mEffect == Effect::Outline)
                {
                    Color4B tmpClr(mEffectColor.r, mEffectColor.g, mEffectColor.b,
                        static_cast<GLubyte>((static_cast<float>(mEffectColor.a) / 255.0f) * this->getDisplayedOpacity()));

                    std::array<Vec2, 4> pos =
                    {
                        Vec2(tmpPos.x + mBorderSize, tmpPos.y + mBorderSize),
                        Vec2(tmpPos.x - mBorderSize, tmpPos.y - mBorderSize),
                        Vec2(tmpPos.x + mBorderSize, tmpPos.y - mBorderSize),
                        Vec2(tmpPos.x - mBorderSize, tmpPos.y + mBorderSize)
                    };

                    for (unsigned int i = 0; i < pos.size(); ++i)
                    {
                        auto border = _createFontSprite(textures[definition->texIndex], pos[i], tmpClr, scaleX, scaleY, iter.italic);
                        mShadows.pushBack(border);
                        //mOrders.pushBack(border);
                    }
                }

                Color4B tmpFontClr = iter.ignoreColor ? Color4B(iter.color.r, iter.color.g, iter.color.b,
                    static_cast<GLubyte>((static_cast<float>(iter.color.a) / 255.0f) * this->getDisplayedOpacity())) :
                    Color4B(mFontColor.r, mFontColor.g, mFontColor.b,
                    static_cast<GLubyte>((static_cast<float>(mFontColor.a) / 255.0f) * this->getDisplayedOpacity()));

                auto sprite = _createFontSprite(textures[definition->texIndex], tmpPos, tmpFontClr, scaleX, scaleY, iter.italic);
                mCharacters.pushBack(sprite);
                mOrders.pushBack(sprite);
            }

            // Merge all the sprites.
            _mergeQuads();

#if CC_LABEL_DEBUG_DRAW
            mDebugDrawNode->clear();
            Vec2 vertices[4] =
            {
                Vec2::ZERO,
                Vec2(_contentSize.width, 0),
                Vec2(_contentSize.width, _contentSize.height),
                Vec2(0, _contentSize.height)
            };
            mDebugDrawNode->drawPoly(vertices, 4, true, Color4F::WHITE);
#endif
        }

        void WeCLabel::_updateQuads()
        {
            // Parse a information list for generating the quads.
            // LetterInfoList letterInfos;
            mLetterInfos.clear();

            Size contentSize = Size::ZERO;
            mLastSpriteScale = 1.0f;

            _parseLetterInfo(mLetterInfos, contentSize, mLastSpriteScale);

            // mFullContentSize = contentSize;

            contentSize.width = std::ceil(contentSize.width / WeCCharFontManager::getInstance()->sFontScale);
            contentSize.height = std::ceil(contentSize.height / WeCCharFontManager::getInstance()->sFontScale);

            // Update content size.
            setContentSize(contentSize); // TODO: should scale.

            _updateQuadsInternal();
        }

        void WeCLabel::_clearQuads()
        {
            mOrders.clear();
            // TODO(eranzhao): object pool.
            for (auto &iter : mCharacters)
            {
                // this->removeProtectedChild(iter);
                iter->onExit();
                iter->release();
            }

            mCharacters.clear();

            for (auto &iter : mShadows)
            {
                // this->removeProtectedChild(iter);
                iter->onExit();
                iter->release();
            }

            mShadows.clear();

            for (auto &iter : mImages)
            {
                // this->removeProtectedChild(iter);
                iter->onExit();
                iter->release();
            }

            mImages.clear();

            mQuads.clear();
            mImgQuads.clear();
            mQuadAlphas.clear();
        }

        void WeCLabel::_mergeQuads()
        {
            // Merge the quads when the label changed.
            for (auto &ch : mShadows)
            {
                auto texId = ch->getTexture()->getName();
                auto size = ch->getTextureRect().size;

                if (texId == 0)
                    continue;

                auto found = mQuads.find(texId);
                if (found == mQuads.end())
                {
                    mQuads.emplace(texId, QuadList());
                    found = mQuads.find(texId);
                }

                if (found != mQuads.end())
                {
                    auto &mat = ch->getNodeToParentTransform();
                    auto q = ch->getRotationQuat();
                    Mat4 rot;
                    Mat4::createRotation(q, &rot);
                    Mat4 curMat = rot * mat;
                    auto quad = ch->getQuad();
                    curMat.transformPoint(&(quad.bl.vertices));
                    curMat.transformPoint(&(quad.br.vertices));
                    curMat.transformPoint(&(quad.tl.vertices));
                    curMat.transformPoint(&(quad.tr.vertices));
                    found->second.emplace_back(quad);
                }
            }

            for (auto &ch : mCharacters)
            {
                auto texId = ch->getTexture()->getName();
                auto size = ch->getTextureRect().size;

                if (texId == 0)
                    continue;

                auto found = mQuads.find(texId);
                if (found == mQuads.end())
                {
                    mQuads.emplace(texId, QuadList());
                    found = mQuads.find(texId);
                }

                if (found != mQuads.end())
                {
                    auto &mat = ch->getNodeToParentTransform();
                    auto q = ch->getRotationQuat();
                    Mat4 rot;
                    Mat4::createRotation(q, &rot);
                    Mat4 curMat = rot * mat;
                    auto quad = ch->getQuad();
                    curMat.transformPoint(&(quad.bl.vertices));
                    curMat.transformPoint(&(quad.br.vertices));
                    curMat.transformPoint(&(quad.tl.vertices));
                    curMat.transformPoint(&(quad.tr.vertices));

                    // If gradient enabled, should update the color.
                    if (mGradientEnabled)
                    {
                        Color4F gradientTopClr(mGradientTopColor);
                        Color4F gradientBottomClr(mGradientBottomColor);

                        // there's only one color before.
                        Color4F fontClr(quad.bl.colors);

                        gradientTopClr.r *= fontClr.r;
                        gradientTopClr.g *= fontClr.g;
                        gradientTopClr.b *= fontClr.b;
                        gradientTopClr.a *= fontClr.a;

                        gradientBottomClr.r *= fontClr.r;
                        gradientBottomClr.g *= fontClr.g;
                        gradientBottomClr.b *= fontClr.b;
                        gradientBottomClr.a *= fontClr.a;

                        quad.tl.colors = Color4B(gradientTopClr);
                        quad.tr.colors = Color4B(gradientTopClr);
                        quad.bl.colors = Color4B(gradientBottomClr);
                        quad.br.colors = Color4B(gradientBottomClr);
                    }

                    found->second.emplace_back(quad);
                }
            }

            for (auto &ch : mImages)
            {
                auto texId = ch->getTexture()->getName();
                // auto tex2Id = ch->getTexture()->getAlphaName();
                auto size = ch->getTextureRect().size;

                if (texId == 0)
                    continue;

                auto found = mImgQuads.find(texId);
                if (found == mImgQuads.end())
                {
                    mImgQuads.emplace(texId, QuadList());
                    found = mImgQuads.find(texId);
                }

                /*
                if (tex2Id != 0)
                {
                    auto alphaFound = mQuadAlphas.find(texId);
                    if (alphaFound == mQuadAlphas.end())
                        mQuadAlphas.emplace(texId, tex2Id);
                }
                */

                if (found != mImgQuads.end())
                {
                    auto &mat = ch->getNodeToParentTransform();
                    auto q = ch->getRotationQuat();
                    Mat4 rot;
                    Mat4::createRotation(q, &rot);
                    Mat4 curMat = rot * mat;
                    auto quad = ch->getQuad();
                    curMat.transformPoint(&(quad.bl.vertices));
                    curMat.transformPoint(&(quad.br.vertices));
                    curMat.transformPoint(&(quad.tl.vertices));
                    curMat.transformPoint(&(quad.tr.vertices));
                    found->second.emplace_back(quad);
                }
            }

            mQuadCmd.resize(mQuads.size());
            mImgQuadCmd.resize(mImgQuads.size());
        }

        void WeCLabel::_clearOnlyQuads()
        {
            mQuads.clear();
            mImgQuads.clear();
            mQuadAlphas.clear();
        }

        Sprite* WeCLabel::_createFontSprite(Texture2D *tex, const Vec2 &Pos, const Color4B color, float scaleX, float scaleY, bool italic)
        {
            auto sprite = Sprite::createWithTexture(tex);
#ifdef CC_STUDIO_ENABLED_VIEW
            sprite->setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(sShaderName));
#else
            sprite->setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_WEC_LABEL));
#endif
            sprite->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
#ifdef CC_STUDIO_ENABLED_VIEW
            sprite->setTextureRect(mReusedRect, false, mReusedRect.size);
#else
            sprite->setTextureRect(mReusedRect, mReusedTexRect, false, mReusedRect.size);
#endif
            sprite->setPosition(Pos);
            sprite->setScaleX(scaleX);
            sprite->setScaleY(scaleY);
            sprite->setRotationSkewX(italic ? 15 : 0);
            sprite->setColor(Color3B(color.r, color.g, color.b));
            sprite->setOpacity(color.a);
            sprite->retain();

            if (this->isRunning())
                sprite->onEnter();

            // this->addProtectedChild(sprite);

            return sprite;
        }

        void WeCLabel::onEnter()
        {
            // Widget::onEnter();

            for (auto &iter : mCharacters)
                iter->onEnter();

            for (auto &iter : mShadows)
                iter->onEnter();

            for (auto &iter : mImages)
                iter->onEnter();

            // If the Widget is create by the code, _usingLayoutComponent is always false
            // And it will effect the mDirty flag.
            if (!_usingLayoutComponent && mDirty)
                updateSizeAndPosition();

            sWeCLabelCount++;

            // Becasue the wec label is dirty when onExit,
            // the font will re-generated when visit the label.

            mWillDraw = true;

            ProtectedNode::onEnter();
        }

        void WeCLabel::onExit()
        {
            for (auto &iter : mCharacters)
                iter->onExit();

            for (auto &iter : mShadows)
                iter->onExit();

            for (auto &iter : mImages)
                iter->onExit();

            sWeCLabelCount--;

            mWillDraw = false;

            Widget::onExit();
        }

        void WeCLabel::_parseLetterInfo(LetterInfoList &infos, Size &contentSize, float &scale)
        {
            // Initialize the value.
            scale = 1.0f;

            auto inst = WeCCharFontManager::getInstance();
            std::string key = inst->generateKey(mFontName, mFontSize, mStrokerOutline);

            float fontScale = inst->sFontScale;

            // If there is system font, use the font size as line height.
            auto freeType = inst->getFontFreeType(key);
            int lineHeight = freeType != nullptr ? freeType->getPrintedLineHeight() : mFontSize * fontScale;

            // Get the characters, get the max line height.
            for (auto &iter : mReuseU16Text)
            {
                auto tmpDef = WeCCharFontManager::getInstance()->getCharFontDefinition(key, iter);
                if (tmpDef != nullptr)
                    lineHeight = tmpDef->printHeight > lineHeight ? tmpDef->printHeight : lineHeight;
            }

            infos.reserve(mReuseU16Text.size()); // Optimize the performance.
            mPositions.clear();
            // Convert the text to the u16string.

            if (!StringUtils::UTF8ToUTF16(mText, mReuseU16Text))
                mReuseU16Text.clear(); // Convert error, clear the label.

            int codeCount = static_cast<int>(mReuseU16Text.size());
            int lineIndex = 0;
            float nextX = 0;
            float nextY = 0;
            float maxLineWidth = mFullContentSize.width;
            float lineHeightOffset = (lineHeight + mSpacingY * fontScale) * mScale; // The spacing is in design resolution.
            float spacingX = mSpacingX * fontScale * mScale; // The spacing is in design resolution.

            // Style.
            int sub = 0;
            bool bold = false;
            bool italic = false;
            bool underline = false;
            bool strike = false;
            bool ignoreColor = false;
            bool raw = false;
            std::u16string imageName;

            float underlineLenth = 0;
            float underlineStartXOffset = 0;
            bool lastUnderline = false;

            std::vector<float> lineOffset;
            std::vector<Color4B> colors;
            for (int i = 0; i < codeCount;)
            {
                // Does the character is a line break?
                {
                    auto ch = mReuseU16Text[i];
                    if (ch == '\n')
                    {
                        // TODO(eranzhao): If in underline or strike, end the underline
                        // and start a new one.
                        // Color4B tmpClr = infos.size() > 0 ? infos[infos.size() - 1].color : Color4B::WHITE;
                        //bool tmpIgnoreColor = infos.size() > 0 ? infos[infos.size() - 1].ignoreColor : false;
                        if (mBbcode && underline == true)
                        {
                            _insertSpecialCode('_', mFontName, mFontSize, mStrokerOutline, mScale, lineIndex, sub, bold, italic,
                                false, Color4B::WHITE, underlineStartXOffset, nextY, nextX - underlineStartXOffset, infos);
                        }

                        underlineLenth = 0;
                        underlineStartXOffset = 0;

                        lineOffset.emplace_back(nextX - spacingX); // Record the width of the line when make a new line.
                        nextX = 0;
                        nextY -= lineHeightOffset;
                        i++; // Go to next character.
                        lineIndex++; // has make a new line.

                        if (mMaxLines > 0 && lineIndex >= mMaxLines)
                        {
                            lineIndex -= 1;
                            break;
                        }

                        continue;
                    }
                }

                // Parse the bbcode if enabled.
        {
            if (mBbcode && _parseSymbol(mReuseU16Text, i, colors, sub,
                bold, italic, underline, strike, ignoreColor, imageName, raw))
            {
                // If underline changed. collect the length of the underline.
                // Than create a sprite with custom length;
                if (lastUnderline != underline)
                {
                    if (underline == true)
                    {
                        underlineLenth = 0;
                        underlineStartXOffset = nextX;
                    }
                    else
                    {
                        //Color4B tmpClr = infos.size() > 0 ? infos[infos.size() - 1].color : Color4B::WHITE;
                        //bool tmpIgnoreColor = infos.size() > 0 ? infos[infos.size() - 1].ignoreColor : false;
                        _insertSpecialCode('_', mFontName, mFontSize, mStrokerOutline, mScale, lineIndex, sub, bold, italic,
                            false, Color4B::WHITE, underlineStartXOffset, nextY, nextX - underlineStartXOffset, infos);
                    }

                    lastUnderline = underline;
                }

                if (!imageName.empty())
                {
                    // Insert a image here.
                    // Try to find the image in the cache.
                    // The image height should less equal to the line height.
                    // then calculate the width of the image, break the line
                    // if needed.
                    // If the image can not found, skip.
                    std::string utf8ImgName;

                    bool breakFor = false;
                    do
                    {
                        if (!StringUtils::UTF16ToUTF8(imageName, utf8ImgName))
                            break;

                        utf8ImgName += ".png";

                        auto img = cocos2d::SpriteFrameCache::getInstance()->getSpriteFrameByName(utf8ImgName);

                        if (nullptr == img)
                            break;

                        auto imgSize = img->getOriginalSize();
                        float imgScale = (lineHeight * mScale) / imgSize.height;
                        float imgWidth = imgSize.width * imgScale;

                        if (mOverflow == Overflow::ResizeHeight &&  nextX + imgWidth + DEFAULT_IMG_BORDER > maxLineWidth &&
                            imgWidth <= maxLineWidth) // if the image is too long should not make a new line.
                        {
                            if (mBbcode && underline == true)
                            {
                                _insertSpecialCode('_', mFontName, mFontSize, mStrokerOutline, mScale, lineIndex, sub, bold, italic,
                                    false, Color4B::WHITE, underlineStartXOffset, nextY, nextX - underlineStartXOffset, infos);
                            }

                            underlineLenth = 0;
                            underlineStartXOffset = 0;

                            // make a new line.
                            lineOffset.emplace_back(nextX - spacingX); // Record the width of the line when make a new line.
                            nextX = 0;
                            nextY -= lineHeightOffset;
                            lineIndex++;

                            if (mMaxLines > 0 && lineIndex >= mMaxLines)
                            {
                                lineIndex -= 1;
                                breakFor = true;
                                break;
                            }
                        }

                        // Insert the image.
                        float xPos = nextX + DEFAULT_IMG_BORDER;
                        float yPos = nextY;


                        // Because the image use the different matrix, should
                        // process the size and the position.
                        LetterInfo tmpImgInfo
                        {
                            Vec2(xPos, yPos),
                            Color4B::WHITE,
                            utf8ImgName,
                            0,
                            lineIndex,
                            imgWidth,
                            false,
                            false,
                            false,
                            false
                        };
                        infos.emplace_back(tmpImgInfo);
                        nextX += imgWidth + spacingX + DEFAULT_IMG_BORDER;

                    } while (0);

                    // Break for.
                    if (breakFor)
                        break;

                    imageName.clear();
                }

                continue;
            }
        }

        bool breakLineInWord = false;

        // Get a word
        {
            int wordLen = _parseFirstWord(mReuseU16Text, i);

            // for overflow type ResizeHeight, check the line can hold the word or not.
            if (mOverflow == Overflow::ResizeHeight)
            {
                float tmpLen = 0;
                for (int j = i; j < i + wordLen; ++j)
                {
                    auto ch = mReuseU16Text[j];

                    // Ignore the '\r', only use '\n' instead.
                    if (ch == '\r')
                        continue;

                    // Get the character information from the WeCChartFontManager.
                    auto definition = WeCCharFontManager::getInstance()->getCharFontDefinition(key, ch);

                    // Could not found the character, just return.
                    if (nullptr == definition)
                        continue;

                    // Get the offset x of the character.
                    auto xadvance = definition->printXAdvance * mScale;
                    tmpLen += xadvance + spacingX;
                }

                if (tmpLen + nextX > maxLineWidth && tmpLen <= maxLineWidth)
                {
                    // TODO(eranzhao): If in underline or strike, end the underline
                    // and start a new one.
                    // Color4B tmpClr = infos.size() > 0 ? infos[infos.size() - 1].color : Color4B::WHITE;
                    // bool tmpIgnoreColor = infos.size() > 0 ? infos[infos.size() - 1].ignoreColor : false;

                    if (mBbcode && underline == true)
                    {
                        _insertSpecialCode('_', mFontName, mFontSize, mStrokerOutline, mScale, lineIndex, sub, bold, italic,
                            false, Color4B::WHITE, underlineStartXOffset, nextY, nextX - underlineStartXOffset, infos);
                    }

                    underlineLenth = 0;
                    underlineStartXOffset = 0;

                    // make a new line.
                    lineOffset.emplace_back(nextX - spacingX); // Record the width of the line when make a new line.
                    nextX = 0;
                    nextY -= lineHeightOffset;
                    lineIndex++;

                    if (mMaxLines > 0 && lineIndex >= mMaxLines)
                    {
                        lineIndex -= 1;
                        break;
                    }
                }
                else if (tmpLen + nextX > maxLineWidth && tmpLen > maxLineWidth)
                {
                    // The word is too long, should break the line when process the word.
                    breakLineInWord = true;
                }
            }

            bool canPutPoints = true;

            // put the word to the line.
            for (int j = i; j < i + wordLen; ++j)
            {
                auto ch = mReuseU16Text[j];

                // Ignore the '\r', only use '\n' instead.
                if (ch == '\r')
                    continue;

                // Get the character information from the WeCChartFontManager.
                auto definition = WeCCharFontManager::getInstance()->getCharFontDefinition(key, ch);
                WeCChartFontDefinitionPtr nextDefinition = nullptr;
                if ((size_t)j < mReuseU16Text.size() - 1)
                {
                    nextDefinition = WeCCharFontManager::getInstance()->getCharFontDefinition(key, mReuseU16Text[j + 1]);
                }
                // Could not found the character, just return.
                if (nullptr == definition)
                    continue;

                // If allowed break line in the word, check the word is too long or not.
                if (breakLineInWord && nextX + definition->printXAdvance + spacingX > maxLineWidth)
                {
                    if (mBbcode && underline == true)
                    {
                        _insertSpecialCode('_', mFontName, mFontSize, mStrokerOutline, mScale, lineIndex, sub, bold, italic,
                            false, Color4B::WHITE, underlineStartXOffset, nextY, nextX - underlineStartXOffset, infos);
                    }

                    underlineLenth = 0;
                    underlineStartXOffset = 0;

                    // make a new line.
                    lineOffset.emplace_back(nextX - spacingX); // Record the width of the line when make a new line.
                    nextX = 0;
                    nextY -= lineHeightOffset;
                    lineIndex++;

                    if (mMaxLines > 0 && lineIndex >= mMaxLines)
                    {
                        lineIndex -= 1;
                        break;
                    }
                }

                // float xOffset = definition->offsetX * mScale + spacingX;
                float xOffset = definition->printOffsetX * mScale;
                float xPos = xOffset + nextX;
                float lineOffset = (lineHeight - definition->printlineHeight) * mScale;
                float yOffset = definition->printOffsetY * mScale;
                float yPos = nextY - yOffset - lineOffset;

                {
					int tmpClrCount = colors.size();	
                    Color4B tmpClr = tmpClrCount > 0 ? colors[tmpClrCount - 1] : Color4B::WHITE;
                    int pointsWidth = 4 * (_getSpecialCodeXAdvance('.', mFontName, mFontSize, mStrokerOutline) * mScale + spacingX);
                    if (mClampWithPoints && nextDefinition != nullptr)
                    {
                        if (mOverflow == Overflow::Clamp)
                        {
                            int nextPutWith = pointsWidth;
                            if (nextDefinition->printXAdvance * mScale + spacingX > pointsWidth)
                            {
                                nextPutWith = nextDefinition->printXAdvance * mScale + spacingX;
                            }
                            if (xPos + nextPutWith > maxLineWidth)
                            {
                                canPutPoints = false;
                            }
                        }
                        else if (mOverflow == Overflow::ResizeHeight && mMaxLines > 0)
                        {
                            if (lineIndex == mMaxLines - 1)
                            {
                                int nextPutWith = pointsWidth;
                                if (nextDefinition->printXAdvance * mScale + spacingX > pointsWidth)
                                {
                                    nextPutWith = nextDefinition->printXAdvance * mScale + spacingX;
                                }
                                if (xPos + nextPutWith > maxLineWidth)
                                {
                                    canPutPoints = false;
                                }
                            }
                        }
                    }

                    if (mClampWithPoints && !canPutPoints)
                    {
                        int pointRealWidth = _getSpecialCodeXAdvance('.', mFontName, mFontSize, mStrokerOutline) * 3;
                        _insertSpecialCode('.', mFontName, mFontSize, mStrokerOutline, mScale, lineIndex, sub, bold, italic,
                            false, tmpClr, maxLineWidth - pointRealWidth, nextY, pointRealWidth, infos);
                        break;
                    }
                    else
                    {
                        LetterInfo info =
                        {
                            Vec2(xPos, yPos),
                            tmpClr,
                            "",
                            ch,
                            lineIndex,
                            -1, // Use the size in definition.
                            sub,
                            bold,
                            italic,
                            ignoreColor
                        };
                        infos.emplace_back(info);
                        mPositions.push_back(Vec2(nextX + definition->printXAdvance * mScale + spacingX, nextY));
                    }
                }
                nextX += definition->printXAdvance * mScale + spacingX;
            }
            if (mClampWithPoints && !canPutPoints)
            {
                break;
            }
            i += wordLen;
        }
            }

            // Record the line last time.
            lineOffset.emplace_back(nextX - spacingX);

            mCurMaxWidth = 0.0f;
            for (unsigned int i = 0; i < lineOffset.size(); ++i)
            {
                mCurMaxWidth = lineOffset[i] > mCurMaxWidth ? lineOffset[i] : mCurMaxWidth;
            }

            switch (mOverflow)
            {
            case cocos2d::ui::WeCLabel::Overflow::Shrink:
            case cocos2d::ui::WeCLabel::Overflow::Clamp:
            {
                // Keep the content size and not changed.
                contentSize = mFullContentSize;
            }
            break;
            case cocos2d::ui::WeCLabel::Overflow::ResizeFreely:
            {
                float tmpMax = 0;
                for (auto &iter : lineOffset)
                {
                    tmpMax = tmpMax < iter ? iter : tmpMax;
                }

                contentSize.width = tmpMax;

                int tmpLineCount = lineIndex + 1;
                if (mMaxLines > 0)
                    tmpLineCount = tmpLineCount > mMaxLines ? mMaxLines : tmpLineCount;

                contentSize.height = tmpLineCount * lineHeightOffset - mSpacingY * mScale;
            }
            break;
            case cocos2d::ui::WeCLabel::Overflow::ResizeHeight:
            {
                contentSize.width = maxLineWidth;

                int tmpLineCount = lineIndex + 1;
                if (mMaxLines > 0)
                    tmpLineCount = tmpLineCount > mMaxLines ? mMaxLines : tmpLineCount;

                contentSize.height = tmpLineCount * lineHeightOffset - mSpacingY * mScale;
            }
            break;
            default:
                break;
            }

            // Calculate the scaling.
            if (mOverflow == Overflow::Shrink)
            {
                scale = contentSize.width / mCurMaxWidth;
                scale = scale > 1.0f ? 1.0f : scale;
            }

            // Process line by verAlign.
            float yOffset = 0;
            float tmpTextHeight = (lineIndex + 1) * lineHeightOffset - mSpacingY * mScale;
            switch (mVerAlign)
            {
            case cocos2d::ui::WeCLabel::VerAlign::Top:
                yOffset = contentSize.height;
                break;
            case cocos2d::ui::WeCLabel::VerAlign::Center:
                yOffset = (tmpTextHeight * scale + contentSize.height) * 0.5f;
                break;
            case cocos2d::ui::WeCLabel::VerAlign::Bottom:
                yOffset = tmpTextHeight * scale;
                break;
            default:
                break;
            }

            // Process the line by the HorAlign.
            for (unsigned int i = 0; i < lineOffset.size(); ++i)
            {
                switch (mHorAlign)
                {
                case cocos2d::ui::WeCLabel::HorAlign::Left:
                    lineOffset[i] = 0;
                    break;
                case cocos2d::ui::WeCLabel::HorAlign::Center:
                    lineOffset[i] = (contentSize.width - lineOffset[i] * scale) * 0.5f;
                    break;
                case cocos2d::ui::WeCLabel::HorAlign::Right:
                    lineOffset[i] = contentSize.width - lineOffset[i] * scale;
                    break;
                default:
                    break;
                }
            }

            // Apply the alignment.
            for (auto &iter : infos)
            {
                iter.pos.y = iter.pos.y * scale + yOffset;
                iter.pos.x = iter.pos.x * scale + lineOffset[iter.lineIndex];
            }

            mLineCount = lineIndex + 1;
        }

        int WeCLabel::getLineCount() const
        {
            return mLineCount;
        }

        Color4B WeCLabel::_parseColor24(const std::u16string &text, int offset)
        {
            GLubyte r = (_hexToDecimal(text[offset]) << 4) | _hexToDecimal(text[offset + 1]);
            GLubyte g = (_hexToDecimal(text[offset + 2]) << 4) | _hexToDecimal(text[offset + 3]);
            GLubyte b = (_hexToDecimal(text[offset + 4]) << 4) | _hexToDecimal(text[offset + 5]);
            GLubyte a = 255;
            return Color4B(r, g, b, a);
        }

        Color4B WeCLabel::_parseColor32(const std::u16string &text, int offset)
        {
            GLubyte r = (_hexToDecimal(text[offset]) << 4) | _hexToDecimal(text[offset + 1]);
            GLubyte g = (_hexToDecimal(text[offset + 2]) << 4) | _hexToDecimal(text[offset + 3]);
            GLubyte b = (_hexToDecimal(text[offset + 4]) << 4) | _hexToDecimal(text[offset + 5]);
            GLubyte a = (_hexToDecimal(text[offset + 6]) << 4) | _hexToDecimal(text[offset + 7]);
            return Color4B(r, g, b, a);
        }


        float WeCLabel::_getSpecialCodeXOffset(unsigned short ch, const std::string &fontName, int fontSize, int outline)
        {
            std::string key = WeCCharFontManager::generateKey(fontName, fontSize, outline);
            WeCCharFontManager::getInstance()->collectCharacters({ ch }, fontName, fontSize, outline);
            auto tmpDef = WeCCharFontManager::getInstance()->getCharFontDefinition(key, ch);
            if (tmpDef == nullptr)
                return 0.0f;
            return  tmpDef->printOffsetX;
        }

        float WeCLabel::_getSpecialCodeXAdvance(unsigned short ch, const std::string &fontName, int fontSize, int outline)
        {
            std::string key = WeCCharFontManager::generateKey(fontName, fontSize, outline);
            WeCCharFontManager::getInstance()->collectCharacters({ ch }, fontName, fontSize, outline);
            auto tmpDef = WeCCharFontManager::getInstance()->getCharFontDefinition(key, ch);
            if (tmpDef == nullptr)
                return 0.0f;
            return tmpDef->printXAdvance;
        }

        void WeCLabel::_insertSpecialCode(unsigned short ch, const std::string &fontName, int fontSize, int outline,
            float scale, int lineIndex, int sub, bool bold, bool italic,
            bool ignoreColor, Color4B color, float nextX, float nextY, int width, LetterInfoList &infos)
        {
            std::string key = WeCCharFontManager::generateKey(fontName, fontSize, outline);

            // Query the information of the character.
            WeCCharFontManager::getInstance()->collectCharacters({ ch }, fontName, fontSize, outline);

            // Get the character information.
            auto tmpDef = WeCCharFontManager::getInstance()->getCharFontDefinition(key, ch);

            if (tmpDef == nullptr)
                return;

            float tmpX = nextX;
            while (tmpX < nextX + width)
            {
                // Add the character to the infos with custom length.
                float xOffset = tmpDef->printOffsetX * scale;
                float xPos = xOffset + tmpX;
                float yOffset = tmpDef->printOffsetY * scale;
                float yPos = nextY - yOffset;
                tmpX += tmpDef->printXAdvance;
                float tmpWidth = tmpX > nextX + width ? tmpDef->printXAdvance - (tmpX - (nextX + width)) : -1;

                LetterInfo info =
                {
                    Vec2(xPos, yPos),
                    color,
                    "",
                    ch,
                    lineIndex,
                    tmpWidth,
                    sub,
                    bold,
                    italic,
                    ignoreColor
                };

                infos.emplace_back(info);
            }
        }

        int WeCLabel::_parseFirstWord(const std::u16string &str, int offset)
        {
            int textLen = static_cast<int>(str.size());

            auto character = str[offset];
            if (StringUtils::isCJKUnicode(character) ||
                StringUtils::isUnicodeSpace(character) ||
                character == '\n' ||
                character == '[')
            {
                return 1;
            }

            int len = 1;
            for (int index = offset + 1; index < textLen; ++index)
            {
                character = str[index];
                if (character == '\n' || character == '[' || StringUtils::isUnicodeSpace(character) ||
                    StringUtils::isCJKUnicode(character))
                {
                    break;
                }
                len++;
            }

            return len;
        }


		// ���ݵ�ǰchar��ȡ��Ӧ�Ľ�����
		void WeCLabel::_getParserForChar(char ch, std::vector<DXLabelParseOper*>& matchParserList)
		{
			std::string strCh;
			for (auto const &ent : m_parserMap)
			{
				std::string strSymbol = ent.first;
				if (0 == strSymbol.compare(strCh))
				{
					matchParserList.emplace_back( ent.second);
				}
			}
		}

		void WeCLabel::registeParser(std::string symbol, DXLabelParseOper* parser)
		{
			m_parserMap[symbol] = parser;
			// test
			m_parserMap["["] = new DXLabelBBCodeParser();// BBCode
			m_parserMap["\\"] = new DXLabelEscapeParser();// 转义

		}

		void WeCLabel::_shap(const std::u16string &text)
		{
			for (auto &comp : m_compMap)
			{
				m_shaper->doShap(comp.second);
			}
		}		

		// 排版，是一种有规则的装箱算法
        void WeCLabel::_typo(const std::u16string &text)
        {
            int cur_offset=0;
            int iMax = text.length();

            int curLine = 0;
            Size maxSize;
            Size curLineSize;
            // {linenumber, comp}
            std::map<int, std::vector<LabelComponent*>> lineCompMap;
            while(cur_offset < iMax)
            {
                // 有没有comp可以塞
                if(m_compMap.find(cur_offset)!=m_compMap.end())
                {
                    auto curComp = m_compMap[cur_offset];
                    // comp的size是shap后得到的printSize
                    int compWidth = curComp->getTextMaxWidth();
                    int compHeight = curComp->getTextMaxHeight();

                    // 能塞在一行就塞
                    if(curLineSize.width + compWidth <= maxSize.width)
                    {
                        lineCompMap[curLine].emplace_back(curComp);
                        curLineSize.width += compWidth;
                        curLineSize.height = MAX(curLineSize.height, compHeight);//行高
                    }
                    // 事先根据断词规则划分成不可分割的comp// 塞不到一行就换行
                    // 但是如果原子单词都比最大size要长就必须分割了？
                    else
                    {
                        // 换行了，结束当前行的
                        

						// 初始化新的一行
                        curLine++;
                        curLineSize = Size::ZERO;

                        lineCompMap[curLine].emplace_back(curComp);
                        curLineSize.width += compWidth;
                        curLineSize.height = MAX(curLineSize.height, compHeight);//行高
                    }
                    cur_offset += curComp->getCContent().length();
                }
                else if(actionMap.find(cur_offset)!=actionMap.end())
                {
                    // action如何可以集成式，输入lineSize和curLine？
                }
            }
        }

		bool WeCLabel::_parse(const std::u16string &text)
		{
			std::string strUtf8;
			StringUtils::UTF16ToUTF8(text, strUtf8);

			int cur_offset = 0;
			int end_offset;
            int iLength =  strUtf8.size();
			LabelComponent* ptrComp = nullptr;
			LabelAction* ptrAction = nullptr;

            std::string simpleWord; // 没有解析器识别的字符暂存串

			while (cur_offset != iLength)
			{
				// 如果有解析器可以解析
				std::vector<DXLabelParseOper*> matchParserList;
				_getParserForChar(strUtf8[cur_offset], matchParserList);
				bool rs = false;
				for (auto parser : matchParserList)
				{
					bool rs = parser->TryParse(strUtf8, cur_offset, end_offset, &ptrComp, &ptrAction);
					if (rs == true)
						break;
				}
				if (rs == true)
				{
					// 如果当前寄存的零散字符串不为空，则需要生成纯文本comp
					if (simpleWord.length() > 0)
					{
						LabelComponent* simpleComp = LabelComponent::create();
						simpleComp->setContent(simpleWord.c_str());
						// 以comp起始index作为索引
						m_compMap[cur_offset - simpleWord.length()] = simpleComp;
						log("在第%d处开始，加入了长度%d的%d类型的comp"，cur_offset, simpleWord.length(), type);
						simpleWord.clear();
					}
					// 如果解析后得到comp
					if (ptrComp != nullptr)
					{
						m_compMap[cur_offset] = ptrComp;
						cur_offset = end_offset + 1;
					}
					else if (ptrAction != nullptr)
					{
						actionMap[cur_offset] = ptrAction;
						cur_offset = end_offset + 1;
					}
				}
				// 解析器解析失败
				else
				{
					simpleWord.emplace_back(strUtf8[cur_offset]);
				}
			}
		}

        bool WeCLabel::_parseSymbol(const std::u16string &text, int &index, std::vector<Color4B> &colors,
            int &sub, bool &bold, bool &italic, bool &underline, bool &strike, bool &ignoreColor,
            std::u16string &imageName, bool &raw)
        {
            int length = text.size();

            if (text.empty())
                return false;

            if (index < 0 || index >= length)
                return false;

            // At least 3 characters.
            if (index + 3 > length || text[index] != '[') // Different.
                return false;

            // If is in raw mode, do not execute this block.
            if (text[index + 2] == ']')
            {
                if (raw == true)
                    return false;

                if (text[index + 1] == '-') // Use [-] as an color end tag.
                {
                    // Remove color.
                    if (colors.size() > 0)
                        colors.pop_back();

                    if (colors.size() <= 0)
                        ignoreColor = false;

                    index += 3;
                    return true;
                }

                std::u16string sub3 = text.substr(index, 3);

                if (sub3 == BOLD_TAG_BEGIN)
                {
                    bold = true;
                    index += 3;
                    return true;
                }

                if (sub3 == ITALIC_TAG_BEGIN)
                {
                    italic = true;
                    index += 3;
                    return true;
                }

                if (sub3 == UNDERLINE_TAG_BEGIN)
                {
                    underline = true;
                    index += 3;
                    return true;
                }

                if (sub3 == STRIKE_TAG_BEGIN)
                {
                    strike = true;
                    index += 3;
                    return true;
                }

                if (sub3 == COLOR_TAG_BEGIN)
                {
                    ignoreColor = true;
                    index += 3;
                    return true;
                }

                if (sub3 == RAW_TAG_BEGIN)
                {
                    raw = true;
                    index += 3;
                    return true;
                }

                return false;
            }

            if (index + 4 > length)
                return false;

            if (text[index + 3] == ']')
            {
                std::u16string sub4 = text.substr(index, 4);

                // If in raw mode.
                if (raw == true && sub4 == RAW_TAB_END)
                {
                    raw = false;
                    index += 4;
                    return true;
                }

                if (raw == true)
                    return false;

                if (sub4 == BOLD_TAG_END)
                {
                    bold = false;
                    index += 4;
                    return true;
                }

                if (sub4 == ITALIC_TAG_END)
                {
                    italic = false;
                    index += 4;
                    return true;
                }

                if (sub4 == UNDERLINE_TAG_END)
                {
                    underline = false;
                    index += 4;
                    return true;
                }

                if (sub4 == STRIKE_TAG_END)
                {
                    strike = false;
                    index += 4;
                    return true;
                }

                if (sub4 == COLOR_TAG_END)
                {
                    ignoreColor = false;
                    index += 4;
                    return true;
                }

                // default:
        {
            char ch0 = text[index + 1];
            char ch1 = text[index + 2];

            if (_isHex(ch0) && _isHex(ch1))
            {
                int a = (_hexToDecimal(ch0) << 4) | _hexToDecimal(ch1);
                // a;
                index += 4;
                return true;
            }
        }

        return false;
            }

            if (index + 5 > length)
                return false;

            if (text[index + 4] == ']')
            {
                if (raw == true)
                    return false;

                std::u16string sub5 = text.substr(index, 5);

                if (sub5 == SUB_TAG_BEGIN)
                {
                    sub = 1;
                    index += 5;
                    return true;
                }

                if (sub5 == SUP_TAG_BEGIN)
                {
                    sub = 2;
                    index += 5;
                    return true;
                }

                return false;
            }

            if (index + 6 > length)
                return false;

            if (text[index + 5] == ']')
            {
                if (raw == true)
                    return false;

                std::u16string sub6 = text.substr(index, 6);

                if (sub6 == SUB_TAG_END)
                {
                    sub = 0;
                    index += 6;
                    return true;
                }

                if (sub6 == SUP_TAG_END)
                {
                    sub = 0;
                    index += 6;
                    return true;
                }

                return false;
            }

            if (text[index + 1] == 'i' &&
                text[index + 2] == 'm' &&
                text[index + 3] == 'g' &&
                text[index + 4] == '=')
            {
                if (raw == true)
                    return false;

                int closeingBracket = text.find(']', index + 4);

                if (closeingBracket != text.npos)
                {
                    // Get the imageName.
                    imageName = text.substr(index + 5, closeingBracket - (index + 5));
                    index = closeingBracket + 1;
                    return true;
                }
                else
                {
                    index = text.size();
                    return true;
                }
            }

            if (index + 8 > length)
                return false;

            if (text[index + 7] == ']')
            {
                if (raw == true)
                    return false;

                for (int idx = 1; idx < 7; ++idx)
                {
                    if (StringUtils::isCJKUnicode(text[index + idx]))
                    {
                        return false;
                    }
                }

                if (text.find(STRIP_TAG_END, index + 7) == std::u16string::npos)
                    return false;

                // Parse color24;
                Color4B color = _parseColor24(text, index + 1);
                colors.emplace_back(color);

                ignoreColor = true;

                index += 8;
                return true;
            }

            if (index + 10 > length)
                return false;

            if (text[index + 9] == ']')
            {
                if (raw == true)
                    return false;

                for (int idx = 1; idx < 7; ++idx)
                {
                    if (StringUtils::isCJKUnicode(text[index + idx]))
                    {
                        return false;
                    }
                }

                if (text.find(STRIP_TAG_END, index + 9) == std::u16string::npos)
                    return false;

                // Parse color32.
                Color4B color = _parseColor32(text, index + 1);
                colors.emplace_back(color);

                ignoreColor = true;

                index += 10;
                return true;
            }

            return false;
        }

        bool WeCLabel::_isHex(char ch)
        {
            return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
        }

        int WeCLabel::_hexToDecimal(char ch)
        {
            switch (ch)
            {
            case '0':
                return 0x0;
            case '1':
                return 0x1;
            case '2':
                return 0x2;
            case '3':
                return 0x3;
            case '4':
                return 0x4;
            case '5':
                return 0x5;
            case '6':
                return 0x6;
            case '7':
                return 0x7;
            case '8':
                return 0x8;
            case '9':
                return 0x9;
            case 'a':
            case 'A':
                return 0xA;
            case 'b':
            case 'B':
                return 0xB;
            case 'c':
            case 'C':
                return 0xC;
            case 'd':
            case 'D':
                return 0xD;
            case 'e':
            case 'E':
                return 0xE;
            case 'f':
            case 'F':
                return 0xF;
            }
            return 0xF;
        }

        std::u16string WeCLabel::_stripSymbols(const std::u16string &text)
        {
            std::u16string str = text;

            if (str.empty())
                return str;

            for (int i = 0, imax = str.size(); i < imax;)
            {
                auto ch = str[i];

                if (ch == '[')
                {
                    int sub = 0;
                    bool bold = false;
                    bool italic = false;
                    bool underline = false;
                    bool strikethrough = false;
                    bool ignoreColor = false;
                    std::vector<Color4B> colors;
                    std::u16string imageName;
                    int retVal = i;
                    bool raw = false;

                    if (_parseSymbol(str, retVal, colors, sub, bold, italic,
                        underline, strikethrough, ignoreColor, imageName, raw))
                    {

                        str = str.erase(i, retVal - i);
                        imax = str.size();
                        continue;
                    }
                }
                ++i;
            }

            return str;
        }


        Widget* WeCLabel::createCloneInstance()
        {
            return WeCLabel::create();
        }

        void WeCLabel::copySpecialProperties(Widget* model)
        {
            WeCLabel * label = dynamic_cast<WeCLabel*>(model);
            if (label)
            {
                setFontColor(label->getFontColor());
                mEffectColor = label->mEffectColor;
                setFontName(label->getFontName());
                setText(label->getText());
                setSpacingX(label->getSpacingX());
                setSpacingY(label->getSpacingY());
                setShadowOffsetX(label->getShadowOffsetX());
                setShadowOffsetY(label->getShadowOffsetY());
                setShadowOffsetY(label->getShadowOffsetY());
                setFontSize(label->getFontSize());
                setMaxLines(label->getMaxLines());
                setLevel(label->getLevel());
                setBbcode(label->getBbcode());
                setHorAlign(label->getHorAlign());
                setVerAlign(label->getVerAlign());
                setOverflow(label->getOverflow());
                setEffect(label->getEffect());
                setBorderSize(label->getBorderSize());
                setScale(label->getScale());
                setContentSize(label->getContentSize());
            }
        }

        void WeCLabel::setFontName(const std::string &val)
        {
            if (mFontName != val)
            {
                mFontName = val;
                mDirty = true;
            }
        }

        void WeCLabel::setFontSize(int val)
        {
            if (mFontSize != val)
            {
                mFontSize = val;
                mDirty = true;
            }
        }

        void WeCLabel::setFontColor(const cocos2d::Color4B &val)
        {
            if (mFontColor != val)
            {
                mFontColor = val;
                mDirty = true;
            }
        }
        void WeCLabel::setEffectColor(const cocos2d::Color4B &val)
        {
            if (mEffectColor != val)
            {
                mEffectColor = val;
                mDirty = true;
            }
        }

#ifdef CC_STUDIO_ENABLED_VIEW
        void WeCLabel::setHorAlign(int val)
        {
            mHorAlign = static_cast<HorAlign>(val);
            mDirty = true;
        }

        void WeCLabel::setVerAlign(int val)
        {
            mVerAlign = static_cast<VerAlign>(val);
            mDirty = true;
        }

        void WeCLabel::setOverflow(int val)
        {
            mOverflow = static_cast<Overflow>(val);
            mDirty = true;
        }

        void WeCLabel::setEffect(int val)
        {
            mEffect = static_cast<Effect>(val);
            mDirty = true;
        }
#else
        void WeCLabel::setHorAlign(HorAlign val)
        {
            if (mHorAlign != val)
            {
                mHorAlign = val;
                mDirty = true;
            }
        }

        void WeCLabel::setVerAlign(VerAlign val)
        {
            if (mVerAlign != val)
            {
                mVerAlign = val;
                mDirty = true;
            }
        }

        void WeCLabel::setOverflow(Overflow val)
        {
            if (mOverflow != val)
            {
                mOverflow = val;
                mDirty = true;
            }
        }

        void WeCLabel::setEffect(Effect val)
        {
            if (mEffect != val)
            {
                mEffect = val;
                mDirty = true;
            }
        }
#endif

        void WeCLabel::setSpacingX(float val)
        {
            if (mSpacingX != val)
            {
                mSpacingX = val;
                mDirty = true;
            }
        }

        void WeCLabel::setSpacingY(float val)
        {
            if (mSpacingY != val)
            {
                mSpacingY = val;
                mDirty = true;
            }
        }

        void WeCLabel::setMaxLines(int val)
        {
            if (mMaxLines != val)
            {
                mMaxLines = val;
                mDirty = true;
            }
        }

        void WeCLabel::setLevel(int val)
        {
            if (mLevel != val)
            {
                mLevel = val;
                mDirty = true;
            }
        }

        void WeCLabel::setBbcode(bool val)
        {
            if (mBbcode != val)
            {
                mBbcode = val;
                mDirty = true;
            }
        }

        void WeCLabel::setBorderSize(int val)
        {
            if (mBorderSize != val)
            {
                mBorderSize = val;
                mDirty = true;
            }
        }

        void WeCLabel::setShadowOffsetX(int val)
        {
            if (mShadowOffsetX != val)
            {
                mShadowOffsetX = val;
                mDirty = true;
            }
        }

        void WeCLabel::setShadowOffsetY(int val)
        {
            if (mShadowOffsetY != val)
            {
                mShadowOffsetY = val;
                mDirty = true;
            }
        }

        void WeCLabel::setText(const std::string &val)
        {
            if (mText != val)
            {
                mText = val;
                mDirty = true;
            }
        }


        void WeCLabel::setClampWithPoints(bool val)
        {
            mClampWithPoints = val;
        }

        std::u16string WeCLabel::stripSymbols(const std::u16string& text)
        {
            return _stripSymbols(text);
        }

        int WeCLabel::getU16TextLength() const
        {
            return static_cast<int>(mReuseU16Text.size());
        }

        const std::string& WeCLabel::getFontName() const
        {
            return mFontName;
        }

        int WeCLabel::getFontSize() const
        {
            return mFontSize;
        }

        const cocos2d::Color4B& WeCLabel::getFontColor() const
        {
            return mFontColor;
        }

        const cocos2d::Color4B& WeCLabel::getEffectColor() const
        {
            return mEffectColor;
        }

#ifdef CC_STUDIO_ENABLED_VIEW
        int WeCLabel::getHorAlign() const
        {
            return static_cast<int>(mHorAlign);
        }

        int WeCLabel::getVerAlign() const
        {
            return static_cast<int>(mVerAlign);
        }

        int WeCLabel::getOverflow() const
        {
            return static_cast<int>(mOverflow);
        }

        int WeCLabel::getEffect() const
        {
            return static_cast<int>(mEffect);
        }
#else
        WeCLabel::HorAlign WeCLabel::getHorAlign() const
        {
            return mHorAlign;
        }

        WeCLabel::VerAlign WeCLabel::getVerAlign() const
        {
            return mVerAlign;
        }

        WeCLabel::Overflow WeCLabel::getOverflow() const
        {
            return mOverflow;
        }

        WeCLabel::Effect WeCLabel::getEffect() const
        {
            return mEffect;
        }
#endif

        float WeCLabel::getSpacingX() const
        {
            return mSpacingX;
        }

        float WeCLabel::getSpacingY() const
        {
            return mSpacingY;
        }

        int WeCLabel::getMaxLines() const
        {
            return mMaxLines;
        }

        int WeCLabel::getLevel() const
        {
            return mLevel;
        }

        bool WeCLabel::getBbcode() const
        {
            return mBbcode;
        }

        int WeCLabel::getBorderSize() const
        {
            return mBorderSize;
        }

        int WeCLabel::getShadowOffsetX() const
        {
            return mShadowOffsetX;
        }

        int WeCLabel::getShadowOffsetY() const
        {
            return mShadowOffsetY;
        }

        const std::string& WeCLabel::getText() const
        {
            return mText;
        }

        float WeCLabel::getStrokerOutline() const
        {
            return mStrokerOutline;
        }

        void WeCLabel::setStrokerOutline(float val)
        {
            mStrokerOutline = val;
            mDirty = true;
        }

        bool WeCLabel::getGradientEnabled() const
        {
            return mGradientEnabled;
        }

        const cocos2d::Color4B& WeCLabel::getGradientTopColor() const
        {
            return mGradientTopColor;
        }

        const cocos2d::Color4B& WeCLabel::getGradientBottomColor() const
        {
            return mGradientBottomColor;
        }

        void WeCLabel::setGradientEnabled(bool val)
        {
            mGradientEnabled = val;
            mDirty = true;
        }

        void WeCLabel::setGradientTopColor(const cocos2d::Color4B& val)
        {
            mGradientTopColor = val;
            mDirty = true;
        }

        void WeCLabel::setGradientBottomColor(const cocos2d::Color4B& val)
        {
            mGradientBottomColor = val;
            mDirty = true;
        }

    }  // namespace ui
}  // namespace cocos2d
