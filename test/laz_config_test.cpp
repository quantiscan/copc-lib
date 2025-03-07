#include <catch2/catch_all.hpp>
#include <copc-lib/copc/copc_config.hpp>
#include <copc-lib/geometry/vector3.hpp>
#include <copc-lib/las/header.hpp>
#include <copc-lib/las/laz_config.hpp>
#include <copc-lib/las/utils.hpp>
#include <copc-lib/las/vlr.hpp>
#include <string>

using namespace copc;

TEST_CASE("LazConfig", "[LazConfig]")
{

    int8_t point_format_id = 7;
    int num_eb_items = 1;
    Vector3 test_scale(1, 1, 1);
    Vector3 test_offset(50, 50, 50);
    std::string test_wkt = "test_wkt";

    las::EbVlr test_extra_bytes_vlr;
    test_extra_bytes_vlr.addField(
        []()
        {
            auto field = lazperf::eb_vlr::ebfield();
            field.data_type = 0;
            field.options = 4;
            field.name = "eb1";
            return field;
        }());

    std::string wkt(test_wkt);

    las::LasHeader header(point_format_id, las::PointBaseByteSize(point_format_id) + test_extra_bytes_vlr.size(),
                          test_scale, test_offset, true);

    las::LazConfig cfg(header, wkt, test_extra_bytes_vlr);

    REQUIRE(cfg.LasHeader().PointFormatId() == point_format_id);
    REQUIRE(cfg.LasHeader().Scale() == test_scale);
    REQUIRE(cfg.LasHeader().Offset() == test_offset);

    REQUIRE(cfg.Wkt() == test_wkt);

    REQUIRE(cfg.ExtraBytesVlr().items[0].name == test_extra_bytes_vlr.items[0].name);
}

TEST_CASE("CopcConfig_To_LazConfig")
{
    int8_t point_format_id = 7;
    int num_eb_items = 1;
    double test_spacing = 12;
    double test_intensity_min = 5;
    double test_intensity_max = 155;
    Vector3 test_scale(1, 1, 1);
    Vector3 test_offset(50, 50, 50);
    std::string test_wkt = "test_wkt";

    las::EbVlr test_extra_bytes_vlr;
    test_extra_bytes_vlr.addField(
        []()
        {
            auto field = lazperf::eb_vlr::ebfield();
            field.data_type = 0;
            field.options = 4;
            field.name = "eb1";
            return field;
        }());

    CopcInfo copc_info;
    copc_info.spacing = test_spacing;

    std::string wkt(test_wkt);

    las::LasHeader header(point_format_id, las::PointBaseByteSize(point_format_id) + test_extra_bytes_vlr.size(),
                          test_scale, test_offset, true);

    CopcConfig copc_cfg(header, copc_info, wkt, test_extra_bytes_vlr);
    las::LazConfigWriter cfg(copc_cfg);

    REQUIRE(cfg.LasHeader()->PointFormatId() == point_format_id);
    REQUIRE(cfg.LasHeader()->Scale() == test_scale);
    REQUIRE(cfg.LasHeader()->Offset() == test_offset);

    REQUIRE(cfg.Wkt() == test_wkt);

    REQUIRE(cfg.ExtraBytesVlr().items[0].name == test_extra_bytes_vlr.items[0].name);

    std::cout << copc_cfg.LasHeader().min.ToString();
    REQUIRE(copc_cfg.LasHeader().min == Vector3(0, 0, 0));
    REQUIRE(copc_cfg.LasHeader().max == Vector3(0, 0, 0));
    Vector3 test_min(3, 4, 5);
    Vector3 test_max(30, 43, 33);
    cfg.LasHeader()->min = test_min;
    cfg.LasHeader()->max = test_max;
    REQUIRE(copc_cfg.LasHeader().min == Vector3(0, 0, 0));
    REQUIRE(copc_cfg.LasHeader().max == Vector3(0, 0, 0));
    REQUIRE(cfg.LasHeader()->min == test_min);
    REQUIRE(cfg.LasHeader()->max == test_max);
}

TEST_CASE("LazConfigWriter", "[LazConfigWriter]")
{
    int8_t point_format_id = 7;
    int num_eb_items = 1;
    Vector3 test_scale(1, 1, 1);
    Vector3 test_offset(50, 50, 50);
    Vector3 test_min(-5, -6, -7);
    Vector3 test_max(5, 6, 7);
    std::string test_wkt = "test_wkt";

    las::EbVlr test_extra_bytes_vlr;
    test_extra_bytes_vlr.addField(
        []()
        {
            auto field = lazperf::eb_vlr::ebfield();
            field.data_type = 0;
            field.options = 4;
            field.name = "eb1";
            return field;
        }());

    SECTION("Constructor with default arguments")
    {
        las::LazConfigWriter cfg(point_format_id);

        REQUIRE(cfg.LasHeader()->PointFormatId() == point_format_id);
        REQUIRE(cfg.LasHeader()->Scale() == Vector3::DefaultScale());

        REQUIRE(cfg.Wkt().empty());

        REQUIRE(cfg.ExtraBytesVlr().items.empty());
    }

    SECTION("Constructor with specified arguments")
    {
        las::LazConfigWriter cfg(point_format_id, test_scale, test_offset, test_wkt, test_extra_bytes_vlr);

        REQUIRE(cfg.LasHeader()->PointFormatId() == point_format_id);
        REQUIRE(cfg.LasHeader()->Scale() == test_scale);
        REQUIRE(cfg.LasHeader()->Offset() == test_offset);

        REQUIRE(cfg.Wkt() == test_wkt);

        REQUIRE(cfg.ExtraBytesVlr().items[0].name == test_extra_bytes_vlr.items[0].name);
    }

    SECTION("Updating Config Values")
    {
        las::LazConfigWriter cfg(point_format_id, test_scale, test_offset, test_wkt, test_extra_bytes_vlr);

        // Las Header
        cfg.LasHeader()->min = test_min;
        cfg.LasHeader()->max = test_max;
        REQUIRE(cfg.LasHeader()->min == test_min);
        REQUIRE(cfg.LasHeader()->max == test_max);
    }
}
