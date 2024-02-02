#include <gtest/gtest.h>

#include <stub/stub.hpp>
#include <estimation.hpp>
namespace estimation {

static network::macaddress_t make_macaddress(uint32_t node_id, LocalLocation local_location, bool is_root) {
    // if node_id == 0 then is_root must be true
    assert((node_id & estimation::NODE_ID_MASK) == 0 && is_root == false);
    return (node_id << estimation::NODE_ID_POSITION)
           | (static_cast<uint8_t>(local_location) << estimation::LOCAL_LOCATION_POSITION)
           | (is_root << estimation::IS_ROOT_POSITION);
}

TEST(Estimation, local_location) {
    auto location = estimation::LocalLocation::UpperLeft;
    ASSERT_EQ(estimation::diagonal_location(location), estimation::LocalLocation::LowerRight);
    ASSERT_EQ(estimation::rotate_clockwise(location), estimation::LocalLocation::UpperRight);
    ASSERT_EQ(estimation::rotate_counterclockwise(location), estimation::LocalLocation::LowerLeft);
    estimation::coordinate_t coordinate = {0, 1};
    ASSERT_EQ(estimation::get_root_coordinate(location), coordinate);

    location = estimation::LocalLocation::UpperRight;
    ASSERT_EQ(estimation::diagonal_location(location), estimation::LocalLocation::LowerLeft);
    ASSERT_EQ(estimation::rotate_clockwise(location), estimation::LocalLocation::LowerRight);
    ASSERT_EQ(estimation::rotate_counterclockwise(location), estimation::LocalLocation::UpperLeft);
    coordinate = {1, 1};
    ASSERT_EQ(estimation::get_root_coordinate(location), coordinate);

    location = estimation::LocalLocation::LowerLeft;
    ASSERT_EQ(estimation::diagonal_location(location), estimation::LocalLocation::UpperRight);
    ASSERT_EQ(estimation::rotate_clockwise(location), estimation::LocalLocation::UpperLeft);
    ASSERT_EQ(estimation::rotate_counterclockwise(location), estimation::LocalLocation::LowerRight);
    coordinate = {0, 0};
    ASSERT_EQ(estimation::get_root_coordinate(location), coordinate);

    location = estimation::LocalLocation::LowerRight;
    ASSERT_EQ(estimation::diagonal_location(location), estimation::LocalLocation::UpperLeft);
    ASSERT_EQ(estimation::rotate_clockwise(location), estimation::LocalLocation::LowerLeft);
    ASSERT_EQ(estimation::rotate_counterclockwise(location), estimation::LocalLocation::UpperRight);
    coordinate = {1, 0};
    ASSERT_EQ(estimation::get_root_coordinate(location), coordinate);
}

TEST(Estimation, macaddress) {
    // 32 bit macaddress
    auto macaddress = 0b0000'0000'0000'0000'0000'0000'0000'0001;
    ASSERT_EQ(estimation::get_unit_node_id(macaddress), 0);
    ASSERT_EQ(estimation::get_local_location(macaddress), estimation::LocalLocation::UpperLeft);
    ASSERT_TRUE(estimation::is_root(macaddress));
    ASSERT_TRUE(estimation::is_same_unit_node(macaddress, macaddress));
    ASSERT_EQ(estimation::macaddress_to_ip_address(macaddress), 1);

    auto macaddress1 = 0b0000'0000'0000'0000'0000'0000'0000'0011;
    ASSERT_EQ(estimation::get_unit_node_id(macaddress1), 0);
    ASSERT_EQ(estimation::get_local_location(macaddress1), estimation::LocalLocation::UpperRight);
    ASSERT_TRUE(estimation::is_root(macaddress1));
    ASSERT_TRUE(estimation::is_same_unit_node(macaddress, macaddress1));
    ASSERT_EQ(estimation::macaddress_to_ip_address(macaddress1), 0b11);

    auto macaddress2 = 0b0000'0000'0000'0000'0000'0000'0000'1100;
    ASSERT_EQ(estimation::get_unit_node_id(macaddress2), 1);
    ASSERT_EQ(estimation::get_local_location(macaddress2), estimation::LocalLocation::LowerRight);
    ASSERT_FALSE(estimation::is_root(macaddress2));
    ASSERT_FALSE(estimation::is_same_unit_node(macaddress, macaddress2));
    ASSERT_FALSE(estimation::is_same_unit_node(macaddress1, macaddress2));
    ASSERT_EQ(estimation::macaddress_to_ip_address(macaddress2), 0b1100);
    0b0000'0000'0000'0000'0000'0000'0000'0001;
    auto macaddress3 = 0b0000'0000'0000'0000'0000'0000'0001'1110;
    ASSERT_EQ(estimation::get_unit_node_id(macaddress3), 3);
    ASSERT_EQ(estimation::get_local_location(macaddress3), estimation::LocalLocation::LowerLeft);
    ASSERT_FALSE(estimation::is_root(macaddress3));
    ASSERT_FALSE(estimation::is_same_unit_node(macaddress, macaddress3));
    ASSERT_FALSE(estimation::is_same_unit_node(macaddress1, macaddress3));
    ASSERT_FALSE(estimation::is_same_unit_node(macaddress2, macaddress3));
    ASSERT_EQ(estimation::macaddress_to_ip_address(macaddress3), 0b11110);
}

TEST(Estimation, util_function) {
    {
        // is_any_one_distance_neighbor
        std::vector<std::pair<network::macaddress_t, estimation::coordinate_t>> coordinates;
        coordinates.push_back(std::make_pair(0u, std::make_pair(0, 0)));
        ASSERT_FALSE(estimation::is_any_one_distance_neighbor(coordinates));
        coordinates.push_back(std::make_pair(1, std::make_pair(0, 1)));
        ASSERT_TRUE(estimation::is_any_one_distance_neighbor(coordinates));
        coordinates.push_back(std::make_pair(2, std::make_pair(1, 0)));
        ASSERT_TRUE(estimation::is_any_one_distance_neighbor(coordinates));
        coordinates.push_back(std::make_pair(3, std::make_pair(1, 1)));
        ASSERT_TRUE(estimation::is_any_one_distance_neighbor(coordinates));

        coordinates.clear();
        coordinates.push_back(std::make_pair(0, std::make_pair(0, 0)));
        ASSERT_FALSE(estimation::is_any_one_distance_neighbor(coordinates));
        coordinates.push_back(std::make_pair(1, std::make_pair(1, 1)));
        ASSERT_FALSE(estimation::is_any_one_distance_neighbor(coordinates));
        coordinates.push_back(std::make_pair(2, std::make_pair(2, 2)));
        ASSERT_FALSE(estimation::is_any_one_distance_neighbor(coordinates));
        coordinates.push_back(std::make_pair(3, std::make_pair(3, 3)));
        ASSERT_FALSE(estimation::is_any_one_distance_neighbor(coordinates));
        coordinates.push_back(std::make_pair(4, std::make_pair(3, 4)));
        ASSERT_TRUE(estimation::is_any_one_distance_neighbor(coordinates));
        coordinates.pop_back();
        ASSERT_FALSE(estimation::is_any_one_distance_neighbor(coordinates));
        coordinates.push_back(std::make_pair(5, std::make_pair(4, 3)));
        ASSERT_TRUE(estimation::is_any_one_distance_neighbor(coordinates));
        coordinates.pop_back();
        ASSERT_FALSE(estimation::is_any_one_distance_neighbor(coordinates));
        coordinates.push_back(std::make_pair(6, std::make_pair(4, 4)));
        ASSERT_FALSE(estimation::is_any_one_distance_neighbor(coordinates));
    }
}
TEST(Estimation, get_confirmed_coordinate) {
    {
        // root case
        auto this_id = 0b0000'0000'0000'0000'0000'0000'0000'0001;
        auto coordinates = std::vector<std::pair<uint32_t, estimation::coordinate_t>>();
        ASSERT_TRUE(estimation::is_finished(this_id, coordinates));
        ASSERT_TRUE(coordinates.empty());
        coordinate_t coordinate = {0, 1};
        ASSERT_EQ(get_coordinate(this_id, coordinates), coordinate);

        this_id = 0b0000'0000'0000'0000'0000'0000'0000'0011;
        ASSERT_TRUE(estimation::is_finished(this_id, coordinates));
        ASSERT_TRUE(coordinates.empty());
        coordinate = {1, 1};
        ASSERT_EQ(get_coordinate(this_id, coordinates), coordinate);

        this_id = 0b0000'0000'0000'0000'0000'0000'0000'0101;
        ASSERT_TRUE(estimation::is_finished(this_id, coordinates));
        ASSERT_TRUE(coordinates.empty());
        coordinate = {1, 0};
        ASSERT_EQ(get_coordinate(this_id, coordinates), coordinate);

        this_id = 0b0000'0000'0000'0000'0000'0000'0000'0111;
        ASSERT_TRUE(estimation::is_finished(this_id, coordinates));
        ASSERT_TRUE(coordinates.empty());
        coordinate = {0, 0};
        ASSERT_EQ(get_coordinate(this_id, coordinates), coordinate);
    }
    {
        // normal case
        // like below:
        // this_node, root1(0, 1)
        // neighbor_this_node, root2(0, 0)
        // other_node(-1, -1)
        auto this_id = estimation::macaddress_to_ip_address(0b0000'0000'0000'0000'0000'0000'0000'1000);
        auto root_id = 0b0000'0000'0000'0000'0000'0000'0000'0001;
        auto coordinates = std::vector<std::pair<uint32_t, estimation::coordinate_t>>();
        // send from root_id to this_id
        coordinates.push_back(std::make_pair(root_id, get_coordinate(root_id, coordinates)));
        ASSERT_FALSE(estimation::is_finished(this_id, coordinates));

        // send from neighbor_this_node(root2 information) to this_id
        auto neighbor_this_unit_id = 0b0000'0000'0000'0000'0000'0000'0000'1010;
        auto root2_id = 0b0000'0000'0000'0000'0000'0000'0000'0111;
        coordinates.push_back(std::make_pair(neighbor_this_unit_id, get_coordinate(root2_id, coordinates)));
        ASSERT_TRUE(estimation::is_finished(this_id, coordinates));
        coordinate_t coordinate = {-1, 1};
        ASSERT_EQ(get_coordinate(this_id, coordinates), coordinate);
        coordinates.clear();

        // send from this_id to neighbor_this_unit_id
        coordinate = {-1, 0};
        coordinates.push_back(std::make_pair(neighbor_this_unit_id, coordinate));
        ASSERT_TRUE(estimation::is_finished(neighbor_this_unit_id, coordinates));
        ASSERT_EQ(get_coordinate(neighbor_this_unit_id, coordinates), coordinate);
        coordinates.clear();

        // send from root_id to this_id
        coordinate = {0, 1};
        coordinates.push_back(std::make_pair(root_id, coordinate));
        // send from neighbor_this_node(othernode information) to this_id
        coordinate = {-1, -1};
        coordinates.push_back(std::make_pair(neighbor_this_unit_id, coordinate));
        // cannot estimate
        ASSERT_FALSE(estimation::is_finished(this_id, coordinates));
    }
    {
        // test all pattern
        //
        // same y axis
        //
        // this(lowerleft), same_unit(lowerright)
        // neighbor_this(confirmed, upperleft), diagonal(confirmed, upperright)

        auto this_id
            = estimation::macaddress_to_ip_address(make_macaddress(1, estimation::LocalLocation::LowerLeft, false));
        auto same_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(1, estimation::LocalLocation::LowerRight, false));
        auto neighbor_this_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(2, estimation::LocalLocation::UpperLeft, false));
        auto diagonal_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(2, estimation::LocalLocation::UpperRight, false));

