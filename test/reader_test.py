from sys import float_info

import copclib as copc
import pytest

from .utils import get_autzen_file


def test_reader():
    # Given a valid file path
    reader = copc.FileReader(get_autzen_file())

    # Given an invalid file path
    with pytest.raises(RuntimeError):
        assert copc.FileReader("invalid_path/non_existant_file.copc.laz")

    # LasHeader Test
    header = reader.copc_config.las_header
    assert header.point_format_id == 7
    assert header.point_count == 10653336
    assert header.point_record_length == 36
    assert header.EbByteSize() == 0

    # GetCopcInfo Test
    copc_info = reader.copc_config.copc_info
    assert copc_info.center_x == pytest.approx(637905.5448, 0.0001)
    assert copc_info.center_y == pytest.approx(851209.9048, 0.0001)
    assert copc_info.center_z == pytest.approx(2733.8948, 0.0001)
    assert copc_info.halfsize == pytest.approx(2327.7548, 0.0001)
    assert copc_info.spacing == pytest.approx(36.3711, 0.0001)
    assert copc_info.root_hier_offset == 73017045
    assert copc_info.root_hier_size == 8896

    # WKT Test
    wkt = reader.copc_config.wkt
    assert len(wkt) == 0


def test_find_key():
    # Given a valid file path
    reader = copc.FileReader(get_autzen_file())

    key = copc.VoxelKey.RootKey()
    hier_entry = reader.FindNode(key)

    assert hier_entry.IsValid() == True
    assert hier_entry.key == key
    assert hier_entry.point_count == 60978

    key = (5, 9, 7, 0)  # Test implicit conversion of key to tuple
    hier_entry = reader.FindNode(key)

    assert hier_entry.IsValid() == True
    assert hier_entry.key == key
    assert hier_entry.point_count == 12021


def test_get_all_children():
    reader = copc.FileReader(get_autzen_file())

    # Get root key
    nodes = reader.GetAllNodes()
    assert len(nodes) == 278

    # Get an invalid key
    nodes = reader.GetAllChildrenOfPage(copc.VoxelKey.InvalidKey())
    assert len(nodes) == 0

    # Get an existing key
    nodes = reader.GetAllChildrenOfPage((5, 9, 7, 0))
    assert len(nodes) == 1
    assert nodes[0].IsValid()
    assert nodes[0].key == (5, 9, 7, 0)  # Test implicit conversion of key to tuple

    # Get a non-existing key
    nodes = reader.GetAllChildrenOfPage((20, 20, 20, 20))
    assert len(nodes) == 0


def test_get_all_page_keys():
    reader = copc.FileReader(get_autzen_file())
    assert len(reader.GetPageList()) == 1


# TODO[Leo]: Make this test optional

# def test_get_all_points():
#     reader = copc.FileReader(get_autzen_file())
#     assert len(reader.GetAllPoints()) == reader.las_header.point_count


def test_point_error_handling():
    reader = copc.FileReader(get_autzen_file())

    invalid_node = copc.Node()
    with pytest.raises(RuntimeError):
        reader.GetPointData(node=invalid_node)
    valid_node = reader.FindNode((5, 9, 7, 0))
    reader.GetPointData(valid_node)

    assert len(reader.GetPointData(key=invalid_node.key)) == 0
    assert len(reader.GetPointData(valid_node.key)) != 0

    with pytest.raises(RuntimeError):
        reader.GetPoints(invalid_node)
    reader.GetPoints(valid_node)

    assert len(reader.GetPoints(invalid_node.key)) == 0
    assert len(reader.GetPoints(valid_node.key)) != 0

    with pytest.raises(RuntimeError):
        reader.GetPointDataCompressed(invalid_node)
    reader.GetPointDataCompressed(valid_node)


def test_spatial_query_functions():
    reader = copc.FileReader(get_autzen_file())

    # Make horizontal 2D box of [400,400] roughly in the middle of the point cloud.
    middle = (reader.copc_config.las_header.max + reader.copc_config.las_header.min) / 2
    middle_box = (middle.x - 200, middle.y - 200, middle.x + 200, middle.y + 200)

    # GetNodesWithinBox

    ## Check that no nodes fit in a zero-sized box
    subset_nodes = reader.GetNodesWithinBox(copc.Box.EmptyBox())
    assert len(subset_nodes) == 0

    ## Check that all nodes fit in a max-sized box
    subset_nodes = reader.GetNodesWithinBox(copc.Box.MaxBox())
    all_nodes = reader.GetAllNodes()
    assert len(subset_nodes) == len(all_nodes)

    # GetNodesIntersectBox

    subset_nodes = reader.GetNodesIntersectBox(middle_box)
    assert len(subset_nodes) == 13

    # GetPointsWithinBox

    ## Check that no points fit in a zero-sized box
    subset_points = reader.GetPointsWithinBox(copc.Box.EmptyBox())
    assert len(subset_points) == 0

    # TODO[Leo]: Make this test optional
    ## Check that all points fit in a box sized from header
    # header = reader.las_header
    # subset_points = reader.GetPointsWithinBox(
    #     copc.Box(
    #         math.floor(header.min.x),
    #         math.floor(header.min.y),
    #         math.floor(header.min.z),
    #         math.ceil(header.max.x),
    #         math.ceil(header.max.y),
    #         math.ceil(header.max.z),
    #     )
    # )
    # assert len(subset_points) == header.point_count

    subset_points = reader.GetPointsWithinBox(middle_box)
    assert len(subset_points) == 91178

    # GetDepthAtResolution
    assert reader.GetDepthAtResolution(3) == 4
    assert reader.GetDepthAtResolution(0) == 5
    assert reader.GetDepthAtResolution(float_info.min) == 5
    assert reader.GetDepthAtResolution(float_info.max) == 0

    # GetMaxDepth
    assert reader.GetMaxDepth() == 5

    # GetNodesAtResolution
    subset_nodes = reader.GetNodesAtResolution(3)
    assert len(subset_nodes) == 192
    for node in reader.GetNodesAtResolution(0):
        assert node.key.d == 5

    # GetNodesWithinResolution
    subset_nodes = reader.GetNodesWithinResolution(3)
    assert len(subset_nodes) == 257
    assert len(reader.GetNodesWithinResolution(0)) == len(reader.GetAllNodes())
