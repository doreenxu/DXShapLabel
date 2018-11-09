
#include "base/ccTypes.h"

/*  Custom include block. You should write
    your own include in this block only. Do not
    change the Tag "CUSTOM_INCLUDE_BEGIN" &
    "CUSTOM_INCLUDE_END", The generator use these
    tags to determine the block which need to
    copy at next auto-generation.
    */

// CUSTOM_INCLUDE_BEGIN
#include <string>
#include <array>
#include <vector>
#include <unordered_map>

#include "2d/CCNode.h"
#include "2d/CCSprite.h"
#include "renderer/CCQuadCommand.h"
#include "ui/UIWidget.h"




#define WEC_LABEL_VBO_SIZE 65536

// CUSTOM_INCLUDE_END

#ifndef _CUSTOM_WIDGET_WeCLabel
#define _CUSTOM_WIDGET_WeCLabel

namespace cocos2d
{

    class WeCCharFontManager;

    /*  Custom namespace block. You should write
        your own include in this block only. Do not
        change the Tag "CUSTOM_NAMESPACE_START_BEGIN" &
        "CUSTOM_NAMESPACE_START_END", The generator use these
        tags to determine the block which need to
        copy at next auto-generation.
        */

    // force namespace to ui, because the auto generate binding config is only one namespace.
    namespace ui
    {
		class DXLabelParseOper;
		class DXShaper;

		class WeCLabel : public Widget
        {
            DECLARE_CLASS_GUI_INFO
            
            /*  Custom code block. You should write
                your own code in this block only. Do not
                change the Tag "CUSTOM_CODE_BEGIN" &
                "CUSTOM_CODE_END", The generator use these
                tags to determine the block which need to
                copy at next auto-generation.
                */

            // CUSTOM_CODE_BEGIN
        public:
            friend class CC_DLL cocos2d::WeCCharFontManager;

            enum class HorAlign
            {
                Left = 0,
                Center = 1,
                Right = 2,
            };

            enum class VerAlign
            {
                Top = 0,
                Center = 1,
                Bottom = 2,
            };

            enum class Overflow
            {
                Shrink = 0,
                Clamp = 1,
                ResizeFreely = 2,
                ResizeHeight = 3,
            };

            enum class Effect
            {
                None = 0,
                Shadow = 1,
                Outline = 2,
            };

        public:
            virtual bool                    init() override;
            Size                            getLabelSize();
            
            virtual void                    setContentSize(const Size& contentSize) override;

            virtual void                    onEnter() override;
            virtual void                    onExit() override;

            int                             getLineCount() const;

#ifdef CC_STUDIO_ENABLED_VIEW
            virtual int                     getHorAlign() const;
            virtual int                     getVerAlign() const;
            virtual int                     getOverflow() const;
            virtual int                     getEffect() const;
#else
            virtual HorAlign                getHorAlign() const;
            virtual VerAlign                getVerAlign() const;
            virtual Overflow                getOverflow() const;
            virtual Effect                  getEffect() const;
#endif

#ifdef CC_STUDIO_ENABLED_VIEW
            virtual void                    setHorAlign(int val);
            virtual void                    setVerAlign(int val);
            virtual void                    setOverflow(int val);
            virtual void                    setEffect(int val);
#else
            virtual void                    setHorAlign(HorAlign val);
            virtual void                    setVerAlign(VerAlign val);
            virtual void                    setOverflow(Overflow val);
            virtual void                    setEffect(Effect val);
#endif


            virtual void                    visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags) override;
            virtual void                    draw(Renderer *renderer, const Mat4& transform, uint32_t flags) override;

            virtual cocos2d::Sprite*        getLetter(int index);
            // Get Letter position in design resolution.
            virtual cocos2d::Vec2           getLetterPosition(int index);
            virtual void                    setOpacity(GLubyte opacity) override;
            virtual void                    updateDisplayedOpacity(GLubyte parentOpacity) override;
            void                            forceUpdateQuads();

            // Get the line height of the label in design resolution.
            float                           getLineHeight();