        auto neighbor_this_coordinate = std::make_pair(0, 0);
        auto diagonal_coordinate = std::make_pair(1, 0);

        auto coordinates = std::vector<std::pair<uint32_t, estimation::coordinate_t>>();
        coordinates.push_back(std::make_pair(neighbor_this_unit_id, neighbor_this_coordinate));
        ASSERT_FALSE(estimation::is_finished(this_id, coordinates));
        coordinates.push_back(std::make_pair(same_unit_id, diagonal_coordinate));
        ASSERT_TRUE(estimation::is_finished(this_id, coordinates));
        coordinate_t expected_coordinate = std::make_pair(0, 1);
        ASSERT_EQ(get_coordinate(this_id, coordinates), expected_coordinate);
        coordinates.clear();

        // same_unit(lowerleft), this(lowerright)
        // diagonal(confirmed, upperleft), neighbor_this(confirmed, upperright)
        this_id
            = estimation::macaddress_to_ip_address(make_macaddress(1, estimation::LocalLocation::LowerRight, false));
        same_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(1, estimation::LocalLocation::LowerLeft, false));
        neighbor_this_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(2, estimation::LocalLocation::UpperRight, false));
        diagonal_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(2, estimation::LocalLocation::UpperLeft, false));

        neighbor_this_coordinate = std::make_pair(1, 0);
        diagonal_coordinate = std::make_pair(0, 0);
        coordinates.push_back(std::make_pair(neighbor_this_unit_id, neighbor_this_coordinate));
        ASSERT_FALSE(estimation::is_finished(same_unit_id, coordinates));
        coordinates.push_back(std::make_pair(diagonal_unit_id, diagonal_coordinate));
        ASSERT_TRUE(estimation::is_finished(same_unit_id, coordinates));
        expected_coordinate = std::make_pair(1, 1);
        ASSERT_EQ(get_coordinate(same_unit_id, coordinates), expected_coordinate);
        coordinates.clear();

        // neighbor_this(confirmed, lowerleft), diagonal(confirmed, lowerright)
        // this(upperleft), same_unit(upperright)
        this_id = estimation::macaddress_to_ip_address(make_macaddress(1, estimation::LocalLocation::UpperLeft, false));
        same_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(1, estimation::LocalLocation::UpperRight, false));
        neighbor_this_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(2, estimation::LocalLocation::LowerLeft, false));
        diagonal_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(2, estimation::LocalLocation::LowerRight, false));

        neighbor_this_coordinate = std::make_pair(0, 0);
        diagonal_coordinate = std::make_pair(1, 0);
        coordinates.push_back(std::make_pair(neighbor_this_unit_id, neighbor_this_coordinate));
        ASSERT_FALSE(estimation::is_finished(this_id, coordinates));
        coordinates.push_back(std::make_pair(same_unit_id, diagonal_coordinate));
        ASSERT_TRUE(estimation::is_finished(this_id, coordinates));
        expected_coordinate = std::make_pair(0, -1);
        ASSERT_EQ(get_coordinate(this_id, coordinates), expected_coordinate);
        coordinates.clear();

        // diagonal(confirmed, lowerleft), neighbor_this(confirmed, lowerright)
        // same_unit(upperleft), this(upperright)
        this_id
            = estimation::macaddress_to_ip_address(make_macaddress(1, estimation::LocalLocation::UpperRight, false));
        same_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(1, estimation::LocalLocation::UpperLeft, false));
        neighbor_this_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(2, estimation::LocalLocation::LowerRight, false));
        diagonal_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(2, estimation::LocalLocation::LowerLeft, false));

        neighbor_this_coordinate = std::make_pair(1, 0);
        diagonal_coordinate = std::make_pair(0, 0);
        coordinates.push_back(std::make_pair(neighbor_this_unit_id, neighbor_this_coordinate));
        ASSERT_FALSE(estimation::is_finished(this_id, coordinates));
        coordinates.push_back(std::make_pair(same_unit_id, diagonal_coordinate));
        ASSERT_TRUE(estimation::is_finished(this_id, coordinates));
        expected_coordinate = std::make_pair(1, -1);
        ASSERT_EQ(get_coordinate(this_id, coordinates), expected_coordinate);
        coordinates.clear();

        // same x axis

        // this(upperright), neighbor_this(confirmed, upperleft)
        // same_unit(lowerright), diagonal(confirmed, lowerleft)
        this_id
            = estimation::macaddress_to_ip_address(make_macaddress(1, estimation::LocalLocation::UpperRight, false));
        same_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(1, estimation::LocalLocation::LowerRight, false));
        neighbor_this_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(2, estimation::LocalLocation::UpperLeft, false));
        diagonal_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(2, estimation::LocalLocation::LowerLeft, false));

        neighbor_this_coordinate = std::make_pair(0, 1);
        diagonal_coordinate = std::make_pair(0, 0);
        coordinates.push_back(std::make_pair(neighbor_this_unit_id, neighbor_this_coordinate));
        ASSERT_FALSE(estimation::is_finished(this_id, coordinates));
        coordinates.push_back(std::make_pair(same_unit_id, diagonal_coordinate));
        ASSERT_TRUE(estimation::is_finished(this_id, coordinates));
        expected_coordinate = std::make_pair(-1, 1);
        ASSERT_EQ(get_coordinate(this_id, coordinates), expected_coordinate);
        coordinates.clear();

        // neighbor_this(confirmed, upperright), this(upperleft)
        // diagonal(confirmed, lowerleft), same_unit(lowerright)
        this_id = estimation::macaddress_to_ip_address(make_macaddress(1, estimation::LocalLocation::UpperLeft, false));
        same_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(1, estimation::LocalLocation::LowerLeft, false));
        neighbor_this_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(2, estimation::LocalLocation::UpperRight, false));
        diagonal_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(2, estimation::LocalLocation::LowerRight, false));

        neighbor_this_coordinate = std::make_pair(0, 1);
        diagonal_coordinate = std::make_pair(0, 0);
        coordinates.push_back(std::make_pair(neighbor_this_unit_id, neighbor_this_coordinate));
        ASSERT_FALSE(estimation::is_finished(this_id, coordinates));
        coordinates.push_back(std::make_pair(same_unit_id, diagonal_coordinate));
        ASSERT_TRUE(estimation::is_finished(this_id, coordinates));
        expected_coordinate = std::make_pair(1, 1);
        ASSERT_EQ(get_coordinate(this_id, coordinates), expected_coordinate);
        coordinates.clear();

        // same_unit(upperright), diagonal(confirmed, upperleft)
        // this(lowerright), neighbor_this(confirmed, lowerleft)
        this_id
            = estimation::macaddress_to_ip_address(make_macaddress(1, estimation::LocalLocation::LowerRight, false));
        same_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(1, estimation::LocalLocation::UpperRight, false));
        neighbor_this_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(2, estimation::LocalLocation::LowerLeft, false));
        diagonal_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(2, estimation::LocalLocation::UpperLeft, false));

        neighbor_this_coordinate = std::make_pair(0, 0);
        diagonal_coordinate = std::make_pair(0, 1);
        coordinates.push_back(std::make_pair(neighbor_this_unit_id, neighbor_this_coordinate));
        ASSERT_FALSE(estimation::is_finished(same_unit_id, coordinates));
        coordinates.push_back(std::make_pair(diagonal_unit_id, diagonal_coordinate));
        ASSERT_TRUE(estimation::is_finished(same_unit_id, coordinates));
        expected_coordinate = std::make_pair(-1, 0);
        ASSERT_EQ(get_coordinate(same_unit_id, coordinates), expected_coordinate);
        coordinates.clear();

        // diagonal(confirmed, upperright), same_unit(upperleft)
        // neighbor_this(confirmed, lowerright), this(lowerleft)
        this_id = estimation::macaddress_to_ip_address(make_macaddress(1, estimation::LocalLocation::LowerLeft, false));
        same_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(1, estimation::LocalLocation::UpperLeft, false));
        neighbor_this_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(2, estimation::LocalLocation::LowerRight, false));
        diagonal_unit_id
            = estimation::macaddress_to_ip_address(make_macaddress(2, estimation::LocalLocation::UpperRight, false));

        neighbor_this_coordinate = std::make_pair(0, 0);
        diagonal_coordinate = std::make_pair(0, 1);
        coordinates.push_back(std::make_pair(neighbor_this_unit_id, neighbor_this_coordinate));
        ASSERT_FALSE(estimation::is_finished(this_id, coordinates));
        coordinates.push_back(std::make_pair(same_unit_id, diagonal_coordinate));
        ASSERT_TRUE(estimation::is_finished(this_id, coordinates));
        expected_coordinate = std::make_pair(1, 0);
        ASSERT_EQ(get_coordinate(this_id, coordinates), expected_coordinate);
        coordinates.clear();
    }
}
TEST(Estimation, process_data) {
    {
        network::ip_address_t src_ip_address
            = macaddress_to_ip_address(make_macaddress(1, LocalLocation::LowerLeft, false));
        network::ip_address_t this_ip_address
            = macaddress_to_ip_address(make_macaddress(1, LocalLocation::LowerRight, false));
        // emtpy data
        const std::vector<uint8_t> data;
        std::vector<std::pair<network::macaddress_t, coordinate_t>> confirmed_coordinates;
        auto err = process_data(src_ip_address, this_ip_address, data, confirmed_coordinates);
        ASSERT_EQ(err, network::NetworkError::EMPTY);
        ASSERT_EQ(confirmed_coordinates.size(), 0);
    }
    {
        // same unit node is confirmed
        network::ip_address_t src_ip_address
            = macaddress_to_ip_address(make_macaddress(1, LocalLocation::LowerLeft, false));
        network::ip_address_t this_ip_address
            = macaddress_to_ip_address(make_macaddress(1, LocalLocation::LowerRight, false));
        // src ip address (0,0), this ip address (0,1)
        auto vector = std::vector<std::pair<LocalLocation, coordinate_t>>{
            // {LocalLocation::LowerLeft, {0, 0}},
            {LocalLocation::LowerRight, {0, 1}},
            {LocalLocation::UpperLeft, {1, 0}},
            {LocalLocation::UpperRight, {1, 1}},
        };

        std::vector<uint8_t> data = {0xFF};
        for (auto [local_location, coordinate] : vector) {
            auto macaddress = make_macaddress(1, local_location, false);
            auto ip_address = macaddress_to_ip_address(macaddress);
            data.push_back((ip_address >> 8) & 0xFF);
            data.push_back(ip_address & 0xFF);
            data.push_back((coordinate.first >> 8) & 0xFF);
            data.push_back(coordinate.first & 0xFF);
            data.push_back((coordinate.second >> 8) & 0xFF);
            data.push_back(coordinate.second & 0xFF);
        }
        std::vector<std::pair<network::macaddress_t, coordinate_t>> confirmed_coordinates;
        auto err = process_data(src_ip_address, this_ip_address, data, confirmed_coordinates);
        ASSERT_EQ(err, network::NetworkError::OK);
        ASSERT_EQ(confirmed_coordinates.size(), 1);
        ASSERT_EQ(confirmed_coordinates[0].first, make_macaddress(1, LocalLocation::LowerRight, false));
        coordinate_t coordinate = {0, 1};
        ASSERT_EQ(confirmed_coordinates[0].second, coordinate);
    }
    {
        // same unit node is confirmed, but invalid data
        network::ip_address_t src_ip_address
            = macaddress_to_ip_address(make_macaddress(1, LocalLocation::LowerLeft, false));
        network::ip_address_t this_ip_address
            = macaddress_to_ip_address(make_macaddress(1, LocalLocation::LowerRight, false));
        // src ip address (0,0), this ip address (0,1)
        auto vector = std::vector<std::pair<LocalLocation, coordinate_t>>{
            {LocalLocation::LowerLeft, {0, 0}},
            // {LocalLocation::LowerRight, {0, 1}},
            {LocalLocation::UpperLeft, {1, 0}},
            {LocalLocation::UpperRight, {1, 1}},
        };

        std::vector<uint8_t> data = {0xFF};
        for (auto [local_location, coordinate] : vector) {
            auto macaddress = make_macaddress(1, local_location, false);
            auto ip_address = macaddress_to_ip_address(macaddress);
            data.push_back((ip_address >> 8) & 0xFF);
            data.push_back(ip_address & 0xFF);
            data.push_back((coordinate.first >> 8) & 0xFF);
            data.push_back(coordinate.first & 0xFF);
            data.push_back((coordinate.second >> 8) & 0xFF);
            data.push_back(coordinate.second & 0xFF);
        }
        std::vector<std::pair<network::macaddress_t, coordinate_t>> confirmed_coordinates;
        auto err = process_data(src_ip_address, this_ip_address, data, confirmed_coordinates);
        ASSERT_EQ(err, network::NetworkError::INVALID_DATA);
    }
    {
        // same unit node is confirmed, negative coordinate
        network::ip_address_t src_ip_address
            = macaddress_to_ip_address(make_macaddress(1, LocalLocation::LowerLeft, false));
        network::ip_address_t this_ip_address
            = macaddress_to_ip_address(make_macaddress(1, LocalLocation::LowerRight, false));
        // src ip address (-2,-2), this ip address (0,1)
        auto vector = std::vector<std::pair<LocalLocation, coordinate_t>>{
            // {LocalLocation::LowerLeft, {-2, -2}},
            {LocalLocation::UpperLeft, {-2, -1}},
            {LocalLocation::UpperRight, {-1, -1}},
            {LocalLocation::LowerRight, {-1, -2}},
        };

        std::vector<uint8_t> data = {0xFF};
        for (auto [local_location, coordinate] : vector) {
            auto macaddress = make_macaddress(1, local_location, false);
            auto ip_address = macaddress_to_ip_address(macaddress);
            data.push_back((ip_address >> 8) & 0xFF);
            data.push_back(ip_address & 0xFF);
            data.push_back((coordinate.first >> 8) & 0xFF);
            data.push_back(coordinate.first & 0xFF);
            data.push_back((coordinate.second >> 8) & 0xFF);
            data.push_back(coordinate.second & 0xFF);
        }
        std::vector<std::pair<network::macaddress_t, coordinate_t>> confirmed_coordinates;
        auto err = process_data(src_ip_address, this_ip_address, data, confirmed_coordinates);
        ASSERT_EQ(err, network::NetworkError::OK);
        ASSERT_EQ(confirmed_coordinates.size(), 1);
        ASSERT_EQ(confirmed_coordinates[0].first, make_macaddress(1, LocalLocation::LowerRight, false));
        coordinate_t coordinate = {-1, -2};
        ASSERT_EQ(confirmed_coordinates[0].second, coordinate);
    }
    {
        // same unit node is not confirmed
        network::ip_address_t src_ip_address
            = macaddress_to_ip_address(make_macaddress(1, LocalLocation::LowerLeft, false));
        network::ip_address_t this_ip_address
            = macaddress_to_ip_address(make_macaddress(1, LocalLocation::LowerRight, false));
        // src ip address (1,-1), this ip address (2,1)
        auto vector = std::vector<coordinate_t>{
            std::make_pair(1, 0),
            std::make_pair(0, 1),
        };

        std::vector<uint8_t> data = {0x00};
        for (auto coordinate : vector) {
            data.push_back((src_ip_address >> 8) & 0xFF);
            data.push_back(src_ip_address & 0xFF);
            data.push_back((coordinate.first >> 8) & 0xFF);
            data.push_back(coordinate.first & 0xFF);
            data.push_back((coordinate.second >> 8) & 0xFF);
            data.push_back(coordinate.second & 0xFF);
        }

        std::vector<std::pair<network::macaddress_t, coordinate_t>> confirmed_coordinates;
        auto err = process_data(src_ip_address, this_ip_address, data, confirmed_coordinates);
        ASSERT_EQ(err, network::NetworkError::OK);
        ASSERT_EQ(confirmed_coordinates.size(), 2);
        ASSERT_EQ(confirmed_coordinates[0].first, src_ip_address);
        ASSERT_EQ(confirmed_coordinates[0].second, vector[0]);
        ASSERT_EQ(confirmed_coordinates[1].first, src_ip_address);
        ASSERT_EQ(confirmed_coordinates[1].second, vector[1]);
    }
    {
        // other unit node is confirmed
        network::ip_address_t src_ip_address
            = macaddress_to_ip_address(make_macaddress(2, LocalLocation::LowerLeft, false));
        network::ip_address_t this_ip_address
            = macaddress_to_ip_address(make_macaddress(1, LocalLocation::LowerRight, false));
        // src ip address (-2,-2), this ip address (0,1)
        coordinate_t src_ip_address_coordinate = {-2, -2};

        std::vector<uint8_t> data = {
            0xFF,
            (uint8_t)((src_ip_address >> 8) & 0xFF),
            (uint8_t)(src_ip_address & 0xFF),
            (uint8_t)((src_ip_address_coordinate.first >> 8) & 0xFF),
            (uint8_t)(src_ip_address_coordinate.first & 0xFF),
            (uint8_t)((src_ip_address_coordinate.second >> 8) & 0xFF),
            (uint8_t)(src_ip_address_coordinate.second & 0xFF),
        };
        std::vector<std::pair<network::macaddress_t, coordinate_t>> confirmed_coordinates;
        auto err = process_data(src_ip_address, this_ip_address, data, confirmed_coordinates);
        ASSERT_EQ(err, network::NetworkError::OK);
        ASSERT_EQ(confirmed_coordinates.size(), 1);
        ASSERT_EQ(confirmed_coordinates[0].first, src_ip_address);
        coordinate_t coordinate = {-2, -2};
        ASSERT_EQ(confirmed_coordinates[0].second, coordinate);
    }
    {
        // other unit node not confirmed(error)
        network::ip_address_t src_ip_address
            = macaddress_to_ip_address(make_macaddress(2, LocalLocation::LowerLeft, false));
        network::ip_address_t this_ip_address
            = macaddress_to_ip_address(make_macaddress(1, LocalLocation::LowerRight, false));

        coordinate_t src_ip_address_coordinate = {-2, -2};
        std::vector<uint8_t> data = {
            0x00,
            (uint8_t)((src_ip_address >> 8) & 0xFF),
            (uint8_t)(src_ip_address & 0xFF),
            (uint8_t)((src_ip_address_coordinate.first >> 8) & 0xFF),
            (uint8_t)(src_ip_address_coordinate.first & 0xFF),
            (uint8_t)((src_ip_address_coordinate.second >> 8) & 0xFF),
            (uint8_t)(src_ip_address_coordinate.second & 0xFF),
        };
        std::vector<std::pair<network::macaddress_t, coordinate_t>> confirmed_coordinates;
        auto err = process_data(src_ip_address, this_ip_address, data, confirmed_coordinates);
        ASSERT_EQ(err, network::NetworkError::INVALID_DATA);
    }
}

