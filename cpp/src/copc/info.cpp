#include "copc-lib/copc/info.hpp"

#include <sstream>

namespace copc
{
CopcInfo::CopcInfo(const lazperf::copc_info_vlr &copc_info_vlr)
{
    center_x = copc_info_vlr.center_x;
    center_y = copc_info_vlr.center_y;
    center_z = copc_info_vlr.center_z;
    halfsize = copc_info_vlr.halfsize;
    spacing = copc_info_vlr.spacing;
    root_hier_offset = copc_info_vlr.root_hier_offset;
    root_hier_size = copc_info_vlr.root_hier_size;
    gpstime_minimum = copc_info_vlr.gpstime_minimum;
    gpstime_maximum = copc_info_vlr.gpstime_maximum;
}

lazperf::copc_info_vlr CopcInfo::ToLazPerf() const
{
    lazperf::copc_info_vlr copc_info_vlr;

    copc_info_vlr.center_x = center_x;
    copc_info_vlr.center_y = center_y;
    copc_info_vlr.center_z = center_z;
    copc_info_vlr.halfsize = halfsize;
    copc_info_vlr.spacing = spacing;
    copc_info_vlr.gpstime_minimum = gpstime_minimum;
    copc_info_vlr.gpstime_maximum = gpstime_maximum;
    copc_info_vlr.root_hier_offset = root_hier_offset;
    copc_info_vlr.root_hier_size = root_hier_size;

    return copc_info_vlr;
}

std::string CopcInfo::ToString() const
{
    std::stringstream ss;
    ss << "CopcInfo:" << std::endl;
    ss << "\tcenter_x: " << center_x << std::endl;
    ss << "\tcenter_y: " << center_y << std::endl;
    ss << "\tcenter_z: " << center_z << std::endl;
    ss << "\thalfsize: " << halfsize << std::endl;
    ss << "\tspacing: " << spacing << std::endl;
    ss << "\troot_hier_offset: " << root_hier_offset << std::endl;
    ss << "\troot_hier_size: " << root_hier_size << std::endl;
    ss << "\tgpstime_minimum: " << gpstime_minimum << std::endl;
    ss << "\tgpstime_maximum: " << gpstime_maximum << std::endl;

    return ss.str();
}

} // namespace copc
