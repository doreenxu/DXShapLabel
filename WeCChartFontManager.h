#ifndef WECCHARTFONTMANAGER_H_
#define WECCHARTFONTMANAGER_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <set>
#include <ft2build.h>
#include "platform/CCPlatformMacros.h"
#include "renderer/CCTexture2D.h"
#include "base/CCData.h"
#include "base/CCEventCustom.h"
#include "base/CCEventListenerCustom.h"

#include FT_FREETYPE_H
#include FT_STROKER_H

NS_CC_BEGIN

class GLProgramState;

namespace ui
{
    class WeCLabel;
}
typedef std::unordered_map<unsigned int, cocos2d::ui::WeCLabel*> WeCLabelMap;

//////////////////////////////////////////////////////////////////////////////////
// WeCChartFontDefinition

/** Specified the definition of the char. */
struct WeCChartFontDefinition
{
public:
    /** The texture coordinate in pixel. */
    float           U;
    /** The texture coordinate in pixel. */
    float           V;
    /** The print width of the character in pixel. */
    float           printWidth;
    /** The print height of the character in pixel. */
    float           printHeight;
    /** The offset x of the character. */
    float           printOffsetX;
    /** The offset y of the character. */
    float           printOffsetY;
    /** The advance of the character. */
    int             printXAdvance;
    /** The texture handle of OpenGL. */
    int             texIndex;
    /** The line height of this code.*/
    int             printlineHeight;
    /** The code of the character. */
    unsigned short  code;
};

typedef std::shared_ptr<WeCChartFontDefinition>                         WeCChartFontDefinitionPtr;
typedef std::unordered_map<unsigned short, WeCChartFontDefinitionPtr>   WeCChartFontDefinitionList;
typedef std::unordered_map<std::string, WeCChartFontDefinitionList>     WeCChartFontDefinitionListMap;

// Specified the information of the font.
enum class WeCSystemFontStyle
{
    Normal = 0,
    Bold = 1,
    Italic = 2,
};

//////////////////////////////////////////////////////////////////////////////////
// WeCSystemFontInfo

/** Specified the font name and code set for font selection. */
struct WeCSystemFontInfo
{
public:
    WeCSystemFontInfo();
    ~WeCSystemFontInfo();

public:
    typedef std::set<unsigned long>            WeCFontCodeSet;

    /** The name of the font. */
    std::string         name;
    /** The code map of the font. */
    WeCFontCodeSet      codeSet;
};

typedef std::shared_ptr<WeCSystemFontInfo>                              WeCSystemFontInfoPtr;
typedef std::unordered_map<std::string, WeCSystemFontInfoPtr>           WeCSystemFontInfoMap;
typedef std::vector<WeCSystemFontInfoPtr>                               WeCSystemFontInfoList;

//////////////////////////////////////////////////////////////////////////////////
// WeCFontTextureInfo

/** Font texture info for update the font texture. */
struct WeCFontTextureInfo
{
public:
    WeCFontTextureInfo(void);

    // Cached data.
    unsigned char*                      textureCachedData;
    // Pixel format.
    cocos2d::Texture2D::PixelFormat     format;
    // The usage of the texture.
    std::vector<bool>                   usage;

    bool dirty;
};

typedef std::shared_ptr<WeCFontTextureInfo>             WeCFontTextureInfoPtr;
typedef std::vector<WeCFontTextureInfoPtr>              WeCFontTextureInfoList;

//////////////////////////////////////////////////////////////////////////////////
// WeCFontAtlasLineInfo

/** Atlas line info for manager the font texture memory. */
struct WeCFontAtlasLineInfo
{
public:
    WeCFontAtlasLineInfo(void);

    // typedef std::vector<unsigned short> CharList;

    /** The chars over there. */
    // CharList chars;
    /** The next offset of the line. */
    int                 nextXOffset;
    /** The offset in the texture. */
    int                 lineOffset;
    /** The line count of this line in atlas. */
    int                 lineCount;
    /** The index of the texture. */
    int                 textureIndex;
};

typedef std::shared_ptr<WeCFontAtlasLineInfo>                       WeCFontAtlasLineInfoPtr;
typedef std::vector<WeCFontAtlasLineInfoPtr>                        WeCFontAtlasLineInfoList;
typedef std::unordered_map<std::string, WeCFontAtlasLineInfoList>   WeCFontAtlasLineInfoMap;

typedef std::unordered_map<unsigned short, unsigned int>            CharCodeRefMap;
typedef std::unordered_map<std::string, CharCodeRefMap>             CharCodeRefWithConfigMap;

typedef std::vector<Texture2D*> TextureList;

