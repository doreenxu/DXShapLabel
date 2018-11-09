// automatically generated, do not modify

#include "flatbuffers/flatbuffers.h"

namespace flatbuffers {

struct WeCLabelOptions;

struct WeCLabelOptions : private flatbuffers::Table {
  const WidgetOptions *nodeOptions() const { return GetPointer<const WidgetOptions *>(4); }
  const flatbuffers::String *fontName() const { return GetPointer<const flatbuffers::String *>(6); }
  int32_t fontSize() const { return GetField<int32_t>(8, 0); }
  const Color *fontColor() const { return GetStruct<const Color *>(10); }
  int32_t horAlign() const { return GetField<int32_t>(12, 0); }
  int32_t verAlign() const { return GetField<int32_t>(14, 0); }
  int32_t overflow() const { return GetField<int32_t>(16, 0); }
  float spacingX() const { return GetField<float>(18, 0); }
  float spacingY() const { return GetField<float>(20, 0); }
  int32_t maxLines() const { return GetField<int32_t>(22, 0); }
  int32_t level() const { return GetField<int32_t>(24, 0); }
  uint8_t bbcode() const { return GetField<uint8_t>(26, 0); }
  int32_t effect() const { return GetField<int32_t>(28, 0); }
  const Color *effectColor() const { return GetStruct<const Color *>(30); }
  int32_t borderSize() const { return GetField<int32_t>(32, 0); }
  int32_t shadowOffsetX() const { return GetField<int32_t>(34, 0); }
  int32_t shadowOffsetY() const { return GetField<int32_t>(36, 0); }
  const flatbuffers::String *text() const { return GetPointer<const flatbuffers::String *>(38); }
};

struct WeCLabelOptionsBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_nodeOptions(flatbuffers::Offset<WidgetOptions> nodeOptions) { fbb_.AddOffset(4, nodeOptions); }
  void add_fontName(flatbuffers::Offset<flatbuffers::String> fontName) { fbb_.AddOffset(6, fontName); }
  void add_fontSize(int32_t fontSize) { fbb_.AddElement<int32_t>(8, fontSize, 0); }
  void add_fontColor(const Color *fontColor) { fbb_.AddStruct(10, fontColor); }
  void add_horAlign(int32_t horAlign) { fbb_.AddElement<int32_t>(12, horAlign, 0); }
  void add_verAlign(int32_t verAlign) { fbb_.AddElement<int32_t>(14, verAlign, 0); }
  void add_overflow(int32_t overflow) { fbb_.AddElement<int32_t>(16, overflow, 0); }
  void add_spacingX(float spacingX) { fbb_.AddElement<float>(18, spacingX, 0); }
  void add_spacingY(float spacingY) { fbb_.AddElement<float>(20, spacingY, 0); }
  void add_maxLines(int32_t maxLines) { fbb_.AddElement<int32_t>(22, maxLines, 0); }
  void add_level(int32_t level) { fbb_.AddElement<int32_t>(24, level, 0); }
  void add_bbcode(uint8_t bbcode) { fbb_.AddElement<uint8_t>(26, bbcode, 0); }
  void add_effect(int32_t effect) { fbb_.AddElement<int32_t>(28, effect, 0); }
  void add_effectColor(const Color *effectColor) { fbb_.AddStruct(30, effectColor); }
  void add_borderSize(int32_t borderSize) { fbb_.AddElement<int32_t>(32, borderSize, 0); }
  void add_shadowOffsetX(int32_t shadowOffsetX) { fbb_.AddElement<int32_t>(34, shadowOffsetX, 0); }
  void add_shadowOffsetY(int32_t shadowOffsetY) { fbb_.AddElement<int32_t>(36, shadowOffsetY, 0); }
  void add_text(flatbuffers::Offset<flatbuffers::String> text) { fbb_.AddOffset(38, text); }
  WeCLabelOptionsBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  flatbuffers::Offset<WeCLabelOptions> Finish() { return flatbuffers::Offset<WeCLabelOptions>(fbb_.EndTable(start_, 18)); }
};

inline flatbuffers::Offset<WeCLabelOptions> CreateWeCLabelOptions(flatbuffers::FlatBufferBuilder &_fbb, flatbuffers::Offset<WidgetOptions> nodeOptions, flatbuffers::Offset<flatbuffers::String> fontName, int32_t fontSize, const Color *fontColor, int32_t horAlign, int32_t verAlign, int32_t overflow, float spacingX, float spacingY, int32_t maxLines, int32_t level, uint8_t bbcode, int32_t effect, const Color *effectColor, int32_t borderSize, int32_t shadowOffsetX, int32_t shadowOffsetY, flatbuffers::Offset<flatbuffers::String> text) {
  WeCLabelOptionsBuilder builder_(_fbb);
  builder_.add_text(text);
  builder_.add_shadowOffsetY(shadowOffsetY);
  builder_.add_shadowOffsetX(shadowOffsetX);
  builder_.add_borderSize(borderSize);
  builder_.add_effectColor(effectColor);
  builder_.add_effect(effect);
  builder_.add_level(level);
  builder_.add_maxLines(maxLines);
  builder_.add_spacingY(spacingY);
  builder_.add_spacingX(spacingX);
  builder_.add_overflow(overflow);
  builder_.add_verAlign(verAlign);
  builder_.add_horAlign(horAlign);
  builder_.add_fontColor(fontColor);
  builder_.add_fontSize(fontSize);
  builder_.add_fontName(fontName);
  builder_.add_nodeOptions(nodeOptions);
  builder_.add_bbcode(bbcode);
  return builder_.Finish();
}

}; // namespace flatbuffers
