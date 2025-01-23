#include "copc-lib/las/vlr.hpp"
#include "copc-lib/las/utils.hpp"

#include <numeric>

namespace copc::las
{

std::array<uint8_t, 31> EXTRA_BYTE_DATA_TYPE{0, 1,  1,  2, 2,  4, 4, 8, 8, 4,  8,  2,  2,  4,  4, 8,
                                             8, 16, 16, 8, 16, 3, 3, 6, 6, 12, 12, 24, 24, 12, 24};

uint8_t FieldToByteLength(const copc::las::EbVlr::ebfield &field)
{
    if (field.data_type == 0)
    {
        return field.options;
    }

    return EXTRA_BYTE_DATA_TYPE.at(field.data_type);
}

int NumBytesFromExtraBytes(const std::vector<EbVlr::ebfield> &items)
{
    return std::accumulate(items.begin(), items.end(), 0, [](const std::size_t &input, const EbVlr::ebfield field)
                           { return input + FieldToByteLength(field); });
}

std::size_t EbVlrItemToPosition(const EbVlr &vlr, const std::string &name)
{
    std::size_t i = 0;
    for (const auto &item : vlr.items)
    {
        if (item.name == name)
        {
            return i;
        }

        i += FieldToByteLength(item);
    }

    throw std::runtime_error("Name not found in VLR");
}

VlrHeader::VlrHeader(const lazperf::vlr_header &vlr_header) : evlr_flag(false)
{
    reserved = vlr_header.reserved;
    user_id = vlr_header.user_id;
    record_id = vlr_header.record_id;
    data_length = vlr_header.data_length;
    description = vlr_header.description;
}

VlrHeader::VlrHeader(const VlrHeader &vlr_header) : evlr_header(vlr_header) { evlr_flag = vlr_header.evlr_flag; }

lazperf::vlr_header VlrHeader::ToLazperfVlrHeader() const
{
    lazperf::vlr_header header;

    header.reserved = reserved;
    header.user_id = user_id;
    header.record_id = record_id;
    header.data_length = data_length;
    header.description = description;

    return header;
}
lazperf::evlr_header VlrHeader::ToLazperfEvlrHeader() const
{
    lazperf::evlr_header header;

    header.reserved = reserved;
    header.user_id = user_id;
    header.record_id = record_id;
    header.data_length = data_length;
    header.description = description;

    return header;
}

} // namespace copc::las
