#include "data/test_data.h"
#include <catch2/catch_all.hpp>
#include <copc-lib/io/copc_reader.hpp>
#include <copc-lib/io/copc_writer.hpp>
#include <sstream>

using namespace copc;
using namespace std;

TEST_CASE("Writer Node Uncompressed", "[Writer]")
{
    SECTION("Add one")
    {
        stringstream out_stream;

        CopcConfigWriter cfg(7);
        Writer writer(out_stream, cfg);

        REQUIRE_THROWS(writer.AddNode(VoxelKey::InvalidKey(), std::vector<char>()));
        REQUIRE_THROWS(writer.AddNode(VoxelKey(0, 0, 0, 0), std::vector<char>()));

        std::vector<char> root_node(first_20_pts, first_20_pts + sizeof(first_20_pts));
        REQUIRE_NOTHROW(writer.AddNode(VoxelKey(0, 0, 0, 0), root_node));

        writer.Close();

        Reader reader(&out_stream);
        REQUIRE(reader.CopcConfig().CopcInfo().root_hier_offset > 0);
        REQUIRE(reader.CopcConfig().CopcInfo().root_hier_size == 32);

        auto node = reader.FindNode(VoxelKey::RootKey());
        REQUIRE(node.IsValid());
        auto root_node_test = reader.GetPointData(node);

        REQUIRE(root_node_test == root_node);
    }

    SECTION("Add multiple")
    {
        stringstream out_stream;

        CopcConfigWriter cfg(7);
        Writer writer(out_stream, cfg);

        std::vector<char> twenty(first_20_pts, first_20_pts + sizeof(first_20_pts));
        REQUIRE_NOTHROW(writer.AddNode(VoxelKey(0, 0, 0, 0), twenty));

        std::vector<char> twelve(next_12_pts, next_12_pts + sizeof(next_12_pts));
        REQUIRE_NOTHROW(writer.AddNode(VoxelKey(1, 1, 1, 1), twelve));

        std::vector<char> sixty(last_60_pts, last_60_pts + sizeof(last_60_pts));
        REQUIRE_NOTHROW(writer.AddNode(VoxelKey(1, 1, 1, 0), sixty));

        writer.Close();

        std::string ostr = out_stream.str();
        Reader reader(&out_stream);
        REQUIRE(reader.CopcConfig().CopcInfo().root_hier_offset > 0);
        REQUIRE(reader.CopcConfig().CopcInfo().root_hier_size == 32 * 3);

        {
            auto node = reader.FindNode(VoxelKey::RootKey());
            REQUIRE(node.IsValid());
            auto node_data = reader.GetPointData(node);
            REQUIRE(node_data == twenty);
        }

        {
            auto node = reader.FindNode(VoxelKey(1, 1, 1, 1));
            REQUIRE(node.IsValid());
            auto node_data = reader.GetPointData(node);
            REQUIRE(node_data == twelve);
        }

        {
            auto node = reader.FindNode(VoxelKey(1, 1, 1, 0));
            REQUIRE(node.IsValid());
            auto node_data = reader.GetPointData(node);
            REQUIRE(node_data == sixty);
        }
    }

    SECTION("Add hierarchy")
    {
        stringstream out_stream;

        CopcConfigWriter cfg(7);
        Writer writer(out_stream, cfg);

        VoxelKey sub_page1_key(1, 0, 0, 0);
        VoxelKey sub_page2_key(1, 1, 1, 1);

        std::vector<char> twenty(first_20_pts, first_20_pts + sizeof(first_20_pts));
        REQUIRE_NOTHROW(writer.AddNode(VoxelKey(0, 0, 0, 0), twenty));

        std::vector<char> twelve(next_12_pts, next_12_pts + sizeof(next_12_pts));
        REQUIRE_NOTHROW(writer.AddNode(VoxelKey(1, 0, 0, 0), twelve, sub_page1_key));

        std::vector<char> sixty(last_60_pts, last_60_pts + sizeof(last_60_pts));
        REQUIRE_NOTHROW(writer.AddNode(VoxelKey(1, 1, 1, 1), sixty, sub_page2_key));
        REQUIRE_NOTHROW(writer.AddNode(VoxelKey(2, 2, 2, 2), twenty, sub_page2_key));

        // Can't add a node that's not a child of the page
        REQUIRE_THROWS(writer.AddNode(VoxelKey(1, 2, 2, 2), twenty, sub_page2_key));

        writer.Close();

        /*
            hierarchy should look like this:
            Page 0-0-0-0
                Node 0-0-0-0
                Page 1-0-0-0
                    Node 1-0-0-0
                Page 1-1-1-1
                    Node 1-1-1-1
                    Node 2-2-2-2
        */

        Reader reader(&out_stream);
        REQUIRE(reader.CopcConfig().CopcInfo().root_hier_offset > 0);
        REQUIRE(reader.CopcConfig().CopcInfo().root_hier_size == 32 * 3);

        {
            auto sub_node = reader.FindNode(VoxelKey(2, 2, 2, 2));
            REQUIRE(sub_node.IsValid());
            auto sub_node_data = reader.GetPointData(sub_node);
            REQUIRE(sub_node_data == twenty);
        }
        {
            auto sub_node = reader.FindNode(VoxelKey(1, 1, 1, 1));
            REQUIRE(sub_node.IsValid());
            auto sub_node_data = reader.GetPointData(sub_node);
            REQUIRE(sub_node_data == sixty);
        }
        {
            auto sub_node = reader.FindNode(VoxelKey(1, 0, 0, 0));
            REQUIRE(sub_node.IsValid());
            auto sub_node_data = reader.GetPointData(sub_node);
            REQUIRE(sub_node_data == twelve);
        }
        {
            auto sub_node = reader.FindNode(VoxelKey(0, 0, 0, 0));
            REQUIRE(sub_node.IsValid());
            auto sub_node_data = reader.GetPointData(sub_node);
            REQUIRE(sub_node_data == twenty);
        }
    }
}