/** Class for manager the font face, file stream and memory stream. */
struct WeCFontFileInfo
{
public:
    FT_Face         face;
    FT_Encoding     encode;
    unsigned char*  data;       // Is null means use the stream.
    unsigned int    dataSize;
    int             reference;
};

typedef std::unordered_map<std::string, WeCFontFileInfo> WeCFontFileInfoMap;

//////////////////////////////////////////////////////////////////////////////////
// WeCFontFreeType

class WeCFontFreeType
{
public:
    /** Constructor. */
                                        WeCFontFreeType(void);
    /** Destructor. */
    virtual                             ~WeCFontFreeType(void);

    /** Generate the bitmap to use by the specified char code. */
    virtual unsigned char*              getBitmap(unsigned short code, int &outwith,
        int &outheight, Rect &outrect, int &xadvance, int &ascender);
    
    /** Check is validate to use or not. */
    bool                                isValid(void) const;

    virtual int                         getPrintedLineHeight(void) const;

protected:
    /** Get the instance of FreeType engine, if the engine is not created, create a new singleton. */
    static FT_Library                   _getFTLibrary(void);
    /** Get the font face by the specified name and size. */
    static bool                         _getFontFace(const std::string &name, FT_Face &face, FT_Encoding &encode);
    /** Destroy the font face by the specified name, will remove the font info from map. */
    static bool                         _doneFontface(const std::string &name);

    unsigned char*                      _getBitmap(unsigned short code, FT_Face face, FT_Stroker stroker,
        FT_Encoding encoding, float outline, int &outWidth, int &outHeight, Rect &outRect, int &xadvance, int &ascender);

    unsigned char*                      _getBitmapByStroker();

    unsigned short                      _convertUnicode2GB2312(unsigned short ch);

protected:
    static WeCFontFileInfoMap           sFontFileInfos;

#ifdef CC_STUDIO_ENABLED_VIEW
    static bool                         initFreeType();
    static FT_Library                   sFTlibrary;
    static bool                         sFTLibraryInit;
#endif

    void                                *_iconv;

protected:
    bool                                mIsValid;
};

/** TTF font class. */
class WeCFontFreeTypeTTF : public WeCFontFreeType
{
public:
                                        WeCFontFreeTypeTTF(const std::string &fontName,
                                            int size, float outline, bool ignoreScale = false);
    virtual                             ~WeCFontFreeTypeTTF(void);

    virtual unsigned char*              getBitmap(unsigned short code, int &outwith, int &outheight,
                                            Rect &outrect, int &xadvance, int &ascender) override;

    virtual int                         getPrintedLineHeight(void) const override;

    bool                                prepareRender(void);

protected:
    FT_Face                             mFace;
    FT_Stroker                          mStroker;
    FT_Encoding                         mEncoding;
    std::string                         mFontName;
    int                                 mFontSize;
    float                               mOutline;
    int                                 mLineHeight;
    bool                                mIgnoreScale;
};

typedef std::shared_ptr<WeCFontFreeType>                    WeCFontFreeTypePtr;
typedef std::unordered_map<std::string, WeCFontFreeTypePtr> WeCFontFreeTypeMap;

//////////////////////////////////////////////////////////////////////////////////
// WeCCharFontManager

class WeCCharFontManager
{
public:
                                        WeCCharFontManager(void);
                                        ~WeCCharFontManager(void);

    /** Collect the characters from each label in one frame. */
    void                                collectCharacters(const std::u16string &str,
                                            const std::string &fontName, int fontSize, float outline);
    /** Query the information of the char used to update the quads in the label. */
    WeCChartFontDefinitionPtr           getCharFontDefinition(const std::string &configId,
                                            unsigned short code);
    /** get the textures of the WeCCharFontManager. */
    const TextureList&                  getTextures() const;
    /** Get the free type by the config id. */
    WeCFontFreeTypePtr                  getFontFreeType(const std::string &configId);
    /** Create the font free type, only for internal use. */
    WeCFontFreeTypePtr                  createFontFreeType(const std::string &fontName,
                                            int fontSize, float outline, bool ignoreScale);
    /** Print debug messages. */
    std::string                         printDebugMsg();

    // Try to get the stroker cache, if the memory is not enough,
    // Program will create a suitable memory block.
    unsigned char*                      tryGetStrokerReusedCache(int sizeInBytes);

private:
    /** When collected end, process the fonts. */
    void                                _process(void);
    /** Update textures. */
    void                                _updateTextures();
    /** Draw the the character to the atlas.
        @return 1 means success, 0 means general error, -1 means memory not enough.
    */
    int                                 _drawCharacterToAtlas(const std::string &configId,
                                            unsigned short code, bool &hasNoMem);
    /** try to process the collected chars. */
    bool                                _processCollectedChars(bool &hasNoMem);