        private:
            struct LetterInfo
            {
                Vec2                        pos;
                Color4B                     color;
                std::string                 imageName;
                int                         code;
                int                         lineIndex;
                float                       customWidth;
                int                         sub;
                bool                        bold;
                bool                        italic;
                bool                        ignoreColor;
            };

            typedef std::vector<LetterInfo> LetterInfoList;

            void                            _process();
            void                            _updateQuads();
            void                            _parseLetterInfo(LetterInfoList &infos, Size &contentSize, float &scale);
            Sprite*                         _createFontSprite(Texture2D *tex, const Vec2 &Pos, const Color4B color,
                                                float scaleX, float scaleY, bool italic);
            void                            _mergeQuads();
            void                            _clearQuads();
            void                            _clearOnlyQuads();
            Sprite*                         _createImageSprite(const std::string &imgName, const Vec2 &Pos, float scale, float customWidth);
            void                            _updateQuadsInternal();

        private:
            static int                      _parseFirstWord(const std::u16string &str, int offset);

            static bool                     _parseSymbol(const std::u16string &text, int &index, std::vector<Color4B> &color,
                                                int &sub, bool &bold, bool &italic, bool &underline, bool &strike,
                                                bool &ignoreColor, std::u16string &imageName, bool &raw);
            static bool                     _isHex(char ch);
            static int                      _hexToDecimal(char ch);
            static std::u16string           _stripSymbols(const std::u16string &text);
            static void                     _insertSpecialCode(unsigned short ch, const std::string &fontName, int fontSize, int outline,
                                                float scale, int lineIndex, int sub, bool bold, bool italic,
                                                bool ignoreColor, Color4B color, float nextX, float nextY, int width, LetterInfoList &infos);
            static Color4B                  _parseColor24(const std::u16string &text, int offset);
            static Color4B                  _parseColor32(const std::u16string &text, int offset);
            static float                    _getSpecialCodeXOffset(unsigned short ch, const std::string &fontName, int fontSize, int outline);
            static float                    _getSpecialCodeXAdvance(unsigned short ch, const std::string &fontName, int fontSize, int outline);


        private:
            typedef Vector<Sprite*>                         SpriteList;
            typedef std::vector<Vec2>                       PointList;
            typedef std::vector<V3F_C4B_T2F_Quad>           QuadList;
            typedef std::unordered_map<GLuint, QuadList>    QuadListMap;
            typedef std::unordered_map<int, int>            QuadAlphaMap;

            struct TriangleInfo
            {
                std::vector<V3F_C4B_T2F>    triangles;
                std::vector<unsigned short> indices;
            };

            typedef std::vector<QuadCommand>                QuadCommandList;

            Size                            mFullContentSize;

            /** Unicode string used to generate the font bitmap. */
            std::u16string                  mReuseU16Text;
            /** List of characters. */
            SpriteList                      mCharacters;
            /** Shadow characters. */
            SpriteList                      mShadows;
            /** Images. */
            SpriteList                      mImages;
            /** Orders without effect sprite*/
            SpriteList                      mOrders;
            /**letters postion*/
            PointList                       mPositions;
            /** */
            Rect                            mReusedRect;
            /** */
            Rect                            mReusedTexRect;
            /** Blend function to use. */
            BlendFunc                       mBlend;

            QuadListMap                     mQuads;
            QuadListMap                     mImgQuads;

            QuadCommandList                 mQuadCmd;
            QuadCommandList                 mImgQuadCmd;

            QuadAlphaMap                    mQuadAlphas;

            LetterInfoList                  mLetterInfos;

            float                           mLastSpriteScale;
            float                           mCurMaxWidth;

            /** Content Scale when shrink mode is enabled. */
            float                           mScale;
            unsigned int                    mWeCLabelId;
            int                             mLineCount;

#if CC_LABEL_DEBUG_DRAW
            DrawNode*                       mDebugDrawNode;
#endif
            /** Check the label is need to process or not. */
            bool                            mDirty;
            bool                            mWillDraw;