TEST(TemplateEstimation, serial) {
    test::SerialMock serial;
    network::macaddress_t this_id = make_macaddress(1, LocalLocation::LowerRight, false);
    network::ip_address_t this_ip_address = macaddress_to_ip_address(this_id);
    network::macaddress_t src_id = make_macaddress(2, LocalLocation::LowerLeft, false);
    network::ip_address_t src_ip_address = macaddress_to_ip_address(src_id);
    // this -> serial
    {
        auto packet = make_request(this_id);
        auto err = packet.send(serial, this_id, network::DefaultRouting());
        ASSERT_EQ(err, traits::SerialError::Ok);
    }
    // src <- serial
    {
        auto packet = network::Packet();
        auto err = packet.receive(serial, src_id);
        ASSERT_TRUE(err);
        ASSERT_EQ(packet.get_header(), network::Header::COORDINATE_ESTIMATION);
    }
    // src -> serial -
    {
        auto packet = make_response_to_other_unit(this_ip_address, coordinate_t{2, 2});
        auto err = packet.send(serial, src_ip_address, network::DefaultRouting());
        ASSERT_EQ(err, traits::SerialError::Ok);
    }
    // this <- serial
    {
        auto packet = network::Packet();
        auto err = packet.receive(serial, this_id);
        ASSERT_TRUE(err);
        ASSERT_EQ(packet.get_header(), network::Header::COORDINATE_ESTIMATION_RSP);
        auto data = packet.get_data();
        auto confirmed_coordinates = std::vector<std::pair<network::macaddress_t, coordinate_t>>();
        process_data(src_ip_address, this_ip_address, data, confirmed_coordinates);
        ASSERT_EQ(confirmed_coordinates.size(), 1);
        ASSERT_EQ(confirmed_coordinates[0].first, src_ip_address);
        ASSERT_EQ(confirmed_coordinates[0].second, (coordinate_t{2, 2}));
    }
}

TEST(TemplateEstimation, init_coordinate) {
    // ASSERT_EQ("TODO", "implement");
}

}  // namespace estimation