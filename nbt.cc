#include "nbt.h"

namespace nbt
{

namespace detail
{

void encode(std::ostream& os, Is_tag_end auto)
{
    os.put(TAG_END);
}

void encode(std::ostream& os, Is_tag_numeric auto value)
{
    os.write(reinterpret_cast<char*>(&value), sizeof(value));
}

void encode(std::ostream& os, const Is_tag_string auto& string)
{
    encode<TagShort>(os, string.length());
    os.write(string.data(), string.length());
}

void encode(std::ostream& os, const Is_tag_array auto& vector)
{
    encode<TagInt>(os, vector.size());
    for (auto element : vector)
        encode(os, element);
}

void encode(std::ostream& os, const Is_tag_or_tag_list auto& value)
{
    value.encode(os);
}

void encode(std::ostream& os, const Is_tag_compound auto& compound)
{
    for (const auto& [key, tag] : compound)
    {
        encode<TagByte>(os, tag.index());
        encode(os, key);
        encode(os, tag);
    }
    encode<TagEnd>(os);
}

template<Is_tag_end T>
T decode(std::istream& is)
{
    is.ignore();
    return {};
}

template<Is_tag_numeric T>
T decode(std::istream& is)
{
    T value;
    is.read(reinterpret_cast<char*>(&value), sizeof(value));
    return value;
}

template<Is_tag_string T>
T decode(std::istream& is)
{
    auto len = decode<TagShort>(is);
    TagString string(len, '\0');
    is.read(string.data(), len);
    return string;
}

template<Is_tag_array T>
T decode(std::istream& is)
{
    T vector(decode<TagInt>(is));
    for (auto& element : vector)
        element = decode<typename T::value_type>(is);
    return vector;
}

template<Is_tag_list T>
T decode(std::istream& is)
{
    return TagList {is};
}

template<Is_tag_compound T>
T decode(std::istream& is)
{
    TagCompound compound;
    for (auto type = decode<TagByte>(is); type != TAG_END; type = decode<TagByte>(is))
    {
        auto key = decode<TagString>(is);
        compound[key] = Tag {is, static_cast<TagType>(type)};
    }
    return compound;
}

}  // namespace detail

invalid_tag_type::invalid_tag_type() noexcept
{ }

const char* invalid_tag_type::what() const noexcept
{
    return this->reason;
}

TagList::TagList(std::istream& is)
{
    this->decode(is);
}

void TagList::encode(std::ostream& os) const
{
    detail::encode<TagByte>(os, this->index());
    const auto encoder = [&](const auto& vector)
    {
        detail::encode<TagInt>(os, vector.size());
        for (const auto& value : vector)
            detail::encode(os, value);
    };
    std::visit(encoder, *static_cast<const variant*>(this));
}

void TagList::decode(std::istream& is)
{
    switch (detail::decode<TagByte>(is))
    {
        case TAG_END: *this = this->decode_vector<TagEnd>(is); break;
        case TAG_BYTE: *this = this->decode_vector<TagByte>(is); break;
        case TAG_SHORT: *this = this->decode_vector<TagShort>(is); break;
        case TAG_INT: *this = this->decode_vector<TagInt>(is); break;
        case TAG_LONG: *this = this->decode_vector<TagLong>(is); break;
        case TAG_FLOAT: *this = this->decode_vector<TagFloat>(is); break;
        case TAG_DOUBLE: *this = this->decode_vector<TagDouble>(is); break;
        case TAG_BYTE_ARRAY: *this = this->decode_vector<TagByteArray>(is); break;
        case TAG_STRING: *this = this->decode_vector<TagString>(is); break;
        case TAG_LIST: *this = this->decode_vector<TagList>(is); break;
        case TAG_COMPOUND: *this = this->decode_vector<TagCompound>(is); break;
        case TAG_INT_ARRAY: *this = this->decode_vector<TagIntArray>(is); break;
        case TAG_LONG_ARRAY: *this = this->decode_vector<TagLongArray>(is); break;
        default: throw invalid_tag_type {};
    }
}

template<typename T>
const std::vector<T>& TagList::data() const requires detail::Is_tag_type<T>
{
    return std::get<std::vector<T>>(*this);
}

template const std::vector<TagEnd>& TagList::data() const;
template const std::vector<TagByte>& TagList::data() const;
template const std::vector<TagShort>& TagList::data() const;
template const std::vector<TagInt>& TagList::data() const;
template const std::vector<TagLong>& TagList::data() const;
template const std::vector<TagFloat>& TagList::data() const;
template const std::vector<TagDouble>& TagList::data() const;
template const std::vector<TagByteArray>& TagList::data() const;
template const std::vector<TagString>& TagList::data() const;
template const std::vector<TagList>& TagList::data() const;
template const std::vector<TagCompound>& TagList::data() const;
template const std::vector<TagIntArray>& TagList::data() const;
template const std::vector<TagLongArray>& TagList::data() const;

template<typename T>
std::vector<T> TagList::decode_vector(std::istream& is) requires detail::Is_tag_type<T>
{
    std::vector<T> vector(detail::decode<TagInt>(is));
    for (auto& element : vector)
        element = detail::decode<T>(is);
    return vector;
}

Tag::Tag(std::istream& is, TagType type)
{
    this->decode(is, type);
}

void Tag::encode(std::ostream& os) const
{
    const auto encoder = [&](const auto& value) { detail::encode(os, value); };
    std::visit(encoder, *static_cast<const variant*>(this));
}

void Tag::decode(std::istream& is, TagType type)
{
    switch (type)
    {
        case TAG_END: *this = detail::decode<TagEnd>(is); break;
        case TAG_BYTE: *this = detail::decode<TagByte>(is); break;
        case TAG_SHORT: *this = detail::decode<TagShort>(is); break;
        case TAG_INT: *this = detail::decode<TagInt>(is); break;
        case TAG_LONG: *this = detail::decode<TagLong>(is); break;
        case TAG_FLOAT: *this = detail::decode<TagFloat>(is); break;
        case TAG_DOUBLE: *this = detail::decode<TagDouble>(is); break;
        case TAG_BYTE_ARRAY: *this = detail::decode<TagByteArray>(is); break;
        case TAG_STRING: *this = detail::decode<TagString>(is); break;
        case TAG_LIST: *this = detail::decode<TagList>(is); break;
        case TAG_COMPOUND: *this = detail::decode<TagCompound>(is); break;
        case TAG_INT_ARRAY: *this = detail::decode<TagIntArray>(is); break;
        case TAG_LONG_ARRAY: *this = detail::decode<TagLongArray>(is); break;
        default: throw invalid_tag_type {};
    }
}

Tag& Tag::operator[](const TagString& key)
{
    return std::get<TagCompound>(*this)[key];
}

template<typename T>
const std::vector<T>& Tag::data() const requires detail::Is_tag_type<T>
{
    return std::get<std::vector<T>>(std::get<TagList>(*this));
}

template const std::vector<TagEnd>& Tag::data() const;
template const std::vector<TagByte>& Tag::data() const;
template const std::vector<TagShort>& Tag::data() const;
template const std::vector<TagInt>& Tag::data() const;
template const std::vector<TagLong>& Tag::data() const;
template const std::vector<TagFloat>& Tag::data() const;
template const std::vector<TagDouble>& Tag::data() const;
template const std::vector<TagByteArray>& Tag::data() const;
template const std::vector<TagString>& Tag::data() const;
template const std::vector<TagList>& Tag::data() const;
template const std::vector<TagCompound>& Tag::data() const;
template const std::vector<TagIntArray>& Tag::data() const;
template const std::vector<TagLongArray>& Tag::data() const;

template<typename T>
T& Tag::at(int index) requires detail::Is_tag_type<T>
{
    return std::get<std::vector<T>>(std::get<TagList>(*this))[index];
}

template TagEnd& Tag::at(int index);
template TagByte& Tag::at(int index);
template TagShort& Tag::at(int index);
template TagInt& Tag::at(int index);
template TagLong& Tag::at(int index);
template TagFloat& Tag::at(int index);
template TagDouble& Tag::at(int index);
template TagByteArray& Tag::at(int index);
template TagString& Tag::at(int index);
template TagList& Tag::at(int index);
template TagCompound& Tag::at(int index);
template TagIntArray& Tag::at(int index);
template TagLongArray& Tag::at(int index);

NBT::NBT(std::istream& is)
{
    this->decode(is);
}

NBT::NBT(std::istream&& is)
{
    this->decode(is);
}

void NBT::encode(std::ostream& os) const
{
    if (this->data_)
    {
        if (const auto tags = std::get_if<TagCompound>(&this->data_->tags))
        {
            detail::encode<TagByte>(os, TAG_COMPOUND);
            detail::encode(os, this->data_->name);
            detail::encode(os, *tags);
        }
        else if (const auto tags = std::get_if<TagList>(&this->data_->tags))
        {
            detail::encode<TagByte>(os, TAG_LIST);
            detail::encode(os, this->data_->name);
            detail::encode(os, *tags);
        }
        else
            throw invalid_tag_type {};
    }
    else
        detail::encode<TagEnd>(os);
}

void NBT::encode(std::ostream&& os) const
{
    this->encode(os);
}

void NBT::decode(std::istream& is)
{
    const auto type = detail::decode<TagByte>(is);

    if (type == TAG_COMPOUND)
        this->data_ = Data {detail::decode<TagString>(is), detail::decode<TagCompound>(is)};
    else if (type == TAG_LIST)
        this->data_ = Data {detail::decode<TagString>(is), detail::decode<TagList>(is)};
    else
        throw invalid_tag_type {};
}

void NBT::decode(std::istream&& is)
{
    this->decode(is);
}

const TagString& NBT::name() const
{
    return this->data_->name;
}

std::optional<NBT::Data>& NBT::data()
{
    return this->data_;
}

Tag& NBT::operator[](const TagString& key)
{
    return std::get<TagCompound>(this->data_->tags)[key];
}

}  // namespace nbt
