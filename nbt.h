/**
 * Author Root (Discord: stdio.h#5797; GitHub: 0x526f6f74)
 **/

#pragma once

#include <istream>
#include <map>
#include <optional>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

namespace nbt
{

enum TagType
{
    TAG_END,
    TAG_BYTE,
    TAG_SHORT,
    TAG_INT,
    TAG_LONG,
    TAG_FLOAT,
    TAG_DOUBLE,
    TAG_BYTE_ARRAY,
    TAG_STRING,
    TAG_LIST,
    TAG_COMPOUND,
    TAG_INT_ARRAY,
    TAG_LONG_ARRAY
};

using TagEnd = std::nullptr_t;

using TagByte = std::int8_t;
using TagShort = std::int16_t;
using TagInt = std::int32_t;
using TagLong = std::int64_t;

using TagFloat = float;
using TagDouble = double;

using TagString = std::string;

using TagByteArray = std::vector<TagByte>;
using TagIntArray = std::vector<TagInt>;
using TagLongArray = std::vector<TagLong>;

class Tag;
class TagList;

using TagCompound = std::map<TagString, Tag>;

namespace detail
{

template<typename T>
concept Is_tag_end = std::same_as<T, TagEnd> || std::same_as<T, TagEnd&>;

template<typename T>
concept Is_tag_numeric = std::integral<T> || std::floating_point<T>;

template<typename T>
concept Is_tag_string = std::same_as<T, TagString>;

template<typename T>
concept Is_tag_array = std::same_as<T, TagByteArray> || std::same_as<T, TagIntArray> || std::same_as<T, TagLongArray>;

template<typename T>
concept Is_tag_list = std::same_as<T, TagList>;

template<typename T>
concept Is_tag_compound = std::same_as<T, TagCompound>;

template<typename T>
concept Is_tag_or_tag_list = std::same_as<T, Tag> || Is_tag_list<T>;

template<typename T>
concept Is_tag_type = Is_tag_end<T> || Is_tag_numeric<T> || Is_tag_string<T> || Is_tag_array<T> || Is_tag_list<T> || Is_tag_compound<T>;

void encode(std::ostream& os, Is_tag_end auto = nullptr);

void encode(std::ostream& os, Is_tag_numeric auto value);

void encode(std::ostream& os, const Is_tag_string auto& string);

void encode(std::ostream& os, const Is_tag_array auto& vector);

void encode(std::ostream& os, const Is_tag_or_tag_list auto& value);

void encode(std::ostream& os, const Is_tag_compound auto& compound);

template<Is_tag_end T>
T decode(std::istream& is);

template<Is_tag_numeric T>
T decode(std::istream& is);

template<Is_tag_string T>
T decode(std::istream& is);

template<Is_tag_array T>
T decode(std::istream& is);

template<Is_tag_list T>
T decode(std::istream& is);

template<Is_tag_compound T>
T decode(std::istream& is);

}  // namespace detail

class TagList : public std::variant<
                    std::vector<TagEnd>,
                    std::vector<TagByte>,
                    std::vector<TagShort>,
                    std::vector<TagInt>,
                    std::vector<TagLong>,
                    std::vector<TagFloat>,
                    std::vector<TagDouble>,
                    std::vector<TagByteArray>,
                    std::vector<TagString>,
                    std::vector<TagList>,
                    std::vector<TagCompound>,
                    std::vector<TagIntArray>,
                    std::vector<TagLongArray>>
{
public:
    using variant::variant;

    TagList(std::istream& is);

    void encode(std::ostream& os) const;
    void decode(std::istream& is);

private:
    template<typename T>
    std::vector<T> decode_vector(std::istream& is) requires detail::Is_tag_type<T>;
};

class Tag
  : public std::
        variant<TagEnd, TagByte, TagShort, TagInt, TagLong, TagFloat, TagDouble, TagByteArray, TagString, TagList, TagCompound, TagIntArray, TagLongArray>
{
public:
    using variant::variant;

    Tag(std::istream& is, TagType type);

    template<typename T>
    const T& as() requires detail::Is_tag_type<T>
    {
        return std::get<T>(*this);
    }

    template<typename T>
    const std::vector<T>& as_list_of() requires detail::Is_tag_type<T>
    {
        return std::get<std::vector<T>>(*this);
    }

    void encode(std::ostream& os) const;
    void decode(std::istream& is, TagType type);
};

struct NBTData
{
    TagString name;
    std::variant<TagList, TagCompound> tags;
};

class NBT
{
public:
    NBT(std::istream& is);
    NBT(std::istream&& is);

    void encode(std::ostream& os) const;
    void encode(std::ostream&& os) const;
    void decode(std::istream& is);
    void decode(std::istream&& is);

    const TagString& name() const;

    template<typename T>
    Tag& operator[](int index) requires detail::Is_tag_type<T>;

    Tag& operator[](const TagString& key);

private:
    std::optional<NBTData> data;
};

}  // namespace nbt