			bool _parse(const std::u16string &text);
			void registeParser(std::string symbol, DXLabelParseOper* parser);
			void _getParserForChar(char ch, std::vector<DXLabelParseOper*>& matchParserList);
			void _typo(const std::u16string &text);
			void _shap(const std::u16string &text);

			// {offset, comp}
			std::map<int, LabelComponent* > m_compMap;
			std::map<int, LabelAction* > actionMap;
			// 用parser关心的char注册parser
			std::map<std::string, DXLabelParseOper*> m_parserMap; 
			DXShaper* m_shaper;
        public:
            static unsigned int             sWeCLabelCount;

            // CUSTOM_CODE_END

        public:
                                            WeCLabel(void);
            virtual                         ~WeCLabel(void);

            // Common functions
            static WeCLabel*                create(void);

            virtual Widget*                 createCloneInstance() override;
            virtual void                    copySpecialProperties(Widget* model) override;
            virtual int                     getU16TextLength() const;
            virtual const std::string&      getFontName() const;
            virtual int                     getFontSize() const;
            virtual const cocos2d::Color4B& getFontColor() const;
            virtual const cocos2d::Color4B& getEffectColor() const;
            virtual float                   getSpacingX() const;
            virtual float                   getSpacingY() const;
            virtual int                     getMaxLines() const;
            virtual int                     getLevel() const;
            virtual bool                    getBbcode() const;
            virtual int                     getBorderSize() const;
            virtual int                     getShadowOffsetX() const;
            virtual int                     getShadowOffsetY() const;
            virtual const std::string&      getText() const;
            virtual float                   getStrokerOutline() const;
            virtual bool                    getGradientEnabled() const;
            virtual const cocos2d::Color4B& getGradientTopColor() const;
            virtual const cocos2d::Color4B& getGradientBottomColor() const;

            // Set functions.
            virtual void                    setFontName(const std::string &val);
            virtual void                    setFontSize(int val);
            virtual void                    setFontColor(const cocos2d::Color4B &val);
            virtual void                    setEffectColor(const cocos2d::Color4B &val);
            virtual void                    setSpacingX(float val);
            virtual void                    setSpacingY(float val);
            virtual void                    setMaxLines(int val);
            virtual void                    setLevel(int val);
            virtual void                    setBbcode(bool val);
            virtual void                    setBorderSize(int val);
            virtual void                    setShadowOffsetX(int val);
            virtual void                    setShadowOffsetY(int val);
            virtual void                    setText(const std::string &val);
            virtual void                    setClampWithPoints(bool val);
            virtual void                    setStrokerOutline(float val);
            virtual void                    setGradientEnabled(bool val);
            virtual void                    setGradientTopColor(const cocos2d::Color4B& val);
            virtual void                    setGradientBottomColor(const cocos2d::Color4B& val);

			static std::u16string			stripSymbols(const std::u16string& text);
        private:
            cocos2d::Color4B                mFontColor;
            cocos2d::Color4B                mEffectColor;
            cocos2d::Color4B                mGradientTopColor;
            cocos2d::Color4B                mGradientBottomColor;
            std::string                     mText;
            std::string                     mFontName;
            float                           mSpacingX;
            float                           mSpacingY;
            int                             mBorderSize;
            int                             mShadowOffsetX;
            int                             mShadowOffsetY;
            int                             mFontSize;
            int                             mMaxLines;
            int                             mLevel;
            float                           mStrokerOutline;
            bool                            mBbcode;
            bool                            mClampWithPoints;
            bool                            mGradientEnabled;
            HorAlign                        mHorAlign;
            VerAlign                        mVerAlign;
            Overflow                        mOverflow;
            Effect                          mEffect;
        };

        /*  Custom namespace end block. You should write
            your own include in this block only. Do not
            change the Tag "CUSTOM_NAMESPACE_END_BEGIN" &
            "CUSTOM_NAMESPACE_END_END", The generator use these
            tags to determine the block which need to
            copy at next auto-generation.
            */
    }

}  // namespace cocos2d
#endif  // _CUSTOM_WIDGET_
