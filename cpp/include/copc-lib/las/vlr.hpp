#ifndef COPCLIB_LAS_VLR_H_
#define COPCLIB_LAS_VLR_H_

#include <cstring>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <lazperf/vlr.hpp>

namespace copc::las
{

using WktVlr = lazperf::wkt_vlr;
using EbVlr = lazperf::eb_vlr;

static const int VLR_HEADER_SIZE = 54;
static const int EVLR_HEADER_SIZE = 60;

// Gets the sum of the byte size the extra bytes will take up, for calculating point_record_len
int NumBytesFromExtraBytes(const std::vector<EbVlr::ebfield> &items);
uint8_t FieldToByteLength(const copc::las::EbVlr::ebfield &field);

std::size_t EbVlrItemToPosition(const EbVlr &vlr, const std::string &name);

template <typename T> void CheckIfNoMismatch(const copc::las::EbVlr &extra_bytes_vlr, const std::string &name)
{
    auto result = std::find_if(extra_bytes_vlr.items.begin(), extra_bytes_vlr.items.end(),
                               [&name](const EbVlr::ebfield &item) { return item.name == name; });
    if (result == extra_bytes_vlr.items.end())
    {
        throw std::runtime_error("No extra bytes field with that name found");
    }

    auto item = *result;
    if (FieldToByteLength(item) != sizeof(T))
    {
        std::stringstream ss;
        ss << "Data size " << sizeof(T) << " and data_type " << int(FieldToByteLength(item)) << " mismatch";
        throw std::runtime_error(ss.str());
    }
}

class VlrHeader : public lazperf::evlr_header
{
  public:
    bool evlr_flag{false};

    VlrHeader() = default;
    VlrHeader(const lazperf::evlr_header &evlr_header) : evlr_flag(true), lazperf::evlr_header(evlr_header) {};
    VlrHeader(const lazperf::vlr_header &vlr_header);
    VlrHeader(const VlrHeader &vlr_header);

    lazperf::vlr_header ToLazperfVlrHeader() const;
    lazperf::evlr_header ToLazperfEvlrHeader() const;
};

} // namespace copc::las

namespace lazperf
{
// Equality operations
inline bool operator==(const eb_vlr::ebfield &a, const eb_vlr::ebfield &b)
{
    return std::memcmp(a.reserved, b.reserved, 2) == 0 && a.data_type == b.data_type && a.options == b.options &&
           a.name == b.name && std::memcmp(a.no_data, b.no_data, 3) == 0 && std::memcmp(a.minval, b.minval, 3) == 0 &&
           std::memcmp(a.maxval, b.maxval, 3) == 0 && std::memcmp(a.scale, b.scale, 3) == 0 &&
           std::memcmp(a.offset, b.offset, 3) == 0 && a.description == b.description;
}
} // namespace lazperf

#endif // COPCLIB_LAS_VLR_H_