    /** Try to get the memory line, if not find or generated, return null. */
    WeCFontAtlasLineInfoPtr             _getAtlasLine(const std::string &configId, int width,
                                            int height, bool &hasNoMem);
    WeCFontAtlasLineInfoPtr             _getAtlasLineFromUnusedMemory(const std::string &configId,
                                            int height, bool &hasNoMem);
    WeCFontAtlasLineInfoPtr             _getAtlasLineFromNewTexture(const std::string &configId,
                                            int height);
    /** Update the texture before camera draw. */
    void                                _preCameraDraw(EventCustom* event);
    /** Event after all draw operations. */
    void                                _afterDraw(EventCustom* event);
    /** Clear the memory and textures. */
    void                                _clearMemory();
    /** Collect the characters to the mCollectedChars. */
    void                                _colletInternal(const std::u16string &str,
                                            const std::string &fontName, int fontSize, float outline);

public:
    /** Get the instance of the font manager. */
    static WeCCharFontManager*          getInstance();

    /** Init the WeChartFontManager just once.  */
    static void                         initialize();

    /** Get the shader program of the WeCLabel that has been initialized first.
    should change the projection matrix when camera changed.
    */
    static cocos2d::GLProgramState*     getWeCLabelShader();
    static cocos2d::GLProgramState*     getWeCLabelImgShader();
	static cocos2d::GLProgramState*     getWeCLabelImgETCShader();

private:
    class WeCFontConfig
    {
    public:
        WeCFontConfig(void);
        ~WeCFontConfig(void);

    public:
        std::string                     fontName; // Load TTF, if empty, load system font.
        int                             fontSize;
        float                           outline;
    };

    typedef std::unordered_map<std::string, WeCFontConfig> WeCFontConfigList;

private:
    // ConfigList
    WeCFontConfigList                   mConfigs;
    // Collected characters in each label.
    CharCodeRefWithConfigMap            mCollectedChars;
    // Font atlas texture;
    TextureList                         mFontTextures;
    // Cached texture data;
    WeCFontTextureInfoList              mCachedTextureData;
    // Line info data with config.
    WeCFontAtlasLineInfoMap             mAtlasLineConfigMap;
    // Collect the information of characters which has already been processed.
    WeCChartFontDefinitionListMap       mChars;
    // Collect the generator of fonts.
    WeCFontFreeTypeMap                  mFreeTypeMap;
    // Event before camera draw.
    EventListenerCustom                 *mPreCameraDraw;
    EventListenerCustom                 *mAfterDraw;
    EventListenerCustom                 *mAfterVisit;

    /** The unordered map of the WeCLabel. */
    WeCLabelMap                         mWeCLabelMap;

    // Because the stroker cause the program copy the data.
    // In most time, copy is not needed, and use this cache
    // to avoid memory allocate and deallocate multiple times.
    // Should not use a big cache, that will cause the program
    // use a lot of memory.
    unsigned char*                      mStrokerReusedCache;
    int                                 mStrokerReusedCacheSize;

    // Flag that control the times of clearing in one frame.
    bool mHasNoMem;
    int  mClearMemLimit;

private:
#ifndef CC_STUDIO_ENABLED_VIEW
    static bool                         _parseFontsAndroid(const std::string &path, WeCSystemFontInfoMap & fontInfoMap);
#endif

public:
    static std::string                  generateKey(const std::string &fontName, int fontSize, float outline, bool ignoreScale = false);

public:
    static WeCCharFontManager           *sInstance;
    static WeCSystemFontInfoMap         sfontInfoMap;

    static std::string                  getSystemFontsByCode(unsigned short code);

    static cocos2d::Mat4                sProjectionMat;
    // WeCShaders
    static GLProgramState               *sWecLabelProgram;
    static GLProgramState               *sWecLabelImgProgram;
	static GLProgramState               *sWecLabelImgETCProgram;

    static int                          sAtlasWidth;
    static int                          sAtlasHeight;
    static int                          sAtlasLineOffset;
    static bool                         sHasInit;

    static float                        sFontScale;

    static unsigned int                 sWeCLabelId;

    static void                         emplaceWeCLabel(unsigned int id, cocos2d::ui::WeCLabel *label);
    static void                         eraseWeCLabel(unsigned int id);

    /** Generate the id of the WeCLabel. */
    static unsigned int                 genWeCLabelId();
};

NS_CC_END

#endif  // WECCHARTFONTMANAGER_H_