TEST_CASE("Writer Node Compressed", "[Writer]")
{
    SECTION("Add one")
    {
        stringstream out_stream;

        CopcConfigWriter cfg(7);
        Writer writer(out_stream, cfg);

        REQUIRE_THROWS(writer.AddNodeCompressed(VoxelKey::InvalidKey(), std::vector<char>(), 0));

        std::vector<char> root_node(first_20_pts_compressed, first_20_pts_compressed + sizeof(first_20_pts_compressed));
        REQUIRE_NOTHROW(writer.AddNodeCompressed(VoxelKey(0, 0, 0, 0), root_node, 20));

        writer.Close();

        Reader reader(&out_stream);
        REQUIRE(reader.CopcConfig().CopcInfo().root_hier_offset > 0);
        REQUIRE(reader.CopcConfig().CopcInfo().root_hier_size == 32);

        auto node = reader.FindNode(VoxelKey::RootKey());
        REQUIRE(node.IsValid());
        auto root_node_test = reader.GetPointData(node);

        REQUIRE(root_node_test == std::vector<char>(first_20_pts, first_20_pts + sizeof(first_20_pts)));
    }

    SECTION("Add multiple")
    {
        stringstream out_stream;

        CopcConfigWriter cfg(7);
        Writer writer(out_stream, cfg);

        std::vector<char> twenty(first_20_pts_compressed, first_20_pts_compressed + sizeof(first_20_pts_compressed));
        REQUIRE_NOTHROW(writer.AddNodeCompressed(VoxelKey(0, 0, 0, 0), twenty, 20));

        std::vector<char> twelve(next_12_pts_compressed, next_12_pts_compressed + sizeof(next_12_pts_compressed));
        REQUIRE_NOTHROW(writer.AddNodeCompressed(VoxelKey(1, 1, 1, 1), twelve, 12));

        std::vector<char> sixty(last_60_pts_compressed, last_60_pts_compressed + sizeof(last_60_pts_compressed));
        REQUIRE_NOTHROW(writer.AddNodeCompressed(VoxelKey(1, 1, 1, 0), sixty, 60));

        writer.Close();

        std::string ostr = out_stream.str();
        Reader reader(&out_stream);
        REQUIRE(reader.CopcConfig().CopcInfo().root_hier_offset > 0);
        REQUIRE(reader.CopcConfig().CopcInfo().root_hier_size == 32 * 3);

        {
            auto sub_node = reader.FindNode(VoxelKey::RootKey());
            REQUIRE(sub_node.IsValid());
            auto sub_node_data = reader.GetPointData(sub_node);
            REQUIRE(sub_node_data == std::vector<char>(first_20_pts, first_20_pts + sizeof(first_20_pts)));
        }

        {
            auto sub_node = reader.FindNode(VoxelKey(1, 1, 1, 1));
            REQUIRE(sub_node.IsValid());
            auto sub_node_data = reader.GetPointData(sub_node);
            REQUIRE(sub_node_data == std::vector<char>(next_12_pts, next_12_pts + sizeof(next_12_pts)));
        }

        {
            auto sub_node = reader.FindNode(VoxelKey(1, 1, 1, 0));
            REQUIRE(sub_node.IsValid());
            auto sub_node_data = reader.GetPointData(sub_node);
            REQUIRE(sub_node_data == std::vector<char>(last_60_pts, last_60_pts + sizeof(last_60_pts)));
        }
    }

    SECTION("Add hierarchy")
    {
        stringstream out_stream;

        CopcConfigWriter cfg(7);
        Writer writer(out_stream, cfg);

        VoxelKey sub_page1_key(1, 0, 0, 0);
        VoxelKey sub_page2_key(1, 1, 1, 1);

        std::vector<char> twenty(first_20_pts_compressed, first_20_pts_compressed + sizeof(first_20_pts_compressed));
        REQUIRE_NOTHROW(writer.AddNodeCompressed(VoxelKey(0, 0, 0, 0), twenty, 20));

        std::vector<char> twelve(next_12_pts_compressed, next_12_pts_compressed + sizeof(next_12_pts_compressed));
        REQUIRE_NOTHROW(writer.AddNodeCompressed(VoxelKey(1, 0, 0, 0), twelve, 12), sub_page1_key);

        std::vector<char> sixty(last_60_pts_compressed, last_60_pts_compressed + sizeof(last_60_pts_compressed));
        REQUIRE_NOTHROW(writer.AddNodeCompressed(VoxelKey(1, 1, 1, 1), sixty, 60, sub_page2_key));
        REQUIRE_NOTHROW(writer.AddNodeCompressed(VoxelKey(2, 2, 2, 2), twenty, 20, sub_page2_key));

        // Can't add a node that's not a child of the page
        REQUIRE_THROWS(writer.AddNodeCompressed(VoxelKey(1, 2, 2, 2), twenty, 20, sub_page2_key));

        writer.Close();

        /*
            hierarchy should look like this:
            Page 0-0-0-0
                Node 0-0-0-0
                Page 1-0-0-0
                    Node 1-0-0-0
                Page 1-1-1-1
                    Node 1-1-1-1
                    Node 2-2-2-2
        */

        Reader reader(&out_stream);
        REQUIRE(reader.CopcConfig().CopcInfo().root_hier_offset > 0);
        REQUIRE(reader.CopcConfig().CopcInfo().root_hier_size == 32 * 3);

        {
            auto sub_node = reader.FindNode(VoxelKey(2, 2, 2, 2));
            REQUIRE(sub_node.IsValid());
            auto sub_node_data = reader.GetPointData(sub_node);
            REQUIRE(sub_node_data == std::vector<char>(first_20_pts, first_20_pts + sizeof(first_20_pts)));
        }
        {
            auto sub_node = reader.FindNode(VoxelKey(1, 1, 1, 1));
            REQUIRE(sub_node.IsValid());
            auto sub_node_data = reader.GetPointData(sub_node);
            REQUIRE(sub_node_data == std::vector<char>(last_60_pts, last_60_pts + sizeof(last_60_pts)));
        }
        {
            auto sub_node = reader.FindNode(VoxelKey(1, 0, 0, 0));
            REQUIRE(sub_node.IsValid());
            auto sub_node_data = reader.GetPointData(sub_node);
            REQUIRE(sub_node_data == std::vector<char>(next_12_pts, next_12_pts + sizeof(next_12_pts)));
        }
        {
            auto sub_node = reader.FindNode(VoxelKey(0, 0, 0, 0));
            REQUIRE(sub_node.IsValid());
            auto sub_node_data = reader.GetPointData(sub_node);
            REQUIRE(sub_node_data == std::vector<char>(first_20_pts, first_20_pts + sizeof(first_20_pts)));
        }
    }
}
